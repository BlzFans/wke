#ifndef WKE_WEB_VIEW_H
#define WKE_WEB_VIEW_H


//////////////////////////////////////////////////////////////////////////


#include <WebCore/config.h>
#include <WebCore/ChromeClient.h>
#include <WebCore/FrameLoaderClient.h>
#include <WebCore/ResourceError.h>
#include <WebCore/Page.h>
#include <WebCore/Frame.h>
#include <WebCore/FileChooser.h>
#include <WebCore/FormState.h>
#include <WebCore/HTMLFormElement.h>
#include <WebCore/FrameView.h>
#include <WebCore/BitmapInfo.h>
#include <WebCore/Settings.h>
#include <WebCore/PlatformWheelEvent.h>
#include <WebCore/PlatformKeyboardEvent.h>
#include <WebCore/FocusController.h>
#include <WebCore/ScriptValue.h>
#include <WebCore/BackForwardList.h>
#include <WebCore/TextEncoding.h>
#include <WebCore/ContextMenuController.h>
#include <WebCore/Chrome.h>

//cexer: 必须包含在后面，因为其中的 windows.h 会定义 max、min，导致 WebCore 内部的 max、min 出现错乱。
#include "wkeString.h"


//////////////////////////////////////////////////////////////////////////




namespace wke
{


struct CWebViewHandler
{
    wkeTitleChangedCallback titleChangedCallback;
    void* titleChangedCallbackParam;

    wkeURLChangedCallback urlChangedCallback;
    void* urlChangedCallbackParam;

    wkePaintUpdatedCallback paintUpdatedCallback;
    void* paintUpdatedCallbackParam;

    wkeAlertBoxCallback alertBoxCallback;
    void* alertBoxCallbackParam;

    wkeConfirmBoxCallback confirmBoxCallback;
    void* confirmBoxCallbackParam;

    wkePromptBoxCallback promptBoxCallback;
    void* promptBoxCallbackParam;

    wkeNavigationCallback navigationCallback;
    void* navigationCallbackParam;

    wkeCreateViewCallback createViewCallback;
    void* createViewCallbackParam;

    wkeDocumentReadyCallback documentReadyCallback;
    void* documentReadyCallbackParam;

    wkeLoadingFinishCallback loadingFinishCallback;
    void* loadingFinishCallbackParam;

    wkeConsoleMessageCallback consoleMessageCallback;
    void* consoleMessageCallbackParam;
};


class CWebView
{
public:
    CWebView();
    virtual ~CWebView();

    virtual bool create();
    virtual void destroy();

    const utf8* name() const;
    const wchar_t* nameW() const;

    void setName(const utf8* name);
    void setName(const wchar_t* name);

    bool isTransparent() const;
    void setTransparent(bool transparent);

    void loadURL(const utf8* inUrl);
    void loadURL(const wchar_t* url);
    
    void loadPostURL(const utf8* inUrl,const char * poastData,int nLen );
    void loadPostURL(const wchar_t * inUrl,const char * poastData,int nLen );

    void loadHTML(const utf8* html);
    void loadHTML(const wchar_t* html);

    void loadFile(const utf8* filename);
    void loadFile(const wchar_t* filename);

    void load(const utf8* str);
    void load(const wchar_t* str);

	void setUserAgent(const utf8 * useragent);
    void setUserAgent(const wchar_t * useragent);

    bool isLoadingSucceeded() const;
    bool isLoadingFailed() const;
    bool isLoadingCompleted() const;
    bool isDocumentReady() const;
    void stopLoading();
    void reload();

    const utf8* title();
    const wchar_t* titleW();
    
    virtual void resize(int w, int h);
    int width() const;
    int height() const;

    int contentWidth() const;
    int contentHeight() const;
    
    void setDirty(bool dirty);
    bool isDirty() const;
    void addDirtyArea(int x, int y, int w, int h);

    void layoutIfNeeded();
    void paint(void* bits, int pitch);
    void paint(void* bits, int bufWid, int bufHei, int xDst, int yDst, int w, int h, int xSrc, int ySrc, bool fKeepAlpha);
	bool repaintIfNeeded();
    HDC viewDC();
    
    bool canGoBack() const;
    bool goBack();
    bool canGoForward() const;
    bool goForward();
    
    void editorSelectAll();
    void editorCopy();
    void editorCut();
    void editorPaste();
    void editorDelete();

    const wchar_t* cookieW();
    const utf8* cookie();

    void setCookieW(const wchar_t* val);
    void setCookie(const utf8* val);

    void setCookieEnabled(bool enable);
    bool isCookieEnabled() const;
    
    void setMediaVolume(float volume);
    float mediaVolume() const;
    
    void setHostWindow(HWND win);
    HWND hostWindow() const;

    bool fireMouseEvent(unsigned int message, int x, int y, unsigned int flags);
    bool fireContextMenuEvent(int x, int y, unsigned int flags);
    bool fireMouseWheelEvent(int x, int y, int delta, unsigned int flags);
    bool fireKeyUpEvent(unsigned int virtualKeyCode, unsigned int flags, bool systemKey);
    bool fireKeyDownEvent(unsigned int virtualKeyCode, unsigned int flags, bool systemKey);
    bool fireKeyPressEvent(unsigned int charCode, unsigned int flags, bool systemKey);
    
    void setFocus();
    void killFocus();
    
    wkeRect caretRect();
    
    wkeJSValue runJS(const wchar_t* script);
    wkeJSValue runJS(const utf8* script);
    wkeJSState* globalExec();
    
    void sleep();
    void wake();
    bool isAwake() const;

    void setZoomFactor(float factor);
    float zoomFactor() const;

    void setEditable(bool editable);

    WebCore::Page* page() const { return m_page.get(); }
    WebCore::Frame* mainFrame() const { return m_mainFrame; }

    void onURLChanged(wkeURLChangedCallback callback, void* callbackParam);
    void onTitleChanged(wkeTitleChangedCallback callback, void* callbackParam);
    virtual void onPaintUpdated(wkePaintUpdatedCallback callback, void* callbackParam);

    void onAlertBox(wkeAlertBoxCallback callback, void* callbackParam);
    void onConfirmBox(wkeConfirmBoxCallback callback, void* callbackParam);
    void onPromptBox(wkePromptBoxCallback callback, void* callbackParam);

    void onNavigation(wkeNavigationCallback callback, void* callbackParam);
    void onCreateView(wkeCreateViewCallback callback, void* callbackParam);

    void onConsoleMessage(wkeConsoleMessageCallback callback, void* callbackParam);

    virtual void onLoadingFinish(wkeLoadingFinishCallback callback, void* callbackParam);
    virtual void onDocumentReady(wkeDocumentReadyCallback callback, void* callbackParam);

    void setRepaintInterval(int ms);
    int repaintInterval() const;
    bool repaintIfNeededAfterInterval();

protected:
    void _initHandler();
    void _initPage();
    void _initMemoryDC();

    //按理这些接口应该使用CWebView来实现的，可以把它们想像成一个类，因此设置为友员符合情理。
    friend class ToolTip;
    friend class ChromeClient;
    friend class ContextMenuClient;
    friend class DrawClient;
    friend class EditorClient;
    friend class FrameLoaderClient;
    friend class InspectorClient;
    friend class PlatformStrategies;

    OwnPtr<WebCore::Page> m_page;
    WebCore::Frame* m_mainFrame;
    wke::CString m_title;
    wke::CString m_cookie;

    wke::CString m_name;
    bool m_transparent;

    int m_width;
    int m_height;

    bool m_dirty;
    WebCore::IntRect m_dirtyArea;

    WebCore::GraphicsContext* m_graphicsContext;
    OwnPtr<HDC> m_hdc;
    OwnPtr<HBITMAP> m_hbitmap;
    void* m_pixels;

    bool m_awake;
    HWND m_hostWindow;

    CWebViewHandler m_handler;

    DWORD m_paintInterval;
    DWORD m_lastPaintTimeTick;
};




};//namespace wke




#endif//#ifndef WKE_WEB_VIEW_H
