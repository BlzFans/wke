
#include "wkeChromeClient.h"
#include "wkeFrameLoaderClient.h"
#include "wkeContextMenuClient.h"
#include "wkeInspectorClient.h"
#include "wkeEditorClient.h"
#include "wkeDragClient.h"

#include "icuwin.h"

//cexer: 必须包含在后面，因为其中的 wke.h -> windows.h 会定义 max、min，导致 WebCore 内部的 max、min 出现错乱。
#include "wkeDebug.h"
#include "wkeWebView.h"

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")


namespace wke
{



CWebView::CWebView()
    : m_name("")
    , m_transparent(false)
    , m_dirty(false)
    , m_width(0)
    , m_height(0)
    , m_graphicsContext(NULL)
    , m_awake(true)
    , m_title("")
    , m_cookie("")
    , m_hostWindow(NULL)
{
    _initHandler();
    _initPage();
    _initMemoryDC();
}

CWebView::~CWebView()
{
    delete m_graphicsContext;
    m_mainFrame->loader()->detachFromParent();
}

bool CWebView::create()
{
    return true;
}

void CWebView::destroy()
{
    delete this;
}

const utf8* CWebView::name() const
{
    return m_name.string();
}

const wchar_t* CWebView::nameW() const
{
    return m_name.stringW();
}

void CWebView::setName(const utf8* name)
{
    m_name.assign(name);
}
void CWebView::setName(const wchar_t* name)
{
    m_name.assign(name);
}

bool CWebView::isTransparent() const
{
    return m_transparent;
}

void CWebView::setTransparent(bool transparent)
{
    if (m_transparent == transparent)
        return;

    m_transparent = transparent;
    m_dirtyArea = WebCore::IntRect(0, 0, m_width, m_height);
    setDirty(true);

    if (m_graphicsContext)
    {
        delete m_graphicsContext;
        m_graphicsContext = NULL;
    }

    WebCore::Color backgroundColor = transparent ? WebCore::Color::transparent : WebCore::Color::white;
    m_mainFrame->view()->updateBackgroundRecursively(backgroundColor, transparent);
}

void CWebView::loadPostURL(const utf8* inUrl,const char * poastData,int nLen )
{
	WebCore::KURL url(WebCore::KURL(), WTF::String::fromUTF8(inUrl), WebCore::UTF8Encoding());
	if (!url.isValid())
		url.setProtocol("http:");

	if (!url.isValid())
		return;

	if (WebCore::protocolIsJavaScript(url))
	{
		m_mainFrame->script()->executeIfJavaScriptURL(url);
		return;
	}

	WebCore::ResourceRequest request(url);
	request.setCachePolicy(WebCore::UseProtocolCachePolicy);
	request.setTimeoutInterval(60.f);
	request.setHTTPMethod("POST");
	request.setHTTPBody(WebCore::FormData::create(poastData, nLen));
	m_mainFrame->loader()->load(request, false);
}

void CWebView::loadPostURL(const wchar_t * inUrl,const char * poastData,int nLen )
{
   loadPostURL(String(inUrl).utf8().data(),poastData,nLen);
}

void CWebView::loadURL(const utf8* inUrl)
{
    //cexer 必须调用String::fromUTF8显示构造第二个参数，否则String::String会把inUrl当作latin1处理。
    //WebCore::KURL url(WebCore::KURL(), inUrl, WebCore::UTF8Encoding());
    WebCore::KURL url(WebCore::KURL(), WTF::String::fromUTF8(inUrl), WebCore::UTF8Encoding());
    if (!url.isValid())
        url.setProtocol("http:");

    if (!url.isValid())
        return;

    if (WebCore::protocolIsJavaScript(url))
    {
        m_mainFrame->script()->executeIfJavaScriptURL(url);
        return;
    }

    WebCore::ResourceRequest request(url);
    request.setCachePolicy(WebCore::UseProtocolCachePolicy);
    request.setTimeoutInterval(60.f);
    request.setHTTPMethod("GET");
    m_mainFrame->loader()->load(request, false);
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

    m_mainFrame->loader()->load(request, substituteData, false);
}

void CWebView::loadHTML(const wchar_t* html)
{
    String mime = "text/html";

    RefPtr<WebCore::SharedBuffer> sharedBuffer = WebCore::SharedBuffer::create((const char*)html, wcslen(html)*2);

    WebCore::KURL url(WebCore::KURL(), "");
    WebCore::ResourceRequest request(url);
    WebCore::SubstituteData substituteData(sharedBuffer.release(), mime, WebCore::UTF16LittleEndianEncoding().name(), url);

    m_mainFrame->loader()->load(request, substituteData, false);
}

void CWebView::loadFile(const utf8* filename)
{
    char url[4096+1] = { 0 };
    _snprintf(url, 4096, "file:///%s", filename);
    url[4096] = '\0';
    loadURL(url);
}

void CWebView::loadFile(const wchar_t* filename)
{
    wchar_t url[4096+1] = { 0 };
    _snwprintf(url, 4096, L"file:///%s", filename);
    url[4096] = L'\0';
    loadURL(url);
}

void CWebView::load(const utf8* str)
{
    wchar_t* wstr = NULL;
    int wlen = MultiByteToWideChar(CP_UTF8, 0, str, strlen(str), NULL, 0);
    wstr = new wchar_t[wlen + 1];
    MultiByteToWideChar(CP_UTF8, 0, str, strlen(str), wstr, wlen);
    wstr[wlen] = 0;

    load(wstr);

    delete [] wstr;
}



LPCWSTR GetWorkingDirectory(LPWSTR buffer, size_t bufferSize)
{
    GetCurrentDirectoryW(bufferSize, buffer);
    wcscat(buffer, L"\\");
    return buffer;
}

LPCWSTR GetWorkingPath(LPWSTR buffer, size_t bufferSize, LPCWSTR relatedPath)
{
    WCHAR dir[MAX_PATH + 1] = { 0 };
    GetWorkingDirectory(dir, MAX_PATH);
    _snwprintf(buffer, bufferSize, L"%s%s", dir, relatedPath);
    return buffer;
}

LPCWSTR FormatWorkingPath(LPWSTR buffer, size_t bufferSize, LPCWSTR fmt, ...)
{
    WCHAR relatedPath[MAX_PATH + 1] = { 0 };
    va_list args;
    va_start(args, fmt);
    _vsnwprintf(relatedPath, MAX_PATH, fmt, args);
    va_end(args);

    return GetWorkingPath(buffer, bufferSize, relatedPath);
}

LPCWSTR GetProgramDirectory(LPWSTR buffer, size_t bufferSize)
{
    DWORD i = GetModuleFileNameW(NULL, buffer, bufferSize);

    -- i;
    while (buffer[i] != '\\' && i != 0)
        -- i;

    buffer[i+1] = 0;
    return buffer;
}

LPCWSTR GetProgramPath(LPWSTR buffer, size_t bufferSize, LPCWSTR relatedPath)
{
    WCHAR dir[MAX_PATH + 1] = { 0 };
    GetProgramDirectory(dir, MAX_PATH);
    _snwprintf(buffer, bufferSize, L"%s%s", dir, relatedPath);
    return buffer;
}

LPCWSTR FormatProgramPath(LPWSTR buffer, size_t bufferSize, LPCWSTR fmt, ...)
{
    WCHAR relatedPath[MAX_PATH + 1] = { 0 };
    va_list args;
    va_start(args, fmt);
    _vsnwprintf(relatedPath, MAX_PATH, fmt, args);
    va_end(args);

    return GetProgramPath(buffer, bufferSize, relatedPath);
}

void CWebView::load(const wchar_t* str)
{
    if (!str || str[0] == 0)
    {
        loadHTML(L"no url specificed");
        return;
    }

    //有明确的 scheme，是合法的 url。
    static const wchar_t* HTTP_SCHEME = L"http://";
    static const wchar_t* HTTPS_SCHEME = L"https://";
    static const wchar_t* FILE_SCHEME = L"file:///";
    if (wcsnicmp(str, HTTP_SCHEME, wcslen(HTTP_SCHEME)) == 0 || 
        wcsnicmp(str, HTTPS_SCHEME, wcslen(HTTPS_SCHEME)) == 0 ||
        wcsnicmp(str, FILE_SCHEME, wcslen(FILE_SCHEME)) == 0)
    {
        loadURL(str);
        return;
    }

    //包含 < 和 > 不可能是路径，很有可能是 HTML。
    if (wcschr(str, L'<') && wcschr(str, L'>'))
    {
        loadHTML(str);
        return;
    }

    bool pathValid = true;

    //既不是明确的 url，也没有明确的非路径字符，当作本地路径处理。
    wchar_t fullPath[MAX_PATH + 1] = { 0 };

    //路径中包含 : 字符，说明是绝对路径
    if (wcschr(str, L':'))
    {
        int strLen = wcslen(str);
        wcsncpy(fullPath, str, strLen < MAX_PATH ? strLen : MAX_PATH);
    }
    //相对路径，需要补全为绝对路径。
    else
    {
        do
        {
            //从工作目录中找
            GetWorkingPath(fullPath, MAX_PATH, str);
            if (PathFileExistsW(fullPath))
                break;

            //从EXE目录中找
            GetProgramPath(fullPath, MAX_PATH, str);
            if (PathFileExistsW(fullPath))
                break;

            pathValid = false;
        }
        while (0);       
    }

    //确定是合法文件路径。
    if (pathValid)
    {
        loadFile(fullPath);
        return;
    }

    //不是合法文件路径，只能当作自定义 scheme 的 url 处理。
    loadURL(str);
}

bool CWebView::isLoadingSucceeded() const
{
    FrameLoaderClient* client = (FrameLoaderClient*)mainFrame()->loader()->client();
    return client->isLoaded();
}

bool CWebView::isLoadingFailed() const
{
    FrameLoaderClient* client = (FrameLoaderClient*)mainFrame()->loader()->client();
    return client->isLoadFailed();
}

bool CWebView::isLoadingCompleted() const
{
    return isLoadingSucceeded() || isLoadingFailed();
}

bool CWebView::isDocumentReady() const
{
    FrameLoaderClient* client = (FrameLoaderClient*)mainFrame()->loader()->client();
    return client->isDocumentReady();
}

void CWebView::setUserAgent(const utf8 * useragent)
{
    FrameLoaderClient* client = (FrameLoaderClient*)mainFrame()->loader()->client();
	client->setUserAgent( WTF::String::fromUTF8(useragent));
}

void CWebView::setUserAgent(const wchar_t * useragent )
{
      setUserAgent(String(useragent).utf8().data());
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
    if (!mainFrame()->loader()->documentLoader())
        return "notitle";

    m_title = mainFrame()->loader()->documentLoader()->title().string();
    return m_title.string();
}

const wchar_t* CWebView::titleW()
{
    if (!mainFrame()->loader()->documentLoader())
        return L"notitle";

    m_title = mainFrame()->loader()->documentLoader()->title().string();
    return m_title.stringW();
}

void CWebView::resize(int w, int h)
{
    if (w != m_width || h != m_height)
    {
        m_mainFrame->view()->resize(w, h);

        m_width = w;
        m_height = h;

        m_dirtyArea = WebCore::IntRect(0, 0, w, h);
        setDirty(true);

        if (m_graphicsContext)
        {
            delete m_graphicsContext;
            m_graphicsContext = NULL;
        }
    }
}

int CWebView::width() const 
{ 
    return m_width; 
}

int CWebView::height() const 
{ 
    return m_height; 
}

int CWebView::contentWidth() const
{
    return mainFrame()->view()->contentsWidth();
}

int CWebView::contentHeight() const
{
    return mainFrame()->view()->contentsHeight();
}

void CWebView::setDirty(bool dirty)
{
    m_dirty = dirty;
}

bool CWebView::isDirty() const
{
    return m_dirty;
}

void CWebView::addDirtyArea(int x, int y, int w, int h)
{
    if (w > 0 && h > 0)
    {
        m_dirtyArea.unite(WebCore::IntRect(x, y, w, h));
        m_dirty = true;
    }
}

void CWebView::layoutIfNeeded()
{
    m_mainFrame->view()->updateLayoutAndStyleIfNeededRecursive();
}

void CWebView::repaintIfNeeded()
{
	if(!m_dirty) return;

	layoutIfNeeded();

	if (m_graphicsContext == NULL)
	{
		WebCore::BitmapInfo bmp = WebCore::BitmapInfo::createBottomUp(WebCore::IntSize(m_width, m_height));
		HBITMAP hbmp = ::CreateDIBSection(0, &bmp, DIB_RGB_COLORS, &m_pixels, NULL, 0);
		::SelectObject(m_hdc.get(), hbmp);
		m_hbitmap = adoptPtr(hbmp);

		m_graphicsContext = new WebCore::GraphicsContext(m_hdc.get(), m_transparent);
	}

	m_graphicsContext->save();

	if (m_transparent)
		m_graphicsContext->clearRect(m_dirtyArea);

	m_graphicsContext->clip(m_dirtyArea);

	m_mainFrame->view()->paint(m_graphicsContext, m_dirtyArea);

	m_graphicsContext->restore();
    ChromeClient* client = (ChromeClient*)page()->chrome()->client();
    client->paintPopupMenu(m_pixels,  m_width*4);

	if(m_handler.paintUpdatedCallback)
	{
        WebCore::IntPoint pt = m_dirtyArea.location();
        WebCore::IntSize sz = m_dirtyArea.size();
		m_handler.paintUpdatedCallback(this, m_handler.paintUpdatedCallbackParam, m_hdc.get(),pt.x(),pt.y(),sz.width(),sz.height());	
	}
    m_dirtyArea = WebCore::IntRect();
    m_dirty = false;
}

HDC CWebView::viewDC()
{
    return m_hdc.get();
}

void CWebView::paint(void* bits, int pitch)
{
    if(m_dirty) repaintIfNeeded();

    if (pitch == 0 || pitch == m_width*4)
    {
        memcpy(bits, m_pixels, m_width*m_height*4);
    }
    else
    {
        unsigned char* src = (unsigned char*)m_pixels; 
        unsigned char* dst = (unsigned char*)bits; 
        for(int i = 0; i < m_height; ++i) 
        {
            memcpy(dst, src, m_width*4);
            src += m_width*4;
            dst += pitch;
        }
    }

}

void CWebView::paint(void* bits, int bufWid, int bufHei, int xDst, int yDst, int w, int h, int xSrc, int ySrc, bool bCopyAlpha)
{
    if(m_dirty) repaintIfNeeded();

    
    if(xSrc + w > m_width) w = m_width - xSrc;
    if(ySrc + h > m_height) h = m_height -ySrc;
    
    if(xDst + w > bufWid) w =bufWid - xDst;
    if(yDst + h > bufHei) h = bufHei - yDst;
    
    int pitchDst = bufWid*4;
    int pitchSrc = m_width*4;
    
    unsigned char* src = (unsigned char*)m_pixels; 
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
            src += (m_width - w)*4;
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

void CWebView::editorSelectAll()
{
    mainFrame()->editor()->command("SelectAll").execute();
}

void CWebView::editorCopy()
{
    page()->focusController()->focusedOrMainFrame()->editor()->command("Copy").execute();
}

void CWebView::editorCut()
{
    page()->focusController()->focusedOrMainFrame()->editor()->command("Cut").execute();
}

void CWebView::editorPaste()
{
    page()->focusController()->focusedOrMainFrame()->editor()->command("Paste").execute();
}

void CWebView::editorDelete()
{
    page()->focusController()->focusedOrMainFrame()->editor()->command("Delete").execute();
}

void CWebView::setCookieEnabled(bool enable)
{
    page()->setCookieEnabled(enable);
}

//获取cookies
const wchar_t* CWebView::cookieW()
{
	int e = 0;
	m_cookie	= mainFrame()->document()->cookie(e);
	return m_cookie.stringW();
}

const utf8* CWebView::cookie()
{
	int e = 0;
	m_cookie = mainFrame()->document()->cookie(e);
	return m_cookie.string();
}

void CWebView::setCookieW(const wchar_t* val)
{
    m_cookie = val;

    int e = 0;
    mainFrame()->document()->setCookie(val, e);
}

void CWebView::setCookie(const utf8* val)
{
    m_cookie = val;

    String string = String::fromUTF8(val);
    int e = 0;
    mainFrame()->document()->setCookie(val, e);
}

bool CWebView::isCookieEnabled() const
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

void CWebView::setHostWindow(HWND win)
{
    m_hostWindow = win;
}

HWND CWebView::hostWindow() const
{
    return m_hostWindow;
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


bool CWebView::fireMouseEvent(unsigned int message, int x, int y, unsigned int flags)
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

bool CWebView::fireContextMenuEvent(int x, int y, unsigned int flags)
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

bool CWebView::fireMouseWheelEvent(int x, int y, int wheelDelta, unsigned int flags)
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

bool CWebView::fireKeyUpEvent(unsigned int virtualKeyCode, unsigned int flags, bool systemKey)
{
    LPARAM keyData = MAKELPARAM(0, (WORD)flags);
    WebCore::PlatformKeyboardEvent keyEvent(0, virtualKeyCode, keyData, WebCore::PlatformKeyboardEvent::KeyUp, systemKey);

    WebCore::Frame* frame = page()->focusController()->focusedOrMainFrame();
    return frame->eventHandler()->keyEvent(keyEvent);
}

bool CWebView::fireKeyDownEvent(unsigned int virtualKeyCode, unsigned int flags, bool systemKey)
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

bool CWebView::fireKeyPressEvent(unsigned int charCode, unsigned int flags, bool systemKey)
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

void CWebView::setFocus()
{
    WebCore::FocusController* focusController = page()->focusController();
    focusController->setFocused(true);
}

void CWebView::killFocus()
{
    WebCore::FocusController* focusController = page()->focusController();
    focusController->setFocused(false);
}

wkeRect CWebView::caretRect()
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
    WebCore::ScriptValue value = m_mainFrame->script()->executeScript(string, true);
    if (value.hasNoValue())
        return jsUndefined();

    return JSC::JSValue::encode(value.jsValue());
}

jsValue CWebView::runJS(const utf8* script)
{
    String string = String::fromUTF8(script);
    WebCore::ScriptValue value = m_mainFrame->script()->executeScript(string, true);
    if (value.hasNoValue())
        return jsUndefined();

    return JSC::JSValue::encode(value.jsValue());
}

jsExecState CWebView::globalExec()
{
    return m_mainFrame->script()->globalObject(WebCore::mainThreadNormalWorld())->globalExec();
}

void CWebView::sleep()
{
    m_awake = false;
    page()->setCanStartMedia(false);
    page()->willMoveOffscreen();
}

void CWebView::wake()
{
    m_awake = true;
    page()->didMoveOnscreen();
    page()->setCanStartMedia(true);
}

bool CWebView::isAwake() const
{
    return m_awake;
}

void CWebView::setZoomFactor(float factor)
{
    m_mainFrame->setPageZoomFactor(factor);
}

float CWebView::zoomFactor() const
{
    return m_mainFrame->pageZoomFactor();
}

void CWebView::setEditable(bool editable)
{
    if (page()->isEditable() != editable)
    {
        page()->setEditable(editable);
        page()->setTabKeyCyclesThroughElements(!editable);
        if (editable)
            m_mainFrame->editor()->applyEditingStyleToBodyElement();
    }
}

void CWebView::onTitleChanged(wkeTitleChangedCallback callback, void* callbackParam)
{
    m_handler.titleChangedCallback = callback;
    m_handler.titleChangedCallbackParam = callbackParam;
}

void CWebView::onURLChanged(wkeURLChangedCallback callback, void* callbackParam)
{
    m_handler.urlChangedCallback = callback;
    m_handler.urlChangedCallbackParam = callbackParam;
}

void CWebView::onPaintUpdated(wkePaintUpdatedCallback callback, void* callbackParam)
{
    m_handler.paintUpdatedCallback = callback;
    m_handler.paintUpdatedCallbackParam = callbackParam;
}

void CWebView::onAlertBox(wkeAlertBoxCallback callback, void* callbackParam)
{
    m_handler.alertBoxCallback = callback;
    m_handler.alertBoxCallbackParam = callbackParam;
}

void CWebView::onConfirmBox(wkeConfirmBoxCallback callback, void* callbackParam)
{
    m_handler.confirmBoxCallback = callback;
    m_handler.confirmBoxCallbackParam = callbackParam;
}

void CWebView::onPromptBox(wkePromptBoxCallback callback, void* callbackParam)
{
    m_handler.promptBoxCallback = callback;
    m_handler.promptBoxCallbackParam = callbackParam;
}

void defaultRunAlertBox(wkeWebView webView, void* param, const wkeString msg)
{
    MessageBoxW(NULL, wkeGetStringW(msg), L"wke", MB_OK);
}

bool defaultRunConfirmBox(wkeWebView webView, void* param, const wkeString msg)
{
    int result = MessageBoxW(NULL, wkeGetStringW(msg), L"wke", MB_OKCANCEL);
    return result == IDOK;
}

bool defaultRunPromptBox(wkeWebView webView, void* param, const wkeString msg, const wkeString defaultResult, wkeString result)
{
    return false;
}

void CWebView::_initHandler()
{
    memset(&m_handler, 0, sizeof(m_handler));
    m_handler.alertBoxCallback = defaultRunAlertBox;
    m_handler.confirmBoxCallback = defaultRunConfirmBox;
    m_handler.promptBoxCallback = defaultRunPromptBox;
}

void CWebView::_initPage()
{
    WebCore::Page::PageClients pageClients;
    pageClients.chromeClient = new ChromeClient(this);
    pageClients.contextMenuClient = new ContextMenuClient;
    pageClients.inspectorClient = new InspectorClient;
    pageClients.editorClient = new EditorClient;
    pageClients.dragClient = new DragClient;

    m_page = adoptPtr(new WebCore::Page(pageClients));
    WebCore::Settings* settings = m_page->settings();
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

    FrameLoaderClient* loader = new FrameLoaderClient(this, m_page.get());
    m_mainFrame = WebCore::Frame::create(m_page.get(), NULL, loader).get();
    loader->setFrame(m_mainFrame);
    m_mainFrame->init();

    page()->focusController()->setActive(true);
}

void CWebView::_initMemoryDC()
{
    m_hdc = adoptPtr(::CreateCompatibleDC(0));
}

void CWebView::onNavigation(wkeNavigationCallback callback, void* callbackParam)
{
    m_handler.navigationCallback = callback;
    m_handler.navigationCallbackParam = callbackParam;
}

void CWebView::onCreateView(wkeCreateViewCallback callback, void* callbackParam)
{
    m_handler.createViewCallback = callback;
    m_handler.createViewCallbackParam = callbackParam;
}

void CWebView::onConsoleMessage(wkeConsoleMessageCallback callback, void* callbackParam)
{
    m_handler.consoleMessageCallback = callback;
    m_handler.consoleMessageCallbackParam = callbackParam;
}

void CWebView::onLoadingFinish(wkeLoadingFinishCallback callback, void* callbackParam)
{
    m_handler.loadingFinishCallback = callback;
    m_handler.loadingFinishCallbackParam = callbackParam;
}

void CWebView::onDocumentReady(wkeDocumentReadyCallback callback, void* callbackParam)
{
    m_handler.documentReadyCallback = callback;
    m_handler.documentReadyCallbackParam = callbackParam;
}





};//namespace wke





//static Vector<wke::CWebView*> s_webViews;

wkeWebView wkeCreateWebView()
{
    wke::CWebView* webView = new wke::CWebView;
    //s_webViews.append(webView);
    return webView;
}

//wkeWebView wkeGetWebView(const char* name)
//{
//    for (size_t i = 0; i < s_webViews.size(); ++i)
//    {
//        if (strcmp(s_webViews[i]->name(), name) == 0)
//            return s_webViews[i];
//    }
//
//    return 0;
//}

void wkeDestroyWebView(wkeWebView webView)
{
    //size_t pos = s_webViews.find(webView);

    //ASSERT(pos != notFound);
    //if (pos != notFound)
    //{
    //    s_webViews.remove(pos);
        delete webView;
    //}
}