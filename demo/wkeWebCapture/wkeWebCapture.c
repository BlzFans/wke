

#include <windows.h>
#include <stdio.h>
#include "../libwke/wke.h"

void saveBitmap(void* pixels, int w, int h, const wchar_t* filename);
void convertFilename(wchar_t* filename);

int main(int argc, char** argv)
{
    wkeWebView webView;
    const char* url;
    wchar_t filename[1024];
    void* pixels;
    int w, h;

    printf(wkeVersionString());

    wkeInit();
    webView = wkeCreateWebView();
    wkeResize(webView, 1024, 768);

    url = argc >= 2 ? argv[1] : "http://www.google.com";
    printf("loading url %s ...\n", url);    
    wkeLoadURL(webView, url);

    while (1)
    {
        wkeUpdate();
        if (wkeIsLoadComplete(webView))
            break;
			
        Sleep(10);
    }
	
    /*hidden scrollbar*/
    wkeRunJS(webView, "document.body.style.overflow='hidden'");

    w = wkeContentsWidth(webView);
    h = wkeContentsHeight(webView);
    wkeResize(webView, w, h);

    pixels = malloc(w*h*4);
    wkePaint(webView, pixels, 0);

    //save bitmap
    swprintf(filename, 1024, L"%s.bmp", wkeTitleW(webView));
    convertFilename(filename);
    wprintf(L"%s\n", filename);
    
    saveBitmap(pixels, w, h, filename);

    free(pixels);
    wkeDestroyWebView(webView);
    wkeShutdown();

    return 0;
}

void saveBitmap(void* pixels, int w, int h, const wchar_t* filename)
{
    BITMAPFILEHEADER fileHdr = {0};
    BITMAPINFOHEADER infoHdr = {0};
    FILE * fp = NULL;
    
    fileHdr.bfType = 0x4d42; //'BM'
    fileHdr.bfOffBits = sizeof(fileHdr) + sizeof(infoHdr);
    fileHdr.bfSize = w * h * 4 + fileHdr.bfOffBits;

    infoHdr.biSize = sizeof(BITMAPINFOHEADER);
    infoHdr.biWidth = w;
    infoHdr.biHeight = -h;
    infoHdr.biPlanes = 1;
    infoHdr.biBitCount = 32;
    infoHdr.biCompression = 0;
    infoHdr.biSizeImage = w * h * 4;
    infoHdr.biXPelsPerMeter = 3780;
    infoHdr.biYPelsPerMeter = 3780;

    fp = _wfopen(filename, L"wb");
    if (fp == NULL)
        return;

    fwrite(&fileHdr, sizeof(fileHdr), 1, fp);
    fwrite(&infoHdr, sizeof(infoHdr), 1, fp);
    fwrite(pixels, infoHdr.biSizeImage, 1, fp);
    fclose(fp);
}

void convertFilename(wchar_t* filename)
{
    int i;
    for (i = 0; filename[i]; ++i)
    {
        if( filename[i] == L'\\'
         || filename[i] == L'/'
         || filename[i] == L':'
         || filename[i] == L'*'
         || filename[i] == L'?'
         || filename[i] == L'\"'
         || filename[i] == L'<'
         || filename[i] == L'>'
         || filename[i] == L'|' )
         {
            filename[i] = L'_';
         }
    }
}
