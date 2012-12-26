#include <CEGUI.h>
#include <RendererModules/Direct3D9/CEGUIDirect3D9Renderer.h>
#include <windows.h>
#include "webWindow.h"

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)  { if (p) { (p)->Release(); (p)=NULL; } }
#endif

const CEGUI::String WebWindow::WidgetTypeName("WebWindow");
WebWindow* WebWindow::s_first = NULL;

WebWindow::WebWindow(const CEGUI::String& type, const CEGUI::String& name)
    :CEGUI::Window(type, name)
    ,d_webViewTexture(NULL)
{
    d_wantsMultiClicks = false;

    d_webGeometry = new WebGeometryBuffer(this);
    d_webView = wkeCreateWebView();

    d_next = s_first;
    s_first = this;
}

WebWindow::~WebWindow()
{
    if (s_first == this)
    {
        s_first = d_next;
    }
    else
    {
        WebWindow* walk = s_first;
        while (walk->d_next != this)
            walk = walk->d_next;

        walk->d_next = d_next;
    }

    SAFE_RELEASE(d_webViewTexture);
    d_webView->destroy();
    delete d_webGeometry;
}

void WebWindow::drawSelf(const CEGUI::RenderingContext& ctx)
{
    CEGUI::RenderingSurface* surface;
 
    if (ctx.surface->isRenderingWindow())
        surface = &static_cast<CEGUI::RenderingWindow*>(ctx.surface)->getOwner();
    else
        surface = ctx.surface;
 
    surface->addGeometryBuffer(ctx.queue, *d_webGeometry);

    CEGUI::Window::drawSelf(ctx);
}

bool WebWindow::testClassName_impl(const CEGUI::String& class_name) const
{
    if (class_name == WidgetTypeName)
        return true;

    return CEGUI::Window::testClassName_impl(class_name);
}

bool WebWindow::updateTexture()
{
    if (d_webViewTexture == NULL)
    {
        CEGUI::Rect rect = getUnclippedOuterRect();

        CEGUI::Direct3D9Renderer* d3d_renderer =
            static_cast<CEGUI::Direct3D9Renderer*>(CEGUI::System::getSingleton().getRenderer());

        HRESULT hr = d3d_renderer->getDevice()->CreateTexture(
            UINT(rect.d_right - rect.d_left),
            UINT(rect.d_bottom - rect.d_top),
            1,
            D3DUSAGE_DYNAMIC,
            D3DFMT_A8R8G8B8,
            D3DPOOL_DEFAULT,
            &d_webViewTexture,
            NULL);

        if (FAILED(hr))
            return false;

        d_webView->setDirty(true);
    }

    if (d_webView->isDirty())
    {
        D3DLOCKED_RECT rect;
        d_webViewTexture->LockRect(0, &rect, NULL, D3DLOCK_DISCARD);
        d_webView->paint(rect.pBits, rect.Pitch);
        d_webViewTexture->UnlockRect(0);
    }

    return true;
}

void WebWindow::preD3DReset()
{
    SAFE_RELEASE(d_webViewTexture);
}

void WebWindow::postD3DReset()
{
}

void WebWindow::loadURL(const char* url)
{
    d_webView->loadURL(url);
}

void WebWindow::loadFile(const char* filename)
{
    d_webView->loadFile(filename);
}

void WebWindow::loadHTML(const char* html)
{
    d_webView->loadHTML(html);
}

void WebWindow::setTransparent(bool transparent)
{
    d_webView->setTransparent(transparent);
}

bool WebWindow::transparent() const
{
    return d_webView->transparent();
}

void WebWindow::setClientHandler(wkeClientHandler* handler)
{
    d_webView->setClientHandler(handler);
}


unsigned int WebWindow::getInputFlags()
{
    unsigned int flags = 0;
    if (CEGUI::System::getSingleton().getSystemKeys() & CEGUI::LeftMouse)
        flags |= WKE_LBUTTON;

    if (CEGUI::System::getSingleton().getSystemKeys() & CEGUI::RightMouse)
        flags |= WKE_RBUTTON;

    if (CEGUI::System::getSingleton().getSystemKeys() & CEGUI::MiddleMouse)
        flags |= WKE_MBUTTON;

    if (CEGUI::System::getSingleton().getSystemKeys() & CEGUI::Shift)
        flags |= WKE_SHIFT;

    if (CEGUI::System::getSingleton().getSystemKeys() & CEGUI::Control)
        flags |= WKE_CONTROL;

    return flags;
}

void WebWindow::onMouseMove(CEGUI::MouseEventArgs& e)
{ 
    CEGUI::Rect rect = getUnclippedOuterRect();
    int x = int(e.position.d_x - rect.d_left);
    int y = int(e.position.d_y - rect.d_top);

    e.handled = d_webView->mouseEvent(WKE_MSG_MOUSEMOVE, x, y, getInputFlags());
    CEGUI::Window::onMouseMove(e);
}

void WebWindow::onMouseWheel(CEGUI::MouseEventArgs& e)
{
    CEGUI::Rect rect = getUnclippedOuterRect();
    int x = int(e.position.d_x - rect.d_left);
    int y = int(e.position.d_y - rect.d_top);

    e.handled = d_webView->mouseWheel(x, y, int(e.wheelChange * WHEEL_DELTA), getInputFlags());
    
    CEGUI::Window::onMouseWheel(e);
}

void WebWindow::onMouseButtonDown(CEGUI::MouseEventArgs& e)
{
    CEGUI::Rect rect = getUnclippedOuterRect();
    int x = int(e.position.d_x - rect.d_left);
    int y = int(e.position.d_y - rect.d_top);

    if (e.button == CEGUI::LeftButton)
    {
        e.handled = d_webView->mouseEvent(WKE_MSG_LBUTTONDOWN, x, y, getInputFlags());
    }
    else if (e.button == CEGUI::RightButton)
        e.handled = d_webView->mouseEvent(WKE_MSG_RBUTTONDOWN, x, y, getInputFlags());

    CEGUI::Window::onMouseButtonDown(e);
}

void WebWindow::onMouseButtonUp(CEGUI::MouseEventArgs& e)
{
    CEGUI::Rect rect = getUnclippedOuterRect();
    int x = int(e.position.d_x - rect.d_left);
    int y = int(e.position.d_y - rect.d_top);

    if (e.button == CEGUI::LeftButton)
        e.handled = d_webView->mouseEvent(WKE_MSG_LBUTTONUP, x, y, getInputFlags());
    else if (e.button == CEGUI::RightButton)
        e.handled = d_webView->mouseEvent(WKE_MSG_RBUTTONUP, x, y, getInputFlags());

    CEGUI::Window::onMouseButtonUp(e);
}

void WebWindow::onKeyDown(CEGUI::KeyEventArgs& e)
{
    UINT vk = translateKey(e.scancode);
    if (vk)
    {
        e.handled = d_webView->keyDown(vk, 0, false);
    }

    CEGUI::Window::onKeyDown(e);
}

void WebWindow::onKeyUp(CEGUI::KeyEventArgs& e)
{
    UINT vk = translateKey(e.scancode);
    if (vk)
    {
        e.handled = d_webView->keyUp(vk, 0, false);
    }

    CEGUI::Window::onKeyUp(e);
}

void WebWindow::onCharacter(CEGUI::KeyEventArgs& e)
{
    e.handled = d_webView->keyPress(e.codepoint, getInputFlags(), false);
    CEGUI::Window::onCharacter(e);
}

void WebWindow::onActivated(CEGUI::ActivationEventArgs& e)
{
    CEGUI::Window::onActivated(e);

    d_webView->focus();
}

void WebWindow::onDeactivated(CEGUI::ActivationEventArgs& e)
{
    CEGUI::Window::onDeactivated(e);

    d_webView->unfocus();
}

void WebWindow::onSized(CEGUI::WindowEventArgs& e)
{
    CEGUI::Window::onSized(e);

    SAFE_RELEASE(d_webViewTexture);

    CEGUI::Rect rect = getUnclippedOuterRect();
    d_webView->resize(int(rect.d_right - rect.d_left), int(rect.d_bottom - rect.d_top));
}

WebGeometryBuffer::WebGeometryBuffer(WebWindow* owner)
    :d_owner(owner)
{
}

struct Vertex
{
    float x, y, z;
    DWORD diffuse;
    float u, v;
};

void WebGeometryBuffer::draw() const
{
    d_owner->updateTexture();

    CEGUI::Rect rect = d_owner->getUnclippedOuterRect();

    /*
    0        1
     ********
     *      *
     *      *
     ********
    2        3
    */

    Vertex v[4];
    v[0].x = rect.d_left - 0.5f;
    v[0].y = rect.d_top - 0.5f;
    v[0].z = 0.f;
    v[0].diffuse = 0xFFFFFFFF;
    v[0].u = 0.f;
    v[0].v = 0.f;

    v[1].x = rect.d_right - 0.5f;
    v[1].y = rect.d_top - 0.5f;
    v[1].z = 0.f;
    v[1].diffuse = 0xFFFFFFFF;
    v[1].u = 1.f;
    v[1].v = 0.f;

    v[2].x = rect.d_left - 0.5f;
    v[2].y = rect.d_bottom - 0.5f;
    v[2].z = 0.f;
    v[2].diffuse = 0xFFFFFFFF;
    v[2].u = 0.f;
    v[2].v = 1.f;

    v[3].x = rect.d_right - 0.5f;
    v[3].y = rect.d_bottom - 0.5f;
    v[3].z = 0.f;
    v[3].diffuse = 0xFFFFFFFF;
    v[3].u = 1.f;
    v[3].v = 1.f;

    CEGUI::Direct3D9Renderer* d3d_renderer =
        static_cast<CEGUI::Direct3D9Renderer*>(CEGUI::System::getSingleton().getRenderer());

    RECT clip;
    clip.left   = static_cast<LONG>(rect.d_left);
    clip.top    = static_cast<LONG>(rect.d_top);
    clip.right  = static_cast<LONG>(rect.d_right);
    clip.bottom = static_cast<LONG>(rect.d_bottom);
    d3d_renderer->getDevice()->SetScissorRect(&clip);

    d3d_renderer->getDevice()->SetTexture(0, d_owner->getTexture());
    d3d_renderer->getDevice()->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, v, sizeof(Vertex));
}

static UINT VKMap[256] =
{
0,              //0x00
VK_ESCAPE,      //0x01
'1',            //0x02
'2',            //0x03
'3',            //0x04
'4',            //0x05
'5',            //0x06
'6',            //0x07
'7',            //0x08
'8',            //0x09
'9',            //0x0A
'0',            //0x0B
VK_OEM_MINUS,   //0x0C
VK_OEM_PLUS,    //0x0D
VK_BACK,        //0x0E
VK_TAB,         //0x0F
'Q',            //0x10
'W',            //0x11
'E',            //0x12
'R',            //0x13
'T',            //0x14
'Y',            //0x15
'U',            //0x16
'I',            //0x17
'O',            //0x18
'P',            //0x19
VK_OEM_4,       //0x1A
VK_OEM_6,       //0x1B
VK_RETURN,      //0x1C
VK_CONTROL,     //0x1D
'A',            //0x1E
'S',            //0x1F
'D',            //0x20
'F',            //0x21
'G',            //0x22
'H',            //0x23
'J',            //0x24
'K',            //0x25
'L',            //0x26
VK_OEM_1,       //0x27
VK_OEM_7,       //0x28
VK_OEM_3,       //0x29
VK_SHIFT,       //0x2A
VK_OEM_5,       //0x2B
'Z',            //0x2C
'X',            //0x2D
'C',            //0x2E
'V',            //0x2F
'B',            //0x30
'N',            //0x31
'M',            //0x32
VK_OEM_COMMA,   //0x33
VK_OEM_PERIOD,  //0x34
VK_OEM_2,       //0x35
VK_SHIFT,       //0x36
VK_MULTIPLY,    //0x37
VK_MENU,        //0x38
VK_SPACE,       //0x39
VK_CAPITAL,     //0x3A
VK_F1,          //0x3B
VK_F2,          //0x3C
VK_F3,          //0x3D
VK_F4,          //0x3E
VK_F5,          //0x3F
VK_F6,          //0x40
VK_F7,          //0x41
VK_F8,          //0x42
VK_F9,          //0x43
VK_F10,         //0x44
VK_NUMLOCK,     //0x45
VK_SCROLL,      //0x46
VK_NUMPAD7,     //0x47
VK_NUMPAD8,     //0x48
VK_NUMPAD9,     //0x49
VK_SUBTRACT,    //0x4A
VK_NUMPAD4,     //0x4B
VK_NUMPAD5,     //0x4C
VK_NUMPAD6,     //0x4D
VK_ADD,         //0x4E
VK_NUMPAD1,     //0x4F
VK_NUMPAD2,     //0x50
VK_NUMPAD3,     //0x51
VK_NUMPAD0,     //0x52
VK_DECIMAL,     //0x53
0,              //0x54
0,              //0x55
VK_OEM_102,     //0x56
VK_F11,         //0x57
VK_F12,         //0x58
0,              //0x59
0,              //0x5A
0,              //0x5B
0,              //0x5C
0,              //0x5D
0,              //0x5E
0,              //0x5F
0,              //0x60
0,              //0x61
0,              //0x62
0,              //0x63
VK_F13,         //0x64
VK_F14,         //0x65
VK_F15,         //0x66
0,              //0x67
0,              //0x68
0,              //0x69
0,              //0x6A
0,              //0x6B
0,              //0x6C
0,              //0x6D
0,              //0x6E
0,              //0x6F
VK_KANA,        //0x70
0,              //0x71
0,              //0x72
0,              //0x73
0,              //0x74
0,              //0x75
0,              //0x76
0,              //0x77
0,              //0x78
VK_CONVERT,     //0x79
0,              //0x7A
VK_NONCONVERT,  //0x7B
0,              //0x7C
0,              //0x7D
0,              //0x7E
0,              //0x7F
0,              //0x80
0,              //0x81
0,              //0x82
0,              //0x83
0,              //0x84
0,              //0x85
0,              //0x86
0,              //0x87
0,              //0x88
0,              //0x89
0,              //0x8A
0,              //0x8B
0,              //0x8C
0,              //0x8D
0,              //0x8E
0,              //0x8F
VK_MEDIA_PREV_TRACK,    //0x90
0,              //0x91
0,              //0x92
0,              //0x93
0,              //0x94
0,              //0x95
0,              //0x96
VK_MEDIA_NEXT_TRACK,    //0x97
0,              //0x98
0,              //0x99
0,              //0x9A
0,              //0x9B
0,              //0x9C
VK_CONTROL,     //0x9D
0,              //0x9E
0,              //0x9F
VK_VOLUME_MUTE, //0xA0
0,              //0xA1
VK_MEDIA_PLAY_PAUSE,    //0xA2
0,              //0xA3
VK_MEDIA_STOP,  //0xA4
0,              //0xA5
0,              //0xA6
0,              //0xA7
0,              //0xA8
0,              //0xA9
0,              //0xAA
0,              //0xAB
0,              //0xAC
0,              //0xAD
VK_VOLUME_DOWN, //0xAE
0,              //0xAF
VK_VOLUME_UP,   //0xB0
0,              //0xB1
VK_BROWSER_HOME,    //0xB2
0,              //0xB3
0,              //0xB4
VK_DIVIDE,      //0xB5
0,              //0xB6
VK_PRINT,       //0xB7
VK_RMENU,       //0xB8
0,              //0xB9
0,              //0xBA
0,              //0xBB
0,              //0xBC
0,              //0xBD
0,              //0xBE
0,              //0xBF
0,              //0xC0
0,              //0xC1
0,              //0xC2
0,              //0xC3
0,              //0xC4
VK_PAUSE,       //0xC5
0,              //0xC6
VK_HOME,        //0xC7
VK_UP,          //0xC8
VK_PRIOR,       //0xC9
0,              //0xCA
VK_LEFT,        //0xCB
0,              //0xCC
VK_RIGHT,       //0xCD
0,              //0xCE
VK_END,         //0xCF
VK_DOWN,        //0xD0
VK_NEXT,        //0xD1
VK_INSERT,      //0xD2
VK_DELETE,      //0xD3
0,              //0xD4
0,              //0xD5
0,              //0xD6
0,              //0xD7
0,              //0xD8
0,              //0xD9
0,              //0xDA
VK_LWIN,        //0xDB
VK_RWIN,        //0xDC
VK_APPS,        //0xDD
0,              //0xDE
VK_SLEEP,       //0xDF
0,              //0xE0
0,              //0xE1
0,              //0xE2
0,              //0xE3
0,              //0xE4
VK_BROWSER_SEARCH,      //0xE5
VK_BROWSER_FAVORITES,   //0xE6
VK_BROWSER_REFRESH,     //0xE7
VK_BROWSER_STOP,        //0xE8
VK_BROWSER_FORWARD,     //0xE9
VK_BROWSER_BACK,        //0xEA
0,              //0xEB
VK_LAUNCH_MAIL, //0xEC
VK_LAUNCH_MEDIA_SELECT, //0xED
0,              //0xEE
0,              //0xEF
0,              //0xF0
0,              //0xF1
0,              //0xF2
0,              //0xF3
0,              //0xF4
0,              //0xF5
0,              //0xF6
0,              //0xF7
0,              //0xF8
0,              //0xF9
0,              //0xFA
0,              //0xFB
0,              //0xFC
0,              //0xFD
0,              //0xFE
0,              //0xFF
};

int translateKey(int key)
{
    if (key >= 0 && key < 256)
        return VKMap[key];

    return 0;
}

int translateVirtualKey(int vk)
{
    for (int i = 0; i < 256; ++i)
    {
        if (VKMap[i] == vk)
            return i;
    }

    return 0;
}


