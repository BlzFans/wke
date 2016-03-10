#include "cmdline.h"
#include "getopt.h"


typedef struct
{
    struct option_w option;
    LPCWSTR description;

} OPTIONW;


static OPTIONW s_options[] =
{
    {{ L"help",          ARG_NONE,   NULL,       L'h' }, L"打印帮助信息\n"},
    {{ L"html",          ARG_REQ,    NULL,       L'x' }, L"设置要打开的HTML文件"},
    {{ L"transparent",  ARG_OPT,    0,          L't' }, L"支持使用分层窗口透明"},
    {{ L"cookie",        ARG_OPT,    0,          L'c' }, L"设置cookie文件路径"},
    {{ NULL,              ARG_NULL,   0,             0 }, NULL}
};

void ParseOptions(CommandOptions* options, int argc, LPWSTR* argv)
{
    struct option_w longOptions[100] = { 0 };
    WCHAR shortOptions[100] = { 0 };

    WCHAR val[2] = { 0 };
    OPTIONW* opt = s_options;

    int i = 0;
    do
    {
        longOptions[i++] = opt->option;

        val[0] = opt->option.val;
        wcscat(shortOptions, val);
        if (opt->option.has_arg)
            wcscat(shortOptions, L":");

        ++ opt;
    }
    while (opt->description);

    do
    {
        int option = getopt_long_w(argc, argv, shortOptions, longOptions, NULL);
        if (option == -1)
            break;

        switch (option)
        {
        case L'h':
            options->showHelp = TRUE;
            break;

        case L'x':
            options->html = (WCHAR*)malloc(sizeof(WCHAR) * (wcslen(optarg_w) + 1));
            wcscpy(options->html, optarg_w);
            break;

        case L't':
            if (!optarg_w || 
                _wcsicmp(optarg_w, L"yes") == 0 || 
                _wcsicmp(optarg_w, L"true") == 0)
            {
                options->transparent = TRUE;
            }
            else
            {
                options->transparent = FALSE;
            }
            break;

        case L'c':
            wcscpy(options->cookiePath, optarg_w);
            break;
        }
    }
    while (1);
}

void PrintHelp()
{
    WCHAR helpString[1024] = { 0 };
    int helpLength = 0;

    OPTIONW* opt = s_options;
    do
    {
        WCHAR* buffer = (LPWSTR)helpString + helpLength;
        int capacity = 1024 - helpLength;
        helpLength += _snwprintf(buffer, capacity, L"--%-15s%s\n", opt->option.name, opt->description);

        ++ opt;
    }
    while (opt->description);

    MessageBoxW(NULL, helpString, L"wkexe", MB_OK|MB_ICONINFORMATION);
}


void InitOptions(CommandOptions* options)
{
    memset(options, 0, sizeof(options));
}

void FreeOptions(CommandOptions* options)
{
    if (options->html)
    {
        free(options->html);
        options->html = NULL;
    }
    memset(options, 0, sizeof(options));
}
