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

#include "icuwin.h"
#include "stringTable.h"
#include "wkeWebView.h"

#include "wkeDebug.h"
#include "wkeChromeClient.inl"
#include "wkeFrameLoaderClient.inl"
#include "wkeContextMenuClient.inl"
#include "wkeInspectorClient.inl"
#include "wkeEditorClient.inl"
#include "wkeDragClient.inl"

namespace wke
{

    CWebView::CWebView()
        :name_(StringTable::emptyString())
        ,transparent_(false)
        ,dirty_(false)
        ,width_(0)
        ,height_(0)
        ,gfxContext_(NULL)
        ,awake_(true)
        ,clientHandler_(NULL)
		,bufHandler_(NULL)
    {
        WebCore::Page::PageClients pageClients;
        pageClients.chromeClient = new ChromeClient(this);
        pageClients.contextMenuClient = new ContextMenuClient;
        pageClients.inspectorClient = new InspectorClient;
        pageClients.editorClient = new EditorClient;
        pageClients.dragClient = new DragClient;

        page_ = adoptPtr(new WebCore::Page(pageClients));
        WebCore::Settings* settings = page_->settings();
        settings->setMinimumFontSize(0);
        settings->setMinimumLogicalFontSize(9);
        settings->setDefaultFontSize(16);
        settings->setDefaultFixedFontSize(13);
        settings->setJavaScriptEnabled(true);
        settings->setPluginsEnabled(true);
        settings->setLoadsImagesAutomatically(true);
        settings->setDefaultTextEncodingName(icuwin_getDefaultEncoding());
        
        settings->setStandardFontFamily("Times New Roman");
        settings->setFixedFontFamily("Courier New");
        settings->setSerifFontFamily("Times New Roman");
        settings->setSansSerifFontFamily("Arial");
        settings->setCursiveFontFamily("Comic Sans MS");
        settings->setFantasyFontFamily("Times New Roman");
        settings->setPictographFontFamily("Times New Roman");

        settings->setAllowUniversalAccessFromFileURLs(true);
        settings->setAllowFileAccessFromFileURLs(true);

        settings->setJavaScriptCanAccessClipboard(true);
        settings->setShouldPrintBackgrounds(true);
        settings->setTextAreasAreResizable(true);

        settings->setLocalStorageEnabled(true);
        
        UChar dir[256];
        GetCurrentDirectory(256, dir);
        wcscat(dir, L"\\localStorage");
        settings->setLocalStorageDatabasePath(dir);
        WebCore::DatabaseTracker::tracker().setDatabaseDirectoryPath(dir);

        FrameLoaderClient* loader = new FrameLoaderClient(this, page_.get());
        mainFrame_ = WebCore::Frame::create(page_.get(), NULL, loader).get();
        loader->setFrame(mainFrame_);
        mainFrame_->init();

        page()->focusController()->setActive(true);

        hdc_ = adoptPtr(::CreateCompatibleDC(0));
    }

    CWebView::~CWebView()
    {
        delete gfxContext_;
        mainFrame_->loader()->detachFromParent();
    }

    void CWebView::destroy()
    {
        wkeDestroyWebView(this);
    }

    const char* CWebView::name() const
    {
        return name_;
    }

    void CWebView::setName(const char* name)
    {
        name_ = StringTable::addString(name);
    }

    bool CWebView::transparent() const
    {
        return transparent_;
    }

    void CWebView::setTransparent(bool transparent)
    {
        if (transparent_ == transparent)
            return;

        transparent_ = transparent;
        dirtyArea_ = WebCore::IntRect(0, 0, width_, height_);
        setDirty(true);

        if (gfxContext_)
        {
            delete gfxContext_;
            gfxContext_ = NULL;
        }

        WebCore::Color backgroundColor = transparent ? WebCore::Color::transparent : WebCore::Color::white;
        mainFrame_->view()->updateBackgroundRecursively(backgroundColor, transparent);
    }

    void CWebView::loadURL(const utf8* inUrl)
    {
        WebCore::KURL url(WebCore::KURL(), inUrl, WebCore::UTF8Encoding());
        if (!url.isValid())
            url.setProtocol("http:");

        if (!url.isValid())
            return;

        if (WebCore::protocolIsJavaScript(url))
        {
            mainFrame_->script()->executeIfJavaScriptURL(url);
            return;
        }

        WebCore::ResourceRequest request(url);
        request.setCachePolicy(WebCore::UseProtocolCachePolicy);
        request.setTimeoutInterval(60.f);
        request.setHTTPMethod("GET");
        mainFrame_->loader()->load(request, false);
    }

    void CWebView::loadURL(const wchar_t* url)
    {
        loadURL(String(url).utf8().data());
    }

    void CWebView::loadHTML(const utf8* html)
    {
        String mime = "text/html";

        RefPtr<WebCore::SharedBuffer> sharedBuffer = WebCore::SharedBuffer::create(html, strlen(html));

        WebCore::KURL url(WebCore::KURL(), "");
        WebCore::ResourceRequest request(url);
        WebCore::SubstituteData substituteData(sharedBuffer.release(), mime, WebCore::UTF8Encoding().name(), url);

        mainFrame_->loader()->load(request, substituteData, false);
    }

    void CWebView::loadHTML(const wchar_t* html)
    {
        String mime = "text/html";

        RefPtr<WebCore::SharedBuffer> sharedBuffer = WebCore::SharedBuffer::create((const char*)html, wcslen(html)*2);

        WebCore::KURL url(WebCore::KURL(), "");
        WebCore::ResourceRequest request(url);
        WebCore::SubstituteData substituteData(sharedBuffer.release(), mime, WebCore::UTF16LittleEndianEncoding().name(), url);

        mainFrame_->loader()->load(request, substituteData, false);
    }

    void CWebView::loadFile(const utf8* filename)
    {
        char url[1024];
        _snprintf(url, 1023, "file:///%s", filename);
        url[1023] = '\0';
        loadURL(url);
    }

    void CWebView::loadFile(const wchar_t* filename)
    {
        wchar_t url[1024];
        _snwprintf(url, 1024, L"file:///%s", filename);
        url[1023] = L'\0';
        loadURL(url);
    }

    bool CWebView::isLoaded() const
    {
        FrameLoaderClient* client = (FrameLoaderClient*)mainFrame()->loader()->client();
        return client->isLoaded();
    }

    bool CWebView::isLoadFailed() const
    {
        FrameLoaderClient* client = (FrameLoaderClient*)mainFrame()->loader()->client();
        return client->isLoadFailed();
    }

    bool CWebView::isLoadComplete() const
    {
        return isLoaded() || isLoadFailed();
    }

    bool CWebView::isDocumentReady() const
    {
        FrameLoaderClient* client = (FrameLoaderClient*)mainFrame()->loader()->client();
        return client->isDocumentReady();
    }

    void CWebView::stopLoading()
    {
        mainFrame()->loader()->stopAllLoaders();
    }

    void CWebView::reload()
    {
        mainFrame()->loader()->reload();
    }

    const utf8* CWebView::title()
    {
        if (mainFrame()->loader()->documentLoader())
        {
            const String& str = mainFrame()->loader()->documentLoader()->title().string();
            return StringTable::addString(str.characters(), str.length());
        }

        return StringTable::addString("notitle");
    }

    const wchar_t* CWebView::titleW()
    {
        if (mainFrame()->loader()->documentLoader())
        {
            const String& str = mainFrame()->loader()->documentLoader()->title().string();
            return StringTableW::addString(str.characters(), str.length());
        }

        return StringTableW::addString(L"notitle");
    }

    void CWebView::resize(int w, int h)
    {
        if (w != width_ || h != height_)
        {
            mainFrame_->view()->resize(w, h);

            width_ = w;
            height_ = h;

            dirtyArea_ = WebCore::IntRect(0, 0, w, h);
            setDirty(true);

            if (gfxContext_)
            {
                delete gfxContext_;
                gfxContext_ = NULL;
            }
        }
    }

    int CWebView::width() const 
    { 
        return width_; 
    }

    int CWebView::height() const 
    { 
        return height_; 
    }

    int CWebView::contentsWidth() const
    {
        return mainFrame()->view()->contentsWidth();
    }

    int CWebView::contentsHeight() const
    {
        return mainFrame()->view()->contentsHeight();
    }

    void CWebView::setDirty(bool dirty)
    {
        dirty_ = dirty;
    }

    bool CWebView::isDirty() const
    {
        return dirty_;
    }

    void CWebView::addDirtyArea(int x, int y, int w, int h)
    {
        if (w > 0 && h > 0)
        {
            dirtyArea_.unite(WebCore::IntRect(x, y, w, h));
            dirty_ = true;
        }
    }

    void CWebView::layoutIfNeeded()
    {
        mainFrame_->view()->updateLayoutAndStyleIfNeededRecursive();
    }

	void CWebView::tick()
	{
		if(!dirty_) return;

		layoutIfNeeded();

		if (gfxContext_ == NULL)
		{
			WebCore::BitmapInfo bmp = WebCore::BitmapInfo::createBottomUp(WebCore::IntSize(width_, height_));
			HBITMAP hbmp = ::CreateDIBSection(0, &bmp, DIB_RGB_COLORS, &pixels_, NULL, 0);
			::SelectObject(hdc_.get(), hbmp);
			hbmp_ = adoptPtr(hbmp);

			gfxContext_ = new WebCore::GraphicsContext(hdc_.get(), transparent_);
		}

		gfxContext_->save();

		if (transparent_)
			gfxContext_->clearRect(dirtyArea_);

		gfxContext_->clip(dirtyArea_);

		mainFrame_->view()->paint(gfxContext_, dirtyArea_);

		gfxContext_->restore();
        ChromeClient* client = (ChromeClient*)page()->chrome()->client();
        client->paintPopupMenu(pixels_,  width_*4);

		if(bufHandler_)
		{
            WebCore::IntPoint pt = dirtyArea_.location();
            WebCore::IntSize sz = dirtyArea_.size();
			bufHandler_->onBufUpdated(hdc_.get(),pt.x(),pt.y(),sz.width(),sz.height());	
		}
        dirtyArea_ = WebCore::IntRect();
        dirty_ = false;
	}
    
    HDC CWebView::getViewDC()
    {
        return hdc_.get();
    }
    
    void CWebView::paint(void* bits, int pitch)
    {
        if(dirty_) tick();

        if (pitch == 0 || pitch == width_*4)
        {
            memcpy(bits, pixels_, width_*height_*4);
        }
        else
        {
            unsigned char* src = (unsigned char*)pixels_; 
            unsigned char* dst = (unsigned char*)bits; 
            for(int i = 0; i < height_; ++i) 
            {
                memcpy(dst, src, width_*4);
                src += width_*4;
                dst += pitch;
            }
        }

    }

    void CWebView::paint(void* bits, int bufWid, int bufHei, int xDst, int yDst, int w, int h, int xSrc, int ySrc, bool bCopyAlpha)
    {
        if(dirty_) tick();

        
        if(xSrc + w > width_) w = width_ - xSrc;
        if(ySrc + h > height_) h = height_ -ySrc;
        
        if(xDst + w > bufWid) w =bufWid - xDst;
        if(yDst + h > bufHei) h = bufHei - yDst;
        
        int pitchDst = bufWid*4;
        int pitchSrc = width_*4;
        
        unsigned char* src = (unsigned char*)pixels_; 
        unsigned char* dst = (unsigned char*)bits; 
        src += pitchSrc*ySrc + xSrc*4;
        dst += yDst*pitchDst + xDst*4;
        
        if(bCopyAlpha)
        {
            for(int j = 0; j< h; j++)
            {
                memcpy(dst,src,w*4);
                dst += pitchDst;
                src += pitchSrc;
            }
        }else
        {
            for(int j = 0; j< h; j++)
            {
                for(int i=0;i<w;i++)
                {
                    memcpy(dst,src,3);
                    dst += 4;
                    src += 4;
                }
                dst += (bufWid - w)*4;
                src += (width_ - w)*4;
            }
        }
    }

    bool CWebView::canGoBack() const
    {
        return page()->backForwardList()->backItem() && !page()->defersLoading();
    }

    bool CWebView::goBack()
    {
        return page()->goBack();
    }

    bool CWebView::canGoForward() const
    {
        return page()->backForwardList()->forwardItem() && !page()->defersLoading();
    }

    bool CWebView::goForward()
    {
        return page()->goForward();
    }

    void CWebView::selectAll()
    {
        mainFrame()->editor()->command("SelectAll").execute();
    }

    void CWebView::copy()
    {
        page()->focusController()->focusedOrMainFrame()->editor()->command("Copy").execute();
    }

    void CWebView::cut()
    {
        page()->focusController()->focusedOrMainFrame()->editor()->command("Cut").execute();
    }

    void CWebView::paste()
    {
        page()->focusController()->focusedOrMainFrame()->editor()->command("Paste").execute();
    }

    void CWebView::delete_()
    {
        page()->focusController()->focusedOrMainFrame()->editor()->command("Delete").execute();
    }

    void CWebView::setCookieEnabled(bool enable)
    {
        page()->setCookieEnabled(enable);
    }

    bool CWebView::cookieEnabled() const
    {
        return page()->cookieEnabled();
    }

    void CWebView::setMediaVolume(float volume)
    {
        page()->setMediaVolume(volume);
    }

    float CWebView::mediaVolume() const
    {
        return page()->mediaVolume();
    }

    static WebCore::MouseEventType messageToEventType(unsigned int message)
    {
        switch (message) 
        {
        case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDBLCLK:
            //MSDN docs say double click is sent on mouse down
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
            return WebCore::MouseEventPressed;

        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
            return WebCore::MouseEventReleased;

        case WM_MOUSELEAVE:
        case WM_MOUSEMOVE:
            return WebCore::MouseEventMoved;

        default:
            ASSERT_NOT_REACHED();
            //Move is relatively harmless
            return WebCore::MouseEventMoved;
        }
    }

    static WebCore::MouseButton messageToButtonType(unsigned int message, unsigned int wParam)
    {
        switch (message)
        {
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_LBUTTONDBLCLK:
            return WebCore::LeftButton;

        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_RBUTTONDBLCLK:
            return WebCore::RightButton;

        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_MBUTTONDBLCLK:
            return WebCore::MiddleButton;

        case WM_MOUSEMOVE:
        case WM_MOUSELEAVE:
            if (wParam & MK_LBUTTON)
                return WebCore::LeftButton;

            if (wParam & MK_MBUTTON)
                return WebCore::MiddleButton;

            if (wParam & MK_RBUTTON)
                return WebCore::RightButton;

            return WebCore::NoButton;

        default:
            ASSERT_NOT_REACHED();
            return WebCore::NoButton;
        }
    }

    #define SPI_GETWHEELSCROLLCHARS (0x006C)
    static int horizontalScrollChars()
    {
        static ULONG scrollChars;
        if (!scrollChars && !SystemParametersInfo(SPI_GETWHEELSCROLLCHARS, 0, &scrollChars, 0))
            scrollChars = 1;
        return scrollChars;
    }

    static int verticalScrollLines()
    {
        static ULONG scrollLines;
        if (!scrollLines && !SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &scrollLines, 0))
            scrollLines = 3;
        return scrollLines;
    }


    bool CWebView::mouseEvent(unsigned int message, int x, int y, unsigned int flags)
    {
        if (!mainFrame()->view()->didFirstLayout())
            return true;

        static LONG globalClickCount;
        static WebCore::IntPoint globalPrevPoint;
        static WebCore::MouseButton globalPrevButton;
        static LONG globalPrevMouseDownTime;

        if (message == WM_CANCELMODE) {
            mainFrame()->eventHandler()->lostMouseCapture();
            return true;
        }

        // Create our event.
        // On WM_MOUSELEAVE we need to create a mouseout event, so we force the position
        // of the event to be at (MINSHORT, MINSHORT).
        if (message == WM_MOUSELEAVE)
        {
            x = MINSHORT;
            y = MINSHORT;
        }

        WebCore::IntPoint pos(x, y);
        WebCore::IntPoint globalPos(x, y);

        WebCore::MouseButton button = messageToButtonType(message, flags);
        WebCore::MouseEventType eventType = messageToEventType(message);

        bool shift = flags & WKE_SHIFT;
        bool ctrl = flags & WKE_CONTROL;
        bool alt = GetKeyState(VK_MENU) & 0x8000;
        bool meta = alt;
        double timestamp = ::GetTickCount()*0.001;

        int clickCount = 0;
        WebCore::PlatformMouseEvent mouseEvent(pos, globalPos, button, eventType, clickCount, shift, ctrl, alt, meta, timestamp);

        ChromeClient* client = (ChromeClient*)page()->chrome()->client();
        if (client->popupMenu() && client->popupMenu()->mouseEvent(mouseEvent))
            return true;

        bool insideThreshold = abs(globalPrevPoint.x() - mouseEvent.pos().x()) < ::GetSystemMetrics(SM_CXDOUBLECLK) &&
            abs(globalPrevPoint.y() - mouseEvent.pos().y()) < ::GetSystemMetrics(SM_CYDOUBLECLK);
        LONG messageTime = ::GetMessageTime();

        bool handled = false;

        if (message == WM_LBUTTONDOWN || message == WM_MBUTTONDOWN || message == WM_RBUTTONDOWN) {
            if (((messageTime - globalPrevMouseDownTime) < (LONG)::GetDoubleClickTime()) && 
                insideThreshold &&
                mouseEvent.button() == globalPrevButton)
                globalClickCount++;
            else
                // Reset the click count.
                globalClickCount = 1;
            globalPrevMouseDownTime = messageTime;
            globalPrevButton = mouseEvent.button();
            globalPrevPoint = mouseEvent.pos();

            mouseEvent.setClickCount(globalClickCount);

            page()->focusController()->setActive(true);
            handled = mainFrame()->eventHandler()->handleMousePressEvent(mouseEvent);
        } else if (message == WM_LBUTTONDBLCLK || message == WM_MBUTTONDBLCLK || message == WM_RBUTTONDBLCLK) {
            globalClickCount++;
            mouseEvent.setClickCount(globalClickCount);

            page()->focusController()->setActive(true);
            handled = mainFrame()->eventHandler()->handleMousePressEvent(mouseEvent);
        } else if (message == WM_LBUTTONUP || message == WM_MBUTTONUP || message == WM_RBUTTONUP) {
            // Record the global position and the button of the up.
            globalPrevButton = mouseEvent.button();
            globalPrevPoint = mouseEvent.pos();
            mouseEvent.setClickCount(globalClickCount);
            mainFrame()->eventHandler()->handleMouseReleaseEvent(mouseEvent);
        } else if (message == WM_MOUSELEAVE) {
            mainFrame()->eventHandler()->mouseMoved(mouseEvent);
            handled = true;
        } else if (message == WM_MOUSEMOVE) {
            if (!insideThreshold)
                globalClickCount = 0;
            mouseEvent.setClickCount(globalClickCount);
            handled = mainFrame()->eventHandler()->mouseMoved(mouseEvent);
        }
        return handled;
    }

    bool CWebView::contextMenuEvent(int x, int y, unsigned int flags)
    {
        page()->contextMenuController()->clearContextMenu();

        if (x == -1 && y == -1)
        {   
            WebCore::Frame* focusedFrame = page()->focusController()->focusedOrMainFrame();
            return focusedFrame->eventHandler()->sendContextMenuEventForKey();
        }

        WebCore::IntPoint pos(x, y);
        WebCore::IntPoint globalPos(x, y);

        WebCore::MouseButton button = messageToButtonType(WM_RBUTTONUP, flags);
        WebCore::MouseEventType eventType = messageToEventType(WM_RBUTTONUP);

        bool shift = flags & WKE_SHIFT;
        bool ctrl = flags & WKE_CONTROL;
        bool alt = GetKeyState(VK_MENU) & 0x8000;
        bool meta = alt;
        double timestamp = ::GetTickCount()*0.001;

        int clickCount = 0;
        WebCore::PlatformMouseEvent mouseEvent(pos, globalPos, button, eventType, clickCount, shift, ctrl, alt, meta, timestamp);

        WebCore::IntPoint documentPoint(mainFrame()->view()->windowToContents(pos));
        WebCore::HitTestResult result = mainFrame()->eventHandler()->hitTestResultAtPoint(documentPoint, false);
        WebCore::Frame* targetFrame = result.innerNonSharedNode() ? result.innerNonSharedNode()->document()->frame() : page()->focusController()->focusedOrMainFrame();

        targetFrame->view()->setCursor(WebCore::pointerCursor());
        return targetFrame->eventHandler()->sendContextMenuEvent(mouseEvent);
    }

    bool CWebView::mouseWheel(int x, int y, int wheelDelta, unsigned int flags)
    {
        if (!mainFrame()->view()->didFirstLayout())
            return true;

        WebCore::IntPoint pos(x, y);
        WebCore::IntPoint globalPos(x, y);

        static const float cScrollbarPixelsPerLine = 100.0f / 3.0f;
        float delta = wheelDelta / static_cast<float>(WHEEL_DELTA);
        float wheelTicksX = 0.f;
        float wheelTicksY = delta;

        float deltaX = 0.f;
        float deltaY = 0.f;

        bool shiftKey = flags & WKE_SHIFT;
        bool ctrlKey = flags & WKE_CONTROL;
        bool altKey = GetKeyState(VK_MENU) & 0x8000;

        WebCore::PlatformWheelEventGranularity granularity = WebCore::ScrollByPageWheelEvent;

        if (shiftKey) {
            deltaX = delta * static_cast<float>(horizontalScrollChars()) * cScrollbarPixelsPerLine;
            deltaY = 0;
            granularity = WebCore::ScrollByPixelWheelEvent;
        } else {
            deltaX = 0;
            deltaY = delta;
            int verticalMultiplier = verticalScrollLines();
            granularity = (verticalMultiplier == WHEEL_PAGESCROLL) ? WebCore::ScrollByPageWheelEvent : WebCore::ScrollByPixelWheelEvent;
            if (granularity == WebCore::ScrollByPixelWheelEvent)
                deltaY *= static_cast<float>(verticalMultiplier) * cScrollbarPixelsPerLine;
        }

        WebCore::PlatformWheelEvent wheelEvent(pos, globalPos, deltaX, deltaY, wheelTicksX, wheelTicksY, granularity, shiftKey, ctrlKey, altKey, altKey);

        ChromeClient* client = (ChromeClient*)page()->chrome()->client();
        if (client->popupMenu() && client->popupMenu()->mouseWheel(wheelEvent))
            return true;

        return mainFrame()->eventHandler()->handleWheelEvent(wheelEvent);
    }

    bool CWebView::keyUp(unsigned int virtualKeyCode, unsigned int flags, bool systemKey)
    {
        LPARAM keyData = MAKELPARAM(0, (WORD)flags);
        WebCore::PlatformKeyboardEvent keyEvent(0, virtualKeyCode, keyData, WebCore::PlatformKeyboardEvent::KeyUp, systemKey);

        WebCore::Frame* frame = page()->focusController()->focusedOrMainFrame();
        return frame->eventHandler()->keyEvent(keyEvent);
    }

    bool CWebView::keyDown(unsigned int virtualKeyCode, unsigned int flags, bool systemKey)
    {
        LPARAM keyData = MAKELPARAM(0, (WORD)flags);
        WebCore::PlatformKeyboardEvent keyEvent(0, virtualKeyCode, keyData, WebCore::PlatformKeyboardEvent::RawKeyDown, systemKey);

        ChromeClient* client = (ChromeClient*)page()->chrome()->client();
        if (client->popupMenu() && client->popupMenu()->keyDown(keyEvent))
            return true;

        WebCore::Frame* frame = page()->focusController()->focusedOrMainFrame();
        bool handled = frame->eventHandler()->keyEvent(keyEvent);

        // These events cannot be canceled, and we have no default handling for them.
        // FIXME: match IE list more closely, see <http://msdn2.microsoft.com/en-us/library/ms536938.aspx>.
        if (systemKey && virtualKeyCode != VK_RETURN)
            return false;

        if (handled) {
            MSG msg;
            ::PeekMessage(&msg, NULL, WM_CHAR, WM_CHAR, PM_REMOVE);
            return true;
        }

        // We need to handle back/forward using either Ctrl+Left/Right Arrow keys.
        // FIXME: This logic should probably be in EventHandler::defaultArrowEventHandler().
        // FIXME: Should check that other modifiers aren't pressed.
        if (virtualKeyCode == VK_RIGHT && keyEvent.ctrlKey())
            return page()->goForward();
        if (virtualKeyCode == VK_LEFT && keyEvent.ctrlKey())
            return page()->goBack();

        // Need to scroll the page if the arrow keys, pgup/dn, or home/end are hit.
        WebCore::ScrollDirection direction;
        WebCore::ScrollGranularity granularity;
        switch (virtualKeyCode) {
        case VK_LEFT:
            granularity = WebCore::ScrollByLine;
            direction = WebCore::ScrollLeft;
            break;

        case VK_RIGHT:
            granularity = WebCore::ScrollByLine;
            direction = WebCore::ScrollRight;
            break;

        case VK_UP:
            granularity = WebCore::ScrollByLine;
            direction = WebCore::ScrollUp;
            break;

        case VK_DOWN:
            granularity = WebCore::ScrollByLine;
            direction = WebCore::ScrollDown;
            break;

        case VK_HOME:
            granularity = WebCore::ScrollByDocument;
            direction = WebCore::ScrollUp;
            break;

        case VK_END:
            granularity = WebCore::ScrollByDocument;
            direction = WebCore::ScrollDown;
            break;

        case VK_PRIOR:
            granularity = WebCore::ScrollByPage;
            direction = WebCore::ScrollUp;
            break;

        case VK_NEXT:
            granularity = WebCore::ScrollByPage;
            direction = WebCore::ScrollDown;
            break;
        
        default:
            return false;
        }

        if (frame->eventHandler()->scrollRecursively(direction, granularity))
        {
            MSG msg;
            ::PeekMessage(&msg, NULL, WM_CHAR, WM_CHAR, PM_REMOVE);
            return true;
        }

        return false;
    }

    bool CWebView::keyPress(unsigned int charCode, unsigned int flags, bool systemKey)
    {
        LPARAM keyData = MAKELPARAM(0, (WORD)flags);
        WebCore::PlatformKeyboardEvent keyEvent(0, charCode, keyData, WebCore::PlatformKeyboardEvent::Char, systemKey);

        ChromeClient* client = (ChromeClient*)page()->chrome()->client();
        if (client->popupMenu() && client->popupMenu()->keyPress(keyEvent))
            return true;

        WebCore::Frame* frame = page()->focusController()->focusedOrMainFrame();
        if (systemKey)
            return frame->eventHandler()->handleAccessKey(keyEvent);

        return frame->eventHandler()->keyEvent(keyEvent);
    }

    void CWebView::focus()
    {
        WebCore::FocusController* focusController = page()->focusController();
        focusController->setFocused(true);
    }

    void CWebView::unfocus()
    {
        WebCore::FocusController* focusController = page()->focusController();
        focusController->setFocused(false);
    }

    wkeRect CWebView::getCaret()
    {
        wkeRect rect;
        rect.x = rect.y = 0;
        rect.w = rect.h = 0;
        WebCore::Frame* targetFrame = page()->focusController()->focusedOrMainFrame();
        if (!targetFrame)
            return rect;

        WebCore::IntRect caret;
        if (RefPtr<WebCore::Range> range = targetFrame->selection()->selection().toNormalizedRange()) {
            WebCore::ExceptionCode ec = 0;
            RefPtr<WebCore::Range> tempRange = range->cloneRange(ec);
            caret = targetFrame->editor()->firstRectForRange(tempRange.get());
        }

        caret = targetFrame->view()->contentsToWindow(caret);
        rect.x = caret.x();
        rect.y = caret.y();
        rect.w = caret.width();
        rect.h = caret.height();

        return rect;
    }

    jsValue CWebView::runJS(const wchar_t* script)
    {
        String string(script);
        WebCore::ScriptValue value = mainFrame_->script()->executeScript(string, true);
        if (value.hasNoValue())
            return jsUndefined();

        return JSC::JSValue::encode(value.jsValue());
    }

    jsValue CWebView::runJS(const utf8* script)
    {
        String string = String::fromUTF8(script);
        WebCore::ScriptValue value = mainFrame_->script()->executeScript(string, true);
        if (value.hasNoValue())
            return jsUndefined();

        return JSC::JSValue::encode(value.jsValue());
    }

    jsExecState CWebView::globalExec()
    {
        return mainFrame_->script()->globalObject(WebCore::mainThreadNormalWorld())->globalExec();
    }

    void CWebView::sleep()
    {
        awake_ = false;
        page()->setCanStartMedia(false);
        page()->willMoveOffscreen();
    }

    void CWebView::awaken()
    {
        awake_ = true;
        page()->didMoveOnscreen();
        page()->setCanStartMedia(true);
    }

    bool CWebView::isAwake() const
    {
        return awake_;
    }

    void CWebView::setZoomFactor(float factor)
    {
        mainFrame_->setPageZoomFactor(factor);
    }

    float CWebView::zoomFactor() const
    {
        return mainFrame_->pageZoomFactor();
    }

    void CWebView::setEditable(bool editable)
    {
        if (page()->isEditable() != editable)
        {
            page()->setEditable(editable);
            page()->setTabKeyCyclesThroughElements(!editable);
            if (editable)
                mainFrame_->editor()->applyEditingStyleToBodyElement();
        }
    }

    void CWebView::setClientHandler(const wkeClientHandler* handler)
    {
        clientHandler_ = handler;
    }

    const wkeClientHandler* CWebView::getClientHandler() const
    {
        return clientHandler_;
    }

	void CWebView::setBufHandler( wkeBufHandler *handler )
	{
		bufHandler_ = handler;
	}

	const wkeBufHandler * CWebView::getBufHandler() const
	{
		return bufHandler_;
	}
}

static Vector<wke::IWebView*> s_webViews;

wkeWebView wkeCreateWebView()
{
    wke::CWebView* webView = new wke::CWebView;
    s_webViews.append(webView);
    return webView;
}

wkeWebView wkeGetWebView(const char* name)
{
    for (size_t i = 0; i < s_webViews.size(); ++i)
    {
        if (strcmp(s_webViews[i]->name(), name) == 0)
            return s_webViews[i];
    }

    return 0;
}

void wkeDestroyWebView(wkeWebView webView)
{
    size_t pos = s_webViews.find(webView);

    ASSERT(pos != notFound);
    if (pos != notFound)
    {
        s_webViews.remove(pos);
        delete (wke::CWebView*)webView;
    }
}