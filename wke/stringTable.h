#ifndef STRING_TABLE_H
#define STRING_TABLE_H

class StringTable
{
public:
    static const char* addString(const char* str, unsigned int len = 0);
    static const char* addString(const wchar_t* str, unsigned int len = 0);

    static const char* emptyString();
};

class StringTableW
{
public:
    static const wchar_t* addString(const char* str, unsigned int len = 0);
    static const wchar_t* addString(const wchar_t* str, unsigned int len = 0);

    static const wchar_t* emptyString();
};

void initStringTable();
void destroyStringTable();

#endif