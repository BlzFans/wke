#include "renderD3D.h"

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } }
#endif

typedef IDirect3D9* (WINAPI* LPDIRECT3DCREATE9)( UINT );

CRenderD3D::CRenderD3D()
    :m_hView(NULL)
    ,m_pD3D(NULL)
    ,m_pDevice(NULL)
    ,m_pWebViewTexture(NULL)
{
}

CRenderD3D::~CRenderD3D()
{
    SAFE_RELEASE(m_pWebViewTexture);
    SAFE_RELEASE(m_pDevice);
    SAFE_RELEASE(m_pD3D);
}

D3DPRESENT_PARAMETERS GetPresentParams(HWND hView)
{
    D3DPRESENT_PARAMETERS PresentParams;
    ZeroMemory(&PresentParams, sizeof(PresentParams));

    PresentParams.BackBufferWidth = 0;
    PresentParams.BackBufferHeight = 0;
    PresentParams.BackBufferFormat = D3DFMT_X8R8G8B8;
    PresentParams.BackBufferCount = 1;
    PresentParams.MultiSampleType = D3DMULTISAMPLE_NONE;
    PresentParams.MultiSampleQuality = 0;
    PresentParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
    PresentParams.hDeviceWindow = hView;
    PresentParams.Windowed = TRUE;
    PresentParams.EnableAutoDepthStencil = TRUE;
    PresentParams.AutoDepthStencilFormat = D3DFMT_D24X8;
    PresentParams.Flags = 0;
    PresentParams.FullScreen_RefreshRateInHz = 0;
    PresentParams.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

    return PresentParams;
}


bool CRenderD3D::init(HWND hView)
{
    m_hView = hView;

    HMODULE hModD3D9 = LoadLibrary(L"d3d9.dll");
    if (hModD3D9)
    {
        LPDIRECT3DCREATE9 pfnDirect3DCreate9 = (LPDIRECT3DCREATE9)GetProcAddress(hModD3D9, "Direct3DCreate9");
        m_pD3D = pfnDirect3DCreate9(D3D_SDK_VERSION);

        DWORD BehaviorFlags = D3DCREATE_FPU_PRESERVE | D3DCREATE_PUREDEVICE | D3DCREATE_HARDWARE_VERTEXPROCESSING;
        D3DPRESENT_PARAMETERS PresentParams = GetPresentParams(hView);

        HRESULT hr = m_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, GetParent(hView), BehaviorFlags, &PresentParams, &m_pDevice);
        if (FAILED(hr) && hr != D3DERR_DEVICELOST)
        {
            BehaviorFlags = D3DCREATE_FPU_PRESERVE | D3DCREATE_PUREDEVICE | D3DCREATE_SOFTWARE_VERTEXPROCESSING;
            hr = m_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, GetParent(hView), BehaviorFlags, &PresentParams, &m_pDevice);
            if (FAILED(hr) && hr != D3DERR_DEVICELOST)
                return false;
        }

        resize(PresentParams.BackBufferWidth, PresentParams.BackBufferHeight);
    }

    return false;
}

void CRenderD3D::destroy()
{
    delete this;
}

void CRenderD3D::resize(unsigned int w, unsigned int h)
{
    /*
    0        1
     ********
     *      *
     *      *
     ********
    2        3
    */

    m_Vertex[0].x = -0.5f;
    m_Vertex[0].y = -0.5f;
    m_Vertex[0].z = 0.f;
    m_Vertex[0].w = 1.f;
    m_Vertex[0].u = 0.f;
    m_Vertex[0].v = 0.f;

    m_Vertex[1].x = w - 0.5f;
    m_Vertex[1].y = -0.5f;
    m_Vertex[1].z = 0.f;
    m_Vertex[1].w = 1.f;
    m_Vertex[1].u = 1.f;
    m_Vertex[1].v = 0.f;

    m_Vertex[2].x = -0.5f;
    m_Vertex[2].y = h - 0.5f;
    m_Vertex[2].z = 0.f;
    m_Vertex[2].w = 1.f;
    m_Vertex[2].u = 0.f;
    m_Vertex[2].v = 1.f;

    m_Vertex[3].x = w - 0.5f;
    m_Vertex[3].y = h - 0.5f;
    m_Vertex[3].z = 0.f;
    m_Vertex[3].w = 1.f;
    m_Vertex[3].u = 1.f;
    m_Vertex[3].v = 1.f;

    D3DPRESENT_PARAMETERS PresentParams = GetPresentParams(m_hView);
    m_pDevice->Reset(&PresentParams);

    SAFE_RELEASE(m_pWebViewTexture);
}

void CRenderD3D::render(wkeWebView* webView)
{
    if (!LostDeviceRestore())
        return;

    if (!UpdateTexture(webView))
        return;

    m_pDevice->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(128, 128, 64), 1.0f, 0);
    if( SUCCEEDED(m_pDevice->BeginScene()) )
    {
        m_pDevice->SetTexture(0, m_pWebViewTexture);
        m_pDevice->SetFVF(Vertex::FVF);
        m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        m_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, m_Vertex, sizeof(Vertex));
        m_pDevice->EndScene();

        m_pDevice->Present(NULL, NULL, NULL, NULL);
    }
}

bool CRenderD3D::LostDeviceRestore()
{
    HRESULT hr = m_pDevice->TestCooperativeLevel();
    if (hr == D3DERR_DEVICELOST)
    {
        SAFE_RELEASE(m_pWebViewTexture);
        return false;
    }

    if (hr == D3DERR_DEVICENOTRESET)
    {
        SAFE_RELEASE(m_pWebViewTexture);
        D3DPRESENT_PARAMETERS PresentParams = GetPresentParams(m_hView);
        hr = m_pDevice->Reset(&PresentParams);
        if (FAILED(hr))
            return false;
    }

    return true;
}

bool CRenderD3D::UpdateTexture(wkeWebView* webView)
{
    if (m_pWebViewTexture == NULL)
    {
        RECT rect;
        GetClientRect(m_hView, &rect);
        HRESULT  hr = m_pDevice->CreateTexture(
            rect.right - rect.left,
            rect.bottom - rect.top,
            1,
            D3DUSAGE_DYNAMIC,
            D3DFMT_A8R8G8B8,
            D3DPOOL_DEFAULT,
            &m_pWebViewTexture,
            NULL);

        if (FAILED(hr))
            return false;

        wkeSetDirty(webView, true);
    }

    if (wkeIsDirty(webView))
    {
        D3DLOCKED_RECT rect;
        m_pWebViewTexture->LockRect(0, &rect, NULL, D3DLOCK_DISCARD);
        wkePaint2(webView, rect.pBits, rect.Pitch);
        m_pWebViewTexture->UnlockRect(0);
    }

    return true;
}
