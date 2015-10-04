
//////////////////////////////////////////////////////////////////////////


#include <WebCore/config.h>
#include <WebCore/FrameView.h>
#include <WebCore/TextRun.h>
#include <WebCore/ScrollbarTheme.h>
#include <WebCore/BitmapInfo.h>
#include <WebCore/RenderTheme.h>
#include <WebCore/FileChooser.h>
#include <WebCore/PlatformWheelEvent.h>
#include <WebCore/PlatformKeyboardEvent.h>

#include "wkeChromeClient.h"
#include "wkePopupMenu.h"
#include "wkeWebView.h"


//////////////////////////////////////////////////////////////////////////


// Maximum height of a popup window
static const int maxPopupHeight = 320;
static const int optionSpacingMiddle = 1;
static const int popupWindowBorderWidth = 1;


namespace wke
{




PopupMenu::PopupMenu(WebCore::PopupMenuClient* client, ChromeClient* chromeClient)
    :m_popupClient(client)
    ,m_chromeClient(chromeClient)
    ,m_scrollOffset(0)
    ,m_scrollbar(0)
    ,m_focusedIndex(0)
    ,m_showPopup(false)
    ,m_wheelDelta(0)
    ,m_scrollbarCapturingMouse(false)
    ,m_pixels(NULL)
{
}

PopupMenu::~PopupMenu()
{
    if (m_scrollbar)
        m_scrollbar->setParent(0);

    hide();
}

void PopupMenu::disconnectClient()
{
    m_popupClient = 0;
}

void PopupMenu::show(const WebCore::IntRect& r, WebCore::FrameView* view, int index)
{
	m_showPopup = true;
    calculatePositionAndSize(r, view);
    if (clientRect().isEmpty())
        return;

    if (!m_scrollbar && visibleItems() < client()->listSize()) {
        // We need a scroll bar
        m_scrollbar = client()->createScrollbar(this, WebCore::VerticalScrollbar, WebCore::SmallScrollbar);
        m_scrollbar->styleChanged();

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

    m_chromeClient->setPopupMenu(this);
    invalidate();
}

void PopupMenu::hide()
{
    if (!m_showPopup)
        return;

    m_showPopup = false;
    if (client())
        client()->popupDidHide();

    if (m_chromeClient->popupMenu() == this)
        m_chromeClient->setPopupMenu(NULL);

    invalidate();
}

void PopupMenu::calculatePositionAndSize(const WebCore::IntRect& r, WebCore::FrameView* v)
{
    // r is in absolute document coordinates, but we want to be in screen coordinates

    // First, move to WebView coordinates
    WebCore::IntRect rScreenCoords(v->contentsToWindow(r.location()), r.size());

    // First, determine the popup's height
    int itemCount = client()->listSize();
    m_itemHeight = client()->menuStyle().font().fontMetrics().height() + optionSpacingMiddle;
    int naturalHeight = m_itemHeight * itemCount;
    int popupHeight = std::min(maxPopupHeight, naturalHeight);
    // The popup should show an integral number of items (i.e. no partial items should be visible)
    popupHeight -= popupHeight % m_itemHeight;
    
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

    CWebView* webView = (CWebView*)m_chromeClient->webView();
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
    m_windowRect = popupRect;
    return;
}

bool PopupMenu::setFocusedIndex(int i, bool hotTracking)
{
    if (i < 0 || i >= client()->listSize() || i == focusedIndex())
        return false;

    if (!client()->itemIsEnabled(i))
        return false;

    invalidate();

    m_focusedIndex = i;

    if (!hotTracking)
        client()->setTextFromItem(i);

    scrollToRevealSelection();
    return true;
}

int PopupMenu::visibleItems() const
{
    return clientRect().height() / m_itemHeight;
}

int PopupMenu::listIndexAtPoint(const WebCore::IntPoint& point) const
{
    return m_scrollOffset + point.y() / m_itemHeight;
}

int PopupMenu::focusedIndex() const
{
    return m_focusedIndex;
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
    WebCore::IntRect clientRect = m_windowRect;
    clientRect.inflate(-popupWindowBorderWidth);
    clientRect.setLocation(WebCore::IntPoint(0, 0));
    return clientRect;
}

void PopupMenu::incrementWheelDelta(int delta)
{
    m_wheelDelta += delta;
}

void PopupMenu::reduceWheelDelta(int delta)
{
    ASSERT(delta >= 0);
    ASSERT(delta <= abs(m_wheelDelta));

    if (m_wheelDelta > 0)
        m_wheelDelta -= delta;
    else if (m_wheelDelta < 0)
        m_wheelDelta += delta;
}

bool PopupMenu::scrollToRevealSelection()
{
    if (!m_scrollbar)
        return false;

    int index = focusedIndex();

    if (index < m_scrollOffset) {
        ScrollableArea::scrollToYOffsetWithoutAnimation(index);
        return true;
    }

    if (index >= m_scrollOffset + visibleItems()) {
        ScrollableArea::scrollToYOffsetWithoutAnimation(index - visibleItems() + 1);
        return true;
    }

    return false;
}

void PopupMenu::updateFromElement()
{
    m_focusedIndex = client()->selectedIndex();
    invalidate();

    scrollToRevealSelection();
}

const int separatorPadding = 4;
const int separatorHeight = 1;
WebCore::IntRect PopupMenu::paint(void* bits, int pitch)
{
    if (!m_pixels)
    {
        if (!m_hdc)
            m_hdc = adoptPtr(::CreateCompatibleDC(0));

        WebCore::BitmapInfo bmp = WebCore::BitmapInfo::createBottomUp(m_windowRect.size());
        HBITMAP hbmp = ::CreateDIBSection(0, &bmp, DIB_RGB_COLORS, &m_pixels, NULL, 0);
        SelectObject(m_hdc.get(), hbmp);
        m_hbitmap = adoptPtr(hbmp);

        WebCore::GraphicsContext context(m_hdc.get());

        int itemCount = client()->listSize();
        WebCore::IntRect listRect = m_windowRect;
        listRect.setX(0);
        listRect.setY(0);
        listRect.move(0, m_scrollOffset * m_itemHeight);

        for (int y = listRect.y(); y < listRect.maxY(); y += m_itemHeight) {
            int index = y / m_itemHeight;
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
            itemRect.setY(popupWindowBorderWidth + (index - m_scrollOffset) * m_itemHeight);

            if (m_scrollbar)
                itemRect.setWidth(m_windowRect.width() - popupWindowBorderWidth * 2 - m_scrollbar->frameRect().width());
            else
                itemRect.setWidth(m_windowRect.width() - popupWindowBorderWidth * 2);

            itemRect.setHeight(m_itemHeight);

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

        if (m_scrollbar)
        {
            WebCore::IntRect rect = m_windowRect;
            rect.setX(0);
            rect.setY(0);
            m_scrollbar->paint(&context, rect);
        }

        //border
        WebCore::FloatRect rect = m_windowRect;
        rect.setX(0);
        rect.setY(0);
        context.setStrokeColor(WebCore::Color::gray, WebCore::ColorSpaceDeviceRGB);
        context.strokeRect(rect, (float)popupWindowBorderWidth);
    }

    //copy to dst
    int w = m_windowRect.width();
    int h = m_windowRect.height();
    unsigned char* src = (unsigned char*)m_pixels; 
    unsigned char* dst = (unsigned char*)bits + m_windowRect.y() * pitch + m_windowRect.x()*4;

    for (int i = 0; i < h; ++i)
    {
        memcpy(dst, src, w*4);
        src += w*4;
        dst += pitch;
    }
    return m_windowRect;
}

bool PopupMenu::mouseEvent(const WebCore::PlatformMouseEvent& mouseEvent)
{
    if (scrollbar()) {

        WebCore::IntPoint mousePoint = mouseEvent.pos();
        mousePoint.move(-m_windowRect.x(), -m_windowRect.y());

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
        RECT bounds = m_windowRect;
        if (::PtInRect(&bounds, mousePoint))
        {
            mousePoint.move(-m_windowRect.x(), -m_windowRect.y());
            setFocusedIndex(listIndexAtPoint(mousePoint), true);
            return true;
        }

        return false;
    }
    
    if (mouseEvent.eventType() == WebCore::MouseEventPressed)
    {
        // If the mouse is inside the window, update the focused index. Otherwise, 
        // hide the popup.

        RECT bounds = m_windowRect;
        if (::PtInRect(&bounds, mousePoint))
        {
            mousePoint.move(-m_windowRect.x(), -m_windowRect.y());
            setFocusedIndex(listIndexAtPoint(mousePoint), true);
            return true;
        }

        hide();
        return true;
    }
    
    if (mouseEvent.eventType() == WebCore::MouseEventReleased)
    {
        // Only hide the popup if the mouse is inside the popup window.
        RECT bounds = m_windowRect;
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
    return ((orientation == WebCore::VerticalScrollbar) && m_scrollbar) ? (m_scrollbar->totalSize() - m_scrollbar->visibleSize()) : 0;
}

int PopupMenu::scrollPosition(WebCore::Scrollbar*) const
{
    return m_scrollOffset;
}

void PopupMenu::setScrollOffset(const WebCore::IntPoint& offset)
{
    scrollTo(offset.y());
}

void PopupMenu::scrollTo(int offset)
{
    ASSERT(m_scrollbar);
    if (m_scrollOffset == offset)
        return;

    int scrolledLines = m_scrollOffset - offset;
    m_scrollOffset = offset;

    invalidate();
}

void PopupMenu::invalidate()
{
    CWebView* webView = (CWebView*)m_chromeClient->webView();
    webView->setDirty(true);
    webView->addDirtyArea(m_windowRect.location().x(),m_windowRect.location().y(),m_windowRect.size().width(),m_windowRect.size().height());
    m_pixels = NULL;
}




};//namespace wke;