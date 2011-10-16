#include <windows.h>
#include <mlang.h>
#include <stdio.h>
#include <wchar.h>

#include <unicode/uchar.h>
#include <unicode/ustring.h>
#include <unicode/utf16.h>
#include <unicode/uidna.h>
#include <unicode/unorm.h>
#include <unicode/ucnv.h>
#include <unicode/ucnv_cb.h>
#include <unicode/ubrk.h>
#include <unicode/ucsdet.h>
#include <unicode/ucurr.h>
#include <unicode/ulocdata.h>
#include <unicode/unum.h>
#include <unicode/ucal.h>
#include <unicode/umsg.h>

#define MAX_NAME_LEN 128
#define MAX_CHARSET_COUNT 256
#define MAX_ALIAS_COUNT 16
struct CharsetInfo
{
    char name[MAX_NAME_LEN];
    unsigned int codePage;

    char aliases[MAX_ALIAS_COUNT][MAX_NAME_LEN];
    unsigned int aliasCount;
};

static CharsetInfo gCharsetInfos[MAX_CHARSET_COUNT];
static int gCharsetCount = -1;
static IMultiLanguage* gMultiLanguage = NULL;

void toLatin1(char* latin1, const wchar_t* name)
{
    for (unsigned i = 0; i < MAX_NAME_LEN - 1; ++i) 
    {
        wchar_t ch = name[i];
        latin1[i] = ch > 0xff ? '?' : (char)ch;

        if(ch == 0)
            break;
    }

    latin1[MAX_NAME_LEN - 1] = '\0';
}

void addCodePage(unsigned int codePage, const char* name)
{
    if(name == NULL || name[0] == 0)
        return;

    CharsetInfo* pCharset = NULL;
    for(int i = 0; i < gCharsetCount; ++i)
    {
        if(gCharsetInfos[i].codePage == codePage)
        {
            pCharset = &gCharsetInfos[i];
            break;
        }
    }

    if(pCharset == NULL)
    {
        if(gCharsetCount == MAX_CHARSET_COUNT)
            return;

        CharsetInfo* pCharset = &gCharsetInfos[gCharsetCount];
        ++gCharsetCount;

        pCharset->aliasCount = 0;
        pCharset->codePage = codePage;
        strcpy(pCharset->name, name);
        return;
    }

    if(pCharset->aliasCount == MAX_ALIAS_COUNT)
        return;

    if(strcmp(pCharset->name, name) == 0)
        return;

    for(unsigned int i = 0; i < pCharset->aliasCount; ++i)
    {
        if(strcmp(pCharset->aliases[i], name) == 0)
            return;
    }

    strcpy(pCharset->aliases[pCharset->aliasCount], name);
    ++pCharset->aliasCount;
}

void addCodePage(unsigned int codePage, const wchar_t* name)
{
    if(name == NULL || name[0] == 0)
        return;

    char latin1[MAX_NAME_LEN];
    toLatin1(latin1, name);

    addCodePage(codePage, latin1);
}

const CharsetInfo* getCharset(const char* name, bool alias)
{
    for(int i = 0; i < gCharsetCount; ++i)
    {
        if( _stricmp(gCharsetInfos[i].name, name) == 0 )
        {
            return &gCharsetInfos[i];
        }

        if (alias)
        {
            for(unsigned int j = 0; j < gCharsetInfos[i].aliasCount; ++j)
            {
                if( _stricmp(gCharsetInfos[i].aliases[j], name) == 0 )
                    return &gCharsetInfos[i];
            }
        }
    }

    return NULL;
}

const CharsetInfo* getCharset(unsigned int codePage)
{
    for (int i = 0; i < gCharsetCount; ++i)
    {
        if (gCharsetInfos[i].codePage == codePage)
            return &gCharsetInfos[i];
    }
    
    return NULL;
}

void initCharsets()
{
    gCharsetCount = 0;

    addCodePage(0, "NULL");

    addCodePage(10000, "macintosh");
    addCodePage(10007, "x-mac-cyrillic");

    addCodePage(936, "GBK");
    addCodePage(936, "GB2312");
    addCodePage(54936, "GB18030");
    addCodePage(950, "Big5");

    addCodePage(20866, "KOI8-R");

    addCodePage(874, "windows-874");
    addCodePage(949, "windows-949");
    addCodePage(949, "KSC_5601");
    addCodePage(1250, "windows-1250");
    addCodePage(1251, "windows-1251");
    addCodePage(1253, "windows-1253");
    addCodePage(1254, "windows-1254");
    addCodePage(1255, "windows-1255");
    addCodePage(1256, "windows-1256");
    addCodePage(1257, "windows-1257");
    addCodePage(1258, "windows-1258");

    addCodePage(864, "cp864");
    addCodePage(932, "Shift_JIS");
    addCodePage(20932, "EUC-JP");
    addCodePage(50222, "ISO-2022-JP");

    addCodePage(28591, "ISO-8859-1");
    addCodePage(28592, "ISO-8859-2");
    addCodePage(28593, "ISO-8859-3");
    addCodePage(28594, "ISO-8859-4");
    addCodePage(28595, "ISO-8859-5");
    addCodePage(28596, "ISO-8859-6");
    addCodePage(28597, "ISO-8859-7");
    addCodePage(28598, "ISO-8859-8");
    addCodePage(28599, "ISO-8859-9");
    addCodePage(28600, "ISO-8859-10");
    addCodePage(28600, "ISO-8859-13");
    addCodePage(28604, "ISO-8859-14");
    addCodePage(28605, "ISO-8859-15");
    addCodePage(38598, "ISO-8859-8-I");

    addCodePage(1254, "ISO-8859-9");

    
    if(S_OK != CoCreateInstance(CLSID_CMultiLanguage, 0, CLSCTX_INPROC_SERVER, IID_IMultiLanguage, (void**)&gMultiLanguage))
        return;

    IEnumCodePage* enumInterface;
    if(S_OK != gMultiLanguage->EnumCodePages(MIMECONTF_BROWSER, &enumInterface))
        return;

    MIMECPINFO cpInfo;
    ULONG ccpInfo;
    while(S_OK == enumInterface->Next(1, &cpInfo, &ccpInfo) && ccpInfo)
    {
        if (!IsValidCodePage(cpInfo.uiCodePage))
            continue;

        addCodePage(cpInfo.uiCodePage, cpInfo.wszWebCharset);
        addCodePage(cpInfo.uiCodePage, cpInfo.wszHeaderCharset);
        addCodePage(cpInfo.uiCodePage, cpInfo.wszBodyCharset);

        char buffer[32];
        sprintf(buffer, "cp%d", cpInfo.uiCodePage);
        addCodePage(cpInfo.uiCodePage, buffer);
    }

    enumInterface->Release();
}

UChar32
u_foldCase(UChar32 c, uint32_t options)
{
    return towlower(c);
}

int32_t 
u_memcasecmp(const UChar *s1, const UChar *s2, int32_t length, uint32_t options)
{
    int i;
    for (i = 0; i < length; ++i) {
        wchar_t c1 = towlower(s1[i]);
        wchar_t c2 = towlower(s2[i]);
        if (c1 != c2)
            return c1 - c2;
    }
    return 0;
}

int32_t
u_strToLower(UChar *dest, int32_t destCapacity,
             const UChar *src, int32_t srcLength,
             const char *locale,
             UErrorCode *pErrorCode)
{
    const UChar* sourceIterator = src;
    const UChar* sourceEnd = src + srcLength;
    UChar* resultIterator = dest;
    UChar* resultEnd = dest + destCapacity;

    int remainingCharacters = 0;

    *pErrorCode = U_ZERO_ERROR;
    if (srcLength <= destCapacity)
        while (sourceIterator < sourceEnd)
            *resultIterator++ = towlower(*sourceIterator++);
    else
        while (resultIterator < resultEnd)
            *resultIterator++ = towlower(*sourceIterator++);

    if (sourceIterator < sourceEnd)
        remainingCharacters += sourceEnd - sourceIterator;

    if (resultIterator < resultEnd)
        *resultIterator = 0;

    return (resultIterator - dest) + remainingCharacters;
}

int32_t
u_strToUpper(UChar *dest, int32_t destCapacity,
             const UChar *src, int32_t srcLength,
             const char *locale,
             UErrorCode *pErrorCode)
{
    const UChar* sourceIterator = src;
    const UChar* sourceEnd = src + srcLength;
    UChar* resultIterator = dest;
    UChar* resultEnd = dest + destCapacity;

    int remainingCharacters = 0;

    *pErrorCode = U_ZERO_ERROR;
    if (srcLength <= destCapacity)
        while (sourceIterator < sourceEnd)
            *resultIterator++ = towupper(*sourceIterator++);
    else
        while (resultIterator < resultEnd)
            *resultIterator++ = towupper(*sourceIterator++);

    if (sourceIterator < sourceEnd)
        remainingCharacters += sourceEnd - sourceIterator;

    if (resultIterator < resultEnd)
        *resultIterator = 0;

    return (resultIterator - dest) + remainingCharacters;
}

int32_t
u_strFoldCase(UChar *dest, int32_t destCapacity,
              const UChar *src, int32_t srcLength,
              uint32_t options,
              UErrorCode *pErrorCode)
{
    return u_strToLower(dest, destCapacity, src, srcLength, NULL, pErrorCode);
}

UCharDirection
u_charDirection(UChar32 c)
{
    return U_LEFT_TO_RIGHT;
}

int8_t
u_charType(UChar32 c)
{
    if(iswspace(c))
        return U_SPACE_SEPARATOR;

    return 0;
}

int32_t
u_getIntPropertyValue(UChar32 c, UProperty which)
{
    return 0;
}

UBool
u_ispunct(UChar32 c)
{
    return !!iswpunct(c);
}

UChar32
u_tolower(UChar32 c)
{
    return towlower(c);
}

UChar32
u_totitle(UChar32 c)
{
    return towupper(c);
}

UBool
u_isalnum(UChar32 c)
{
    return !!iswalnum(c);
}

UBool
u_isprint(UChar32 c)
{
    return !!iswprint(c);
}

UChar32
u_toupper(UChar32 c)
{
    return towupper(c);
}

UChar32
u_charMirror(UChar32 c)
{
    return c;
}

uint8_t
u_getCombiningClass(UChar32 c)
{
    return 0;
}

const char *
u_errorName(UErrorCode code)
{
    return "";
}

int32_t
u_formatMessage(const char  *locale,
                 const UChar *pattern,
                int32_t     patternLength,
                UChar       *result,
                int32_t     resultLength,
                UErrorCode  *status,
                ...)
{
    *status = U_INTERNAL_PROGRAM_ERROR;
    return 0;
}

void
u_charsToUChars(const char *cs, UChar *us, int32_t length)
{
    if(us)
        *us = 0;
}


void
u_cleanup(void)
{
    if(gMultiLanguage)
        gMultiLanguage->Release();
}


int32_t
uidna_IDNToASCII(  const UChar *src, int32_t srcLength,
                 UChar* dest, int32_t destCapacity,
                 int32_t options,
                 UParseError *parseError,
                 UErrorCode *status )
{
    *status = U_INTERNAL_PROGRAM_ERROR;
    return 0;
}

int32_t
unorm_normalize( const UChar *src, int32_t srcLength,
                UNormalizationMode mode, int32_t options,
                UChar *dest, int32_t destCapacity,
                UErrorCode *pErrorCode )
{
    *pErrorCode = U_INTERNAL_PROGRAM_ERROR;
    return 0;
}

UNormalizationCheckResult
unorm_quickCheck(const UChar *src,
                 int32_t srcLength, 
                 UNormalizationMode mode,
                 UErrorCode *pErrorCode)
{
    *pErrorCode = U_ZERO_ERROR;
    return UNORM_YES;
}

UBlockCode
ublock_getCode(UChar32 c)
{
    return UBLOCK_NO_BLOCK;
}

static inline bool isSpace(UChar c)
{
    return !!iswspace(c);
}

static inline bool isCharStop(UChar c)
{
    return !iswalnum(c);
}

static inline bool isLineStop(UChar c)
{
    if( c >= '0' && c <= '9')
        return false;

    if( c >= 'a' && c <= 'z' )
        return false;

    if( c >= 'A' && c <= 'Z' )
        return false;

    return true;
}

static inline bool isSentenceStop(UChar c)
{
    return !!iswpunct(c);
}


class TextBreakIterator 
{
public:
    void reset(const UChar* str, int len)
    {
        string = str;
        length = len;
        currentPos = 0;
    }
    virtual int first() = 0;
    virtual int last()
    {
        currentPos = length;
        return currentPos;
    }

    virtual int next() = 0;
    virtual int previous() = 0;
    int following(int position)
    {
        currentPos = position;
        return next();
    }
    int preceding(int position)
    {
        currentPos = position;
        return previous();
    }

    int currentPos;
    const UChar* string;
    int length;
};

struct WordBreakIterator: TextBreakIterator 
{
    virtual int first()
    {
        currentPos = 0;
        return currentPos;
    }

    virtual int next()
    {
        if (currentPos == length) {
            currentPos = -1;
            return currentPos;
        }
        bool haveSpace = false;
        while (currentPos < length) {
            if (haveSpace && !isSpace(string[currentPos]))
                break;
            if (isSpace(string[currentPos]))
                haveSpace = true;
            ++currentPos;
        }
        return currentPos;
    }

    virtual int previous()
    {
        if (!currentPos) {
            currentPos = -1;
            return currentPos;
        }
        bool haveSpace = false;
        while (currentPos > 0) {
            if (haveSpace && !isSpace(string[currentPos]))
                break;
            if (isSpace(string[currentPos]))
                haveSpace = true;
            --currentPos;
        }
        return currentPos;
    }
};

struct CharBreakIterator: TextBreakIterator 
{
    virtual int first()
    {
        currentPos = 0;
        return currentPos;
    }

    virtual int next()
    {
        if (currentPos >= length)
            return -1;
        ++currentPos;
        return currentPos;
    }

    virtual int previous()
    {
        if (currentPos <= 0)
            return -1;
        if (currentPos > length)
            currentPos = length;
        --currentPos;
        return currentPos;
    }
};

struct LineBreakIterator: TextBreakIterator 
{
    virtual int first()
    {
        currentPos = 0;
        return currentPos;
    }

    virtual int next()
    {
        if (currentPos >= length) {
            return -1;
        }
        ++currentPos;
        while (currentPos < length && !isLineStop(string[currentPos]))
            ++currentPos;
        return currentPos;
    }

    virtual int previous()
    {
        if (currentPos <= 0)
            return -1;
        if(currentPos > length)
            currentPos = length;
        --currentPos;
        while (currentPos > 0 && !isLineStop(string[currentPos]))
            --currentPos;
        return currentPos;
    }
};

struct SentenceBreakIterator : TextBreakIterator 
{
    virtual int first()
    {
        currentPos = 0;
        return currentPos;
    }

    virtual int next()
    {
        if (currentPos == length) {
            currentPos = -1;
            return currentPos;
        }
        bool haveSpace = false;
        while (currentPos < length) {
            if (haveSpace && !isSentenceStop(string[currentPos]))
                break;
            if (isSentenceStop(string[currentPos]))
                haveSpace = true;
            ++currentPos;
        }
        return currentPos;
    }

    virtual int previous()
    {
        if (!currentPos) {
            currentPos = -1;
            return currentPos;
        }
        bool haveSpace = false;
        while (currentPos > 0) {
            if (haveSpace && !isSentenceStop(string[currentPos]))
                break;
            if (isSentenceStop(string[currentPos]))
                haveSpace = true;
            --currentPos;
        }
        return currentPos;
    }
};

void
ubrk_setText(UBreakIterator* bi,
             const UChar*    text,
             int32_t         textLength,
             UErrorCode*     status)
{
    TextBreakIterator* iterator = (TextBreakIterator*)bi;
    if(iterator)
    {
        iterator->reset(text, textLength);
        *status = U_ZERO_ERROR;
    }
    else
        *status = U_ILLEGAL_ARGUMENT_ERROR;
}

UBreakIterator*
ubrk_open(UBreakIteratorType type,
          const char *locale,
          const UChar *text,
          int32_t textLength,
          UErrorCode *status)
{
    TextBreakIterator *result = 0;

    switch(type) 
    {
    case UBRK_CHARACTER:
        {
            result = new CharBreakIterator;
        }
        break;

    case UBRK_WORD:
        {
            result = new WordBreakIterator;
        }
        break;

    case UBRK_LINE:
        {
            result = new LineBreakIterator;
        }
        break;

    case UBRK_SENTENCE:
        {
            result = new SentenceBreakIterator;
        }
        break;
    }

    ubrk_setText((UBreakIterator*)result, text, textLength, status);
    return (UBreakIterator*)result;
}

void
ubrk_close(UBreakIterator *bi)
{
    delete bi;
}

int32_t
ubrk_first(UBreakIterator *bi)
{
    return ((TextBreakIterator*)bi)->first();
}

int32_t
ubrk_last(UBreakIterator *bi)
{
    return ((TextBreakIterator*)bi)->last();
}

int32_t
ubrk_next(UBreakIterator *bi)
{
    return ((TextBreakIterator*)bi)->next();
}

int32_t
ubrk_previous(UBreakIterator *bi)
{
    return ((TextBreakIterator*)bi)->previous();
}

int32_t
ubrk_preceding(UBreakIterator *bi,
               int32_t offset)
{
    return ((TextBreakIterator*)bi)->preceding(offset);
}

int32_t
ubrk_following(UBreakIterator *bi,
               int32_t offset)
{
    return ((TextBreakIterator*)bi)->following(offset);
}

int32_t
ubrk_current(const UBreakIterator *bi)
{
    return ((TextBreakIterator*)bi)->currentPos;
}

UBool
ubrk_isBoundary(UBreakIterator *bi, int32_t offset)
{
    UChar c = ((TextBreakIterator*) bi)->string[((TextBreakIterator*)bi)->currentPos + offset];
    if( isSpace(c) )
        return TRUE;

    if(c > 255)
        return TRUE;

    return FALSE;
}

UBreakIterator*
ubrk_openRules(  const UChar        *rules,
               int32_t       rulesLength,
               const UChar        *text,
               int32_t       textLength,
               UParseError  *parseErr,
               UErrorCode   *status)
{
    return ubrk_open(UBRK_CHARACTER, NULL, text, textLength, status);
}





int32_t
ucnv_countAvailable ()
{
    return gCharsetCount;
}

const char*
ucnv_getAvailableName (int32_t n)
{
    if(n < gCharsetCount)
        return gCharsetInfos[n].name;

    return NULL;
}

const char*
ucnv_getStandardName(const char *name, const char *standard, UErrorCode *pErrorCode)
{
    *pErrorCode = U_ZERO_ERROR;
    return name;
}

uint16_t
ucnv_countAliases(const char *name, UErrorCode *pErrorCode)
{
    const CharsetInfo* pCharsetInfo = getCharset(name, false);
    if(pCharsetInfo == NULL)
    {
        *pErrorCode = U_ILLEGAL_ARGUMENT_ERROR;
        return 0;
    }

    *pErrorCode = U_ZERO_ERROR;
    return pCharsetInfo->aliasCount;
}

const char*
ucnv_getAlias(const char *name, uint16_t n, UErrorCode *pErrorCode)
{
    const CharsetInfo* pCharsetInfo = getCharset(name, false);
    if(pCharsetInfo == NULL || n >= pCharsetInfo->aliasCount)
    {
        *pErrorCode = U_ILLEGAL_ARGUMENT_ERROR;
        return NULL;
    }

    *pErrorCode = U_ZERO_ERROR;
    return pCharsetInfo->aliases[n];
}

UConverter*
ucnv_open (const char *name, UErrorCode * err)
{
    *err = U_ZERO_ERROR;

    const CharsetInfo* charset = getCharset(name, true);
    if(charset == NULL)
        charset = getCharset("NULL", true);

    return (UConverter*)charset;
}

void
ucnv_close (UConverter * converter)
{
}

const char*
ucnv_getName (const UConverter * converter, UErrorCode * err)
{
    return ((CharsetInfo*)converter)->name;
}

void
ucnv_setFallback(UConverter *cnv, UBool usesFallback)
{
}

const char* NextUTF8(const char* str, const char* end)
{
    while(str < end)
    {
        ++str;
        if( (*str & 0xC0) != 0x80 )
            break;
    }

    return str;
}

void
ucnv_toUnicode(UConverter *cnv,
               UChar **target, const UChar *targetLimit,
               const char **source, const char *sourceLimit,
               int32_t *offsets,
               UBool flush,
               UErrorCode *err)
{
    unsigned int codePage = ((CharsetInfo*)cnv)->codePage;
    const char* start_source = *source;
    int ret;
    const char* limit;
    while(*source < sourceLimit && *target < targetLimit)
    {
        if(codePage != CP_UTF8)
            limit = CharNextExA(codePage, *source, 0);
        else
            limit = NextUTF8(*source, sourceLimit);
        
        if (limit <= *source)
        {
            ++*source;
            continue;
        }

        if(limit > sourceLimit)
            limit = sourceLimit;

        ret = MultiByteToWideChar(codePage, 0, *source, limit - *source, *target, targetLimit - *target);
        if(ret == 0)
        {
            *source = limit;

            **target = '?';
            ++*target;
        }
        else if(*target + ret <= targetLimit)
        {
            *source = limit;
            *target = *target + ret;
        }
        else
        {
            break;
        }
    }
    
    if(*source < sourceLimit)
        *err = U_BUFFER_OVERFLOW_ERROR;
    else if(start_source < *source)
        *err = U_ZERO_ERROR;
    else
        *err = U_ILLEGAL_ARGUMENT_ERROR;
}

void
ucnv_setToUCallBack (UConverter * converter,
                     UConverterToUCallback newAction,
                     const void* newContext,
                     UConverterToUCallback *oldAction,
                     const void** oldContext,
                     UErrorCode * err)
{
    *err = U_ZERO_ERROR;
}

void
ucnv_fromUnicode(UConverter *cnv,
                 char **target, const char *targetLimit,
                 const UChar **source, const UChar *sourceLimit,
                 int32_t *offsets,
                 UBool flush,
                 UErrorCode *err)
{
    const UChar* start_source = *source;
    unsigned int codePage = ((CharsetInfo*)cnv)->codePage;

    int ret;
    while(*source < sourceLimit && *target < targetLimit)
    {
        ret = WideCharToMultiByte(codePage, 0, *source, 1, *target, targetLimit - *target, NULL, NULL);
        if(ret > 0 && *target + ret <= targetLimit)
        {
            *source += 1;
            *target += ret;
        }
        else
        {
            break;
        }
    }

    if(start_source < *source)
        *err = U_ZERO_ERROR;
    else
        *err = U_ILLEGAL_ARGUMENT_ERROR;
}

void
ucnv_setFromUCallBack (UConverter * converter,
                       UConverterFromUCallback newAction,
                       const void* newContext,
                       UConverterFromUCallback *oldAction,
                       const void** oldContext,
                       UErrorCode * err)
{
    *err = U_ZERO_ERROR;
}

void
ucnv_setSubstChars (UConverter * converter,
                    const char *mySubChar,
                    int8_t len,
                    UErrorCode * err)
{
    *err = U_ZERO_ERROR;
}

void
ucnv_cbFromUWriteBytes (UConverterFromUnicodeArgs *args,
                        const char* source,
                        int32_t length,
                        int32_t offsetIndex,
                        UErrorCode * err)
{
    *err = U_ZERO_ERROR;
}

void
ucnv_cbFromUWriteUChars(UConverterFromUnicodeArgs *args,
                        const UChar** source,
                        const UChar*  sourceLimit,
                        int32_t offsetIndex,
                        UErrorCode * err)
{
    *err = U_ZERO_ERROR;
}

void
UCNV_TO_U_CALLBACK_SUBSTITUTE (
                               const void *context,
                               UConverterToUnicodeArgs *toArgs,
                               const char* codeUnits,
                               int32_t length,
                               UConverterCallbackReason reason,
                               UErrorCode * err)
{
    *err = U_ZERO_ERROR;
}

void
UCNV_FROM_U_CALLBACK_SUBSTITUTE (
                                 const void *context,
                                 UConverterFromUnicodeArgs *fromArgs,
                                 const UChar* codeUnits,
                                 int32_t length,
                                 UChar32 codePoint,
                                 UConverterCallbackReason reason,
                                 UErrorCode * err)
{
    *err = U_ZERO_ERROR;
}

void
UCNV_FROM_U_CALLBACK_ESCAPE (
                             const void *context,
                             UConverterFromUnicodeArgs *fromArgs,
                             const UChar *codeUnits,
                             int32_t length,
                             UChar32 codePoint,
                             UConverterCallbackReason reason,
                             UErrorCode * err)
{
    *err = U_ZERO_ERROR;
}





const char *
ucsdet_getName(const UCharsetMatch *ucsm, UErrorCode *status)
{
    *status = U_INTERNAL_PROGRAM_ERROR;
    return NULL;
}

int32_t
ucsdet_getConfidence(const UCharsetMatch *ucsm, UErrorCode *status)
{
    *status = U_INTERNAL_PROGRAM_ERROR;
    return 0;
}

void
ucsdet_close(UCharsetDetector *ucsd)
{
}

const UCharsetMatch**
ucsdet_detectAll(UCharsetDetector *ucsd,
                 int32_t *maxMatchesFound, UErrorCode *status)
{
    *status = U_INTERNAL_PROGRAM_ERROR;
    return NULL;
}

void
ucsdet_setText(UCharsetDetector *ucsd, const char *textIn, int32_t len, UErrorCode *status)
{
    *status = U_INTERNAL_PROGRAM_ERROR;
}

UBool
ucsdet_enableInputFilter(UCharsetDetector *ucsd, UBool filter)
{
    return FALSE;
}

UCharsetDetector *
ucsdet_open(UErrorCode   *status)
{
    *status = U_INTERNAL_PROGRAM_ERROR;
    return NULL;
}

UEnumeration *
ucurr_openISOCurrencies(uint32_t currType, UErrorCode *pErrorCode)
{
    *pErrorCode = U_INTERNAL_PROGRAM_ERROR;
    return NULL;
}

const UChar*
ucurr_getName(const UChar* currency,
              const char* locale,
              UCurrNameStyle nameStyle,
              UBool* isChoiceFormat,
              int32_t* len,
              UErrorCode* ec)
{
    *ec = U_INTERNAL_PROGRAM_ERROR;
    return L"";
}

double
ucurr_getRoundingIncrement(const UChar* currency,
                           UErrorCode* ec)
{
    *ec = U_INTERNAL_PROGRAM_ERROR;
    return 0;
}

int32_t
ucurr_getDefaultFractionDigits(const UChar* currency,
                               UErrorCode* ec)
{
    *ec = U_INTERNAL_PROGRAM_ERROR;
    return 0;
}

void
uenum_close(UEnumeration* en)
{
}

const char*
uenum_next(UEnumeration* en,
           int32_t* resultLength,
           UErrorCode* status)
{
    *status = U_INTERNAL_PROGRAM_ERROR;
    return "";
}

const UChar*
uenum_unext(UEnumeration* en,
            int32_t* resultLength,
            UErrorCode* status)
{
    *status = U_INTERNAL_PROGRAM_ERROR;
    return L"";
}

void
uset_close(USet* set)
{
}

void
uset_clear(USet* set)
{
}

int32_t
uset_getItemCount(const USet* set)
{
    return 0;
}

int32_t
uset_getItem(const USet* set, int32_t itemIndex,
             UChar32* start, UChar32* end,
             UChar* str, int32_t strCapacity,
             UErrorCode* ec)
{
    *ec = U_INTERNAL_PROGRAM_ERROR;
    return 0;
}

ULocaleData*
ulocdata_open(const char *localeID, UErrorCode *status)
{
    *status = U_INTERNAL_PROGRAM_ERROR;
    return NULL;
}

void
ulocdata_close(ULocaleData *uld)
{
}

USet*
ulocdata_getExemplarSet(ULocaleData *uld, USet *fillIn, 
                        uint32_t options, ULocaleDataExemplarSetType extype, UErrorCode *status)
{
    *status = U_INTERNAL_PROGRAM_ERROR;
    return NULL;
}

UMeasurementSystem
ulocdata_getMeasurementSystem(const char *localeID, UErrorCode *status)
{
    *status = U_ZERO_ERROR;
    return UMS_SI;
}

int32_t uloc_countAvailable(void)
{
    return 0;
}

const char*
uloc_getAvailable(int32_t n)
{
    return NULL;
}

int32_t
uloc_getKeywordValue(const char* localeID,
                     const char* keywordName,
                     char* buffer, int32_t bufferCapacity,
                     UErrorCode* status)
{
    *status = U_INTERNAL_PROGRAM_ERROR;
    return 0;
}

const char* const*
uloc_getISOLanguages(void)
{
    return NULL;
}

const char* const* U_EXPORT2
uloc_getISOCountries(void)
{
    return NULL;
}

int32_t
uloc_getDisplayLanguage(const char* locale,
            const char* displayLocale,
            UChar* language,
            int32_t languageCapacity,
            UErrorCode* status)
{
    *status = U_INTERNAL_PROGRAM_ERROR;
    return 0;
}

int32_t
uloc_getDisplayName(const char* localeID,
            const char* inLocaleID,
            UChar* result,
            int32_t maxResultSize,
            UErrorCode* err)
{
    *err = U_INTERNAL_PROGRAM_ERROR;
    return 0;
}

int32_t
uloc_getDisplayCountry(const char* locale,
                       const char* displayLocale,
                       UChar* country,
                       int32_t countryCapacity,
                       UErrorCode* status)
{
    *status = U_INTERNAL_PROGRAM_ERROR;
    return 0;
}

int32_t
uloc_getDisplayScript(const char* locale,
                      const char* displayLocale,
                      UChar* script,
                      int32_t scriptCapacity,
                      UErrorCode* status)
{
    *status = U_INTERNAL_PROGRAM_ERROR;
    return 0;
}

int32_t
uloc_getDisplayVariant(const char* locale,
                       const char* displayLocale,
                       UChar* variant,
                       int32_t variantCapacity,
                       UErrorCode* status)
{
    *status = U_INTERNAL_PROGRAM_ERROR;
    return 0;
}

int32_t
uloc_getDisplayKeywordValue(   const char* locale,
                               const char* keyword,
                               const char* displayLocale,
                               UChar* dest,
                               int32_t destCapacity,
                               UErrorCode* status)
{
    *status = U_INTERNAL_PROGRAM_ERROR;
    return 0;
}

int32_t
uloc_canonicalize(const char* localeID,
                  char* name,
                  int32_t nameCapacity,
                  UErrorCode* err)
{
    *err = U_INTERNAL_PROGRAM_ERROR;
    return 0;
}

int32_t
uloc_setKeywordValue(const char* keywordName,
                     const char* keywordValue,
                     char* buffer, int32_t bufferCapacity,
                     UErrorCode* status)
{
    *status = U_INTERNAL_PROGRAM_ERROR;
    return 0;
}

UEnumeration*
uloc_openKeywords(const char* localeID, UErrorCode* status)
{
    *status = U_INTERNAL_PROGRAM_ERROR;
    return NULL;
}

int32_t
uloc_getVariant(const char* localeID,
                char* variant,
                int32_t variantCapacity,
                UErrorCode* err)
{
    *err = U_INTERNAL_PROGRAM_ERROR;
    return 0;
}

int32_t
uloc_getCountry(const char*    localeID,
                char* country,
                int32_t countryCapacity,
                UErrorCode* err)
{
    *err = U_INTERNAL_PROGRAM_ERROR;
    return 0;
}

int32_t
uloc_getScript(const char*    localeID,
               char* script,
               int32_t scriptCapacity,
               UErrorCode* err)
{
    *err = U_INTERNAL_PROGRAM_ERROR;
    return 0;
}

int32_t
uloc_getLanguage(const char*    localeID,
                char* language,
                int32_t languageCapacity,
                UErrorCode* err)
{
    *err = U_INTERNAL_PROGRAM_ERROR;
    return 0;
}


UNumberFormat*
unum_open(  UNumberFormatStyle    style,
            const    UChar*    pattern,
            int32_t            patternLength,
            const    char*     locale,
            UParseError*       parseErr,
            UErrorCode*        status)
{
    *status = U_INTERNAL_PROGRAM_ERROR;
    return NULL;
}

void
unum_close(UNumberFormat* fmt)
{
}

int32_t
unum_getAttribute(const UNumberFormat*    fmt,
                  UNumberFormatAttribute  attr)
{
    return 0;
}

int32_t
unum_toPattern( const UNumberFormat* fmt,
                UBool isPatternLocalized,
                UChar* result,
                int32_t resultLength,
                UErrorCode* status)
{
    *status = U_INTERNAL_PROGRAM_ERROR;
    return 0;
}

void
unum_setAttribute( UNumberFormat*          fmt,
                   UNumberFormatAttribute  attr,
                   int32_t                 newValue)
{
}

void
unum_applyPattern( UNumberFormat  *format,
                   UBool          localized,
                   const   UChar  *pattern,
                   int32_t        patternLength,
                   UParseError    *parseError,
                   UErrorCode     *status)
{
    *status = U_INTERNAL_PROGRAM_ERROR;
}

void
unum_setSymbol(UNumberFormat *fmt,
               UNumberFormatSymbol symbol,
               const UChar *value,
               int32_t length,
               UErrorCode *status)
{
    *status = U_INTERNAL_PROGRAM_ERROR;
}

int32_t
unum_formatInt64(const UNumberFormat *fmt,
                 int64_t         number,
                 UChar*          result,
                 int32_t         resultLength,
                 UFieldPosition *pos,
                 UErrorCode*     status)
{
    *status = U_INTERNAL_PROGRAM_ERROR;
    return 0;
}

int32_t
unum_formatDouble(const UNumberFormat*  fmt,
                  double          number,
                  UChar*          result,
                  int32_t         resultLength,
                  UFieldPosition  *pos, /* 0 if ignore */
                  UErrorCode*     status)
{
    *status = U_INTERNAL_PROGRAM_ERROR;
    return 0;
}

double
unum_parseDouble(const   UNumberFormat*  fmt,
                 const   UChar*          text,
                 int32_t         textLength,
                 int32_t         *parsePos /* 0 = start */,
                 UErrorCode      *status)
{
    *status = U_INTERNAL_PROGRAM_ERROR;
    return 0;
}

int64_t
unum_parseInt64(const UNumberFormat*  fmt,
                const UChar*  text,
                int32_t       textLength,
                int32_t       *parsePos /* 0 = start */,
                UErrorCode    *status)
{
    *status = U_INTERNAL_PROGRAM_ERROR;
    return 0;
}

void
unum_setDoubleAttribute(UNumberFormat*          fmt,
                        UNumberFormatAttribute  attr,
                        double                  newValue)
{
}

void
unum_setTextAttribute(UNumberFormat*             fmt,
                      UNumberFormatTextAttribute tag,
                      const    UChar*            newValue,
                      int32_t                    newValueLength,
                      UErrorCode                 *status)
{
    *status = U_INTERNAL_PROGRAM_ERROR;
}

double
unum_getDoubleAttribute(const UNumberFormat*    fmt,
                        UNumberFormatAttribute  attr)
{
    return 0;
}

int32_t
unum_getSymbol(const UNumberFormat *fmt,
               UNumberFormatSymbol symbol,
               UChar *buffer,
               int32_t size,
               UErrorCode *status)
{
    *status = U_INTERNAL_PROGRAM_ERROR;
    return 0;
}

int32_t
unum_getTextAttribute(const    UNumberFormat*       fmt,
                      UNumberFormatTextAttribute    tag,
                      UChar*                        result,
                      int32_t                       resultLength,
                      UErrorCode*                   status)
{
    *status = U_INTERNAL_PROGRAM_ERROR;
    return 0;
}

UCalendar*
ucal_open(const UChar*   zoneID,
          int32_t        len,
          const char*    locale,
          UCalendarType  type,
          UErrorCode*    status)
{
    *status = U_INTERNAL_PROGRAM_ERROR;
    return NULL;
}

void
ucal_close(UCalendar *cal)
{
}

void
ucal_clear(UCalendar* calendar)
{
}

int32_t
ucal_getAttribute(const UCalendar*    cal,
                  UCalendarAttribute  attr)
{
    return 0;
}

void
ucal_setAttribute(UCalendar*          cal,
                  UCalendarAttribute  attr,
                  int32_t             newValue)
{
}

UDate
ucal_getGregorianChange(const UCalendar *cal, UErrorCode *pErrorCode)
{
    *pErrorCode = U_INTERNAL_PROGRAM_ERROR;
    return 0;
}

void
ucal_setGregorianChange(UCalendar *cal, UDate date, UErrorCode *pErrorCode)
{
    *pErrorCode = U_INTERNAL_PROGRAM_ERROR;
}

int32_t
ucal_getLimit(const UCalendar*     cal,
              UCalendarDateFields  field,
              UCalendarLimitType   type,
              UErrorCode*          status)
{
    *status = U_INTERNAL_PROGRAM_ERROR;
    return 0;
}

void
ucal_add(UCalendar*           cal,
         UCalendarDateFields  field,
         int32_t              amount,
         UErrorCode*          status)
{
    *status = U_INTERNAL_PROGRAM_ERROR;
}

int32_t
ucal_get(const UCalendar*     cal,
         UCalendarDateFields  field,
         UErrorCode*          status )
{
    *status = U_INTERNAL_PROGRAM_ERROR;
    return 0;
}

void
ucal_set(UCalendar*           cal,
         UCalendarDateFields  field,
         int32_t              value)
{
}


UDate
ucal_getMillis(const UCalendar*  cal,
               UErrorCode*       status)
{
    *status = U_INTERNAL_PROGRAM_ERROR;
    return 0;
}

void
ucal_setMillis(UCalendar*   cal,
               UDate        dateTime,
               UErrorCode*  status )
{
    *status = U_INTERNAL_PROGRAM_ERROR;
}

void
ucal_roll(UCalendar*           cal,
          UCalendarDateFields  field,
          int32_t              amount,
          UErrorCode*          status)
{
    *status = U_INTERNAL_PROGRAM_ERROR;
}

int32_t
ucal_getTimeZoneDisplayName(const UCalendar*          cal,
                            UCalendarDisplayNameType  type,
                            const char*               locale,
                            UChar*                    result,
                            int32_t                   resultLength,
                            UErrorCode*               status)
{
    *status = U_INTERNAL_PROGRAM_ERROR;
    return 0;
}

void icuwin_init()
{
    initCharsets();
}

const char* icuwin_getDefaultEncoding()
{
    int codePage = GetACP();
    const CharsetInfo* ci = getCharset(codePage);
    if (ci)
        return ci->name;
        
    return ""; 
}