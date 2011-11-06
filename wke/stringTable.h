#ifndef STRING_TABLE_H
#define STRING_TABLE_H

const wchar_t* addString(const wchar_t* str, unsigned int len = 0);
const char* addString(const char* str, unsigned int len = 0);

void initStringTable();
void destroyStringTable();

#endif