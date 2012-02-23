
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <time.h>

#include <wke.h>

#include "wkeBrowser.h"
#include "render.h"
#include "hyperlink.h"

class CTimer
{
public:
    void Start()
    {
        QueryPerformanceCounter(&m_StartCounter);
    }

    void End()
    {
        QueryPerformanceCounter(&m_EndCounter);
    }

    unsigned int GetCounter()
    {
        return m_EndCounter.LowPart - m_StartCounter.LowPart;
    }

    unsigned int GetTime()
    {
        LARGE_INTEGER freq;
        QueryPerformanceFrequency(&freq);

        return unsigned int( (long double)(m_EndCounter.QuadPart - m_StartCounter.QuadPart) / (long double)freq.QuadPart * 1000.f );
    }

private:
    LARGE_INTEGER m_StartCounter;
    LARGE_INTEGER m_EndCounter;
};


#define MAX_LOADSTRING 100
#define URLBAR_HEIGHT  24

HINSTANCE hInst;
HWND hMainWnd;
HWND hURLBarWnd = NULL;
HWND hViewWindow = NULL;
TCHAR szTitle[MAX_LOADSTRING];
TCHAR szWindowClass[MAX_LOADSTRING];

wkeWebView g_webView = NULL;
CRender* g_render = NULL;

ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

WNDPROC DefEditProc = NULL;
LRESULT CALLBACK UrlEditProc(HWND, UINT, WPARAM, LPARAM);

void resizeSubViews();
LRESULT CALLBACK WebViewWndProc(HWND, UINT, WPARAM, LPARAM);
const LPCWSTR wkeWebViewClassName = L"wkeWebView";
bool registerWebViewWindowClass();

jsValue JS_CALL js_msgBox(jsExecState es)
{
    const wchar_t* text = jsToStringW(es, jsArg(es, 0));
    const wchar_t* title = jsToStringW(es, jsArg(es, 1));

    MessageBox(hMainWnd, text, title, 0);

    return jsUndefined();
}

static int s_testCount = 0;
jsValue JS_CALL js_getTestCount(jsExecState es)
{
    return jsInt(s_testCount);
}

jsValue JS_CALL js_setTestCount(jsExecState es)
{
    s_testCount = jsToInt(es, jsArg(es, 0));

    return jsUndefined();
}


int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_WKEBROWSER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	if (!InitInstance (hInstance, nCmdShow))
		return FALSE;

    CTimer t1, t2, t3;

    t1.Start();
    wkeInit();
    t1.End();

    jsBindFunction("msgBox", js_msgBox, 2);
    jsBindGetter("testCount", js_getTestCount);
    jsBindSetter("testCount", js_setTestCount);

    t2.Start();
    g_webView = wkeCreateWebView();
    g_webView->setTransparent(false);
    t2.End();

    t3.Start();
    //g_webView->loadURL("file:///test/test.html");
    g_webView->loadHTML(L"<p style=\"background-color: #00FF00\">Testing</p><img id=\"webkit logo\" src=\"http://webkit.org/images/icon-gold.png\" alt=\"Face\"><div style=\"border: solid blue; background: white;\" contenteditable=\"true\">div with blue border</div><ul><li>foo<li>bar<li>baz</ul>");
    t3.End();

    unsigned int ms1 = t1.GetTime();
    unsigned int ms2 = t2.GetTime();
    unsigned int ms3 = t3.GetTime();

    hURLBarWnd = CreateWindow(L"EDIT", 0,
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_AUTOVSCROLL, 
        0, 0, 0, 0,
        hMainWnd,
        0,
        hInstance, 0);

    registerWebViewWindowClass();
    hViewWindow = CreateWindow(wkeWebViewClassName, 0, 
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        0, 0, 0, 0, 
        hMainWnd, 
        0, 
        hInstance, 0);

    resizeSubViews();

    DefEditProc = reinterpret_cast<WNDPROC>(GetWindowLongPtr(hURLBarWnd, GWL_WNDPROC));
    SetWindowLongPtr(hURLBarWnd, GWL_WNDPROC, reinterpret_cast<LONG_PTR>(UrlEditProc));
    SetFocus(hURLBarWnd);

    g_render = CRender::create(CRender::GDI_RENDER);
    g_render->init(hViewWindow);


	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WKEBROWSER));

    MSG msg;
    msg.message = WM_NULL;
	while (msg.message != WM_QUIT)
	{
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else if (!IsIconic(hMainWnd) && g_webView->isDirty())
        {
            g_render->render(g_webView);
        }
        else
        {
            Sleep(10);
        }
    }

    g_render->destroy();
    g_webView->destroy();
    wkeShutdown();

	return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WKEBROWSER));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_WKEBROWSER);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance;

   hMainWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hMainWnd)
      return FALSE;

   ShowWindow(hMainWnd, nCmdShow);
   UpdateWindow(hMainWnd);

   return TRUE;
}

static int s_currentZoom = 100;
static int s_zoomLevels[] = {30, 50, 67, 80, 90, 100, 110, 120, 133, 150, 170, 200, 240, 300};

void zoom(bool zoomIn)
{
    if (!g_webView)
        return;

    int count = sizeof(s_zoomLevels) / sizeof(int);

    int i = 0;
    for (i = 0; i < count; ++i)
    {
        if (s_zoomLevels[i] == s_currentZoom)
            break;
    }

    if (zoomIn)
        i = i + 1;
    else
        i = i - 1;

    if (i < 0)
        i = 0;

    if (i >= count)
        i = count -1;

    s_currentZoom = s_zoomLevels[i];

    g_webView->setZoomFactor(s_currentZoom / 100.f);
}

void resetZoom()
{
    s_currentZoom = 100;
    if (g_webView)
        g_webView->setZoomFactor(s_currentZoom / 100.f);
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

void saveBitmap(void* pixels, int w, int h, const wchar_t* title)
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

    struct tm t;
    time_t utc_time;
    time(&utc_time);
    localtime_s(&t, &utc_time);

    wchar_t name[1024];
    swprintf(name, 1024, L"%s_%4d%02d%02d_%02d%02d%02d.bmp", title,
        t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);

    convertFilename(name);

    wchar_t pathname[1024];
    swprintf(pathname, 1024, L"screenshots\\%s", name);
    _wmkdir(L"screenshots");
    _wfopen_s(&fp, pathname, L"wb");
    if (fp == NULL)
        return;

    fwrite(&fileHdr, sizeof(fileHdr), 1, fp);
    fwrite(&infoHdr, sizeof(infoHdr), 1, fp);
    fwrite(pixels, infoHdr.biSizeImage, 1, fp);
    fclose(fp);
}

void takeScreenshot()
{
    if (g_webView == NULL)
        return;

    g_webView->runJS("document.body.style.overflow='hidden'");	
	int w = g_webView->contentsWidth();
	int h = g_webView->contentsHeight();
    
    int oldwidth = g_webView->width();
    int oldheight = g_webView->height();
    g_webView->resize(w, h);
    wkeUpdate();

    void* pixels = malloc(w*h*4);
    g_webView->paint(pixels, 0);

    //save bitmap
    saveBitmap(pixels, w, h, g_webView->titleW());

    free(pixels);

    g_webView->resize(oldwidth, oldheight);
    g_webView->runJS("document.body.style.overflow='visible'");
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;

        case ID_FILE_GOBACK:
            if (g_webView)
                g_webView->goBack();
            break;

        case ID_FILE_GOFORWARD:
            if (g_webView)
                g_webView->goForward();
            break;

        case ID_ZOOM_IN:
            zoom(true);
            break;

        case ID_ZOOM_OUT:
            zoom(false);
            break;

        case ID_RESET_ZOOM:
            resetZoom();
            break;

        case ID_TAKE_SCREENSHOT:
            takeScreenshot();
            break;

        case ID_SET_EDITABLE:
            if (g_webView)
            {
                HMENU hMenu = GetMenu(hWnd);
                UINT state = GetMenuState(hMenu, ID_SET_EDITABLE, MF_BYCOMMAND);
                if (state & MF_CHECKED)
                {
                    g_webView->setEditable(false);
                    CheckMenuItem(hMenu, ID_SET_EDITABLE, MF_BYCOMMAND | MF_UNCHECKED); 
                }
                else
                {
                    g_webView->setEditable(true);
                    CheckMenuItem(hMenu, ID_SET_EDITABLE, MF_BYCOMMAND | MF_CHECKED); 
                }
            }
            break;

        case ID_URL_SF:
            SetWindowText(hURLBarWnd, L"http://wke.sf.net");
            SendMessage(hURLBarWnd, WM_CHAR, L'\r', 0);
            break;

        case ID_URL_GITHUB:
            SetWindowText(hURLBarWnd, L"http://www.github.com/BlzFans/wke");
            SendMessage(hURLBarWnd, WM_CHAR, L'\r', 0);
            break;

		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

    case WM_INITMENU:
        {
            bool canGoBack = false;
            bool canGoForward = false;

            if (g_webView && g_webView->canGoBack())
                canGoBack = true;

            if (g_webView && g_webView->canGoForward())
                canGoForward = true;

            EnableMenuItem((HMENU)wParam, ID_FILE_GOBACK, canGoBack ? MF_ENABLED : MF_DISABLED);
            EnableMenuItem((HMENU)wParam, ID_FILE_GOFORWARD, canGoForward ? MF_ENABLED : MF_DISABLED);
        }
        break;


	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

    case WM_SIZE:
        resizeSubViews();
        break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
        {
            RECT rect;
            rect.left = 63;
            rect.top = 40;
            rect.right = 166;
            rect.bottom = 58;
            CHyperlink::create(hDlg, rect, L"http://wke.sf.net", ID_URL_SF);

            rect.left = 63;
            rect.top = 60;
            rect.right = 290;
            rect.bottom = 78;
            CHyperlink::create(hDlg, rect, L"http://www.github.com/BlzFans/wke", ID_URL_GITHUB);
        }
        return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

LRESULT CALLBACK WebViewWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    bool handled = true;
	switch (message)
	{
    case WM_COMMAND:
        SendMessage(GetParent(hWnd), message, wParam, lParam);
        return 0;

    case WM_SIZE:
        if (g_webView && g_render)
        {
            g_webView->resize(LOWORD(lParam), HIWORD(lParam));
            g_render->resize(LOWORD(lParam), HIWORD(lParam));
        }
        break;

    case WM_KEYDOWN:
        {
            unsigned int virtualKeyCode = wParam;
            unsigned int flags = 0;
            if (HIWORD(lParam) & KF_REPEAT)
                flags |= WKE_REPEAT;
            if (HIWORD(lParam) & KF_EXTENDED)
                flags |= WKE_EXTENDED;

            //flags = HIWORD(lParam);

            handled = g_webView->keyDown(virtualKeyCode, flags, false);
        }
        break;

    case WM_KEYUP:
        {
            unsigned int virtualKeyCode = wParam;
            unsigned int flags = 0;
            if (HIWORD(lParam) & KF_REPEAT)
                flags |= WKE_REPEAT;
            if (HIWORD(lParam) & KF_EXTENDED)
                flags |= WKE_EXTENDED;

            //flags = HIWORD(lParam);

            handled = g_webView->keyUp(virtualKeyCode, flags, false);
        }
        break;

    case WM_CHAR:
        {
            unsigned int charCode = wParam;
            unsigned int flags = 0;
            if (HIWORD(lParam) & KF_REPEAT)
                flags |= WKE_REPEAT;
            if (HIWORD(lParam) & KF_EXTENDED)
                flags |= WKE_EXTENDED;

            //flags = HIWORD(lParam);

            handled = g_webView->keyPress(charCode, flags, false);
        }
        break;

    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
    case WM_MBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:

    case WM_MOUSEMOVE:
        {
            if (message == WM_LBUTTONDOWN || message == WM_MBUTTONDOWN || message == WM_RBUTTONDOWN)
            {
                SetFocus(hWnd);
                SetCapture(hWnd);
            }
            else if (message == WM_LBUTTONUP || message == WM_MBUTTONUP || message == WM_RBUTTONUP)
            {
                ReleaseCapture();
            }

            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);

            unsigned int flags = 0;

            if (wParam & MK_CONTROL)
                flags |= WKE_CONTROL;
            if (wParam & MK_SHIFT)
                flags |= WKE_SHIFT;

            if (wParam & MK_LBUTTON)
                flags |= WKE_LBUTTON;
            if (wParam & MK_MBUTTON)
                flags |= WKE_MBUTTON;
            if (wParam & MK_RBUTTON)
                flags |= WKE_RBUTTON;

            //flags = wParam;

            handled = g_webView->mouseEvent(message, x, y, flags);
        }
        break;

    case WM_CONTEXTMENU:
        {
            POINT pt;
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);

            if (pt.x != -1 && pt.y != -1)
                ScreenToClient(hWnd, &pt);

            unsigned int flags = 0;

            if (wParam & MK_CONTROL)
                flags |= WKE_CONTROL;
            if (wParam & MK_SHIFT)
                flags |= WKE_SHIFT;

            if (wParam & MK_LBUTTON)
                flags |= WKE_LBUTTON;
            if (wParam & MK_MBUTTON)
                flags |= WKE_MBUTTON;
            if (wParam & MK_RBUTTON)
                flags |= WKE_RBUTTON;

            handled = g_webView->contextMenuEvent(pt.x, pt.y, flags);
        }
        break;

    case WM_MOUSEWHEEL:
        {
            POINT pt;
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
            ScreenToClient(hWnd, &pt);

            int delta = GET_WHEEL_DELTA_WPARAM(wParam);

            unsigned int flags = 0;

            if (wParam & MK_CONTROL)
                flags |= WKE_CONTROL;
            if (wParam & MK_SHIFT)
                flags |= WKE_SHIFT;

            if (wParam & MK_LBUTTON)
                flags |= WKE_LBUTTON;
            if (wParam & MK_MBUTTON)
                flags |= WKE_MBUTTON;
            if (wParam & MK_RBUTTON)
                flags |= WKE_RBUTTON;

            //flags = wParam;

            handled = g_webView->mouseWheel(pt.x, pt.y, delta, flags);
        }
        break;

    case WM_SETFOCUS:
        g_webView->focus();
        break;

    case WM_KILLFOCUS:
        g_webView->unfocus();
        break;

    case WM_IME_STARTCOMPOSITION:
        {
            wkeRect caret = g_webView->getCaret();

            CANDIDATEFORM form;
            form.dwIndex = 0;
            form.dwStyle = CFS_EXCLUDE;
            form.ptCurrentPos.x = caret.x;
            form.ptCurrentPos.y = caret.y + caret.h;
            form.rcArea.top = caret.y;
            form.rcArea.bottom = caret.y + caret.h;
            form.rcArea.left = caret.x;
            form.rcArea.right = caret.x + caret.w;

            HIMC hIMC = ImmGetContext(hWnd);
            ImmSetCandidateWindow(hIMC, &form);
            ImmReleaseContext(hWnd, hIMC);
        }
        break;

    default:
        handled = false;
        break;
	}
    
    if (!handled)
        return DefWindowProc(hWnd, message, wParam, lParam);

    return 0;
}

void resizeSubViews()
{
    if (hURLBarWnd && hViewWindow)
    {
        RECT rcClient;
        GetClientRect(hMainWnd, &rcClient);
        MoveWindow(hURLBarWnd, 0, 0, rcClient.right, URLBAR_HEIGHT, TRUE);
        MoveWindow(hViewWindow, 0, URLBAR_HEIGHT, rcClient.right, rcClient.bottom - URLBAR_HEIGHT, TRUE);

        g_webView->resize(rcClient.right, rcClient.bottom - URLBAR_HEIGHT);
    }
}

bool registerWebViewWindowClass()
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_DBLCLKS;
    wcex.lpfnWndProc    = WebViewWndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInst;
    wcex.hIcon          = 0;
    wcex.hCursor        = LoadCursor(0, IDC_ARROW);
    wcex.hbrBackground  = 0;
    wcex.lpszMenuName   = 0;
    wcex.lpszClassName  = wkeWebViewClassName;
    wcex.hIconSm        = 0;

    return !!RegisterClassEx(&wcex);
}

#define MAX_URL_LENGTH  1024

LRESULT CALLBACK UrlEditProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_CHAR && wParam == 13) //Enter Key
    {
        wchar_t url[MAX_URL_LENGTH];
        *((LPWORD)url) = MAX_URL_LENGTH; 
        int len = SendMessage(hDlg, EM_GETLINE, 0, (LPARAM)url);
        if (len == 0)
            return 0;

        url[len] = L'\0';
        g_webView->loadURL(url);
        SetFocus(hViewWindow);
        return 0;
    }
    
    return (LRESULT)CallWindowProc((WNDPROC)DefEditProc,hDlg,message,wParam,lParam);
}
