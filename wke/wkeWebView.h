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
#include "wke.h"


//////////////////////////////////////////////////////////////////////////




namespace wke
{




class CWebView
{
public:
    CWebView();
   ~CWebView();

    void destroy();

    const char* name() const;
    void setName(const char* name);
    
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
    
    void resize(int w, int h);
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
	void repaintIfNeeded();
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

    void setCookieEnabled(bool enable);
    bool isCookieEnabled() const;
    
    void setMediaVolume(float volume);
    float mediaVolume() const;
    
    bool fireMouseEvent(unsigned int message, int x, int y, unsigned int flags);
    bool fireContextMenuEvent(int x, int y, unsigned int flags);
    bool fireMouseWheelEvent(int x, int y, int delta, unsigned int flags);
    bool fireKeyUpEvent(unsigned int virtualKeyCode, unsigned int flags, bool systemKey);
    bool fireKeyDownEvent(unsigned int virtualKeyCode, unsigned int flags, bool systemKey);
    bool fireKeyPressEvent(unsigned int charCode, unsigned int flags, bool systemKey);
    
    void setFocus();
    void killFocus();
    
    wkeRect caretRect();
    
    jsValue runJS(const wchar_t* script);
    jsValue runJS(const utf8* script);
    jsExecState globalExec();
    
    void sleep();
    void wake();
    bool isAwake() const;

    void setZoomFactor(float factor);
    float zoomFactor() const;

    void setEditable(bool editable);

    void setHandler(wkeViewHandler* handler);
    wkeViewHandler* handler() const;

    WebCore::Page* page() const { return page_.get(); }
    WebCore::Frame* mainFrame() const { return mainFrame_; }
	
protected:
    OwnPtr<WebCore::Page> page_;
    WebCore::Frame* mainFrame_;

    const char* name_;
    bool transparent_;

    int width_;
    int height_;

    bool dirty_;
    WebCore::IntRect dirtyArea_;

    WebCore::GraphicsContext* gfxContext_;
    OwnPtr<HDC> hdc_;
    OwnPtr<HBITMAP> hbmp_;
    void* pixels_;

    bool awake_;

    wkeViewHandler* clientHandler_;
};




};//namespace wke




#endif//#ifndef WKE_WEB_VIEW_H
