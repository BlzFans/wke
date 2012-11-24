#include <windows.h>
#include <mlang.h>
#include <stdio.h>
#include <wchar.h>

#define USE_ICU_WIN 1

#if USE_ICU_WIN

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
    static int8_t s_charType[] = {
        U_CONTROL_CHAR, //0
        U_CONTROL_CHAR, //1
        U_CONTROL_CHAR, //2
        U_CONTROL_CHAR, //3
        U_CONTROL_CHAR, //4
        U_CONTROL_CHAR, //5
        U_CONTROL_CHAR, //6
        U_CONTROL_CHAR, //7
        U_CONTROL_CHAR, //8
        U_CONTROL_CHAR, //9
        U_CONTROL_CHAR, //10
        U_CONTROL_CHAR, //11
        U_CONTROL_CHAR, //12
        U_CONTROL_CHAR, //13
        U_CONTROL_CHAR, //14
        U_CONTROL_CHAR, //15
        U_CONTROL_CHAR, //16
        U_CONTROL_CHAR, //17
        U_CONTROL_CHAR, //18
        U_CONTROL_CHAR, //19
        U_CONTROL_CHAR, //20
        U_CONTROL_CHAR, //21
        U_CONTROL_CHAR, //22
        U_CONTROL_CHAR, //23
        U_CONTROL_CHAR, //24
        U_CONTROL_CHAR, //25
        U_CONTROL_CHAR, //26
        U_CONTROL_CHAR, //27
        U_CONTROL_CHAR, //28
        U_CONTROL_CHAR, //29
        U_CONTROL_CHAR, //30
        U_CONTROL_CHAR, //31

        U_SPACE_SEPARATOR, //32, space

        U_OTHER_PUNCTUATION, //33, !
        U_OTHER_PUNCTUATION, //34, "
        U_OTHER_PUNCTUATION, //35, #

        U_CURRENCY_SYMBOL, //36, $

        U_OTHER_PUNCTUATION, //37, %
        U_OTHER_PUNCTUATION, //38, &
        U_OTHER_PUNCTUATION, //39, '

        U_START_PUNCTUATION, //40, (
        U_END_PUNCTUATION,   //41, )

        U_OTHER_PUNCTUATION, //42, *
        U_MATH_SYMBOL,       //43, +
        U_OTHER_PUNCTUATION, //44, ,
        U_DASH_PUNCTUATION,  //45, -
        U_OTHER_PUNCTUATION, //46, .
        U_OTHER_PUNCTUATION, //47, /

        U_DECIMAL_DIGIT_NUMBER, //48, 0
        U_DECIMAL_DIGIT_NUMBER, //49, 1
        U_DECIMAL_DIGIT_NUMBER, //50, 2
        U_DECIMAL_DIGIT_NUMBER, //51, 3
        U_DECIMAL_DIGIT_NUMBER, //52, 4
        U_DECIMAL_DIGIT_NUMBER, //53, 5
        U_DECIMAL_DIGIT_NUMBER, //54, 6
        U_DECIMAL_DIGIT_NUMBER, //55, 7
        U_DECIMAL_DIGIT_NUMBER, //56, 8
        U_DECIMAL_DIGIT_NUMBER, //57, 9

        U_OTHER_PUNCTUATION, //58, :
        U_OTHER_PUNCTUATION, //59, ;

        U_MATH_SYMBOL, //60, <
        U_MATH_SYMBOL, //61, =
        U_MATH_SYMBOL, //62, >

        U_OTHER_PUNCTUATION, //63, ?
        U_OTHER_PUNCTUATION, //64, @

        U_UPPERCASE_LETTER, //65, A
        U_UPPERCASE_LETTER, //66, B
        U_UPPERCASE_LETTER, //67, C
        U_UPPERCASE_LETTER, //68, D
        U_UPPERCASE_LETTER, //69, E
        U_UPPERCASE_LETTER, //70, F
        U_UPPERCASE_LETTER, //71, G
        U_UPPERCASE_LETTER, //72, H
        U_UPPERCASE_LETTER, //73, I
        U_UPPERCASE_LETTER, //74, J
        U_UPPERCASE_LETTER, //75, K
        U_UPPERCASE_LETTER, //76, L
        U_UPPERCASE_LETTER, //77, M
        U_UPPERCASE_LETTER, //78, N
        U_UPPERCASE_LETTER, //79, O
        U_UPPERCASE_LETTER, //80, P
        U_UPPERCASE_LETTER, //81, Q
        U_UPPERCASE_LETTER, //82, R
        U_UPPERCASE_LETTER, //83, S
        U_UPPERCASE_LETTER, //84, T
        U_UPPERCASE_LETTER, //85, U
        U_UPPERCASE_LETTER, //86, V
        U_UPPERCASE_LETTER, //87, W
        U_UPPERCASE_LETTER, //88, X
        U_UPPERCASE_LETTER, //89, Y
        U_UPPERCASE_LETTER, //90, Z

        U_START_PUNCTUATION, //91, [
        U_OTHER_PUNCTUATION, //92, \   //
        U_END_PUNCTUATION,   //93, ]
        U_MODIFIER_SYMBOL,   //94, ^

        U_CONNECTOR_PUNCTUATION, //95, _
        U_MODIFIER_SYMBOL,       //96, `

        U_LOWERCASE_LETTER, // 97, a
        U_LOWERCASE_LETTER, // 98, b
        U_LOWERCASE_LETTER, // 99, c
        U_LOWERCASE_LETTER, //100, d
        U_LOWERCASE_LETTER, //101, e
        U_LOWERCASE_LETTER, //102, f
        U_LOWERCASE_LETTER, //103, g
        U_LOWERCASE_LETTER, //104, h
        U_LOWERCASE_LETTER, //105, i
        U_LOWERCASE_LETTER, //106, j
        U_LOWERCASE_LETTER, //107, k
        U_LOWERCASE_LETTER, //108, l
        U_LOWERCASE_LETTER, //109, m
        U_LOWERCASE_LETTER, //110, n
        U_LOWERCASE_LETTER, //111, o
        U_LOWERCASE_LETTER, //112, p
        U_LOWERCASE_LETTER, //113, q
        U_LOWERCASE_LETTER, //114, r
        U_LOWERCASE_LETTER, //115, s
        U_LOWERCASE_LETTER, //116, t
        U_LOWERCASE_LETTER, //117, u
        U_LOWERCASE_LETTER, //118, v
        U_LOWERCASE_LETTER, //119, w
        U_LOWERCASE_LETTER, //120, x
        U_LOWERCASE_LETTER, //121, y
        U_LOWERCASE_LETTER, //122, z

        U_START_PUNCTUATION, //123, {
        U_MATH_SYMBOL,       //124, |
        U_END_PUNCTUATION,   //125, }
        U_MATH_SYMBOL,       //126, ~

        U_CONTROL_CHAR, //127

        U_CONTROL_CHAR, //128
        U_CONTROL_CHAR, //129
        U_CONTROL_CHAR, //130
        U_CONTROL_CHAR, //131
        U_CONTROL_CHAR, //132
        U_CONTROL_CHAR, //133
        U_CONTROL_CHAR, //134
        U_CONTROL_CHAR, //135
        U_CONTROL_CHAR, //136
        U_CONTROL_CHAR, //137
        U_CONTROL_CHAR, //138
        U_CONTROL_CHAR, //139
        U_CONTROL_CHAR, //140
        U_CONTROL_CHAR, //141
        U_CONTROL_CHAR, //142
        U_CONTROL_CHAR, //143
        U_CONTROL_CHAR, //144
        U_CONTROL_CHAR, //145
        U_CONTROL_CHAR, //146
        U_CONTROL_CHAR, //147
        U_CONTROL_CHAR, //148
        U_CONTROL_CHAR, //149
        U_CONTROL_CHAR, //150
        U_CONTROL_CHAR, //151
        U_CONTROL_CHAR, //152
        U_CONTROL_CHAR, //153
        U_CONTROL_CHAR, //154
        U_CONTROL_CHAR, //155
        U_CONTROL_CHAR, //156
        U_CONTROL_CHAR, //157
        U_CONTROL_CHAR, //158
        U_CONTROL_CHAR, //159

        U_SPACE_SEPARATOR, //160
 
        U_OTHER_PUNCTUATION, //161

        U_CURRENCY_SYMBOL, //162
        U_CURRENCY_SYMBOL, //163
        U_CURRENCY_SYMBOL, //164
        U_CURRENCY_SYMBOL, //165

        U_OTHER_SYMBOL,    //166
        U_OTHER_SYMBOL,    //167
        U_MODIFIER_SYMBOL, //168
        U_OTHER_SYMBOL,    //169

        U_LOWERCASE_LETTER, //170

        U_INITIAL_PUNCTUATION, //171
        U_MATH_SYMBOL,         //172
        U_FORMAT_CHAR,         //173
        U_OTHER_SYMBOL,        //174
        U_MODIFIER_SYMBOL,     //175
        U_OTHER_SYMBOL,        //176
        U_MATH_SYMBOL,         //177
        U_OTHER_NUMBER,        //178
        U_OTHER_NUMBER,        //179
        U_MODIFIER_SYMBOL,     //180
        U_LOWERCASE_LETTER,    //181
        U_OTHER_SYMBOL,        //182
        U_OTHER_PUNCTUATION,   //183
        U_MODIFIER_SYMBOL,     //184
        U_OTHER_NUMBER,        //185
        U_LOWERCASE_LETTER,    //186
        U_FINAL_PUNCTUATION,   //187
        U_OTHER_NUMBER,        //188 1/2
        U_OTHER_NUMBER,        //189 1/2
        U_OTHER_NUMBER,        //190 3/4
        U_OTHER_PUNCTUATION,   //191

        U_UPPERCASE_LETTER,    //192
        U_UPPERCASE_LETTER,    //193
        U_UPPERCASE_LETTER,    //194
        U_UPPERCASE_LETTER,    //195
        U_UPPERCASE_LETTER,    //196
        U_UPPERCASE_LETTER,    //197
        U_UPPERCASE_LETTER,    //198
        U_UPPERCASE_LETTER,    //199
        U_UPPERCASE_LETTER,    //200
        U_UPPERCASE_LETTER,    //201
        U_UPPERCASE_LETTER,    //202
        U_UPPERCASE_LETTER,    //203
        U_UPPERCASE_LETTER,    //204
        U_UPPERCASE_LETTER,    //205
        U_UPPERCASE_LETTER,    //206
        U_UPPERCASE_LETTER,    //207
        U_UPPERCASE_LETTER,    //208
        U_UPPERCASE_LETTER,    //209
        U_UPPERCASE_LETTER,    //210
        U_UPPERCASE_LETTER,    //211
        U_UPPERCASE_LETTER,    //212
        U_UPPERCASE_LETTER,    //213
        U_UPPERCASE_LETTER,    //214
        U_MATH_SYMBOL,         //215 multiplication sign
        U_UPPERCASE_LETTER,    //216
        U_UPPERCASE_LETTER,    //217
        U_UPPERCASE_LETTER,    //218
        U_UPPERCASE_LETTER,    //219
        U_UPPERCASE_LETTER,    //220
        U_UPPERCASE_LETTER,    //221
        U_UPPERCASE_LETTER,    //222
        U_LOWERCASE_LETTER,    //223
        U_LOWERCASE_LETTER,    //224
        U_LOWERCASE_LETTER,    //225
        U_LOWERCASE_LETTER,    //226
        U_LOWERCASE_LETTER,    //227
        U_LOWERCASE_LETTER,    //228
        U_LOWERCASE_LETTER,    //229
        U_LOWERCASE_LETTER,    //230
        U_LOWERCASE_LETTER,    //231
        U_LOWERCASE_LETTER,    //232
        U_LOWERCASE_LETTER,    //233
        U_LOWERCASE_LETTER,    //234
        U_LOWERCASE_LETTER,    //235
        U_LOWERCASE_LETTER,    //236
        U_LOWERCASE_LETTER,    //237
        U_LOWERCASE_LETTER,    //238
        U_LOWERCASE_LETTER,    //239
        U_LOWERCASE_LETTER,    //240
        U_LOWERCASE_LETTER,    //241
        U_LOWERCASE_LETTER,    //242
        U_LOWERCASE_LETTER,    //243
        U_LOWERCASE_LETTER,    //244
        U_LOWERCASE_LETTER,    //245
        U_LOWERCASE_LETTER,    //246
        U_MATH_SYMBOL,         //247 division sign
        U_LOWERCASE_LETTER,    //248
        U_LOWERCASE_LETTER,    //249
        U_LOWERCASE_LETTER,    //250
        U_LOWERCASE_LETTER,    //251
        U_LOWERCASE_LETTER,    //252
        U_LOWERCASE_LETTER,    //253
        U_LOWERCASE_LETTER,    //254
        U_LOWERCASE_LETTER     //255
    };

    if (c < sizeof(s_charType)/sizeof(s_charType[0]))
        return s_charType[c];

    if (iswspace(c))
        return U_SPACE_SEPARATOR;

    return U_UNASSIGNED;
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

static const char trailingBytesForUTF8[256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};

/* returns length of next utf-8 sequence */
int u8_seqlen(unsigned char c)
{
    return trailingBytesForUTF8[c] + 1;
}

#define UCNV_MAX_CHAR_LEN (10)
struct UConverter
{
    const CharsetInfo* charset;
    char bytes[UCNV_MAX_CHAR_LEN];
    int bytes_len;

    UConverter()
        :charset(0)
        ,bytes_len(0)
    {
    }

    bool hasValidChar()
    {
        if (bytes_len == 0)
            return false;

        if (charset->codePage != CP_UTF8)
        {
            bytes[bytes_len] = 'A';
            bytes[bytes_len + 1] = '\0';
            char* ptr = CharNextExA(charset->codePage, bytes, 0);
            if (ptr > bytes + bytes_len)
                return false;

            return true;
        }

        //utf8
        if (u8_seqlen(bytes[0]) <= bytes_len)
            return true;

        return false;
    }

    bool conv(unsigned char c, UChar& uc)
    {
        bytes[bytes_len++] = c;
        if (bytes_len + 2 >= UCNV_MAX_CHAR_LEN || hasValidChar())
        {
            int ret = MultiByteToWideChar(charset->codePage, 0, bytes, bytes_len, &uc, 1);
            bytes_len = 0;

            return ret == 1 ? true : false;
        }

        return false;
    }
};

UConverter*
ucnv_open (const char *name, UErrorCode * err)
{
    *err = U_ZERO_ERROR;

    const CharsetInfo* charset = getCharset(name, true);
    if(charset == NULL)
        charset = getCharset("NULL", true);

    UConverter* conv = new UConverter;
    conv->charset = charset;
    return conv;
}

void
ucnv_close (UConverter * converter)
{
    delete converter;
}

const char*
ucnv_getName (const UConverter * converter, UErrorCode * err)
{
    return converter->charset->name;
}

void
ucnv_setFallback(UConverter *cnv, UBool usesFallback)
{
}

void
ucnv_toUnicode(UConverter *cnv,
               UChar **target, const UChar *targetLimit,
               const char **source, const char *sourceLimit,
               int32_t *offsets,
               UBool flush,
               UErrorCode *err)
{
    while (*source < sourceLimit && *target < targetLimit)
    {
        char c = *((*source)++);
        UChar uc;
        if (cnv->conv(c, uc))
        {
            *((*target)++) = uc;
        }
    }

    if (*source < sourceLimit)
        *err = U_BUFFER_OVERFLOW_ERROR;
    else
        *err = U_ZERO_ERROR;
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
    unsigned int codePage = cnv->charset->codePage;

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

#else

#ifdef _DEBUG
#pragma comment(lib, "icuind.lib")
#pragma comment(lib, "icuucd.lib")
#else
#pragma comment(lib, "icuin.lib")
#pragma comment(lib, "icuuc.lib")
#endif


void icuwin_init()
{
}

const char* icuwin_getDefaultEncoding()
{
    return "";
}

#endif
