
typedef unsigned short wchar_t;
typedef unsigned char bool;

#include <windows.h>
#include <stdio.h>

#include "../libwke/wke.h"

#define VIEW_WIDTH (800)
#define VIEW_HEIGHT (600)

void saveBitmap(void* pixels, int w, int h, const char* filename);
void utf8ToAnsi(const char* utf8, char* ansi);

int main(int argc, char** argv)
{
    wkeWebView webView;
    const char* url;
    char filename[512];
    char title[512];
    void* pixels;

    printf(wkeVersionString());

    wkeInit();

    webView = wkeCreateWebView();

    wkeResize(webView, VIEW_WIDTH, VIEW_HEIGHT);

    url = argc >= 2 ? argv[1] : "http://www.google.com";
    printf("url  %s\n", url);    
    wkeLoadURL(webView, url);

    while (1)
    {
        MSG msg;
        while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if(msg.message == WM_QUIT)
                break;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (!wkeIsLoading(webView))
            break;
    }

    pixels = malloc(VIEW_WIDTH*VIEW_HEIGHT*4);
    wkePaint(webView, pixels, 0);

    //save bitmap
    utf8ToAnsi(wkeTitle(webView), title);
    
    sprintf(filename, "%s.bmp", title);
    saveBitmap(pixels, VIEW_WIDTH, VIEW_HEIGHT, filename);
    free(pixels);

    wkeDestroy(webView);

    wkeShutdown();

    return 0;
}

void saveBitmap(void* pixels, int w, int h, const char* filename)
{
    BITMAPFILEHEADER fileHdr = {0};
    BITMAPINFOHEADER infoHdr = {0};
    FILE * fp = NULL;

    int lineWidth = (w + 3) & ~3;

    printf(filename);

    
    fileHdr.bfType = 0x4d42; //'BM'
    fileHdr.bfOffBits = sizeof(fileHdr) + sizeof(infoHdr);
    fileHdr.bfSize = lineWidth * h * 4 + fileHdr.bfOffBits;

    infoHdr.biSize = sizeof(BITMAPINFOHEADER);
    infoHdr.biWidth = w;
    infoHdr.biHeight = -h;
    infoHdr.biPlanes = 1;
    infoHdr.biBitCount = 32;
    infoHdr.biCompression = 0;
    infoHdr.biSizeImage = lineWidth * h * 4;
    infoHdr.biXPelsPerMeter = 3780;
    infoHdr.biYPelsPerMeter = 3780;

    fp = fopen(filename, "wb");
    if (fp == NULL)
        return;

    fwrite(&fileHdr, sizeof(fileHdr), 1, fp);
    fwrite(&infoHdr, sizeof(infoHdr), 1, fp);
    fwrite(pixels, infoHdr.biSizeImage, 1, fp);

    fclose(fp);
}

void utf8ToAnsi(const char* utf8, char* ansi)
{
    int len;
    wchar_t wstr[512];
    
    len = MultiByteToWideChar(CP_UTF8, 0, utf8, strlen(utf8), wstr, 512);
    len = WideCharToMultiByte(CP_ACP, 0, wstr, len, ansi, 512, NULL, NULL);
    ansi[len] = '\0';
}
