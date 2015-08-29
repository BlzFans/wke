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
        
        virtual bool transparent() const;
        virtual void setTransparent(bool transparent);

        virtual void loadURL(const utf8* inUrl);
        virtual void loadURL(const wchar_t* url);

        virtual void loadHTML(const utf8* html);
        virtual void loadHTML(const wchar_t* html);

        virtual void loadFile(const utf8* filename);
        virtual void loadFile(const wchar_t* filename);
        
        virtual bool isLoaded() const;
        virtual bool isLoadFailed() const;
        virtual bool isLoadComplete() const;
        virtual bool isDocumentReady() const;
        virtual void stopLoading();
        virtual void reload();

        virtual const utf8* title();
        virtual const wchar_t* titleW();
        
        virtual void resize(int w, int h);
        virtual int width() const;
        virtual int height() const;

        virtual int contentsWidth() const;
        virtual int contentsHeight() const;
        
        virtual void setDirty(bool dirty);
        virtual bool isDirty() const;
        virtual void addDirtyArea(int x, int y, int w, int h);

        virtual void layoutIfNeeded();
        virtual void paint(void* bits, int pitch);
        virtual void paint(void* bits, int bufWid, int bufHei, int xDst, int yDst, int w, int h, int xSrc, int ySrc, bool bCopyAlpha);
		virtual void tick();
        virtual HDC  getViewDC();
        
        virtual bool canGoBack() const;
        virtual bool goBack();
        virtual bool canGoForward() const;
        virtual bool goForward();
        
        virtual void selectAll();
        virtual void copy();
        virtual void cut();
        virtual void paste();
        virtual void delete_();
        
        virtual void setCookieEnabled(bool enable);
        virtual bool cookieEnabled() const;
        
        virtual void setMediaVolume(float volume);
        virtual float mediaVolume() const;
        
        virtual bool mouseEvent(unsigned int message, int x, int y, unsigned int flags);
        virtual bool contextMenuEvent(int x, int y, unsigned int flags);
        virtual bool mouseWheel(int x, int y, int delta, unsigned int flags);
        virtual bool keyUp(unsigned int virtualKeyCode, unsigned int flags, bool systemKey);
        virtual bool keyDown(unsigned int virtualKeyCode, unsigned int flags, bool systemKey);
        virtual bool keyPress(unsigned int charCode, unsigned int flags, bool systemKey);
        
        virtual void focus();
        virtual void unfocus();
        
        virtual wkeRect getCaret();
        
        virtual jsValue runJS(const wchar_t* script);
        virtual jsValue runJS(const utf8* script);
        virtual jsExecState globalExec();
        
        virtual void sleep();
        virtual void awaken();
        virtual bool isAwake() const;

        void setZoomFactor(float factor);
        float zoomFactor() const;

        void setEditable(bool editable);

        virtual void setClientHandler(const wkeClientHandler* handler);
        virtual const wkeClientHandler* getClientHandler() const;

		virtual void setBufHandler(wkeBufHandler *handler);
		virtual const wkeBufHandler * getBufHandler() const;

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

        const wkeClientHandler* clientHandler_;
		wkeBufHandler * bufHandler_;
    };
}

#endif //
