//////////////////////////////////////////////////////////////////////////

#include "wkeChromeClient.h"

//////////////////////////////////////////////////////////////////////////



namespace wke
{


ToolTip::ToolTip(CWebView* webView) :webView_(webView)
    ,pixels_(NULL)
{

}

void ToolTip::set(const String& title, const WebCore::IntPoint& point)
{
    title_ = title;
    point_ = point;
    pixels_ = NULL;
}

const String& ToolTip::title() const
{
    return title_;
}


WebCore::IntRect ToolTip::paint(void* bits, int pitch)
{
    WebCore::IntRect rcRet;
    if (!pixels_)
    {
        if (title_.isEmpty())
            return rcRet;

        if (!hdc_)
        {
            hdc_ = adoptPtr(::CreateCompatibleDC(0));
            ::SelectObject(hdc_.get(), GetStockObject(DEFAULT_GUI_FONT));
            ::SetTextColor(hdc_.get(), RGB(0x64, 0x64, 0x64));
        }

        int width = webView_->width();
        int height = webView_->height();

        const int marginH = 4;
        const int marginV = 4;
        if (marginH * 2 >= width || marginV * 2 >= height)
            return rcRet;

        RECT rcText;
        rcText.left = marginH;
        rcText.right = rcText.left + width / 2;
        rcText.top = marginV;
        rcText.bottom = rcText.top + height / 2;
        ::DrawText(hdc_.get(), title_.characters(), title_.length(), &rcText, DT_WORDBREAK|DT_CALCRECT);

        if (rcText.right > width - marginH)
            rcText.right = width - marginH;

        if (rcText.bottom > height - marginV)
            rcText.bottom = height - marginV;

        rect_.left = 0;
        rect_.top = 0;
        rect_.right = rcText.right + marginH;
        rect_.bottom = rcText.bottom + marginV;

        WebCore::BitmapInfo bmp = WebCore::BitmapInfo::createBottomUp(WebCore::IntSize(rect_.right, rect_.bottom));
        HBITMAP hbmp = ::CreateDIBSection(0, &bmp, DIB_RGB_COLORS, &pixels_, NULL, 0);
        SelectObject(hdc_.get(), hbmp);
        hbmp_ = adoptPtr(hbmp);

        unsigned int* ptr = (unsigned int*)pixels_;

        //background
        memset(ptr, 0xFA, rect_.right * rect_.bottom * 4);

        //top
        memset(ptr, 0x64, rect_.right * 4);

        //bottom
        ptr = (unsigned int*)pixels_ + rect_.right * (rect_.bottom - 1);
        memset(ptr, 0x64, rect_.right * 4);

        //left and right
        ptr = (unsigned int*)pixels_ + rect_.right;
        for (int i = 1; i < rect_.bottom; ++i)
        {
            ptr[0] = 0x64646464;
            ptr[-1] = 0x64646464;
            ptr += rect_.right;
        }

        //corner
        ptr = (unsigned int*)pixels_;
        ptr[rect_.right + 1] = 0x64646464; 
        ptr[rect_.right * 2 - 2] = 0x64646464;
        ptr[rect_.right * (rect_.bottom - 1) - 2] = 0x64646464;
        ptr[rect_.right * (rect_.bottom - 2) + 1] = 0x64646464;

        ::DrawText(hdc_.get(), title_.characters(), title_.length(), &rcText, DT_WORDBREAK);

        for (int i = 0; i < rect_.right * rect_.bottom; ++i)
        {
            ((unsigned char*)pixels_)[i*4 + 3] = 255;
        }

        const int offset = 10;
        if (point_.x() + offset + rect_.right < width)
            rect_.left = point_.x() + offset;
        else if (point_.x() > rect_.right + offset)
            rect_.left = point_.x() - rect_.right - offset;
        else
            rect_.left = width - rect_.right;

        if (point_.y() + offset + rect_.bottom < height)
            rect_.top = point_.y() + offset;
        else if (point_.y() > rect_.top + offset)
            rect_.top = point_.y() - rect_.bottom - offset;
        else
            rect_.top = height - rect_.bottom;

        rect_.right += rect_.left;
        rect_.bottom += rect_.top;
    }

    //copy to dst
    int w = rect_.right - rect_.left;
    int h = rect_.bottom - rect_.top;
    unsigned char* src = (unsigned char*)pixels_; 
    unsigned char* dst = (unsigned char*)bits + rect_.top * pitch + rect_.left*4;

    //save corner
    memcpy(src, dst, 4);
    memcpy(src + (w - 1) * 4, dst + (w - 1) * 4, 4);
    memcpy(src + w * (h - 1) * 4, dst + pitch * (h - 1), 4);
    memcpy(src + w * h * 4 - 4, dst + pitch * (h - 1) + (w - 1) * 4, 4);

    for (int i = 0; i < h; ++i)
    {
        memcpy(dst, src, w*4);
        src += w*4;
        dst += pitch;
    }

    rcRet = rect_;
    return rcRet;
}




ChromeClient::ChromeClient(CWebView* webView) :webView_(webView)
    ,toolTip_(webView)
    ,popupMenu_(NULL)
{

}
void ChromeClient::chromeDestroyed()
{
    dbgMsg(L"frameLoaderDestroyed\n");
    delete this;
}

WebCore::IntRect ChromeClient::paintToolTip(void* bits, int pitch)
{
    return toolTip_.paint(bits, pitch);
}

WebCore::IntRect ChromeClient::paintPopupMenu(void* bits, int pitch)
{
    WebCore::IntRect rc;
    if (popupMenu_)
        rc=popupMenu_->paint(bits, pitch);
    return rc;
}

void ChromeClient::setPopupMenu(PopupMenu* popupMenu)
{
    popupMenu_ = popupMenu;
}

PopupMenu* ChromeClient::popupMenu()
{
    return popupMenu_;
}

void* ChromeClient::webView() const 
{
    return webView_;
}

void ChromeClient::numWheelEventHandlersChanged(unsigned)
{

}

bool ChromeClient::shouldRubberBandInDirection(WebCore::ScrollDirection) const 
{
    return true;
}

PassRefPtr<WebCore::SearchPopupMenu> ChromeClient::createSearchPopupMenu(WebCore::PopupMenuClient* client) const 
{
    return adoptRef(new EmptySearchPopupMenu);
}

PassRefPtr<WebCore::PopupMenu> ChromeClient::createPopupMenu(WebCore::PopupMenuClient* client) const 
{
    return adoptRef(new PopupMenu(client, (wke::ChromeClient*)this));
}

bool ChromeClient::selectItemAlignmentFollowsMenuWritingDirection()
{
    return false;
}

bool ChromeClient::selectItemWritingDirectionIsNatural()
{
    return true;
}

#if PLATFORM(WIN)
void ChromeClient::setLastSetCursorToCurrentCursor()
{

}
#endif


#if USE(ACCELERATED_COMPOSITING)
void ChromeClient::scheduleCompositingLayerSync()
{

}

void ChromeClient::setNeedsOneShotDrawingSynchronization()
{

}

void ChromeClient::attachRootGraphicsLayer(WebCore::Frame*, WebCore::GraphicsLayer*)
{

}
#endif

void ChromeClient::formStateDidChange(const WebCore::Node*)
{

}

void ChromeClient::loadIconForFiles(const Vector<WTF::String>& filenames, WebCore::FileIconLoader* loader)
{
    loader->notifyFinished(WebCore::Icon::createIconForFiles(filenames));
}

void ChromeClient::runOpenPanel(WebCore::Frame*, PassRefPtr<WebCore::FileChooser> chooser)
{
    //cexer 实现文件选择
    wchar_t* pathList = new wchar_t[10 * MAX_PATH];
    memset(pathList, 0, 10 * MAX_PATH * sizeof(wchar_t));

    OPENFILENAMEW ofn = { 0 };
    ofn.Flags = OFN_DONTADDTORECENT|OFN_EXPLORER|OFN_FILEMUSTEXIST|OFN_NONETWORKBUTTON|OFN_PATHMUSTEXIST;
    if (chooser->settings().allowsMultipleFiles)
        ofn.Flags |= OFN_ALLOWMULTISELECT;

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = L"*.*\0*.*\0\0";
    ofn.lpstrFile = pathList;
    ofn.nMaxFile = 10 * MAX_PATH;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrTitle = NULL;
    if (0 != GetOpenFileNameW(&ofn))
    {
        Vector<String> chosenFiles;

        wchar_t* path = pathList;
        while (*path)
        {
            size_t pathLen = wcslen(path);

            String pathString((const UChar*)path, pathLen);
            chosenFiles.append(pathString);

            path += pathLen + 1;
        }

        if (!chosenFiles.isEmpty())
            chooser->chooseFiles(chosenFiles);
    }

    delete [] pathList;
}

void ChromeClient::cancelGeolocationPermissionRequestForFrame(WebCore::Frame*, WebCore::Geolocation*)
{

}

void ChromeClient::requestGeolocationPermissionForFrame(WebCore::Frame*, WebCore::Geolocation*)
{

}

#if ENABLE(NOTIFICATIONS)
WebCore::NotificationPresenter* ChromeClient::notificationPresenter()
{
    return 0;
}
#endif


#if ENABLE(CONTEXT_MENUS)
void ChromeClient::showContextMenu()
{

}
#endif

void ChromeClient::reachedApplicationCacheOriginQuota(WebCore::SecurityOrigin*, int64_t totalSpaceNeeded)
{

}

void ChromeClient::reachedMaxAppCacheSize(int64_t spaceNeeded)
{

}

#if ENABLE(SQL_DATABASE)
void ChromeClient::exceededDatabaseQuota(WebCore::Frame* frame, const WTF::String& databaseName)
{
    const unsigned long long defaultQuota = 5 * 1024 * 1024;
    WebCore::DatabaseTracker::tracker().setQuota(frame->document()->securityOrigin(), defaultQuota);
}
#endif

void ChromeClient::print(WebCore::Frame*)
{

}

void ChromeClient::setToolTip(const WTF::String& toolTip, WebCore::TextDirection)
{

}

void ChromeClient::mouseDidMoveOverElement(const WebCore::HitTestResult& result, unsigned modifierFlags)
{
    WebCore::TextDirection dir;
    String title = result.title(dir);
    if (title != toolTip_.title())
    {
        WebCore::IntPoint point = ((CWebView*)webView())->mainFrame()->eventHandler()->currentMousePosition();
        toolTip_.set(title, point);
        webView_->setDirty(true);
    }
}

void ChromeClient::contentsSizeChanged(WebCore::Frame*, const WebCore::IntSize&) const 
{

}

void ChromeClient::setCursorHiddenUntilMouseMoves(bool)
{

}

void ChromeClient::setCursor(const WebCore::Cursor& cursor)
{
    HCURSOR platformCursor = cursor.platformCursor()->nativeCursor();
    if (!platformCursor)
        return;

    ::SetCursor(platformCursor);
}

void ChromeClient::scrollbarsModeDidChange() const 
{

}

PlatformPageClient ChromeClient::platformPageClient() const 
{
    return NULL;
}

WebCore::IntRect ChromeClient::windowToScreen(const WebCore::IntRect& pt) const 
{
    return pt;
}

WebCore::IntPoint ChromeClient::screenToWindow(const WebCore::IntPoint& pt) const 
{
    return pt;
}

void ChromeClient::scroll(const WebCore::IntSize&, const WebCore::IntRect&, const WebCore::IntRect&)
{

}

void ChromeClient::invalidateContentsForSlowScroll(const WebCore::IntRect& rect, bool immediate)
{
    webView_->addDirtyArea(rect.x(), rect.y(), rect.width(), rect.height());
    //dbgMsg(L"invalidateContentsForSlowScroll\n");
}

void ChromeClient::invalidateContentsAndWindow(const WebCore::IntRect& rect, bool immediate)
{
    webView_->addDirtyArea(rect.x(), rect.y(), rect.width(), rect.height());
    //dbgMsg(L"invalidateContentsAndWindow\n");
}

void ChromeClient::invalidateWindow(const WebCore::IntRect& rect, bool immediate)
{
    webView_->addDirtyArea(rect.x(), rect.y(), rect.width(), rect.height());
    //dbgMsg(L"invalidateWindow\n");
}

WebCore::IntRect ChromeClient::windowResizerRect() const 
{
    return WebCore::IntRect();
}

WebCore::KeyboardUIMode ChromeClient::keyboardUIMode()
{
    return WebCore::KeyboardAccessDefault;
}

bool ChromeClient::shouldInterruptJavaScript()
{
    return false;
}

void ChromeClient::setStatusbarText(const WTF::String& text)
{
    dbgMsg(L"setStatusbarText %s\n", CSTR(text));
}

bool ChromeClient::runJavaScriptPrompt(WebCore::Frame*, const WTF::String& msg, const WTF::String& defaultValue, WTF::String& result)
{
    outputMsg(L"JavaScript Prompt %s %s\n", CSTR(msg), CSTR(defaultValue));

    wke::CWebViewHandler& handler = webView_->handler_;
    if (!!handler.promptBoxCallback)
        return false;

    wke::CString wkeMsg(msg);
    wke::CString wkeDefaultResult(defaultValue);
    wke::CString wkeResult("");
    if (!handler.promptBoxCallback(webView_, handler.promptBoxCallbackParam, &wkeMsg, &wkeDefaultResult, &wkeResult))
        return false;

    result = wkeResult.original();
    return true;
}

bool ChromeClient::runJavaScriptConfirm(WebCore::Frame*, const WTF::String& msg)
{
    outputMsg(L"JavaScript Confirm %s\n", CSTR(msg));

    wke::CWebViewHandler& handler = webView_->handler_;
    if (!handler.confirmBoxCallback)
        return false;

    wke::CString wkeMsg(msg);
    return handler.confirmBoxCallback(webView_, handler.confirmBoxCallbackParam, &wkeMsg);
}

void ChromeClient::runJavaScriptAlert(WebCore::Frame*, const WTF::String& msg)
{
    outputMsg(L"JavaScript Alert %s\n", CSTR(msg));

    wke::CWebViewHandler& handler = webView_->handler_;
    if (!handler.alertBoxCallback)
        return;

    wke::CString wkeMsg(msg);
    handler.alertBoxCallback(webView_, handler.alertBoxCallbackParam, &wkeMsg);
}

void ChromeClient::closeWindowSoon()
{

}

bool ChromeClient::runBeforeUnloadConfirmPanel(const WTF::String& message, WebCore::Frame* frame)
{
    return true;
}

bool ChromeClient::canRunBeforeUnloadConfirmPanel()
{
    return true;
}

void ChromeClient::addMessageToConsole(WebCore::MessageSource, WebCore::MessageType, WebCore::MessageLevel, const WTF::String& message, unsigned int lineNumber, const WTF::String& url)
{
    outputMsg(L"console message %s %d %s\n", CSTR(message), lineNumber, CSTR(url));
}

void ChromeClient::setResizable(bool)
{

}

bool ChromeClient::menubarVisible()
{
    return false;
}

void ChromeClient::setMenubarVisible(bool)
{

}

bool ChromeClient::scrollbarsVisible()
{
    return false;
}

void ChromeClient::setScrollbarsVisible(bool)
{

}

bool ChromeClient::statusbarVisible()
{
    return false;
}

void ChromeClient::setStatusbarVisible(bool)
{

}

bool ChromeClient::toolbarsVisible()
{
    return false;
}

void ChromeClient::setToolbarsVisible(bool)
{

}

void ChromeClient::runModal()
{

}

bool ChromeClient::canRunModal()
{
    return true;
}

void ChromeClient::show()
{

}

WebCore::Page* ChromeClient::createWindow(WebCore::Frame*, const WebCore::FrameLoadRequest&, const WebCore::WindowFeatures&, const WebCore::NavigationAction&)
{
    dbgMsg(L"createWindow\n");
    return webView_->page();
}

void ChromeClient::focusedFrameChanged(WebCore::Frame*)
{

}

void ChromeClient::focusedNodeChanged(WebCore::Node*)
{

}

void ChromeClient::takeFocus(WebCore::FocusDirection)
{

}

bool ChromeClient::canTakeFocus(WebCore::FocusDirection)
{
    return true;
}

void ChromeClient::unfocus()
{

}

void ChromeClient::focus()
{

}

WebCore::FloatRect ChromeClient::pageRect()
{
    return rect_;
}

WebCore::FloatRect ChromeClient::windowRect()
{
    return rect_;
}

void ChromeClient::setWindowRect(const WebCore::FloatRect& rect)
{
    rect_ = rect;
}



};//namespace wke