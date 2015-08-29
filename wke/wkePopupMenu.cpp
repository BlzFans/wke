
#include <WebCore/config.h>
#include <WebCore/FrameView.h>
#include <WebCore/TextRun.h>
#include <WebCore/ScrollbarTheme.h>
#include <WebCore/BitmapInfo.h>
#include <WebCore/RenderTheme.h>
#include <WebCore/FileChooser.h>
#include <WebCore/PlatformWheelEvent.h>
#include <WebCore/PlatformKeyboardEvent.h>
#include "wkeWebView.h"
#include "wkePopupMenu.h"

#include "wkeChromeClient.inl"

// Maximum height of a popup window
static const int maxPopupHeight = 320;

static const int optionSpacingMiddle = 1;
static const int popupWindowBorderWidth = 1;

namespace wke
{

PopupMenu::PopupMenu(WebCore::PopupMenuClient* client, ChromeClient* chromeClient_)
    :popupClient_(client)
    ,chromeClient_(chromeClient_)
    ,scrollOffset_(0)
    ,scrollbar_(0)
    ,focusedIndex_(0)
    ,showPopup_(false)
    ,wheelDelta_(0)
    ,scrollbarCapturingMouse_(false)
    ,pixels_(NULL)
{
}

PopupMenu::~PopupMenu()
{
    if (scrollbar_)
        scrollbar_->setParent(0);

    hide();
}

void PopupMenu::disconnectClient()
{
    popupClient_ = 0;
}

void PopupMenu::show(const WebCore::IntRect& r, WebCore::FrameView* view, int index)
{
	showPopup_ = true;
    calculatePositionAndSize(r, view);
    if (clientRect().isEmpty())
        return;

    if (!scrollbar_ && visibleItems() < client()->listSize()) {
        // We need a scroll bar
        scrollbar_ = client()->createScrollbar(this, WebCore::VerticalScrollbar, WebCore::SmallScrollbar);
        scrollbar_->styleChanged();

        WebCore::IntSize size(clientRect().size());
        scrollbar()->setFrameRect(WebCore::IntRect(size.width() - scrollbar()->width(), 0, scrollbar()->width(), size.height()));

        int visibleItems = this->visibleItems();
        scrollbar()->setEnabled(visibleItems < client()->listSize());
        scrollbar()->setSteps(1, std::max(1, visibleItems - 1));
        scrollbar()->setProportion(visibleItems, client()->listSize());
    }

    if (client()) {
        int index = client()->selectedIndex();
        if (index >= 0)
            setFocusedIndex(index);
    }

    chromeClient_->setPopupMenu(this);
    invalidate();
}

void PopupMenu::hide()
{
    if (!showPopup_)
        return;

    showPopup_ = false;
    if (client())
        client()->popupDidHide();

    if (chromeClient_->popupMenu() == this)
        chromeClient_->setPopupMenu(NULL);

    invalidate();
}

void PopupMenu::calculatePositionAndSize(const WebCore::IntRect& r, WebCore::FrameView* v)
{
    // r is in absolute document coordinates, but we want to be in screen coordinates

    // First, move to WebView coordinates
    WebCore::IntRect rScreenCoords(v->contentsToWindow(r.location()), r.size());

    // First, determine the popup's height
    int itemCount = client()->listSize();
    itemHeight_ = client()->menuStyle().font().fontMetrics().height() + optionSpacingMiddle;
    int naturalHeight = itemHeight_ * itemCount;
    int popupHeight = std::min(maxPopupHeight, naturalHeight);
    // The popup should show an integral number of items (i.e. no partial items should be visible)
    popupHeight -= popupHeight % itemHeight_;
    
    // Next determine its width
    int popupWidth = 0;
    for (int i = 0; i < itemCount; ++i) {
        String text = client()->itemText(i);
        if (text.isEmpty())
            continue;

        WebCore::Font itemFont = client()->menuStyle().font();
        if (client()->itemIsLabel(i)) {
            WebCore::FontDescription d = itemFont.fontDescription();
            d.setWeight(d.bolderWeight());
            itemFont = WebCore::Font(d, itemFont.letterSpacing(), itemFont.wordSpacing());
            itemFont.update(client()->fontSelector());
        }

        popupWidth = std::max(popupWidth, static_cast<int>(ceilf(itemFont.width(WebCore::TextRun(text.characters(), text.length())))));
    }

    if (naturalHeight > maxPopupHeight)
        // We need room for a scrollbar
        popupWidth += WebCore::ScrollbarTheme::theme()->scrollbarThickness(WebCore::SmallScrollbar);

    // Add padding to align the popup text with the <select> text
    popupWidth += std::max(0, client()->clientPaddingRight() - client()->clientInsetRight()) + std::max(0, client()->clientPaddingLeft() - client()->clientInsetLeft());

    // Leave room for the border
    popupWidth += 2 * popupWindowBorderWidth;
    popupHeight += 2 * popupWindowBorderWidth;

    // The popup should be at least as wide as the control on the page
    popupWidth = std::max(rScreenCoords.width() - client()->clientInsetLeft() - client()->clientInsetRight(), popupWidth);

    // Always left-align items in the popup.  This matches popup menus on the mac.
    int popupX = rScreenCoords.x() + client()->clientInsetLeft();

    WebCore::IntRect popupRect(popupX, rScreenCoords.maxY(), popupWidth, popupHeight);

    IWebView* webView = (IWebView*)chromeClient_->webView();
    // The popup needs to stay within the bounds of the screen and not overlap any toolbars
    WebCore::FloatRect screen = WebCore::FloatRect(0, 0, (float)webView->width(), (float)webView->height());

    // Check that we don't go off the screen vertically
    if (popupRect.maxY() > screen.height()) {
        // The popup will go off the screen, so try placing it above the client
        if (rScreenCoords.y() - popupRect.height() < 0) {
            // The popup won't fit above, either, so place it whereever's bigger and resize it to fit
            if ((rScreenCoords.y() + rScreenCoords.height() / 2) < (screen.height() / 2)) {
                // Below is bigger
                popupRect.setHeight(screen.height() - popupRect.y());
            } else {
                // Above is bigger
                popupRect.setY(20);
                popupRect.setHeight(rScreenCoords.y());
            }
        } else {
            // The popup fits above, so reposition it
            popupRect.setY(rScreenCoords.y() - popupRect.height());
        }
    }

    // Check that we don't go off the screen horizontally
    if (popupRect.x() < screen.x()) {
        popupRect.setWidth(popupRect.width() - (screen.x() - popupRect.x()));
        popupRect.setX(screen.x());
    }
    windowRect_ = popupRect;
    return;
}

bool PopupMenu::setFocusedIndex(int i, bool hotTracking)
{
    if (i < 0 || i >= client()->listSize() || i == focusedIndex())
        return false;

    if (!client()->itemIsEnabled(i))
        return false;

    invalidate();

    focusedIndex_ = i;

    if (!hotTracking)
        client()->setTextFromItem(i);

    scrollToRevealSelection();
    return true;
}

int PopupMenu::visibleItems() const
{
    return clientRect().height() / itemHeight_;
}

int PopupMenu::listIndexAtPoint(const WebCore::IntPoint& point) const
{
    return scrollOffset_ + point.y() / itemHeight_;
}

int PopupMenu::focusedIndex() const
{
    return focusedIndex_;
}

void PopupMenu::focusFirst()
{
    if (!client())
        return;

    int size = client()->listSize();

    for (int i = 0; i < size; ++i)
        if (client()->itemIsEnabled(i)) {
            setFocusedIndex(i);
            break;
        }
}

void PopupMenu::focusLast()
{
    if (!client())
        return;

    int size = client()->listSize();

    for (int i = size - 1; i > 0; --i)
        if (client()->itemIsEnabled(i)) {
            setFocusedIndex(i);
            break;
        }
}

bool PopupMenu::down(unsigned lines)
{
    if (!client())
        return false;

    int size = client()->listSize();

    int lastSelectableIndex, selectedListIndex;
    lastSelectableIndex = selectedListIndex = focusedIndex();
    for (int i = selectedListIndex + 1; i >= 0 && i < size; ++i)
        if (client()->itemIsEnabled(i)) {
            lastSelectableIndex = i;
            if (i >= selectedListIndex + (int)lines)
                break;
        }

    return setFocusedIndex(lastSelectableIndex);
}

bool PopupMenu::up(unsigned lines)
{
    if (!client())
        return false;

    int size = client()->listSize();

    int lastSelectableIndex, selectedListIndex;
    lastSelectableIndex = selectedListIndex = focusedIndex();
    for (int i = selectedListIndex - 1; i >= 0 && i < size; --i)
        if (client()->itemIsEnabled(i)) {
            lastSelectableIndex = i;
            if (i <= selectedListIndex - (int)lines)
                break;
        }

    return setFocusedIndex(lastSelectableIndex);
}

WebCore::IntRect PopupMenu::clientRect() const
{
    WebCore::IntRect clientRect = windowRect_;
    clientRect.inflate(-popupWindowBorderWidth);
    clientRect.setLocation(WebCore::IntPoint(0, 0));
    return clientRect;
}

void PopupMenu::incrementWheelDelta(int delta)
{
    wheelDelta_ += delta;
}

void PopupMenu::reduceWheelDelta(int delta)
{
    ASSERT(delta >= 0);
    ASSERT(delta <= abs(wheelDelta_));

    if (wheelDelta_ > 0)
        wheelDelta_ -= delta;
    else if (wheelDelta_ < 0)
        wheelDelta_ += delta;
}

bool PopupMenu::scrollToRevealSelection()
{
    if (!scrollbar_)
        return false;

    int index = focusedIndex();

    if (index < scrollOffset_) {
        ScrollableArea::scrollToYOffsetWithoutAnimation(index);
        return true;
    }

    if (index >= scrollOffset_ + visibleItems()) {
        ScrollableArea::scrollToYOffsetWithoutAnimation(index - visibleItems() + 1);
        return true;
    }

    return false;
}

void PopupMenu::updateFromElement()
{
    focusedIndex_ = client()->selectedIndex();
    invalidate();

    scrollToRevealSelection();
}

const int separatorPadding = 4;
const int separatorHeight = 1;
WebCore::IntRect PopupMenu::paint(void* bits, int pitch)
{
    if (!pixels_)
    {
        if (!hdc_)
            hdc_ = adoptPtr(::CreateCompatibleDC(0));

        WebCore::BitmapInfo bmp = WebCore::BitmapInfo::createBottomUp(windowRect_.size());
        HBITMAP hbmp = ::CreateDIBSection(0, &bmp, DIB_RGB_COLORS, &pixels_, NULL, 0);
        SelectObject(hdc_.get(), hbmp);
        hbmp_ = adoptPtr(hbmp);

        WebCore::GraphicsContext context(hdc_.get());

        int itemCount = client()->listSize();
        WebCore::IntRect listRect = windowRect_;
        listRect.setX(0);
        listRect.setY(0);
        listRect.move(0, scrollOffset_ * itemHeight_);

        for (int y = listRect.y(); y < listRect.maxY(); y += itemHeight_) {
            int index = y / itemHeight_;
            WebCore::Color optionBackgroundColor, optionTextColor;
            WebCore::PopupMenuStyle itemStyle = client()->itemStyle(index);
            if (index == focusedIndex()) {
                optionBackgroundColor = WebCore::RenderTheme::defaultTheme()->activeListBoxSelectionBackgroundColor();
                optionTextColor = WebCore::RenderTheme::defaultTheme()->activeListBoxSelectionForegroundColor();
            } else {
                optionBackgroundColor = itemStyle.backgroundColor();
                optionTextColor = itemStyle.foregroundColor();
            }

            WebCore::IntRect itemRect;
            itemRect.setX(popupWindowBorderWidth);
            itemRect.setY(popupWindowBorderWidth + (index - scrollOffset_) * itemHeight_);

            if (scrollbar_)
                itemRect.setWidth(windowRect_.width() - popupWindowBorderWidth * 2 - scrollbar_->frameRect().width());
            else
                itemRect.setWidth(windowRect_.width() - popupWindowBorderWidth * 2);

            itemRect.setHeight(itemHeight_);

            if (itemStyle.isVisible())
                context.fillRect(itemRect, optionBackgroundColor, WebCore::ColorSpaceDeviceRGB);

            if (client()->itemIsSeparator(index)) {
                WebCore::IntRect separatorRect(itemRect.x() + separatorPadding, itemRect.y() + (itemRect.height() - separatorHeight) / 2, itemRect.width() - 2 * separatorPadding, separatorHeight);
                context.fillRect(separatorRect, optionTextColor, WebCore::ColorSpaceDeviceRGB);
                continue;
            }

            String itemText = client()->itemText(index);

            unsigned length = itemText.length();
            const UChar* string = itemText.characters();
            WebCore::TextDirection direction = (itemText.defaultWritingDirection() == WTF::Unicode::RightToLeft) ? WebCore::RTL : WebCore::LTR;

            WebCore::TextRun textRun(string, length, false, 0, 0, WebCore::TextRun::AllowTrailingExpansion, direction);
            context.setFillColor(optionTextColor, WebCore::ColorSpaceDeviceRGB);

            WebCore::Font itemFont = client()->menuStyle().font();
            if (client()->itemIsLabel(index)) {
                WebCore::FontDescription d = itemFont.fontDescription();
                d.setWeight(d.bolderWeight());
                itemFont = WebCore::Font(d, itemFont.letterSpacing(), itemFont.wordSpacing());
                itemFont.update(client()->fontSelector());
            }

            // Draw the item text
            if (itemStyle.isVisible()) {
                int textX = std::max(0, client()->clientPaddingLeft() - client()->clientInsetLeft());
                if (WebCore::RenderTheme::defaultTheme()->popupOptionSupportsTextIndent() && itemStyle.textDirection() == WebCore::LTR)
                    textX += itemStyle.textIndent().calcMinValue(itemRect.width());
                int textY = itemRect.y() + itemFont.fontMetrics().ascent() + (itemRect.height() - itemFont.fontMetrics().height()) / 2;
                context.drawBidiText(itemFont, textRun, WebCore::IntPoint(textX, textY));
            }
        }

        if (scrollbar_)
        {
            WebCore::IntRect rect = windowRect_;
            rect.setX(0);
            rect.setY(0);
            scrollbar_->paint(&context, rect);
        }

        //border
        WebCore::FloatRect rect = windowRect_;
        rect.setX(0);
        rect.setY(0);
        context.setStrokeColor(WebCore::Color::gray, WebCore::ColorSpaceDeviceRGB);
        context.strokeRect(rect, (float)popupWindowBorderWidth);
    }

    //copy to dst
    int w = windowRect_.width();
    int h = windowRect_.height();
    unsigned char* src = (unsigned char*)pixels_; 
    unsigned char* dst = (unsigned char*)bits + windowRect_.y() * pitch + windowRect_.x()*4;

    for (int i = 0; i < h; ++i)
    {
        memcpy(dst, src, w*4);
        src += w*4;
        dst += pitch;
    }
    return windowRect_;
}

bool PopupMenu::mouseEvent(const WebCore::PlatformMouseEvent& mouseEvent)
{
    if (scrollbar()) {

        WebCore::IntPoint mousePoint = mouseEvent.pos();
        mousePoint.move(-windowRect_.x(), -windowRect_.y());

        WebCore::IntRect scrollBarRect = scrollbar()->frameRect();
        if (mouseEvent.eventType() == WebCore::MouseEventMoved)
        {
            if (scrollbarCapturingMouse() || scrollBarRect.contains(mousePoint)) {
                // Put the point into coordinates relative to the scroll bar
                mousePoint.move(-scrollBarRect.x(), -scrollBarRect.y());
                WebCore::PlatformMouseEvent event(mousePoint, mousePoint, mouseEvent.button(), mouseEvent.eventType(), mouseEvent.clickCount(), 
                    mouseEvent.shiftKey(), mouseEvent.ctrlKey(), mouseEvent.altKey(), mouseEvent.metaKey(), mouseEvent.timestamp());

                scrollbar()->mouseMoved(event);
                return true;
            }
        }
        else if (mouseEvent.eventType() == WebCore::MouseEventPressed)
        {
            if (scrollBarRect.contains(mousePoint)) {
                // Put the point into coordinates relative to the scroll bar
                mousePoint.move(-scrollBarRect.x(), -scrollBarRect.y());
                WebCore::PlatformMouseEvent event(mousePoint, mousePoint, mouseEvent.button(), mouseEvent.eventType(), mouseEvent.clickCount(), 
                    mouseEvent.shiftKey(), mouseEvent.ctrlKey(), mouseEvent.altKey(), mouseEvent.metaKey(), mouseEvent.timestamp());

                scrollbar()->mouseDown(event);
                setScrollbarCapturingMouse(true);
                return true;
            }
        }
        else if (mouseEvent.eventType() == WebCore::MouseEventReleased)
        {
            if (scrollbarCapturingMouse() || scrollBarRect.contains(mousePoint)) {
                // Put the point into coordinates relative to the scroll bar
                mousePoint.move(-scrollBarRect.x(), -scrollBarRect.y());
                WebCore::PlatformMouseEvent event(mousePoint, mousePoint, mouseEvent.button(), mouseEvent.eventType(), mouseEvent.clickCount(), 
                    mouseEvent.shiftKey(), mouseEvent.ctrlKey(), mouseEvent.altKey(), mouseEvent.metaKey(), mouseEvent.timestamp());

                scrollbar()->mouseUp();
                setScrollbarCapturingMouse(false);
                return true;
            }
        }
    }

    WebCore::IntPoint mousePoint = mouseEvent.pos();
    if (mouseEvent.eventType() == WebCore::MouseEventMoved)
    {
        RECT bounds = windowRect_;
        if (::PtInRect(&bounds, mousePoint))
        {
            mousePoint.move(-windowRect_.x(), -windowRect_.y());
            setFocusedIndex(listIndexAtPoint(mousePoint), true);
            return true;
        }

        return false;
    }
    
    if (mouseEvent.eventType() == WebCore::MouseEventPressed)
    {
        // If the mouse is inside the window, update the focused index. Otherwise, 
        // hide the popup.

        RECT bounds = windowRect_;
        if (::PtInRect(&bounds, mousePoint))
        {
            mousePoint.move(-windowRect_.x(), -windowRect_.y());
            setFocusedIndex(listIndexAtPoint(mousePoint), true);
            return true;
        }

        hide();
        return true;
    }
    
    if (mouseEvent.eventType() == WebCore::MouseEventReleased)
    {
        // Only hide the popup if the mouse is inside the popup window.
        RECT bounds = windowRect_;
        if (client() && ::PtInRect(&bounds, mousePoint)) {
            hide();
            int index = focusedIndex();
            if (index >= 0)
                client()->valueChanged(index);

            return true;
        }

        return false;
    }


    return false;
}

bool PopupMenu::mouseWheel(const WebCore::PlatformWheelEvent& wheelEvent)
{
    if (!scrollbar())
        return false;

    int i = 0;
    for (incrementWheelDelta(wheelEvent.deltaY()); abs(wheelDelta()) >= WHEEL_DELTA; reduceWheelDelta(WHEEL_DELTA)) {
        if (wheelDelta() > 0)
            ++i;
        else
            --i;
    }

    WebCore::ScrollableArea::scroll(i > 0 ? WebCore::ScrollUp : WebCore::ScrollDown, WebCore::ScrollByLine, abs(i));
    return true;
}

bool PopupMenu::keyDown(const WebCore::PlatformKeyboardEvent& keyEvent)
{
    if (!client())
        return false;

    switch (keyEvent.windowsVirtualKeyCode())
    {
        case VK_F4:
            if (!keyEvent.altKey() && !keyEvent.ctrlKey()) {
                int index = focusedIndex();
                ASSERT(index >= 0);
                client()->valueChanged(index);
                hide();
            }
            break;

        case VK_DOWN:
            if (keyEvent.altKey()) {
                int index = focusedIndex();
                ASSERT(index >= 0);
                client()->valueChanged(index);
                hide();
            } else
                down();
            break;

        case VK_RIGHT:
            down();
            break;

        case VK_UP:
            if (keyEvent.altKey()) {
                int index = focusedIndex();
                ASSERT(index >= 0);
                client()->valueChanged(index);
                hide();
            } else {
                up();
            }
            break;

        case VK_LEFT:
            up();
            break;

        case VK_HOME:
            focusFirst();
            break;

        case VK_END:
            focusLast();
            break;

        case VK_PRIOR:
            if (focusedIndex() != scrollOffset()) {
                // Set the selection to the first visible item
                int firstVisibleItem = scrollOffset();
                up(focusedIndex() - firstVisibleItem);
            } else {
                // The first visible item is selected, so move the selection back one page
                up(visibleItems());
            }
            break;

        case VK_NEXT: {
            int lastVisibleItem = scrollOffset() + visibleItems() - 1;
            if (focusedIndex() != lastVisibleItem) {
                // Set the selection to the last visible item
                down(lastVisibleItem - focusedIndex());
            } else {
                // The last visible item is selected, so move the selection forward one page
                down(visibleItems());
            }
            break;
                      }
        case VK_TAB:
            hide();
            break;

        case VK_ESCAPE:
            hide();
            break;
    }

    return true;
}

bool PopupMenu::keyPress(const WebCore::PlatformKeyboardEvent& keyEvent)
{
    if (!client())
        return false;

    int index;
    switch (keyEvent.text()[0]) {
        case 0x0D:   // Enter/Return
            hide();
            index = focusedIndex();
            ASSERT(index >= 0);
            client()->valueChanged(index);
            break;
        
        case 0x1B:   // Escape
            hide();
            break;
    }

    return true;
}

int PopupMenu::scrollSize(WebCore::ScrollbarOrientation orientation) const
{
    return ((orientation == WebCore::VerticalScrollbar) && scrollbar_) ? (scrollbar_->totalSize() - scrollbar_->visibleSize()) : 0;
}

int PopupMenu::scrollPosition(WebCore::Scrollbar*) const
{
    return scrollOffset_;
}

void PopupMenu::setScrollOffset(const WebCore::IntPoint& offset)
{
    scrollTo(offset.y());
}

void PopupMenu::scrollTo(int offset)
{
    ASSERT(scrollbar_);
    if (scrollOffset_ == offset)
        return;

    int scrolledLines = scrollOffset_ - offset;
    scrollOffset_ = offset;

    invalidate();
}

void PopupMenu::invalidate()
{
    IWebView* webView = (IWebView*)chromeClient_->webView();
    webView->setDirty(true);
    webView->addDirtyArea(windowRect_.location().x(),windowRect_.location().y(),windowRect_.size().width(),windowRect_.size().height());
    pixels_ = NULL;
}

}