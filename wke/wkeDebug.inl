
void outputMsg(const char* msg, ...)
{
    char string[1024];

    va_list args;
    va_start(args, msg);
    _vsnprintf(string, 1024, msg, args);
    string[1023] = '\0';
    va_end(args);

    OutputDebugStringA(string);
}

#ifdef _DEBUG
//#define dbgMsg(msg, ...) outputMsg(msg, __VA_ARGS__)
#define dbgMsg(msg, ...) { printf(msg, __VA_ARGS__); printf("\n"); } 
#else
#define dbgMsg(msg, ...) __noop
#endif