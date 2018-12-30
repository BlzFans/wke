
#define WIN32_LEAN_AND_MEAN
#if !defined(_WIN32_WINNT) || (_WIN32_WINNT < 0x501)
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x501
#endif

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <time.h>
#include <stdlib.h>
#include <ShellAPI.h>

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

wkeWebView* g_webView = NULL;
CRender* g_render = NULL;

ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

WNDPROC DefEditProc = NULL;
LRESULT CALLBACK UrlEditProc(HWND, UINT, WPARAM, LPARAM);

void resizeSubViews();
LRESULT CALLBACK WebViewWndProc(HWND, UINT, WPARAM, LPARAM);
const LPCWSTR wkeWebViewClassName = L"wkeWebView*";
bool registerWebViewWindowClass();

wkeJSValue JS_CALL js_msgBox(wkeJSState* es)
{
    wchar_t text[1025] = { 0 };
    wcsncpy(text, wkeJSToTempStringW(es, wkeJSParam(es, 0)), 1024);

    wchar_t title[1025] = { 0 };
    wcsncpy(title, wkeJSToTempStringW(es, wkeJSParam(es, 1)), 1024);

    MessageBoxW(hMainWnd, text, title, MB_OK);

    return wkeJSUndefined(es);
}

static int s_testCount = 0;
wkeJSValue JS_CALL js_getTestCount(wkeJSState* es)
{
    return wkeJSInt(es, s_testCount);
}

wkeJSValue JS_CALL js_setTestCount(wkeJSState* es)
{
    s_testCount = wkeJSToInt(es, wkeJSParam(es, 0));

    return wkeJSUndefined(es);
}

void onTitleChanged(wkeWebView* webView, void* param, const wkeString* title)
{
    SetWindowTextW(hMainWnd, wkeGetStringW(title));
}

void onURLChanged(wkeWebView* webView, void* param, const wkeString* url)
{
    SetWindowTextW(hURLBarWnd, wkeGetStringW(url));
}

bool onNavigation(wkeWebView* webView, void* param, wkeNavigationType type, const wkeString* url_)
{
    const wchar_t* url = wkeGetStringW(url_);
    //if (wcsstr(url, L"baidu.com") != NULL)
    //    return false;

    if (wcsstr(url, L"exec://") == url)
    {
        PROCESS_INFORMATION processInfo = { 0 };
        STARTUPINFOW startupInfo = { 0 };
        startupInfo.cb = sizeof(startupInfo);
        BOOL succeeded = CreateProcessW(NULL, (LPWSTR)url + 7, NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo);
        if (succeeded)
        {
            CloseHandle(processInfo.hProcess);
            CloseHandle(processInfo.hThread);
        }
        return false;
    }

    return true;
}

wkeWebView* onCreateView(wkeWebView* webView, void* param, const wkeNewViewInfo* info)
{
    const wchar_t* target = wkeGetStringW(info->target);
    const wchar_t* url = wkeGetStringW(info->url);

    if (wcscmp(target, L"") == 0 || wcscmp(target, L"_blank") == 0)
    {
        if (wcsnicmp(url, L"file:///", 8) == 0)
            url += 8;
        ShellExecuteW(NULL, L"open", (LPWSTR)url, NULL, NULL, SW_SHOW);
        return NULL;
    }
    else if (wcscmp(target, L"_self") == 0)
    {
        return webView;
    }
    else if (wcscmp(target, L"wontOpen") == 0)
    {
        return NULL;
    }
    else
    {
        wkeWebView* newWindow = wkeCreateWebWindow(WKE_WINDOW_TYPE_POPUP, NULL, info->x, info->y, info->width, info->height);
        wkeShowWindow(newWindow, SW_SHOW);
        return newWindow;
    }

    return NULL;
}

const wchar_t* messageSourceName(int i)
{
    const wchar_t* s_names[] = {
        L"html",
        L"xml",
        L"js",
        L"network",
        L"console",
        L"other"
    };
    return s_names[i];
}

const wchar_t* messageTypeName(int i)
{
    const wchar_t* s_names[] = {
        L"log",
        L"dir",
        L"dir-xml",
        L"trace",
        L"start-group",
        L"start-group-collapsed",
        L"end-group",
        L"assert"
    };
    return s_names[i];
}

const wchar_t* messageLevelName(int i)
{
    const wchar_t* s_names[] = {
        L"tip",
        L"log",
        L"warning",
        L"error",
        L"debug"
    };
    return s_names[i];
}

FILE* g_consoleLog = NULL;
void onConsoleMessage(wkeWebView* webView, void* param, const wkeConsoleMessage* message)
{
    wchar_t msg[1024 * 10 + 1] = { 0 };
    int len = swprintf(msg, 1024*10, L"[%s:%s:%s] %s in %s(%d)",
        messageSourceName(message->source),
        messageTypeName(message->type),
        messageLevelName(message->level),
        wkeGetStringW(message->message),
        wkeGetStringW(message->url), (int)message->lineNumber);
    if (len < 0)
        len = 1024 * 10;
    msg[len] = 0;

    if (g_consoleLog)
        fputws(msg, g_consoleLog);

    OutputDebugStringW(msg);
}

/*
## ���԰󶨶�����
����wkeWebBrowser.exe���ڵ�ַ������`inject`�س�������ע��JS����`test`��ע����JS�пɷ���`test`����ĳ�Ա����`value`�ͳ�Ա����`msgbox`�ˡ�
�ڵ�ַ������`javascript:test.msgbox('1')`���Ե��ó�Ա������
�ڵ�ַ������`javascript:document.write(test.value)`���Է��ʳ�Ա������
*/
struct BindTestFunction
{
    static wkeJSValue js_callAsFunction(wkeJSState* es, wkeJSValue object, wkeJSValue* args, int argCount)
    {
        wchar_t text[1025] = { 0 };
        wchar_t title[1025] = { 0 };

        if (argCount >= 1)
            wcsncpy(text, wkeJSToTempStringW(es, wkeJSParam(es, 0)), 1024);
        if (argCount >= 2)
            wcsncpy(title, wkeJSToTempStringW(es, wkeJSParam(es, 1)), 1024);

        MessageBoxW(NULL, text, title[0] ? title : NULL, MB_OK);
        return wkeJSInt(es, 0);
    }

    static void js_releaseFunction(wkeJSData* data)
    {
        delete data;
    }

    static void bindToGlobal(wkeJSState* es)
    {
        wkeJSData* data = new wkeJSData();
        memset(data, 0, sizeof(wkeJSData));
        strcpy(data->typeName, "Function");
        data->callAsFunction = js_callAsFunction;
        data->finalize = js_releaseFunction;

        wkeJSValue func = wkeJSFunction(es, data);
        wkeJSSetGlobal(es, "popup", func);
    }

    static void bindToObject(wkeJSState* es, wkeJSValue obj)
    {
        wkeJSData* data = new wkeJSData();
        memset(data, 0, sizeof(wkeJSData));
        strcpy(data->typeName, "Function");
        data->callAsFunction = js_callAsFunction;
        data->finalize = js_releaseFunction;

        wkeJSValue func = wkeJSFunction(es, data);
        wkeJSSet(es, obj, "popup", func);
    }
};


class BindTestObject : public wkeJSData
{
public:
	BindTestObject()
		: m_value(0)
	{
		wkeJSData* data = this;
        memset(data, 0, sizeof(wkeJSData));
        strcpy(data->typeName, "Object");
        data->propertyGet = js_getObjectProp;
        data->propertySet = js_setObjectProp;
        data->finalize = js_releaseObject;
	}

	void msgbox(const wchar_t* msg, const wchar_t* title)
	{
		MessageBoxW(NULL, msg, title, MB_OK);
	}

protected:
	static bool js_setObjectProp(wkeJSState* es, wkeJSValue object, const char* propertyName, wkeJSValue value)
	{
		BindTestObject* pthis = (BindTestObject*)wkeJSGetData(es, object);
		if (strcmp(propertyName, "value") == 0)
			return pthis->m_value = wkeJSToInt(es, value), true;
		else
			return false;
	}

	static void js_releaseObject(wkeJSData* data)
	{
		BindTestObject* pthis = (BindTestObject*)data;
		delete pthis;
	}

	class BindTestMsgbox : public wkeJSData
	{
	public:
		BindTestMsgbox(BindTestObject* obj)
		{
			wkeJSData* data = this;
			memset(data, 0, sizeof(wkeJSData));
			strcpy(data->typeName, "Function");
			data->callAsFunction = js_callAsFunction;
			data->finalize = js_releaseFunction;

			m_obj = obj;
		}

	protected:
		static void js_releaseFunction(wkeJSData* data)
		{
			BindTestMsgbox* pthis = (BindTestMsgbox*)data;
			delete pthis;
		}

		static wkeJSValue js_callAsFunction(wkeJSState* es, wkeJSValue object, wkeJSValue* args, int argCount)
		{
			BindTestMsgbox* pthis = (BindTestMsgbox*)wkeJSGetData(es, object);
    
            wchar_t text[1025] = { 0 };
            wchar_t title[1025] = { 0 };
            
			if (argCount >= 1)
				wcsncpy(text, wkeJSToTempStringW(es, wkeJSParam(es, 0)), 1024);
			if (argCount >= 2)
				wcsncpy(title, wkeJSToTempStringW(es, wkeJSParam(es, 1)), 1024);

            pthis->m_obj->msgbox(text, title[0] ? title : NULL);
			return wkeJSInt(es, 0);
		}

	protected:
		BindTestObject* m_obj;
	};

	static wkeJSValue js_getObjectProp(wkeJSState* es, wkeJSValue object, const char* propertyName)
	{
		BindTestObject* pthis = (BindTestObject*)wkeJSGetData(es, object);
		if (strcmp(propertyName, "value") == 0)
			return wkeJSInt(es, pthis->m_value);

		else if (strcmp(propertyName, "msgbox") == 0)
		{
			return wkeJSFunction(es, new BindTestMsgbox(pthis));
		}
		else
			return wkeJSUndefined(es);
	}

protected:
	int m_value;
};


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
    wkeInitialize();
    t1.Start();

    wkeSettings settings;
    memset(&settings, 0, sizeof(settings));

#if defined(WKE_BROWSER_USE_LOCAL_PROXY)
    settings.proxy.type = WKE_PROXY_SOCKS5;
    strcpy(settings.proxy.hostname, "127.0.0.1");
    settings.proxy.port = 1080;
    settings.mask |= WKE_SETTING_PROXY;
#endif
    //wkeInitializeEx(&settings);
    wkeConfigure(&settings);

    t1.End();

	//jsBindObject("testObj", js_getObjectProp, js_setObjectProp);
    wkeJSBindFunction("msgBox", js_msgBox, 2);
    wkeJSBindGetter("testCount", js_getTestCount);
    wkeJSBindSetter("testCount", js_setTestCount);

    t2.Start();
    g_webView = wkeCreateWebView();
    t2.End();


    t3.Start();
    wkeSetTransparent(g_webView, false);
    wkeOnTitleChanged(g_webView, onTitleChanged, NULL);
    wkeOnURLChanged(g_webView, onURLChanged, NULL);
    wkeOnNavigation(g_webView, onNavigation, NULL);
    wkeOnCreateView(g_webView, onCreateView, NULL);

    if (g_consoleLog = fopen("wkeBrowserConsole.txt", "wb"))
        fwrite("\xFF\xFE", 2, 1, g_consoleLog);

    wkeOnConsoleMessage(g_webView, onConsoleMessage, NULL);
    //wkeLoadUrl("file:///test/test.html");

    //����UserAgent
    wkeSetUserAgent(g_webView, "Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/41.0.2228.0 Safari/537.36");

    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argc > 1)
        wkeLoadW(g_webView, argv[1]);
    else
        wkeLoadHTMLW(g_webView, L"<p style=\"background-color: #00FF00\">Testing</p><img id=\"webkit logo\" src=\"http://webkit.org/images/icon-gold.png\" alt=\"Face\"><div style=\"border: solid blue; background: white;\" contenteditable=\"true\">div with blue border</div><ul><li>foo<li>bar<li>baz</ul>");
    LocalFree(argv);
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

    //�д��ھ�������������
    wkeSetHostWindow(g_webView, hViewWindow);

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
        else if (!IsIconic(hMainWnd) && wkeIsDirty(g_webView))
        {
            g_render->render(g_webView);
        }
        else
        {
            Sleep(10);
        }
    }

    g_render->destroy();
    wkeDestroyWebView(g_webView);
    wkeFinalize();

    if (g_consoleLog)
        fclose(g_consoleLog);

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

    wkeSetZoomFactor(g_webView, s_currentZoom / 100.f);
}

void resetZoom()
{
    s_currentZoom = 100;
    if (g_webView)
        wkeSetZoomFactor(g_webView, s_currentZoom / 100.f);
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

    wkeRunJS(g_webView, "document.body.style.overflow='hidden'");	
	int w = wkeGetContentWidth(g_webView);
	int h = wkeGetContentHeight(g_webView);
    
    int oldwidth = wkeGetWidth(g_webView);
    int oldheight = wkeGetHeight(g_webView);
    wkeResize(g_webView, w, h);
    wkeUpdate();

    void* pixels = malloc(w*h*4);
    wkePaint2(g_webView, pixels, 0);

    //save bitmap
    saveBitmap(pixels, w, h, wkeGetTitleW(g_webView));

    free(pixels);

    wkeResize(g_webView, oldwidth, oldheight);
    wkeRunJS(g_webView, "document.body.style.overflow='visible'");
}

void viewCookie()
{
    if (g_webView == NULL)
        return;

    const wchar_t* cookie = wkeGetCookieW(g_webView);
    MessageBoxW(NULL, cookie, L"Cookie", MB_OK|MB_ICONINFORMATION);
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
                wkeGoBack(g_webView);
            break;

        case ID_FILE_GOFORWARD:
            if (g_webView)
                wkeGoForward(g_webView);
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

        case ID_TOOLS_VIEWCOOKIE:
            viewCookie();
            break;

        case ID_SET_EDITABLE:
            if (g_webView)
            {
                HMENU hMenu = GetMenu(hWnd);
                UINT state = GetMenuState(hMenu, ID_SET_EDITABLE, MF_BYCOMMAND);
                if (state & MF_CHECKED)
                {
                    wkeSetEditable(g_webView, false);
                    CheckMenuItem(hMenu, ID_SET_EDITABLE, MF_BYCOMMAND | MF_UNCHECKED); 
                }
                else
                {
                    wkeSetEditable(g_webView, true);
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

            if (g_webView && wkeCanGoBack(g_webView))
                canGoBack = true;

            if (g_webView && wkeCanGoForward(g_webView))
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
    //cexer �����Ϸż���
    case WM_CREATE:
        {
            DragAcceptFiles(hWnd, TRUE);
        }
        return 0;

    case WM_DROPFILES:
        {
            if (g_webView)
            {
                wchar_t szFile[MAX_PATH + 8] = {0};
                wcscpy(szFile, L"file:///");

                HDROP hDrop = reinterpret_cast<HDROP>(wParam);

                UINT uFilesCount = DragQueryFileW(hDrop, 0xFFFFFFFF, szFile, MAX_PATH);
                if (uFilesCount != 0)
                {
                    UINT uRet = DragQueryFileW(hDrop, 0, (wchar_t*)szFile + 8, MAX_PATH);
                    if ( uRet != 0)
                    {
                        wkeLoadURLW(g_webView, szFile);
                        SetWindowTextW(hWnd, szFile);
                    }
                }
                DragFinish(hDrop);
            }
        }
        return 0;

    case WM_COMMAND:
        SendMessage(GetParent(hWnd), message, wParam, lParam);
        return 0;

    case WM_SIZE:
        if (g_webView && g_render)
        {
            wkeResize(g_webView, LOWORD(lParam), HIWORD(lParam));
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

            handled = wkeFireKeyDownEvent(g_webView, virtualKeyCode, flags, false);
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

            handled = wkeFireKeyUpEvent(g_webView, virtualKeyCode, flags, false);
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

            handled = wkeFireKeyPressEvent(g_webView, charCode, flags, false);
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

            handled = wkeFireMouseEvent(g_webView, message, x, y, flags);
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

            handled = wkeFireContextMenuEvent(g_webView, pt.x, pt.y, flags);
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

            handled = wkeFireMouseWheelEvent(g_webView, pt.x, pt.y, delta, flags);
        }
        break;

    case WM_SETFOCUS:
        wkeSetFocus(g_webView);
        break;

    case WM_KILLFOCUS:
        wkeKillFocus(g_webView);
        break;

    case WM_IME_STARTCOMPOSITION:
        {
            wkeRect caret = wkeGetCaretRect(g_webView);

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

        wkeResize(g_webView, rcClient.right, rcClient.bottom - URLBAR_HEIGHT);
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
		wchar_t url[MAX_URL_LENGTH] = {0};
        *((LPWORD)url) = MAX_URL_LENGTH; 
        int len = SendMessage(hDlg, EM_GETLINE, 0, (LPARAM)url);
        if (len == 0)
            return 0;

		if (wcsstr(url, L"inject") == url)
		{
			//jsObjectData* data = new jsObjectData();
   //         memset(data, 0, sizeof(jsObjectData));
   //         strcpy(data->typeName, "Test");
   //         data->propertyGet = js_getObjectProp;
   //         data->propertySet = js_setObjectProp;
   //         data->finalize = js_releaseObject;

			BindTestObject* testObj = new BindTestObject();
            wkeJSState* es = wkeGlobalExec(g_webView);
            wkeJSValue obj = wkeJSObject(es, testObj);
            wkeJSSetGlobal(es, "test", obj);

            BindTestFunction::bindToGlobal(es);
		}
		else if (wcsstr(url, L"javascript:") == url)
		{
			url[len] = L'\0';
			wkeRunJSW(g_webView, url + wcslen(L"javascript:"));
		}
		else if (wcsstr(url, L"call") == url)
		{
			wkeJSState* es = wkeGlobalExec(g_webView);
			wkeJSValue jsDocument = wkeJSGet(es, wkeJSGlobalObject(es), "document");

			{
				char prop[10] = { 0 };
				strcpy(prop, "URL");
				wkeJSValue jsUrl = wkeJSGet(es, jsDocument, prop);
				MessageBoxW(NULL, wkeJSToTempStringW(es, jsUrl), NULL, MB_OK);
			}

			{
				char prop[10] = { 0 };
				strcpy(prop, "title");
				wkeJSValue jsTitle = wkeJSGet(es, jsDocument, prop);
				MessageBoxW(NULL, wkeJSToTempStringW(es, jsTitle), NULL, MB_OK);
			}

			{
				char prop[10] = { 0 };
				strcpy(prop, "cookie");
				wkeJSValue jsCookie = wkeJSGet(es, jsDocument, prop);
				MessageBoxW(NULL, wkeJSToTempStringW(es, jsCookie), NULL, MB_OK);
			}

		}
		else
		{
			url[len] = L'\0';
			wkeLoadURLW(g_webView, url);
			SetFocus(hViewWindow);
		}
        return 0;
    }
    
    return (LRESULT)CallWindowProc((WNDPROC)DefEditProc,hDlg,message,wParam,lParam);
}

