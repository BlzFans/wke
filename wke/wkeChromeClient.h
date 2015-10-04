#ifndef WKE_CHROME_CLLIENT_H
#define WKE_CHROME_CLLIENT_H

//////////////////////////////////////////////////////////////////////////

#include <WebCore/config.h>
#include <WebCore/FileIconLoader.h>
#include <WebCore/Icon.h>
#include <WebCore/SearchPopupMenu.h>
#include <WebCore/ChromeClient.h>
#include <WebCore/HitTestResult.h>
#include <WebCore/Frame.h>
#include <WebCore/BitmapInfo.h>
#include <WebCore/FileChooser.h>
#include <WebCore/DatabaseTracker.h>
#include <WebCore/WindowFeatures.h>
#include <WebCore/NotificationPresenter.h>
#include <CommDlg.h>

#include "wkeDebug.h"
#include "wkePopupMenu.h"
#include "wkeWebView.h"

//////////////////////////////////////////////////////////////////////////




namespace wke
{



class EmptySearchPopupMenu : public WebCore::SearchPopupMenu {
public:
    virtual WebCore::PopupMenu* popupMenu() { return NULL; }
    virtual void saveRecentSearches(const AtomicString&, const Vector<String>&) {}
    virtual void loadRecentSearches(const AtomicString&, Vector<String>&) {}
    virtual bool enabled() { return false; }
};



class ToolTip
{
public:
    ToolTip(CWebView* webView);

    void set(const String& title, const WebCore::IntPoint& point);
    const String& title() const;
    WebCore::IntRect paint(void* bits, int pitch);

private:
    String m_title;
    WebCore::IntPoint m_point;

    OwnPtr<HDC> m_hdc;
    OwnPtr<HBITMAP> m_hbitmap;
    void* m_pixels;
    RECT m_rect;

    CWebView* m_webView;
};



class ChromeClient : public WebCore::ChromeClient
{
public:
    ChromeClient(CWebView* webView);

    virtual void chromeDestroyed() override;

    virtual void setWindowRect(const WebCore::FloatRect& rect) override;
    virtual WebCore::FloatRect windowRect() override;
    virtual WebCore::FloatRect pageRect() override;

    virtual void focus() override;
    virtual void unfocus() override;
    virtual bool canTakeFocus(WebCore::FocusDirection) override;
    virtual void takeFocus(WebCore::FocusDirection) override;
    virtual void focusedNodeChanged(WebCore::Node*) override;
    void focusedFrameChanged(WebCore::Frame*) override;

    virtual WebCore::Page* createWindow(WebCore::Frame*, const WebCore::FrameLoadRequest&, const WebCore::WindowFeatures&, const WebCore::NavigationAction&) override;
    virtual void closeWindowSoon() override;

    virtual void show() override;
    virtual bool canRunModal() override;
    virtual void runModal() override;

    virtual void setToolbarsVisible(bool) override;
    virtual bool toolbarsVisible() override;
    virtual void setStatusbarVisible(bool) override;
    virtual bool statusbarVisible() override;
    virtual void setScrollbarsVisible(bool) override;
    virtual bool scrollbarsVisible() override;
    virtual void setMenubarVisible(bool) override;
    virtual bool menubarVisible() override;
    virtual void setResizable(bool) override;

    virtual void addMessageToConsole(WebCore::MessageSource, WebCore::MessageType, WebCore::MessageLevel, const WTF::String& message, unsigned int lineNumber, const WTF::String& url) override;

    virtual bool canRunBeforeUnloadConfirmPanel() override;
    virtual bool runBeforeUnloadConfirmPanel(const WTF::String& message, WebCore::Frame* frame) override;

    virtual void runJavaScriptAlert(WebCore::Frame*, const WTF::String& msg) override;
    virtual bool runJavaScriptConfirm(WebCore::Frame*, const WTF::String& msg) override;
    virtual bool runJavaScriptPrompt(WebCore::Frame*, const WTF::String& msg, const WTF::String& defaultValue, WTF::String& result) override;

    virtual void setStatusbarText(const WTF::String& text) override;

    virtual bool shouldInterruptJavaScript() override;

    WebCore::KeyboardUIMode keyboardUIMode() override;

    virtual WebCore::IntRect windowResizerRect() const override;

    virtual void invalidateWindow(const WebCore::IntRect& rect, bool immediate) override;
    virtual void invalidateContentsAndWindow(const WebCore::IntRect& rect, bool immediate) override;
    virtual void invalidateContentsForSlowScroll(const WebCore::IntRect& rect, bool immediate) override;

    virtual void scroll(const WebCore::IntSize&, const WebCore::IntRect&, const WebCore::IntRect&) override;

    virtual WebCore::IntPoint screenToWindow(const WebCore::IntPoint& pt) const override;
    virtual WebCore::IntRect windowToScreen(const WebCore::IntRect& pt) const override;

    virtual PlatformPageClient platformPageClient() const override;

    virtual void scrollbarsModeDidChange() const override;

    virtual void setCursor(const WebCore::Cursor& cursor) override;
    virtual void setCursorHiddenUntilMouseMoves(bool) override;

    virtual void contentsSizeChanged(WebCore::Frame*, const WebCore::IntSize&) const override;

    virtual void mouseDidMoveOverElement(const WebCore::HitTestResult& result, unsigned modifierFlags) override;

    virtual void setToolTip(const WTF::String& toolTip, WebCore::TextDirection) override;

    virtual void print(WebCore::Frame*) override;

#if ENABLE(SQL_DATABASE)
    virtual void exceededDatabaseQuota(WebCore::Frame* frame, const WTF::String& databaseName) override;
#endif

    virtual void reachedMaxAppCacheSize(int64_t spaceNeeded) override;
    virtual void reachedApplicationCacheOriginQuota(WebCore::SecurityOrigin*, int64_t totalSpaceNeeded) override;

#if ENABLE(CONTEXT_MENUS)
    virtual void showContextMenu() override;
#endif

#if ENABLE(NOTIFICATIONS)
    virtual WebCore::NotificationPresenter* notificationPresenter() override;
#endif
    virtual void requestGeolocationPermissionForFrame(WebCore::Frame*, WebCore::Geolocation*) override;

    virtual void cancelGeolocationPermissionRequestForFrame(WebCore::Frame*, WebCore::Geolocation*) override;

    virtual void runOpenPanel(WebCore::Frame*, PassRefPtr<WebCore::FileChooser> chooser) override;

    virtual void loadIconForFiles(const Vector<WTF::String>& filenames, WebCore::FileIconLoader* loader) override;

    virtual void formStateDidChange(const WebCore::Node*) override;

#if USE(ACCELERATED_COMPOSITING)
    virtual void attachRootGraphicsLayer(WebCore::Frame*, WebCore::GraphicsLayer*) override;
    virtual void setNeedsOneShotDrawingSynchronization() override;
    virtual void scheduleCompositingLayerSync() override;
#endif


#if PLATFORM(WIN)
    virtual void setLastSetCursorToCurrentCursor() override;
#endif

    virtual bool selectItemWritingDirectionIsNatural() override;

    bool selectItemAlignmentFollowsMenuWritingDirection() override;

    virtual PassRefPtr<WebCore::PopupMenu> createPopupMenu(WebCore::PopupMenuClient* client) const override;
    virtual PassRefPtr<WebCore::SearchPopupMenu> createSearchPopupMenu(WebCore::PopupMenuClient* client) const override;

    virtual bool shouldRubberBandInDirection(WebCore::ScrollDirection) const override;

    virtual void numWheelEventHandlersChanged(unsigned) override;

    virtual void* webView() const override;

    PopupMenu* popupMenu();
    void setPopupMenu(PopupMenu* popupMenu);
    WebCore::IntRect paintPopupMenu(void* bits, int pitch);
    WebCore::IntRect paintToolTip(void* bits, int pitch);

protected:
    WebCore::FloatRect m_rect;
    CWebView* m_webView;
    ToolTip m_tooltip;
    PopupMenu* m_popupMenu;
};





};//namespace wke




#endif //#ifndef WKE_CHROME_CLLIENT_H


