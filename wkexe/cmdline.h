#ifndef WKEXE_CMDLINE_H
#define WKEXE_CMDLINE_H


#include <windows.h>


typedef struct
{
    int transparent;
    WCHAR* html;
    int showHelp;
    WCHAR cookiePath[MAX_PATH];

} CommandOptions;

void InitOptions(CommandOptions* options);
void ParseOptions(CommandOptions* options, int argc, LPWSTR* argv);
void FreeOptions(CommandOptions* options);

void PrintHelp();


#endif//#ifndef WKEXE_CMDLINE_H