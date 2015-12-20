//////////////////////////////////////////////////////////////////////////

#include "wkeChromeClient.h"
#include <WebCore/FrameLoadRequest.h>

//////////////////////////////////////////////////////////////////////////



namespace wke
{


ToolTip::ToolTip(CWebView* webView) :m_webView(webView)
    ,m_pixels(NULL)
{

}

void ToolTip::set(const String& title, const WebCore::IntPoint& point)
{
    m_title = title;
    m_point = point;
    m_pixels = NULL;
}

const String& ToolTip::title() const
{
    return m_title;
}


WebCore::IntRect ToolTip::paint(void* bits, int pitch)
{
    WebCore::IntRect rcRet;
    if (!m_pixels)
    {
        if (m_title.isEmpty())
            return rcRet;

        if (!m_hdc)
        {
            m_hdc = adoptPtr(::CreateCompatibleDC(0));
            ::SelectObject(m_hdc.get(), GetStockObject(DEFAULT_GUI_FONT));
            ::SetTextColor(m_hdc.get(), RGB(0x64, 0x64, 0x64));
        }

        int width = m_webView->width();
        int height = m_webView->height();

        const int marginH = 4;
        const int marginV = 4;
        if (marginH * 2 >= width || marginV * 2 >= height)
            return rcRet;

        RECT rcText;
        rcText.left = marginH;
        rcText.right = rcText.left + width / 2;
        rcText.top = marginV;
        rcText.bottom = rcText.top + height / 2;
        ::DrawText(m_hdc.get(), m_title.characters(), m_title.length(), &rcText, DT_WORDBREAK|DT_CALCRECT);

        if (rcText.right > width - marginH)
            rcText.right = width - marginH;

        if (rcText.bottom > height - marginV)
            rcText.bottom = height - marginV;

        m_rect.left = 0;
        m_rect.top = 0;
        m_rect.right = rcText.right + marginH;
        m_rect.bottom = rcText.bottom + marginV;

        WebCore::BitmapInfo bmp = WebCore::BitmapInfo::createBottomUp(WebCore::IntSize(m_rect.right, m_rect.bottom));
        HBITMAP hbmp = ::CreateDIBSection(0, &bmp, DIB_RGB_COLORS, &m_pixels, NULL, 0);
        SelectObject(m_hdc.get(), hbmp);
        m_hbitmap = adoptPtr(hbmp);

        unsigned int* ptr = (unsigned int*)m_pixels;

        //background
        memset(ptr, 0xFA, m_rect.right * m_rect.bottom * 4);

        //top
        memset(ptr, 0x64, m_rect.right * 4);

        //bottom
        ptr = (unsigned int*)m_pixels + m_rect.right * (m_rect.bottom - 1);
        memset(ptr, 0x64, m_rect.right * 4);

        //left and right
        ptr = (unsigned int*)m_pixels + m_rect.right;
        for (int i = 1; i < m_rect.bottom; ++i)
        {
            ptr[0] = 0x64646464;
            ptr[-1] = 0x64646464;
            ptr += m_rect.right;
        }

        //corner
        ptr = (unsigned int*)m_pixels;
        ptr[m_rect.right + 1] = 0x64646464; 
        ptr[m_rect.right * 2 - 2] = 0x64646464;
        ptr[m_rect.right * (m_rect.bottom - 1) - 2] = 0x64646464;
        ptr[m_rect.right * (m_rect.bottom - 2) + 1] = 0x64646464;

        ::DrawText(m_hdc.get(), m_title.characters(), m_title.length(), &rcText, DT_WORDBREAK);

        for (int i = 0; i < m_rect.right * m_rect.bottom; ++i)
        {
            ((unsigned char*)m_pixels)[i*4 + 3] = 255;
        }

        const int offset = 10;
        if (m_point.x() + offset + m_rect.right < width)
            m_rect.left = m_point.x() + offset;
        else if (m_point.x() > m_rect.right + offset)
            m_rect.left = m_point.x() - m_rect.right - offset;
        else
            m_rect.left = width - m_rect.right;

        if (m_point.y() + offset + m_rect.bottom < height)
            m_rect.top = m_point.y() + offset;
        else if (m_point.y() > m_rect.top + offset)
            m_rect.top = m_point.y() - m_rect.bottom - offset;
        else
            m_rect.top = height - m_rect.bottom;

        m_rect.right += m_rect.left;
        m_rect.bottom += m_rect.top;
    }

    //copy to dst
    int w = m_rect.right - m_rect.left;
    int h = m_rect.bottom - m_rect.top;
    unsigned char* src = (unsigned char*)m_pixels; 
    unsigned char* dst = (unsigned char*)bits + m_rect.top * pitch + m_rect.left*4;

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

    rcRet = m_rect;
    return rcRet;
}




ChromeClient::ChromeClient(CWebView* webView) :m_webView(webView)
    ,m_tooltip(webView)
    ,m_popupMenu(NULL)
{

}
void ChromeClient::chromeDestroyed()
{
    dbgMsg(L"frameLoaderDestroyed\n");
    delete this;
}

WebCore::IntRect ChromeClient::paintToolTip(void* bits, int pitch)
{
    return m_tooltip.paint(bits, pitch);
}

WebCore::IntRect ChromeClient::paintPopupMenu(void* bits, int pitch)
{
    WebCore::IntRect rc;
    if (m_popupMenu)
        rc=m_popupMenu->paint(bits, pitch);
    return rc;
}

void ChromeClient::setPopupMenu(PopupMenu* popupMenu)
{
    m_popupMenu = popupMenu;
}

PopupMenu* ChromeClient::popupMenu()
{
    return m_popupMenu;
}

void* ChromeClient::webView() const 
{
    return m_webView;
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
    if (title != m_tooltip.title())
    {
        WebCore::IntPoint point = ((CWebView*)webView())->mainFrame()->eventHandler()->currentMousePosition();
        m_tooltip.set(title, point);
        m_webView->setDirty(true);
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
    return m_webView ? m_webView->hostWindow() : NULL;
}

WebCore::IntRect ChromeClient::windowToScreen(const WebCore::IntRect& windowPoint) const 
{
    HWND hwnd = m_webView ? m_webView->hostWindow() : NULL;
    if (!IsWindow(hwnd))
        return windowPoint;

    POINT tempPoint = { windowPoint.x(), windowPoint.y() };
    ClientToScreen(hwnd, &tempPoint);
    
    WebCore::IntRect screePoint(tempPoint.x, tempPoint.y, windowPoint.width(), windowPoint.height());
    return screePoint;
}

WebCore::IntPoint ChromeClient::screenToWindow(const WebCore::IntPoint& screePoint) const 
{
    HWND hwnd = m_webView ? m_webView->hostWindow() : NULL;
    if (!IsWindow(hwnd))
        return screePoint;

    POINT tempPoint = { screePoint.x(), screePoint.y() };
    ScreenToClient(hwnd, &tempPoint);

    WebCore::IntPoint windowPoint(tempPoint.x, tempPoint.y);
    return windowPoint;
}

void ChromeClient::scroll(const WebCore::IntSize&, const WebCore::IntRect&, const WebCore::IntRect&)
{

}

void ChromeClient::invalidateContentsForSlowScroll(const WebCore::IntRect& rect, bool immediate)
{
    m_webView->addDirtyArea(rect.x(), rect.y(), rect.width(), rect.height());
    //dbgMsg(L"invalidateContentsForSlowScroll\n");
}

void ChromeClient::invalidateContentsAndWindow(const WebCore::IntRect& rect, bool immediate)
{
    m_webView->addDirtyArea(rect.x(), rect.y(), rect.width(), rect.height());
    //dbgMsg(L"invalidateContentsAndWindow\n");
}

void ChromeClient::invalidateWindow(const WebCore::IntRect& rect, bool immediate)
{
    m_webView->addDirtyArea(rect.x(), rect.y(), rect.width(), rect.height());
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

    wke::CWebViewHandler& handler = m_webView->m_handler;
    if (!!handler.promptBoxCallback)
        return false;

    wke::CString wkeMsg(msg);
    wke::CString wkeDefaultResult(defaultValue);
    wke::CString wkeResult("");
    if (!handler.promptBoxCallback(m_webView, handler.promptBoxCallbackParam, &wkeMsg, &wkeDefaultResult, &wkeResult))
        return false;

    result = wkeResult.original();
    return true;
}

bool ChromeClient::runJavaScriptConfirm(WebCore::Frame*, const WTF::String& msg)
{
    outputMsg(L"JavaScript Confirm %s\n", CSTR(msg));

    wke::CWebViewHandler& handler = m_webView->m_handler;
    if (!handler.confirmBoxCallback)
        return false;

    wke::CString wkeMsg(msg);
    return handler.confirmBoxCallback(m_webView, handler.confirmBoxCallbackParam, &wkeMsg);
}

void ChromeClient::runJavaScriptAlert(WebCore::Frame*, const WTF::String& msg)
{
    outputMsg(L"JavaScript Alert %s\n", CSTR(msg));

    wke::CWebViewHandler& handler = m_webView->m_handler;
    if (!handler.alertBoxCallback)
        return;

    wke::CString wkeMsg(msg);
    handler.alertBoxCallback(m_webView, handler.alertBoxCallbackParam, &wkeMsg);
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

void ChromeClient::addMessageToConsole(WebCore::MessageSource source, WebCore::MessageType type, WebCore::MessageLevel level, const WTF::String& message, unsigned int lineNumber, const WTF::String& url)
{
    outputMsg(L"console message %s %d %s\n", CSTR(message), lineNumber, CSTR(url));

    wke::CWebViewHandler& handler = m_webView->m_handler;
    if (!handler.consoleMessageCallback)
        return;

    wkeConsoleMessage msgStruct;
    msgStruct.source = (wkeMessageSource)source;
    msgStruct.type = (wkeMessageType)type;
    msgStruct.level = (wkeMessageLevel)level;
    msgStruct.lineNumber = lineNumber;

    wke::CString csMessage = message;
    wke::CString csUrl = url;
    msgStruct.message = &csMessage;
    msgStruct.url = &csUrl;
    handler.consoleMessageCallback(m_webView, handler.consoleMessageCallbackParam, &msgStruct);
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

WebCore::Page* ChromeClient::createWindow(WebCore::Frame*, const WebCore::FrameLoadRequest& request, const WebCore::WindowFeatures& features, const WebCore::NavigationAction& action)
{
    // cexer 实现新窗口控制
    wke::CWebViewHandler& handler = m_webView->m_handler;
    if (!handler.createViewCallback)
        return m_webView->page();

    wkeNewViewInfo info;
    info.navigationType = (wkeNavigationType)action.type();
    info.x = features.xSet ? features.x : CW_USEDEFAULT;
    info.y = features.ySet ? features.y : CW_USEDEFAULT;
    info.width = features.widthSet ? features.width : CW_USEDEFAULT;
    info.height = features.heightSet ? features.height : CW_USEDEFAULT;
    info.locationBarVisible = features.locationBarVisible;
    info.menuBarVisible = features.menuBarVisible;
    info.resizable = features.resizable;
    info.statusBarVisible = features.statusBarVisible;
    info.toolBarVisible = features.toolBarVisible;
    info.fullscreen = features.fullscreen;

    wke::CString url = action.url().string();
    info.url = &url;

    wke::CString target = request.frameName();
    info.target = &target;

    wke::CWebView* createdWebView = handler.createViewCallback(m_webView, handler.createViewCallbackParam, &info);
    if (!createdWebView)
        return NULL;

    return createdWebView->page();
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
    return m_rect;
}

WebCore::FloatRect ChromeClient::windowRect()
{
    return m_rect;
}

void ChromeClient::setWindowRect(const WebCore::FloatRect& rect)
{
    m_rect = rect;
}



};//namespace wke