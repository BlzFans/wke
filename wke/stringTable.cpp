#include <WebCore/config.h>
#include <wtf/HashSet.h>
#include <wtf/StringHasher.h>
#include <wtf/text/CString.h>
#include <wtf/text/WTFString.h>

#include "stringTable.h"

struct StringTableHash {

    static unsigned hash(const char* key) 
    {
        return StringHasher::computeHash(key, strlen(key));
    }

    static bool equal(const char* a, const char* b)
    {
        return strcmp(a, b) == 0;
    }

    static unsigned hash(const wchar_t* key) 
    { 
        return StringHasher::computeHash(key, wcslen(key));
    }

    static bool equal(const wchar_t* a, const wchar_t* b)
    {
        return wcscmp(a, b) == 0;
    }

    static const bool safeToCompareToEmptyOrDeleted = false;
};

struct CharBuffer {
    const char* s;
    unsigned length;
};

struct CharBufferTranslator {
    static unsigned hash(const CharBuffer& buf)
    {
        return StringHasher::computeHash(buf.s, buf.length);
    }

    static bool equal(const char* const& str, const CharBuffer& buf)
    {
        return strncmp(str, buf.s, buf.length) == 0;
    }

    static void translate(const char*& location, const CharBuffer& buf, unsigned hash)
    {
        char* str = static_cast<char*>(fastMalloc(buf.length + 1));
        memcpy(str, buf.s, buf.length);
        str[buf.length] = '\0';

        location = str;
    }
};

struct WCharBuffer {
    const wchar_t* s;
    unsigned length;
};

struct WCharBufferTranslator {
    static unsigned hash(const WCharBuffer& buf)
    {
        return StringHasher::computeHash(buf.s, buf.length);
    }

    static bool equal(const wchar_t* const& str, const WCharBuffer& buf)
    {
        return wcsncmp(str, buf.s, buf.length) == 0;
    }

    static void translate(const wchar_t*& location, const WCharBuffer& buf, unsigned hash)
    {
        wchar_t* str = static_cast<wchar_t*>(fastMalloc((buf.length + 1)*sizeof(wchar_t)));
        memcpy(str, buf.s, buf.length*sizeof(wchar_t));
        str[buf.length] = L'\0';

        location = str;
    }
};


namespace WTF {

template<typename T> struct DefaultHash;
template<> struct DefaultHash<const char*> {
    typedef StringTableHash Hash;
};

template<> struct DefaultHash<const wchar_t*> {
    typedef StringTableHash Hash;
};

}

HashSet<const char*> s_stringTable;
HashSet<const wchar_t*> s_stringTableW;

static const wchar_t* s_empty = L"\0";

const char* StringTable::addString(const char* str, unsigned int len/* = 0*/)
{
    if (str == NULL || str[0] == '\0')
        return emptyString();

    if (len == 0)
        len = strlen(str);

    CharBuffer buf = { str, len };
    return *s_stringTable.add<CharBuffer, CharBufferTranslator>(buf).first;
}

const char* StringTable::addString(const wchar_t* str, unsigned int len/* = 0*/)
{
    if (str == NULL || str[0] == L'\0')
        return emptyString();

    if (len == 0)
        len = wcslen(str);

    CString s = String(str, len).utf8();
    return addString(s.data(), s.length());
}

const char* StringTable::emptyString()
{
    return (const char*)s_empty;
}

const wchar_t* StringTableW::addString(const wchar_t* str, unsigned int len/* = 0*/)
{
    if (str == NULL || str[0] == L'\0')
        return emptyString();

    if (len == 0)
        len = wcslen(str);

    WCharBuffer buf = { str, len };
    return *s_stringTableW.add<WCharBuffer, WCharBufferTranslator>(buf).first;
}

const wchar_t* StringTableW::addString(const char* str, unsigned int len/* = 0*/)
{
    if (str == NULL || str[0] == '\0')
        return emptyString();

    if (len == 0)
        len = strlen(str);

    String s = String::fromUTF8(str, len);
    return addString(s.characters(), s.length());
}

const wchar_t* StringTableW::emptyString()
{
    return s_empty;
}

void initStringTable()
{
}

void destroyStringTable()
{
    for (HashSet<const char*>::iterator itr = s_stringTable.begin(); itr != s_stringTable.end(); ++itr)
        fastFree((void*)*itr);

    s_stringTable.clear();

    for (HashSet<const wchar_t*>::iterator itr = s_stringTableW.begin(); itr != s_stringTableW.end(); ++itr)
        fastFree((void*)*itr);

    s_stringTableW.clear();
}

