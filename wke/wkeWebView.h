#ifndef WKE_WEB_VIEW_H
#define WKE_WEB_VIEW_H

#include "wke.h"

namespace wke
{
    class CWebView : public IWebView
    {
    public:
        CWebView();
        ~CWebView();

        virtual void destroy();

        virtual const char* name() const;
        virtual void setName(const char* name);
        
        virtual bool isTransparent() const;
        virtual void setTransparent(bool transparent);

        virtual void loadURL(const utf8* inUrl);
        virtual void loadURL(const wchar_t* url);
        
        virtual void loadPostURL(const utf8* inUrl,const char * poastData,int nLen );
        virtual void loadPostURL(const wchar_t * inUrl,const char * poastData,int nLen );

        virtual void loadHTML(const utf8* html);
        virtual void loadHTML(const wchar_t* html);

        virtual void loadFile(const utf8* filename);
        virtual void loadFile(const wchar_t* filename);
        
        virtual bool isLoadingSucceeded() const;
        virtual bool isLoadingFailed() const;
        virtual bool isLoadingCompleted() const;
        virtual bool isDocumentReady() const;
        virtual void stopLoading();
        virtual void reload();

        virtual const utf8* title();
        virtual const wchar_t* titleW();
        
        virtual void resize(int w, int h);
        virtual int width() const;
        virtual int height() const;

        virtual int contentWidth() const;
        virtual int contentHeight() const;
        
        virtual void setDirty(bool dirty);
        virtual bool isDirty() const;
        virtual void addDirtyArea(int x, int y, int w, int h);

        virtual void layoutIfNeeded();
        virtual void paint(void* bits, int pitch);
        virtual void paint(void* bits, int bufWid, int bufHei, int xDst, int yDst, int w, int h, int xSrc, int ySrc, bool fKeepAlpha);
		virtual void repaintIfNeeded();
        virtual HDC  viewDC();
        
        virtual bool canGoBack() const;
        virtual bool goBack();
        virtual bool canGoForward() const;
        virtual bool goForward();
        
        virtual void editorSelectAll();
        virtual void editorCopy();
        virtual void editorCut();
        virtual void editorPaste();
        virtual void editorDelete();

        virtual const wchar_t* cookieW();
        virtual const utf8* cookie();

        virtual void setCookieEnabled(bool enable);
        virtual bool isCookieEnabled() const;
        
        virtual void setMediaVolume(float volume);
        virtual float mediaVolume() const;
        
        virtual bool fireMouseEvent(unsigned int message, int x, int y, unsigned int flags);
        virtual bool fireContextMenuEvent(int x, int y, unsigned int flags);
        virtual bool fireMouseWheelEvent(int x, int y, int delta, unsigned int flags);
        virtual bool fireKeyUpEvent(unsigned int virtualKeyCode, unsigned int flags, bool systemKey);
        virtual bool fireKeyDownEvent(unsigned int virtualKeyCode, unsigned int flags, bool systemKey);
        virtual bool fireKeyPressEvent(unsigned int charCode, unsigned int flags, bool systemKey);
        
        virtual void setFocus();
        virtual void killFocus();
        
        virtual wkeRect caretRect();
        
        virtual jsValue runJS(const wchar_t* script);
        virtual jsValue runJS(const utf8* script);
        virtual jsExecState globalExec();
        
        virtual void sleep();
        virtual void wake();
        virtual bool isAwake() const;

        void setZoomFactor(float factor);
        float zoomFactor() const;

        void setEditable(bool editable);

        virtual void setHandler(wkeViewHandler* handler);
        virtual wkeViewHandler* handler() const;

        WebCore::Page* page() const { return page_.get(); }
        WebCore::Frame* mainFrame() const { return mainFrame_; }

		void * getPixels(){
			return pixels_;
		}
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
}

#endif //
