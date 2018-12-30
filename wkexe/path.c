
#include "path.h"
#include <stdio.h>


#include <shellapi.h>
#pragma comment(lib, "shell32.lib")

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")


#pragma warning(disable:4996) //'xxxxx': This function or variable may be unsafe. Consider using xxxxxx_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.


LPCWSTR GetWorkingDirectory(LPWSTR buffer, size_t bufferSize)
{
    GetCurrentDirectoryW(bufferSize, buffer);
    wcscat(buffer, L"\\");
    return buffer;
}

LPCWSTR GetWorkingPath(LPWSTR buffer, size_t bufferSize, LPCWSTR relatedPath)
{
    WCHAR dir[MAX_PATH + 1] = { 0 };
    GetWorkingDirectory(dir, MAX_PATH);
    _snwprintf(buffer, bufferSize, L"%s%s", dir, relatedPath);
    return buffer;
}

LPCWSTR FormatWorkingPath(LPWSTR buffer, size_t bufferSize, LPCWSTR fmt, ...)
{
    WCHAR relatedPath[MAX_PATH + 1] = { 0 };
    va_list args;
    va_start(args, fmt);
    _vsnwprintf(relatedPath, MAX_PATH, fmt, args);
    va_end(args);

    return GetWorkingPath(buffer, bufferSize, relatedPath);
}

LPCWSTR GetProgramDirectory(LPWSTR buffer, size_t bufferSize)
{
    DWORD i = GetModuleFileNameW(NULL, buffer, bufferSize);

    -- i;
    while (buffer[i] != '\\' && i != 0)
        -- i;

    buffer[i+1] = 0;
    return buffer;
}

LPCWSTR GetProgramPath(LPWSTR buffer, size_t bufferSize, LPCWSTR relatedPath)
{
    WCHAR dir[MAX_PATH + 1] = { 0 };
    GetProgramDirectory(dir, MAX_PATH);
    _snwprintf(buffer, bufferSize, L"%s%s", dir, relatedPath);
    return buffer;
}

LPCWSTR FormatProgramPath(LPWSTR buffer, size_t bufferSize, LPCWSTR fmt, ...)
{
    WCHAR relatedPath[MAX_PATH + 1] = { 0 };
    va_list args;
    va_start(args, fmt);
    _vsnwprintf(relatedPath, MAX_PATH, fmt, args);
    va_end(args);

    return GetProgramPath(buffer, bufferSize, relatedPath);
}


LPWSTR FixupHtmlFilePath(LPWSTR path)
{
    WCHAR fullPath[MAX_PATH + 1] = { 0 };
    LPWSTR fullUrl = NULL;
    
    do
    {
        if (path)
        {
            GetWorkingPath(fullPath, MAX_PATH, path);
            if (PathFileExistsW(fullPath))
                break;

            GetProgramPath(fullPath, MAX_PATH, path);
            if (PathFileExistsW(fullPath))
                break;   
        }

        GetWorkingPath(fullPath, MAX_PATH, L"main.html");
        if (PathFileExistsW(fullPath))
            break;

        GetWorkingPath(fullPath, MAX_PATH, L"wkexe.html");
        if (PathFileExistsW(fullPath))
            break;    

        GetProgramPath(fullPath, MAX_PATH, L"index.html");
        if (PathFileExistsW(fullPath))
            break;    

        GetProgramPath(fullPath, MAX_PATH, L"main.html");
        if (PathFileExistsW(fullPath))
            break;

        GetProgramPath(fullPath, MAX_PATH, L"wkexe.html");
        if (PathFileExistsW(fullPath))
            break;

        return path;
    }
    while (0);

    fullUrl = (WCHAR*)malloc(sizeof(WCHAR) * (MAX_PATH + 1 + 10));
    _snwprintf(fullUrl, MAX_PATH + 1 + 10, L"file:///%s", fullPath);
    return fullUrl;
}