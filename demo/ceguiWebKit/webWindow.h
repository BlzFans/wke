#ifndef WEB_WINDOW_H
#define WEB_WINDOW_H

#include <CEGUIWindow.h>
#include <CEGUIGeometryBuffer.h>
#include <d3d9.h>
#include "wke.h"

class WebWindow : public CEGUI::Window
{
public:
    static const CEGUI::String WidgetTypeName;

    WebWindow(const CEGUI::String& type, const CEGUI::String& name);
    virtual ~WebWindow();

    bool updateTexture();
    IDirect3DTexture9* getTexture() const;
    void preD3DReset();
    void postD3DReset();

    void loadURL(const char* url);
    void loadFile(const char* filename);
    void loadHTML(const char* html);

    void setTransparent(bool transparent);
    bool transparent() const;

    void setClientHandler(wkeClientHandler* handler);

    unsigned int getInputFlags();
    virtual void onMouseMove(CEGUI::MouseEventArgs& e);
    virtual void onMouseWheel(CEGUI::MouseEventArgs& e);
    virtual void onMouseButtonDown(CEGUI::MouseEventArgs& e);
    virtual void onMouseButtonUp(CEGUI::MouseEventArgs& e);

    virtual void onKeyDown(CEGUI::KeyEventArgs& e);
    virtual void onKeyUp(CEGUI::KeyEventArgs& e);
    virtual void onCharacter(CEGUI::KeyEventArgs& e);

    virtual void onActivated(CEGUI::ActivationEventArgs& e);
    virtual void onDeactivated(CEGUI::ActivationEventArgs& e);

    static WebWindow* first() { return s_first; }
    WebWindow* next() { return d_next; }

protected:
    virtual void drawSelf(const CEGUI::RenderingContext& ctx);
    virtual bool testClassName_impl(const CEGUI::String& class_name) const;

    virtual void onSized(CEGUI::WindowEventArgs& e);

    CEGUI::GeometryBuffer* d_webGeometry;
    wke::IWebView* d_webView;
    IDirect3DTexture9* d_webViewTexture;

    WebWindow* d_next;
    static WebWindow* s_first;
};

class WebGeometryBuffer : public CEGUI::GeometryBuffer
{
public:
    WebGeometryBuffer(WebWindow* owner);

    virtual void draw() const;
    virtual void setTranslation(const CEGUI::Vector3& v) {}
    virtual void setRotation(const CEGUI::Vector3& r) {}
    virtual void setPivot(const CEGUI::Vector3& p) {}
    virtual void setClippingRegion(const CEGUI::Rect& region) {}
    virtual void appendVertex(const CEGUI::Vertex& vertex) {}
    virtual void appendGeometry(const CEGUI::Vertex* const vbuff, CEGUI::uint vertex_count) {}
    virtual void setActiveTexture(CEGUI::Texture* texture) {}
    virtual void reset() {}
    virtual CEGUI::Texture* getActiveTexture() const {return 0;}
    virtual CEGUI::uint getVertexCount() const {return 0;}
    virtual CEGUI::uint getBatchCount() const {return 0;}
    virtual void setRenderEffect(CEGUI::RenderEffect* effect) {}
    virtual CEGUI::RenderEffect* getRenderEffect() {return 0;}

protected:
    WebWindow* d_owner;
};

inline IDirect3DTexture9* WebWindow::getTexture() const
{
    return d_webViewTexture;
}


int translateVirtualKey(int vk);
int translateKey(int key);

#endif