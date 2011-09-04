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

//
//c++ interface
//-----------------------------------------------------------------------------------------------------------
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
        virtual bool mouseWheel(WPARAM wParam, int x, int y, int globalX, int globalY) = 0;
        virtual bool keyUp(unsigned int virtualKeyCode, int keyData, bool systemKey) = 0;
        virtual bool keyDown(WPARAM virtualKeyCode, LPARAM keyData, bool systemKey) = 0;
        virtual bool keyPress(unsigned int charCode, int keyData, bool systemKey) = 0;

        virtual void onSetFocus() = 0;
        virtual void onKillFocus() = 0;

        virtual void getCaret(wkeRect& rect) = 0;

        virtual void runJS(const utf8* script) = 0;
    };
}

typedef wke::IWebView* wkeWebView;

#else

typedef void* wkeWebView;

#endif

//
//c interface
//----------------------------------------------------------------------------------------------------------
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
WKE_API void wkeLoadURL_Unicode(wkeWebView webView, const wchar_t* url);
WKE_API void wkeLoadHTML(wkeWebView webView, const utf8* html);
WKE_API void wkeLoadHTML_Unicode(wkeWebView webView, const wchar_t* html);
WKE_API bool wkeIsLoading(wkeWebView webView);
WKE_API void wkeStopLoading(wkeWebView webView);
WKE_API void wkeReload(wkeWebView webView);
WKE_API const utf8* wkeTitle(wkeWebView webView);
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
WKE_API bool wkeMouseWheel(wkeWebView webView, WPARAM wParam, int x, int y, int globalX, int globalY);
WKE_API bool wkeKeyUp(wkeWebView webView, unsigned int virtualKeyCode, int keyData, bool systemKey);
WKE_API bool wkeKeyDown(wkeWebView webView, WPARAM virtualKeyCode, LPARAM keyData, bool systemKey);
WKE_API bool wkeKeyPress(wkeWebView webView, unsigned int charCode, int keyData, bool systemKey);
WKE_API void wkeOnSetFocus(wkeWebView webView);
WKE_API void wkeOnKillFocus(wkeWebView webView);
WKE_API void wkeGetCaret(wkeWebView webView, wkeRect* rect);
WKE_API void wkeRunJS(wkeWebView webView, const utf8* script);

#ifdef __cplusplus
}
#endif


#endif //WKE_VIEW_H