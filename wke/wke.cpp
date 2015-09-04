
#include <WebCore/config.h>

#include <JavaScriptCore/InitializeThreading.h>
#include <JavaScriptCore/MainThread.h>
#include <WebCore/Page.h>
#include <WebCore/PageGroup.h>
#include <WebCore/IconDatabase.h>
#include <WebCore/WebCoreInstanceHandle.h>
#include <WebCore/RenderThemeWin.h>
#include <WebCore/ResourceHandleManager.h>
#include <WebCore/Console.h>
#include <WebCore/SecurityOrigin.h>

#include "wkePlatformStrategies.inl"

#include "wke.h"
#include "icuwin.h"
#include "stringTable.h"

void wkeInitialize()
{
    //double-precision float
    _controlfp(_PC_53, _MCW_PC);

    CoInitialize(NULL);

    icuwin_init();
    initStringTable();

    JSC::initializeThreading();
    WTF::initializeMainThread();
    wke::PlatformStrategies::initialize();

    //cexer 解决不能加载本地图片的BUG。
    WebCore::SecurityOrigin::setLocalLoadPolicy(WebCore::SecurityOrigin::AllowLocalLoadsForAll);

    //WebCore::Console::setShouldPrintExceptions(true);
    //WebCore::ResourceHandleManager::sharedInstance()->setCookieJarFileName("cookie.txt");
}

void wkeFinalize()
{
    wkeUpdate();

    WebCore::iconDatabase().close();
    WebCore::PageGroup::closeLocalStorage();

    destroyStringTable();

    CoUninitialize();
}

void wkeUpdate()
{
    static HWND hTimer = NULL;
    if (!hTimer)
        hTimer = FindWindow(L"TimerWindowClass", NULL);

    if (hTimer)
    {
        MSG msg;
        while(PeekMessage(&msg, hTimer, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}


#define MAJOR_VERSION   (1)
#define MINOR_VERSION   (2)
#define WEBKIT_BUILD    (98096)

unsigned int wkeGetVersion()
{
    return (MAJOR_VERSION << 8) + MINOR_VERSION;
}

const utf8* wkeGetVersionString()
{
    static utf8 s_versionString[128];
    sprintf(s_versionString, "wke version %d.%02d\n"
                             "webkit build %d\n"
                             "build time %s\n",
                             MAJOR_VERSION, MINOR_VERSION,
                             WEBKIT_BUILD,
                             __TIMESTAMP__);

    return s_versionString;
}


extern "C" void libcurl_set_file_system(FILE_OPEN pfn_open, FILE_CLOSE pfn_close, FILE_SIZE pfn_size, FILE_READ pfn_read, FILE_SEEK  pfn_seek);
void wkeSetFileSystem(FILE_OPEN pfn_open, FILE_CLOSE pfn_close, FILE_SIZE pfn_size, FILE_READ pfn_read, FILE_SEEK pfn_seek)
{
    libcurl_set_file_system(pfn_open, pfn_close, pfn_size, pfn_read, pfn_seek);
}

const char* wkeGetName(wkeWebView webView)
{
    return webView->name();
}

void wkeSetName(wkeWebView webView, const char* name)
{
    webView->setName(name);
}

bool wkeIsTransparent(wkeWebView webView)
{
    return webView->isTransparent();
}

void wkeSetTransparent(wkeWebView webView, bool transparent)
{
    webView->setTransparent(transparent);
}

void wkePostURL(wkeWebView wkeView,const utf8 * url,const char *szPostData,int nLen)
{
	wkeView->loadPostURL(url,szPostData,nLen);
}
void wkePostURLW(wkeWebView wkeView,const wchar_t * url,const char *szPostData,int nLen)
{
    wkeView->loadPostURL(url,szPostData,nLen);
}
void wkeLoadURL(wkeWebView webView, const utf8* url)
{
    webView->loadURL(url);
}

void wkeLoadURLW(wkeWebView webView, const wchar_t* url)
{
    webView->loadURL(url);
}

void wkeLoadHTML(wkeWebView webView, const utf8* html)
{
    webView->loadHTML(html);
}

void wkeLoadHTMLW(wkeWebView webView, const wchar_t* html)
{
    webView->loadHTML(html);
}

void wkeLoadFile(wkeWebView webView, const utf8* filename)
{
    return webView->loadFile(filename);
}

void wkeLoadFileW(wkeWebView webView, const wchar_t* filename)
{
    return webView->loadFile(filename);
}

bool wkeIsLoadingSucceeded(wkeWebView webView)
{
    return webView->isLoadingSucceeded();
}

bool wkeIsLoadingFailed(wkeWebView webView)
{
    return webView->isLoadingFailed();
}

bool wkeIsLoadingCompleted(wkeWebView webView)
{
    return webView->isLoadingCompleted();
}

bool wkeIsDocumentReady(wkeWebView webView)
{
    return webView->isDocumentReady();
}

void wkeStopLoading(wkeWebView webView)
{
    webView->stopLoading();
}

void wkeReload(wkeWebView webView)
{
    webView->reload();
}

const utf8* wkeGetTitle(wkeWebView webView)
{
    return webView->title();
}

const wchar_t* wkeGetTitleW(wkeWebView webView)
{
    return webView->titleW();
}

void wkeResize(wkeWebView webView, int w, int h)
{
    webView->resize(w, h);
}

int wkeGetWidth(wkeWebView webView)
{
    return webView->width();
}

int wkeGetHeight(wkeWebView webView)
{
    return webView->height();
}

int wkeGetContentWidth(wkeWebView webView)
{
    return webView->contentWidth();
}

int wkeGetContentHeight(wkeWebView webView)
{
    return webView->contentHeight();
}

void wkeSetDirty(wkeWebView webView, bool dirty)
{
    webView->setDirty(dirty);
}

bool wkeIsDirty(wkeWebView webView)
{
    return webView->isDirty();
}

void wkeAddDirtyArea(wkeWebView webView, int x, int y, int w, int h)
{
    webView->addDirtyArea(x, y, w, h);
}

void wkeLayoutIfNeeded(wkeWebView webView)
{
    webView->layoutIfNeeded();
}

void wkePaint(wkeWebView webView,void* bits, int bufWid, int bufHei, int xDst, int yDst, int w, int h, int xSrc, int ySrc, bool bCopyAlpha)
{
    webView->paint(bits, bufWid,  bufHei,  xDst,  yDst,  w,  h,  xSrc,  ySrc, bCopyAlpha);
}

void wkePaint2(wkeWebView webView, void* bits,int pitch)
{
    webView->paint(bits, pitch);
}

bool wkeCanGoBack(wkeWebView webView)
{
    return webView->canGoBack();
}

bool wkeGoBack(wkeWebView webView)
{
    return webView->goBack();
}

bool wkeCanGoForward(wkeWebView webView)
{
    return webView->canGoForward();
}

bool wkeGoForward(wkeWebView webView)
{
    return webView->goForward();
}

void wkeEditorSelectAll(wkeWebView webView)
{
    webView->editorSelectAll();
}

void wkeEditorCopy(wkeWebView webView)
{
    webView->editorCopy();
}

void wkeEditorCut(wkeWebView webView)
{
    webView->editorCut();
}

void wkeEditorPaste(wkeWebView webView)
{
    webView->editorPaste();
}

void wkeEditorDelete(wkeWebView webView)
{
    webView->editorDelete();
}

const wchar_t * wkeGetCookieW(wkeWebView webView)
{
    return webView->cookieW();
}

const utf8* wkeGetCookie(wkeWebView webView)
{
    return webView->cookie();
}


void wkeSetCookieEnabled(wkeWebView webView, bool enable)
{
    webView->setCookieEnabled(enable);
}

bool wkeIsCookieEnabled(wkeWebView webView)
{
    return webView->isCookieEnabled();
}

void wkeSetMediaVolume(wkeWebView webView, float volume)
{
    webView->setMediaVolume(volume);
}

float wkeGetMediaVolume(wkeWebView webView)
{
    return webView->mediaVolume();
}

bool wkeFireMouseEvent(wkeWebView webView, unsigned int message, int x, int y, unsigned int flags)
{
    return webView->fireMouseEvent(message, x, y, flags);
}

bool wkeFireContextMenuEvent(wkeWebView webView, int x, int y, unsigned int flags)
{
    return webView->fireContextMenuEvent(x, y, flags);
}

bool wkeFireMouseWheelEvent(wkeWebView webView, int x, int y, int delta, unsigned int flags)
{
    return webView->fireMouseWheelEvent(x, y, delta, flags);
}

bool wkeFireKeyUpEvent(wkeWebView webView, unsigned int virtualKeyCode, unsigned int flags, bool systemKey)
{
    return webView->fireKeyUpEvent(virtualKeyCode, flags, systemKey);
}

bool wkeFireKeyDownEvent(wkeWebView webView, unsigned int virtualKeyCode, unsigned int flags, bool systemKey)
{
    return webView->fireKeyDownEvent(virtualKeyCode, flags, systemKey);
}

bool wkeFireKeyPressEvent(wkeWebView webView, unsigned int charCode, unsigned int flags, bool systemKey)
{
    return webView->fireKeyPressEvent(charCode, flags, systemKey);
}

void wkeSetFocus(wkeWebView webView)
{
    webView->setFocus();
}

void wkeKillFocus(wkeWebView webView)
{
    webView->killFocus();
}

wkeRect wkeGetCaret(wkeWebView webView)
{
    return webView->caretRect();
}

jsValue wkeRunJS(wkeWebView webView, const utf8* script)
{
    return webView->runJS(script);
}

jsValue wkeRunJSW(wkeWebView webView, const wchar_t* script)
{
    return webView->runJS(script);
}

jsExecState wkeGlobalExec(wkeWebView webView)
{
    return webView->globalExec();
}

void wkeSleep(wkeWebView webView)
{
    webView->sleep();
}

void wkeWake(wkeWebView webView)
{
    webView->wake();
}

bool wkeIsAwake(wkeWebView webView)
{
    return webView->isAwake();
}

void wkeSetZoomFactor(wkeWebView webView, float factor)
{
    webView->setZoomFactor(factor);
}

float wkeGetZoomFactor(wkeWebView webView)
{
    return webView->zoomFactor();
}

void wkeSetEditable(wkeWebView webView, bool editable)
{
    webView->setEditable(editable);
}

WKE_API void wkeSetHandler(wkeWebView webView, wkeViewHandler* handler)
{
    webView->setHandler(handler);
}

WKE_API const wkeViewHandler* wkeGetHandler(wkeWebView webView)
{
    return webView->handler();
}

WKE_API const utf8* wkeToString(const wkeString string)
{
    const String* str = (const String*)string;
    return StringTable::addString(str->characters(), str->length());
}

WKE_API const wchar_t* wkeToStringW(const wkeString string)
{
    const String* str = (const String*)string;
    return StringTableW::addString(str->characters(), str->length());
}




//FIXME: We should consider moving this to a new file for cross-project functionality
PassRefPtr<WebCore::SharedBuffer> loadResourceIntoBuffer(const char* name)
{
    return 0;
}

extern void __CFInitialize(void);

void init_libs()
{
    _putenv("WEBKIT_IGNORE_SSL_ERRORS=1");
    pthread_win32_process_attach_np ();
    __CFInitialize();
}

typedef void (__cdecl* _PVFV) ();
#pragma section(".CRT$XCG", long, read)
__declspec(allocate(".CRT$XCG")) _PVFV init_section[] = { init_libs };


extern "C" BOOL WINAPI CoreFoundationDllMain( HINSTANCE hInstance, DWORD dwReason, LPVOID pReserved );
STDAPI_(BOOL) DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID /*lpReserved*/)
{
    BOOL ret = FALSE;
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            WebCore::setInstanceHandle(hModule);
            ret = TRUE;
            break;

        case DLL_PROCESS_DETACH:
            WebCore::RenderThemeWin::setWebKitIsBeingUnloaded();
            pthread_win32_thread_detach_np ();
            break;

        case DLL_THREAD_ATTACH:
            pthread_win32_thread_attach_np ();
            break;

        case DLL_THREAD_DETACH:
            pthread_win32_thread_detach_np ();
            break;
    }

    CoreFoundationDllMain(hModule, ul_reason_for_call, 0);
    return ret;
}