

#include <windows.h>
#include <stdio.h>

#include "../libwke/wke.h"

#define FILENAME_LENGTH (1024)
void saveBitmap(void* pixels, int w, int h, const char* filename);
void convertFilename(const char* utf8, char* ansi);

int main(int argc, char** argv)
{
    wkeWebView webView;
    const char* url;
    char filename[FILENAME_LENGTH];
    char title[FILENAME_LENGTH];
    void* pixels;
	int w, h;

    printf(wkeVersionString());

    wkeInit();
    webView = wkeCreateWebView("");
    wkeResize(webView, 1024, 768);

    url = argc >= 2 ? argv[1] : "http://www.google.com";
    printf("loading url %s ...\n", url);    
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
    convertFilename(wkeTitle(webView), title);
    sprintf(filename, "%s.bmp", title);
	printf("%s\n", filename);
    saveBitmap(pixels, w, h, filename);
    free(pixels);

    wkeDestroyWebView(webView);
    wkeShutdown();

    return 0;
}

void saveBitmap(void* pixels, int w, int h, const char* filename)
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

    fp = fopen(filename, "wb");
    if (fp == NULL)
        return;

    fwrite(&fileHdr, sizeof(fileHdr), 1, fp);
    fwrite(&infoHdr, sizeof(infoHdr), 1, fp);
    fwrite(pixels, infoHdr.biSizeImage, 1, fp);
    fclose(fp);
}

void convertFilename(const char* utf8, char* ansi)
{
    int len, i;
    wchar_t wstr[FILENAME_LENGTH];
    
    len = MultiByteToWideChar(CP_UTF8, 0, utf8, strlen(utf8), wstr, FILENAME_LENGTH);
	for (i = 0; i < len; ++i)
	{
		if( wstr[i] == L'\\'
		 || wstr[i] == L'/'
         || wstr[i] == L':'
         || wstr[i] == L'*'
         || wstr[i] == L'?'
         || wstr[i] == L'\"'
         || wstr[i] == L'<'
         || wstr[i] == L'>'
		 || wstr[i] == L'|' )
		 {
			wstr[i] = L'_';
		 }
	}
	
    len = WideCharToMultiByte(CP_ACP, 0, wstr, len, ansi, FILENAME_LENGTH - 8, NULL, NULL);
    ansi[len] = '\0';
}
