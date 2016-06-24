#ifndef RENDER_D3D_H
#define RENDER_D3D_H

#include "render.h"
#include <d3d9.h>

class CRenderD3D : public CRender
{
public:
    CRenderD3D();
    ~CRenderD3D();

    virtual bool init(HWND hView);
    virtual void destroy();
    virtual void resize(unsigned int w, unsigned int h);
    virtual void render(wkeWebView* webView);

    bool LostDeviceRestore();
    bool UpdateTexture(wkeWebView* webView);

private:
    HWND m_hView;
    IDirect3D9* m_pD3D;
    IDirect3DDevice9* m_pDevice;
    IDirect3DTexture9* m_pWebViewTexture;

    struct Vertex
    {
        float x, y, z, w;
        float u, v;

        enum 
        {
            FVF = D3DFVF_XYZRHW|D3DFVF_TEX1,
        };
    };

    Vertex m_Vertex[4];
};

#endif