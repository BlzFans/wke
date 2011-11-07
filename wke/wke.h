/*
 * 
 * BlzFans@hotmail.com
 * http://wke.sf.net
 * licence LGPL
 *
 */

#ifndef WKE_VIEW_H
#define WKE_VIEW_H


#ifdef BUILDING_wke
#   define WKE_API __declspec(dllexport)
#else
#   define WKE_API __declspec(dllimport)
#endif

typedef char utf8;

typedef struct {
    int x;
    int y;
} wkePoint;

typedef struct {
    int x;
    int y;
    int w;
    int h;
} wkeRect;

typedef void* jsExecState;

/*
 *c++ interface
 *-----------------------------------------------------------------------------------------------------------
 *
 */

#ifdef __cplusplus

namespace wke
{
    class IWebView
    {
    public:
        virtual void destroy() = 0;

        virtual void loadURL(const utf8* url) = 0;
        virtual void loadURL(const wchar_t* url) = 0;

        virtual void loadHTML(const utf8* html) = 0;
        virtual void loadHTML(const wchar_t* html) = 0;

        virtual bool isLoading() const = 0;
        virtual void stopLoading() = 0;
        virtual void reload() = 0;

        virtual const utf8* title() = 0;
        virtual const wchar_t* titleW() = 0;

        virtual void resize(int w, int h) = 0;
        virtual int width() const = 0;
        virtual int height() const = 0;

        virtual void setDirty(bool dirty) = 0;
        virtual bool isDirty() const = 0;
        virtual void addDirtyArea(int x, int y, int w, int h) = 0;

        virtual void layoutIfNeeded() = 0;
        virtual void paint(void* dst, int pitch) = 0;

        virtual bool canGoBack() const = 0;
        virtual bool goBack() = 0;
        virtual bool canGoForward() const = 0;
        virtual bool goForward() = 0;

        virtual void selectAll() = 0;
        virtual void copy() = 0;
        virtual void cut() = 0;
        virtual void paste() = 0;
        virtual void delete_() = 0;

        virtual void setCookieEnabled(bool enable) = 0;
        virtual bool cookieEnabled() const = 0;

        virtual void setMediaVolume(float volume) = 0;
        virtual float mediaVolume() const = 0;

        virtual bool mouseEvent(unsigned int message, unsigned int wParam, int x, int y, int globalX, int globalY) = 0;
        virtual bool mouseWheel(unsigned int wParam, int x, int y, int globalX, int globalY) = 0;
        virtual bool keyUp(unsigned int virtualKeyCode, int keyData, bool systemKey) = 0;
        virtual bool keyDown(unsigned int virtualKeyCode, int keyData, bool systemKey) = 0;
        virtual bool keyPress(unsigned int charCode, int keyData, bool systemKey) = 0;

        virtual void focus() = 0;
        virtual void unfocus() = 0;

        virtual void getCaret(wkeRect& rect) = 0;

        virtual void runJS(const utf8* script) = 0;
        virtual void runJS(const wchar_t* script) = 0;
        virtual jsExecState execState() = 0;
    };
}

typedef wke::IWebView* wkeWebView;

#else

typedef void* wkeWebView;

#ifndef HAVE_WCHAR_T
typedef unsigned short wchar_t;
#endif

#ifndef HAVE_BOOL
typedef unsigned char bool;
#define true 1
#define false 0
#endif

#endif

/*
 *c interface
 *----------------------------------------------------------------------------------------------------------
 *
 */

#ifdef __cplusplus
extern "C"
{
#endif

WKE_API void wkeInit();
WKE_API void wkeShutdown();
WKE_API unsigned int wkeVersion();
WKE_API const utf8* wkeVersionString();

WKE_API wkeWebView wkeCreateWebView();
WKE_API void wkeDestroy(wkeWebView webView);

WKE_API void wkeLoadURL(wkeWebView webView, const utf8* url);
WKE_API void wkeLoadURLW(wkeWebView webView, const wchar_t* url);

WKE_API void wkeLoadHTML(wkeWebView webView, const utf8* html);
WKE_API void wkeLoadHTMLW(wkeWebView webView, const wchar_t* html);

WKE_API bool wkeIsLoading(wkeWebView webView);
WKE_API void wkeStopLoading(wkeWebView webView);
WKE_API void wkeReload(wkeWebView webView);

WKE_API const utf8* wkeTitle(wkeWebView webView);
WKE_API const wchar_t* wkeTitleW(wkeWebView webView);

WKE_API void wkeResize(wkeWebView webView, int w, int h);
WKE_API int wkeWidth(wkeWebView webView);
WKE_API int wkeHeight(wkeWebView webView);

WKE_API void wkeSetDirty(wkeWebView webView, bool dirty);
WKE_API bool wkeIsDirty(wkeWebView webView);
WKE_API void wkeAddDirtyArea(wkeWebView webView, int x, int y, int w, int h);
WKE_API void wkeLayoutIfNeeded(wkeWebView webView);
WKE_API void wkePaint(wkeWebView webView, void* dst, int pitch);

WKE_API bool wkeCanGoBack(wkeWebView webView);
WKE_API bool wkeGoBack(wkeWebView webView);
WKE_API bool wkeCanGoForward(wkeWebView webView);
WKE_API bool wkeGoForward(wkeWebView webView);

WKE_API void wkeSelectAll(wkeWebView webView);
WKE_API void wkeCopy(wkeWebView webView);
WKE_API void wkeCut(wkeWebView webView);
WKE_API void wkePaste(wkeWebView webView);
WKE_API void wkeDelete(wkeWebView webView);

WKE_API void wkeSetCookieEnabled(wkeWebView webView, bool enable);
WKE_API bool wkeCookieEnabled(wkeWebView webView);

WKE_API void wkeSetMediaVolume(wkeWebView webView, float volume);
WKE_API float wkeMediaVolume(wkeWebView webView);

WKE_API bool wkeMouseEvent(wkeWebView webView, unsigned int message, unsigned int wParam, int x, int y, int globalX, int globalY);
WKE_API bool wkeMouseWheel(wkeWebView webView, unsigned int wParam, int x, int y, int globalX, int globalY);
WKE_API bool wkeKeyUp(wkeWebView webView, unsigned int virtualKeyCode, int keyData, bool systemKey);
WKE_API bool wkeKeyDown(wkeWebView webView, unsigned int virtualKeyCode, int keyData, bool systemKey);
WKE_API bool wkeKeyPress(wkeWebView webView, unsigned int charCode, int keyData, bool systemKey);

WKE_API void wkeFocus(wkeWebView webView);
WKE_API void wkeUnfocus(wkeWebView webView);

WKE_API void wkeGetCaret(wkeWebView webView, wkeRect* rect);

WKE_API void wkeRunJS(wkeWebView webView, const utf8* script);
WKE_API void wkeRunJSW(wkeWebView webView, const wchar_t* script);

WKE_API jsExecState wkeExecState(wkeWebView webView);

/***JavaScript Bind***/

typedef __int64 jsValue;
typedef enum
{
	JSTYPE_NUMBER,
	JSTYPE_STRING,
	JSTYPE_BOOLEAN,
	JSTYPE_OBJECT,
	JSTYPE_FUNCTION,
	JSTYPE_UNDEFINED,
} jsType;

WKE_API int jsArgCount(jsExecState es);
WKE_API jsType jsArgType(jsExecState es, int argIdx);
WKE_API jsValue jsArg(jsExecState es, int argIdx);

WKE_API jsType jsTypeOf(jsValue v);
WKE_API bool jsIsNumber(jsValue v);
WKE_API bool jsIsString(jsValue v);
WKE_API bool jsIsBoolean(jsValue v);
WKE_API bool jsIsObject(jsValue v);
WKE_API bool jsIsFunction(jsValue v);
WKE_API bool jsIsUndefined(jsValue v);
WKE_API bool jsIsNull(jsValue v);
WKE_API bool jsIsArray(jsValue v);
WKE_API bool jsIsTrue(jsValue v);
WKE_API bool jsIsFalse(jsValue v);

WKE_API int jsToInt(jsExecState es, jsValue v);
WKE_API float jsToFloat(jsExecState es, jsValue v);
WKE_API double jsToDouble(jsExecState es, jsValue v);
WKE_API bool jsToBoolean(jsExecState es, jsValue v);
WKE_API const utf8* jsToString(jsExecState es, jsValue v);
WKE_API const wchar_t* jsToStringW(jsExecState es, jsValue v);

WKE_API jsValue jsInt(int n);
WKE_API jsValue jsFloat(float f);
WKE_API jsValue jsDouble(double d);
WKE_API jsValue jsBoolean(bool b);

WKE_API jsValue jsUndefined();
WKE_API jsValue jsNull();
WKE_API jsValue jsTrue();
WKE_API jsValue jsFalse();

WKE_API jsValue jsString(jsExecState es, const utf8* str);
WKE_API jsValue jsStringW(jsExecState es, const wchar_t* str);
WKE_API jsValue jsObject(jsExecState es);
WKE_API jsValue jsArray(jsExecState es);

//return the window object
WKE_API jsValue jsGlobalObject(jsExecState es);

WKE_API jsValue jsEval(jsExecState es, const utf8* str);
WKE_API jsValue jsEvalW(jsExecState es, const wchar_t* str);

WKE_API jsValue jsCall(jsExecState es, jsValue func, jsValue thisObject, jsValue* args, int argCount);
WKE_API jsValue jsCallGlobal(jsExecState es, jsValue func, jsValue* args, int argCount);

WKE_API jsValue jsGet(jsExecState es, jsValue object, const char* prop);
WKE_API void jsSet(jsExecState es, jsValue object, const char* prop, jsValue v);

WKE_API jsValue jsGetGlobal(jsExecState es, const char* prop);
WKE_API void jsSetGlobal(jsExecState es, const char* prop, jsValue v);

WKE_API jsValue jsGetAt(jsExecState es, jsValue object, int index);
WKE_API void jsSetAt(jsExecState es, jsValue object, int index, jsValue v);

WKE_API int jsGetLength(jsExecState es, jsValue object);
WKE_API void jsSetLength(jsExecState es, jsValue object, int length);

WKE_API void jsGC(); //garbage collect

#define JS_CALL __fastcall
typedef jsValue (JS_CALL *jsFunction) (jsExecState es);
WKE_API void jsBindFunction(const char* name, jsFunction fn, unsigned int argCount);

#ifdef __cplusplus
}
#endif


#endif