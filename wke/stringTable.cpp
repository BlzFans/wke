#include <WebCore/config.h>
#include <wtf/HashSet.h>
#include <wtf/StringHasher.h>

struct StringHash {

    static unsigned hash(const char* key) 
    { 
        ASSERT(false);
        return 0;
    }

    static bool equal(const char* a, const char* b)
    {
        ASSERT(false);
        return false;
    }

    static unsigned hash(const wchar_t* key) 
    { 
        ASSERT(false);
        return 0;
    }

    static bool equal(const wchar_t* a, const wchar_t* b)
    {
        ASSERT(false);
        return false;
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
    typedef StringHash Hash;
};

template<> struct DefaultHash<const wchar_t*> {
    typedef StringHash Hash;
};

}

HashSet<const char*> s_stringTable;
HashSet<const wchar_t*> s_stringTableW;

const char* addString(const char* str, unsigned int len/* = 0*/)
{
    if (str == NULL)
        return "";

    if (len == 0)
        len = strlen(str);

    CharBuffer buf = { str, len };
    return *s_stringTable.add<CharBuffer, CharBufferTranslator>(buf).first;
}

const wchar_t* addString(const wchar_t* str, unsigned int len/* = 0*/)
{
    if (str == NULL)
        return L"";

    if (len == 0)
        len = wcslen(str);

    WCharBuffer buf = { str, len };
    return *s_stringTableW.add<WCharBuffer, WCharBufferTranslator>(buf).first;
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

