
void outputMsg(const char* msg, ...);
void outputMsg(const wchar_t* msg, ...);

#ifdef _DEBUG
#define dbgMsg(msg, ...) outputMsg(msg, __VA_ARGS__)
#else
#define dbgMsg(msg, ...) ((void)0)
#endif

#define CSTR(str) (WTF::String(str.impl()).charactersWithNullTermination())