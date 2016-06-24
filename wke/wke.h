/*
 * 
 * BlzFans@hotmail.com
 * http://wke.sf.net
 * http://www.github.com/BlzFans/wke
 * licence LGPL
 *
 */

#ifndef WKE_H
#define WKE_H


//////////////////////////////////////////////////////////////////////////


#if defined(__cplusplus)
    #define WKE_EXTERN_C extern "C"
#else
    #define WKE_EXTERN_C
#endif


#if defined(BUILDING_wke)
#   define WKE_SYMBOL __declspec(dllexport)
#else
#   define WKE_SYMBOL __declspec(dllimport)
#endif


#define WKE_CALL _cdecl
#define WKE_API  WKE_EXTERN_C WKE_SYMBOL




typedef struct 
{
    int x;
    int y;
    int w;
    int h;

} wkeRect;


typedef enum
{
    WKE_LBUTTON = 0x01,
    WKE_RBUTTON = 0x02,
    WKE_SHIFT   = 0x04,
    WKE_CONTROL = 0x08,
    WKE_MBUTTON = 0x10,

} wkeMouseFlags;


typedef enum
{
    WKE_EXTENDED = 0x0100,
    WKE_REPEAT = 0x4000,

} wkeKeyFlags;


typedef enum
{
    WKE_MSG_MOUSEMOVE       =  0x0200,
    WKE_MSG_LBUTTONDOWN     =  0x0201,
    WKE_MSG_LBUTTONUP       =  0x0202,
    WKE_MSG_LBUTTONDBLCLK   =  0x0203,
    WKE_MSG_RBUTTONDOWN     =  0x0204,
    WKE_MSG_RBUTTONUP       =  0x0205,
    WKE_MSG_RBUTTONDBLCLK   =  0x0206,
    WKE_MSG_MBUTTONDOWN     =  0x0207,
    WKE_MSG_MBUTTONUP       =  0x0208,
    WKE_MSG_MBUTTONDBLCLK   =  0x0209,
    WKE_MSG_MOUSEWHEEL      =  0x020A,

} wkeMouseMsg;



#if !defined(__cplusplus)
    #ifndef HAVE_WCHAR_T
        typedef unsigned short wchar_t;
    #endif

    #ifndef HAVE_BOOL
        typedef unsigned char bool;
        #define true 1
        #define false 0
    #endif
#endif


typedef char utf8;
typedef struct _tagWkeJSState wkeJSState;
typedef __int64 wkeJSValue;


#if defined(__cplusplus)
    namespace wke{ class CWebView; };
    typedef wke::CWebView wkeWebView;

    namespace wke{ class CString; };
    typedef wke::CString wkeString;

#else
    typedef struct __wkeWebView wkeWebView;
    typedef struct __wkeString wkeString;
#endif



typedef enum
{
    WKE_PROXY_NONE,
    WKE_PROXY_HTTP,
    WKE_PROXY_SOCKS4,
    WKE_PROXY_SOCKS4A,
    WKE_PROXY_SOCKS5,
    WKE_PROXY_SOCKS5HOSTNAME

} wkeProxyType;

typedef struct 
{
   wkeProxyType type;
   char hostname[100];
   unsigned short port;
   char username[50];
   char password[50];

} wkeProxy;

enum wkeSettingMask 
{
    WKE_SETTING_PROXY = 1,
    WKE_SETTING_COOKIE_FILE_PATH = 1<<1
};

typedef struct 
{
    wkeProxy proxy;
    char cookieFilePath[1024];
    unsigned int mask;

} wkeSettings;


/*
 *c interface
 *----------------------------------------------------------------------------------------------------------
 *
 */

#if defined(__cplusplus)
extern "C"
{
#endif

WKE_API void        WKE_CALL wkeInitialize();
WKE_API void        WKE_CALL wkeInitializeEx(const wkeSettings* settings);
WKE_API void        WKE_CALL wkeConfigure(const wkeSettings* settings);

WKE_API void        WKE_CALL wkeFinalize();
WKE_API void        WKE_CALL wkeUpdate();
WKE_API unsigned int WKE_CALL wkeGetVersion();
WKE_API const utf8* WKE_CALL wkeGetVersionString();

typedef void*     (WKE_CALL *FILE_OPEN) (const char* path);
typedef void      (WKE_CALL *FILE_CLOSE) (void* handle);
typedef size_t     (WKE_CALL *FILE_SIZE) (void* handle);
typedef int       (WKE_CALL *FILE_READ) (void* handle, void* buffer, size_t size);
typedef int       (WKE_CALL *FILE_SEEK) (void* handle, int offset, int origin);
WKE_API void        WKE_CALL wkeSetFileSystem(FILE_OPEN pfn_open, FILE_CLOSE pfn_close, FILE_SIZE pfn_size, FILE_READ pfn_read, FILE_SEEK pfn_seek);


WKE_API wkeWebView*  WKE_CALL wkeCreateWebView();
WKE_API wkeWebView*  WKE_CALL wkeGetWebView(const char* name);
WKE_API void        WKE_CALL wkeDestroyWebView(wkeWebView* webView);

WKE_API const char* WKE_CALL wkeGetName(wkeWebView* webView);
WKE_API void        WKE_CALL wkeSetName(wkeWebView* webView, const char* name);

WKE_API bool        WKE_CALL wkeIsTransparent(wkeWebView* webView);
WKE_API void        WKE_CALL wkeSetTransparent(wkeWebView* webView, bool transparent);

WKE_API void        WKE_CALL wkeSetUserAgent(wkeWebView* webView, const utf8* userAgent);
WKE_API void        WKE_CALL wkeSetUserAgentW(wkeWebView* webView, const wchar_t* userAgent);

WKE_API void        WKE_CALL wkeLoadURL(wkeWebView* webView, const utf8* url);
WKE_API void        WKE_CALL wkeLoadURLW(wkeWebView* webView, const wchar_t* url);
WKE_API void        WKE_CALL wkePostURL(wkeWebView* wkeView,const utf8* url, const char* postData,int  postLen); 
WKE_API void        WKE_CALL wkePostURLW(wkeWebView* wkeView,const wchar_t* url, const char* postData, int postLen);

WKE_API void        WKE_CALL wkeLoadHTML(wkeWebView* webView, const utf8* html);
WKE_API void        WKE_CALL wkeLoadHTMLW(wkeWebView* webView, const wchar_t* html);

WKE_API void        WKE_CALL wkeLoadFile(wkeWebView* webView, const utf8* filename);
WKE_API void        WKE_CALL wkeLoadFileW(wkeWebView* webView, const wchar_t* filename);

WKE_API void        WKE_CALL wkeLoad(wkeWebView* webView, const utf8* str);
WKE_API void        WKE_CALL wkeLoadW(wkeWebView* webView, const wchar_t* str);

WKE_API bool        WKE_CALL wkeIsLoading(wkeWebView* webView);
WKE_API bool        WKE_CALL wkeIsLoadingSucceeded(wkeWebView* webView);
WKE_API bool        WKE_CALL wkeIsLoadingFailed(wkeWebView* webView);
WKE_API bool        WKE_CALL wkeIsLoadingCompleted(wkeWebView* webView);
WKE_API bool        WKE_CALL wkeIsDocumentReady(wkeWebView* webView);
WKE_API void        WKE_CALL wkeStopLoading(wkeWebView* webView);
WKE_API void        WKE_CALL wkeReload(wkeWebView* webView);

WKE_API const utf8* WKE_CALL wkeGetTitle(wkeWebView* webView);
WKE_API const wchar_t* WKE_CALL wkeGetTitleW(wkeWebView* webView);

WKE_API void        WKE_CALL wkeResize(wkeWebView* webView, int w, int h);
WKE_API int         WKE_CALL wkeGetWidth(wkeWebView* webView);
WKE_API int         WKE_CALL wkeGetHeight(wkeWebView* webView);
WKE_API int         WKE_CALL wkeGetContentWidth(wkeWebView* webView);
WKE_API int         WKE_CALL wkeGetContentHeight(wkeWebView* webView);

WKE_API void        WKE_CALL wkeSetDirty(wkeWebView* webView, bool dirty);
WKE_API bool        WKE_CALL wkeIsDirty(wkeWebView* webView);
WKE_API void        WKE_CALL wkeAddDirtyArea(wkeWebView* webView, int x, int y, int w, int h);
WKE_API void        WKE_CALL wkeLayoutIfNeeded(wkeWebView* webView);
WKE_API void        WKE_CALL wkePaint(wkeWebView* webView, void* bits,int bufWid, int bufHei, int xDst, int yDst, int w, int h, int xSrc, int ySrc, bool bCopyAlpha);
WKE_API void        WKE_CALL wkePaint2(wkeWebView* webView, void* bits,int pitch);
WKE_API bool        WKE_CALL wkeRepaintIfNeeded(wkeWebView* webView);
WKE_API void*       WKE_CALL wkeGetViewDC(wkeWebView* webView);

WKE_API void        WKE_CALL wkeSetRepaintInterval(wkeWebView* webView, int ms);
WKE_API int         WKE_CALL wkeGetRepaintInterval(wkeWebView* webView);
WKE_API bool        WKE_CALL wkeRepaintIfNeededAfterInterval(wkeWebView* webView);
WKE_API bool        WKE_CALL wkeRepaintAllNeeded();
WKE_API int         WKE_CALL wkeRunMessageLoop(const bool *quit);

WKE_API bool        WKE_CALL wkeCanGoBack(wkeWebView* webView);
WKE_API bool        WKE_CALL wkeGoBack(wkeWebView* webView);
WKE_API bool        WKE_CALL wkeCanGoForward(wkeWebView* webView);
WKE_API bool        WKE_CALL wkeGoForward(wkeWebView* webView);

WKE_API void        WKE_CALL wkeEditorSelectAll(wkeWebView* webView);
WKE_API void        WKE_CALL wkeEditorCopy(wkeWebView* webView);
WKE_API void        WKE_CALL wkeEditorCut(wkeWebView* webView);
WKE_API void        WKE_CALL wkeEditorPaste(wkeWebView* webView);
WKE_API void        WKE_CALL wkeEditorDelete(wkeWebView* webView);

WKE_API const wchar_t* WKE_CALL wkeGetCookieW(wkeWebView* webView);
WKE_API const utf8* WKE_CALL wkeGetCookie(wkeWebView* webView);
WKE_API void        WKE_CALL wkeSetCookieEnabled(wkeWebView* webView, bool enable);
WKE_API bool        WKE_CALL wkeIsCookieEnabled(wkeWebView* webView);

WKE_API void        WKE_CALL wkeSetMediaVolume(wkeWebView* webView, float volume);
WKE_API float       WKE_CALL wkeGetMediaVolume(wkeWebView* webView);

WKE_API bool        WKE_CALL wkeFireMouseEvent(wkeWebView* webView, unsigned int message, int x, int y, unsigned int flags);
WKE_API bool        WKE_CALL wkeFireContextMenuEvent(wkeWebView* webView, int x, int y, unsigned int flags);
WKE_API bool        WKE_CALL wkeFireMouseWheelEvent(wkeWebView* webView, int x, int y, int delta, unsigned int flags);
WKE_API bool        WKE_CALL wkeFireKeyUpEvent(wkeWebView* webView, unsigned int virtualKeyCode, unsigned int flags, bool systemKey);
WKE_API bool        WKE_CALL wkeFireKeyDownEvent(wkeWebView* webView, unsigned int virtualKeyCode, unsigned int flags, bool systemKey);
WKE_API bool        WKE_CALL wkeFireKeyPressEvent(wkeWebView* webView, unsigned int charCode, unsigned int flags, bool systemKey);

WKE_API void        WKE_CALL wkeSetFocus(wkeWebView* webView);
WKE_API void        WKE_CALL wkeKillFocus(wkeWebView* webView);

WKE_API wkeRect     WKE_CALL wkeGetCaretRect(wkeWebView* webView);

WKE_API wkeJSValue WKE_CALL wkeRunJS(wkeWebView* webView, const utf8* script);
WKE_API wkeJSValue WKE_CALL wkeRunJSW(wkeWebView* webView, const wchar_t* script);

WKE_API wkeJSState* WKE_CALL wkeGlobalExec(wkeWebView* webView);

WKE_API void        WKE_CALL wkeSleep(wkeWebView* webView);
WKE_API void        WKE_CALL wkeWake(wkeWebView* webView);
WKE_API bool        WKE_CALL wkeIsAwake(wkeWebView* webView);

WKE_API void        WKE_CALL wkeSetZoomFactor(wkeWebView* webView, float factor);
WKE_API float       WKE_CALL wkeGetZoomFactor(wkeWebView* webView);

WKE_API void        WKE_CALL wkeSetEditable(wkeWebView* webView, bool editable);

WKE_API void        WKE_CALL wkeSetHostWindow(wkeWebView* webWindow, void* hostWindow);
WKE_API void*       WKE_CALL wkeGetHostWindow(wkeWebView* webWindow);


WKE_API const utf8* WKE_CALL wkeGetString(const wkeString* string);
WKE_API const wchar_t* WKE_CALL wkeGetStringW(const wkeString* string);

WKE_API void        WKE_CALL wkeSetString(wkeString* string, const utf8* str, size_t len);
WKE_API void        WKE_CALL wkeSetStringW(wkeString* string, const wchar_t* str, size_t len);

typedef void      (WKE_CALL *wkeTitleChangedCallback)(wkeWebView* webView, void* param, const wkeString* title);
WKE_API void        WKE_CALL wkeOnTitleChanged(wkeWebView* webView, wkeTitleChangedCallback callback, void* callbackParam);

typedef void      (WKE_CALL *wkeURLChangedCallback)(wkeWebView* webView, void* param, const wkeString* url);
WKE_API void        WKE_CALL wkeOnURLChanged(wkeWebView* webView, wkeURLChangedCallback callback, void* callbackParam);

typedef void      (WKE_CALL *wkePaintUpdatedCallback)(wkeWebView* webView, void* param, const void* hdc, int x, int y, int cx, int cy);
WKE_API void        WKE_CALL wkeOnPaintUpdated(wkeWebView* webView, wkePaintUpdatedCallback callback, void* callbackParam);

typedef void      (WKE_CALL *wkeAlertBoxCallback)(wkeWebView* webView, void* param, const wkeString* msg);
WKE_API void        WKE_CALL wkeOnAlertBox(wkeWebView* webView, wkeAlertBoxCallback callback, void* callbackParam);

typedef bool      (WKE_CALL *wkeConfirmBoxCallback)(wkeWebView* webView, void* param, const wkeString* msg);
WKE_API void        WKE_CALL wkeOnConfirmBox(wkeWebView* webView, wkeConfirmBoxCallback callback, void* callbackParam);

typedef bool      (WKE_CALL *wkePromptBoxCallback)(wkeWebView* webView, void* param, const wkeString* msg, const wkeString* defaultResult, wkeString* result);
WKE_API void        WKE_CALL wkeOnPromptBox(wkeWebView* webView, wkePromptBoxCallback callback, void* callbackParam);


typedef enum 
{
    WKE_MESSAGE_SOURCE_HTML,
    WKE_MESSAGE_SOURCE_XML,
    WKE_MESSAGE_SOURCE_JS,
    WKE_MESSAGE_SOURCE_NETWORK,
    WKE_MESSAGE_SOURCE_CONSOLE_API,
    WKE_MESSAGE_SOURCE_OTHER

} wkeMessageSource;

typedef enum 
{
    WKE_MESSAGE_TYPE_LOG,
    WKE_MESSAGE_TYPE_DIR,
    WKE_MESSAGE_TYPE_DIR_XML,
    WKE_MESSAGE_TYPE_TRACE,
    WKE_MESSAGE_TYPE_START_GROUP,
    WKE_MESSAGE_TYPE_START_GROUP_COLLAPSED,
    WKE_MESSAGE_TYPE_END_GROUP,
    WKE_MESSAGE_TYPE_ASSERT

} wkeMessageType;

typedef enum 
{
    WKE_MESSAGE_LEVEL_TIP,
    WKE_MESSAGE_LEVEL_LOG,
    WKE_MESSAGE_LEVEL_WARNING,
    WKE_MESSAGE_LEVEL_ERROR,
    WKE_MESSAGE_LEVEL_DEBUG

} wkeMessageLevel;

typedef struct
{
    wkeMessageSource source;
    wkeMessageType type;
    wkeMessageLevel level;
    wkeString* message;
    wkeString* url;
    unsigned int lineNumber;

} wkeConsoleMessage;

typedef void      (WKE_CALL *wkeConsoleMessageCallback)(wkeWebView* webView, void* param, const wkeConsoleMessage* message);
WKE_API void        WKE_CALL wkeOnConsoleMessage(wkeWebView* webView, wkeConsoleMessageCallback callback, void* callbackParam);



typedef enum
{
    WKE_NAVIGATION_TYPE_LINKCLICK,
    WKE_NAVIGATION_TYPE_FORMSUBMITTE,
    WKE_NAVIGATION_TYPE_BACKFORWARD,
    WKE_NAVIGATION_TYPE_RELOAD,
    WKE_NAVIGATION_TYPE_FORMRESUBMITT,
    WKE_NAVIGATION_TYPE_OTHER

} wkeNavigationType;

typedef bool      (WKE_CALL *wkeNavigationCallback)(wkeWebView* webView, void* param, wkeNavigationType navigationType, const wkeString* url);
WKE_API void        WKE_CALL wkeOnNavigation(wkeWebView* webView, wkeNavigationCallback callback, void* param);


typedef struct  
{
    wkeNavigationType navigationType;
    wkeString* url;
    wkeString* target;

    int x;
    int y;
    int width;
    int height;

    bool menuBarVisible;
    bool statusBarVisible;
    bool toolBarVisible;
    bool locationBarVisible;
    bool scrollbarsVisible;
    bool resizable;
    bool fullscreen;

} wkeNewViewInfo;


typedef wkeWebView* (WKE_CALL *wkeCreateViewCallback)(wkeWebView* webView, void* param, const wkeNewViewInfo* info);
WKE_API void        WKE_CALL wkeOnCreateView(wkeWebView* webView, wkeCreateViewCallback callback, void* param);


typedef struct
{
    wkeString* url;
    wkeJSState* frameJSState;
    wkeJSState* mainFrameJSState;

} wkeDocumentReadyInfo;

typedef void      (WKE_CALL *wkeDocumentReadyCallback)(wkeWebView* webView, void* param, const wkeDocumentReadyInfo* info);
WKE_API void        WKE_CALL wkeOnDocumentReady(wkeWebView* webView, wkeDocumentReadyCallback callback, void* param);


typedef enum
{
    WKE_LOADING_SUCCEEDED,
    WKE_LOADING_FAILED,
    WKE_LOADING_CANCELED

} wkeLoadingResult;

typedef void      (WKE_CALL *wkeLoadingFinishCallback)(wkeWebView* webView, void* param, const wkeString* url, wkeLoadingResult result, const wkeString* failedReason);
WKE_API void        WKE_CALL wkeOnLoadingFinish(wkeWebView* webView, wkeLoadingFinishCallback callback, void* param);


typedef enum
{
    WKE_WINDOW_TYPE_POPUP,
    WKE_WINDOW_TYPE_TRANSPARENT,
    WKE_WINDOW_TYPE_CONTROL

} wkeWindowType;

WKE_API wkeWebView*  WKE_CALL wkeCreateWebWindow(wkeWindowType type, void* parent, int x, int y, int width, int height);
WKE_API void        WKE_CALL wkeDestroyWebWindow(wkeWebView* webWindow);
WKE_API void*       WKE_CALL wkeGetWindowHandle(wkeWebView* webWindow);

typedef bool      (WKE_CALL *wkeWindowClosingCallback)(wkeWebView* webWindow, void* param);
WKE_API void        WKE_CALL wkeOnWindowClosing(wkeWebView* webWindow, wkeWindowClosingCallback callback, void* param);

typedef void      (WKE_CALL *wkeWindowDestroyCallback)(wkeWebView* webWindow, void* param);
WKE_API void        WKE_CALL wkeOnWindowDestroy(wkeWebView* webWindow, wkeWindowDestroyCallback callback, void* param);


WKE_API void        WKE_CALL wkeShowWindow(wkeWebView* webWindow, bool show);
WKE_API void        WKE_CALL wkeEnableWindow(wkeWebView* webWindow, bool enable);

WKE_API void        WKE_CALL wkeMoveWindow(wkeWebView* webWindow, int x, int y, int width, int height);
WKE_API void        WKE_CALL wkeMoveToCenter(wkeWebView* webWindow);
WKE_API void        WKE_CALL wkeResizeWindow(wkeWebView* webWindow, int width, int height);

WKE_API void        WKE_CALL wkeSetWindowTitle(wkeWebView* webWindow, const utf8* title);
WKE_API void        WKE_CALL wkeSetWindowTitleW(wkeWebView* webWindow, const wchar_t* title);


/***JavaScript Bind***/
#define JS_CALL __fastcall
typedef wkeJSValue (JS_CALL *wkeJSNativeFunction) (wkeJSState* es);

typedef enum
{
	JSTYPE_NUMBER,
	JSTYPE_STRING,
	JSTYPE_BOOLEAN,
	JSTYPE_OBJECT,
	JSTYPE_FUNCTION,
	JSTYPE_UNDEFINED,
} wkeJSType;


WKE_API void        WKE_CALL wkeJSBindFunction(const char* name, wkeJSNativeFunction fn, unsigned int argCount);
WKE_API void        WKE_CALL wkeJSBindGetter(const char* name, wkeJSNativeFunction fn); /*get property*/
WKE_API void        WKE_CALL wkeJSBindSetter(const char* name, wkeJSNativeFunction fn); /*set property*/

WKE_API int         WKE_CALL wkeJSParamCount(wkeJSState* es);
WKE_API wkeJSType   WKE_CALL wkeJSParamType(wkeJSState* es, int argIdx);
WKE_API wkeJSValue  WKE_CALL wkeJSParam(wkeJSState* es, int argIdx);

WKE_API wkeJSType   WKE_CALL wkeJSTypeOf(wkeJSValue v);
WKE_API bool        WKE_CALL wkeJSIsNumber(wkeJSValue v);
WKE_API bool        WKE_CALL wkeJSIsString(wkeJSValue v);
WKE_API bool        WKE_CALL wkeJSIsBool(wkeJSValue v);
WKE_API bool        WKE_CALL wkeJSIsObject(wkeJSValue v);
WKE_API bool        WKE_CALL wkeJSIsFunction(wkeJSValue v);
WKE_API bool        WKE_CALL wkeJSIsUndefined(wkeJSValue v);
WKE_API bool        WKE_CALL wkeJSIsNull(wkeJSValue v);
WKE_API bool        WKE_CALL wkeJSIsArray(wkeJSValue v);
WKE_API bool        WKE_CALL wkeJSIsTrue(wkeJSValue v);
WKE_API bool        WKE_CALL wkeJSIsFalse(wkeJSValue v);

WKE_API int         WKE_CALL wkeJSToInt(wkeJSState* es, wkeJSValue v);
WKE_API float       WKE_CALL wkeJSToFloat(wkeJSState* es, wkeJSValue v);
WKE_API double      WKE_CALL wkeJSToDouble(wkeJSState* es, wkeJSValue v);
WKE_API bool        WKE_CALL wkeJSToBool(wkeJSState* es, wkeJSValue v);
WKE_API const utf8* WKE_CALL wkeJSToTempString(wkeJSState* es, wkeJSValue v);
WKE_API const wchar_t* WKE_CALL wkeJSToTempStringW(wkeJSState* es, wkeJSValue v);

WKE_API wkeJSValue  WKE_CALL wkeJSInt(int n);
WKE_API wkeJSValue  WKE_CALL wkeJSFloat(float f);
WKE_API wkeJSValue  WKE_CALL wkeJSDouble(double d);
WKE_API wkeJSValue  WKE_CALL wkeJSBool(bool b);

WKE_API wkeJSValue  WKE_CALL wkeJSUndefined();
WKE_API wkeJSValue  WKE_CALL wkeJSNull();
WKE_API wkeJSValue  WKE_CALL wkeJSTrue();
WKE_API wkeJSValue  WKE_CALL wkeJSFalse();

WKE_API wkeJSValue  WKE_CALL wkeJSString(wkeJSState* es, const utf8* str);
WKE_API wkeJSValue  WKE_CALL wkeJSStringW(wkeJSState* es, const wchar_t* str);
WKE_API wkeJSValue  WKE_CALL wkeJSEmptyObject(wkeJSState* es);
WKE_API wkeJSValue  WKE_CALL wkeJSEmptyArray(wkeJSState* es);



//cexer JS对象、函数绑定支持
typedef struct __wkeJSData wkeJSData;
typedef wkeJSValue (WKE_CALL *wkeJSGetPropertyCallback)(wkeJSState* es, wkeJSValue object, const char* propertyName);
typedef bool       (WKE_CALL *wkeJSSetPropertyCallback)(wkeJSState* es, wkeJSValue object, const char* propertyName, wkeJSValue value);
typedef wkeJSValue (WKE_CALL *wkeJSCallAsFunctionCallback)(wkeJSState* es, wkeJSValue object, wkeJSValue* args, int argCount);
typedef void       (WKE_CALL *wkeJSFinalizeCallback)(wkeJSData* data);

typedef struct __wkeJSData
{
    char typeName[100];
    wkeJSGetPropertyCallback propertyGet;
    wkeJSSetPropertyCallback propertySet;
    wkeJSFinalizeCallback finalize;
    wkeJSCallAsFunctionCallback callAsFunction;

} wkeJSData;

WKE_API wkeJSValue  WKE_CALL wkeJSObject(wkeJSState* es, wkeJSData* obj);
WKE_API wkeJSValue  WKE_CALL wkeJSFunction(wkeJSState* es, wkeJSData* obj);
WKE_API wkeJSData*  WKE_CALL wkeJSGetData(wkeJSState* es, wkeJSValue object);

WKE_API wkeJSValue  WKE_CALL wkeJSGet(wkeJSState* es, wkeJSValue object, const char* prop);
WKE_API void       WKE_CALL wkeJSSet(wkeJSState* es, wkeJSValue object, const char* prop, wkeJSValue v);

WKE_API wkeJSValue  WKE_CALL wkeJSGetAt(wkeJSState* es, wkeJSValue object, int index);
WKE_API void       WKE_CALL wkeJSSetAt(wkeJSState* es, wkeJSValue object, int index, wkeJSValue v);

WKE_API int        WKE_CALL wkeJSGetLength(wkeJSState* es, wkeJSValue object);
WKE_API void       WKE_CALL wkeJSSetLength(wkeJSState* es, wkeJSValue object, int length);


WKE_API wkeJSValue  WKE_CALL wkeJSGlobalObject(wkeJSState* es);
WKE_API wkeWebView* WKE_CALL wkeJSGetWebView(wkeJSState* es);

WKE_API wkeJSValue  WKE_CALL wkeJSEval(wkeJSState* es, const utf8* str);
WKE_API wkeJSValue  WKE_CALL wkeJSEvalW(wkeJSState* es, const wchar_t* str);

WKE_API wkeJSValue  WKE_CALL wkeJSCall(wkeJSState* es, wkeJSValue func, wkeJSValue thisObject, wkeJSValue* args, int argCount);
WKE_API wkeJSValue  WKE_CALL wkeJSCallGlobal(wkeJSState* es, wkeJSValue func, wkeJSValue* args, int argCount);

WKE_API wkeJSValue  WKE_CALL wkeJSGetGlobal(wkeJSState* es, const char* prop);
WKE_API void       WKE_CALL wkeJSSetGlobal(wkeJSState* es, const char* prop, wkeJSValue v);


WKE_API void       WKE_CALL  wkeJSAddRef(wkeJSState* es, wkeJSValue v);
WKE_API void       WKE_CALL  wkeJSReleaseRef(wkeJSState* es, wkeJSValue v);
WKE_API void       WKE_CALL  wkeJSCollectGarbge(); 




#ifdef __cplusplus
}
#endif


#endif//#ifndef WKE_H
