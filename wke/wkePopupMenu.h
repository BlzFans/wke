#ifndef WKE_POPUP_MENU_H
#define WKE_POPUP_MENU_H

#include <WebCore/config.h>
#include <WebCore/ScrollableArea.h>
#include <WebCore/PopupMenu.h>
#include <WebCore/PopupMenuClient.h>

namespace wke
{
    class ChromeClient;

    class PopupMenu : public WebCore::PopupMenu, private WebCore::ScrollableArea {
    public:
        PopupMenu(WebCore::PopupMenuClient* client, ChromeClient* chromeClient);
        ~PopupMenu();

        virtual void show(const WebCore::IntRect&, WebCore::FrameView*, int index);
        virtual void hide();
        virtual void updateFromElement();
        virtual void disconnectClient();

        WebCore::IntRect paint(void* bits, int pitch);

        bool mouseEvent(const WebCore::PlatformMouseEvent& mouseEvent);
        bool mouseWheel(const WebCore::PlatformWheelEvent& wheelEvent);

        bool keyPress(const WebCore::PlatformKeyboardEvent& keyEvent);
        bool keyDown(const WebCore::PlatformKeyboardEvent& keyEvent);

    private:
        WebCore::PopupMenuClient* client() const { return popupClient_; }

        WebCore::Scrollbar* scrollbar() const { return scrollbar_.get(); }

        bool up(unsigned int lines = 1);
        bool down(unsigned int lines = 1);

        int itemHeight() const { return itemHeight_; }
        const WebCore::IntRect& windowRect() const { return windowRect_; }
        WebCore::IntRect clientRect() const;

        int visibleItems() const;

        int listIndexAtPoint(const WebCore::IntPoint& pt) const;

        bool setFocusedIndex(int index, bool hotTracking = false);
        int focusedIndex() const;
        void focusFirst();
        void focusLast();

        int scrollOffset() const { return scrollOffset_; }

        bool scrollToRevealSelection();

        void incrementWheelDelta(int delta);
        void reduceWheelDelta(int delta);
        int wheelDelta() const { return wheelDelta_; }

        bool scrollbarCapturingMouse() const { return scrollbarCapturingMouse_; }
        void setScrollbarCapturingMouse(bool b) { scrollbarCapturingMouse_ = b; }

        // ScrollableArea
        virtual int scrollSize(WebCore::ScrollbarOrientation orientation) const;
        virtual int scrollPosition(WebCore::Scrollbar*) const;
        virtual void setScrollOffset(const WebCore::IntPoint&);
        virtual void invalidateScrollbarRect(WebCore::Scrollbar*, const WebCore::IntRect&) { };
        virtual void invalidateScrollCornerRect(const WebCore::IntRect&) { }
        virtual bool isActive() const { return true; }
        virtual WebCore::ScrollableArea* enclosingScrollableArea() const { return 0; }
        virtual bool isScrollCornerVisible() const { return false; }
        virtual WebCore::IntRect scrollCornerRect() const { return WebCore::IntRect(); }
        virtual WebCore::Scrollbar* verticalScrollbar() const { return scrollbar_.get(); }

        // NOTE: This should only be called by the overriden setScrollOffset from ScrollableArea.
        void scrollTo(int offset);


        void calculatePositionAndSize(const WebCore::IntRect&, WebCore::FrameView*);

        void invalidate();

        WebCore::PopupMenuClient* popupClient_;
        RefPtr<WebCore::Scrollbar> scrollbar_;
        int scrollOffset_;
        int itemHeight_;
        int focusedIndex_;
        bool showPopup_;
        int wheelDelta_;
        bool scrollbarCapturingMouse_;

        ChromeClient* chromeClient_;

        OwnPtr<HDC> hdc_;
        OwnPtr<HBITMAP> hbmp_;
        void* pixels_;
        WebCore::IntRect windowRect_;
    };

}

#endif