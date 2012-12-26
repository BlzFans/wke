
#include <CEGUI.h>
#include <RendererModules/Direct3D9/CEGUIDirect3D9Renderer.h>
#include "webWindow.h"

const LPCWSTR szClassName = L"ceguiWebKitClass";

HWND hMainWnd;
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

ATOM MyRegisterClass()
{
    WNDCLASS wndClass;
    ZeroMemory(&wndClass, sizeof(wndClass));

    wndClass.style        = CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc  = WndProc;
    wndClass.cbClsExtra   = 0;
    wndClass.cbWndExtra   = 0;
    wndClass.hInstance    = GetModuleHandle(0);
    wndClass.hIcon        = LoadIcon(0, IDI_WINLOGO);
    wndClass.hCursor      = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground= (HBRUSH)(COLOR_WINDOW+1);
    wndClass.lpszMenuName   = 0;
    wndClass.lpszClassName  = szClassName;

    return RegisterClass(&wndClass);
}

void initWindow(HINSTANCE hInst, INT nCmdShow)
{
    MyRegisterClass();
    hMainWnd = CreateWindow(szClassName, L"CEGUI WebKit", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInst, NULL);

    if (!hMainWnd)
        return;

    ShowWindow(hMainWnd, nCmdShow);
    UpdateWindow(hMainWnd);
}

static D3DPRESENT_PARAMETERS s_presentParams;
static IDirect3D9* s_d3d = NULL;
static IDirect3DDevice9* s_d3dDevice = NULL;

wkeClientHandler s_clientHandler;

void initRender()
{
    ZeroMemory(&s_presentParams, sizeof(D3DPRESENT_PARAMETERS));
    s_presentParams.BackBufferWidth = 0;
    s_presentParams.BackBufferHeight = 0;
    s_presentParams.BackBufferFormat = D3DFMT_X8R8G8B8;
    s_presentParams.BackBufferCount = 1;
    s_presentParams.MultiSampleType = D3DMULTISAMPLE_NONE;
    s_presentParams.MultiSampleQuality = 0;
    s_presentParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
    s_presentParams.hDeviceWindow = hMainWnd;
    s_presentParams.Windowed = TRUE;
    s_presentParams.EnableAutoDepthStencil = TRUE;
    s_presentParams.AutoDepthStencilFormat = D3DFMT_D24X8;
    s_presentParams.Flags = 0;
    s_presentParams.FullScreen_RefreshRateInHz = 0;
    s_presentParams.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

    DWORD BehaviorFlags = D3DCREATE_FPU_PRESERVE | D3DCREATE_PUREDEVICE | D3DCREATE_HARDWARE_VERTEXPROCESSING;
    s_d3d = Direct3DCreate9(D3D_SDK_VERSION);
    HRESULT hr = s_d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hMainWnd, BehaviorFlags, &s_presentParams, &s_d3dDevice);
    if (FAILED(hr) && hr != D3DERR_DEVICELOST)
    {
        BehaviorFlags = D3DCREATE_FPU_PRESERVE | D3DCREATE_PUREDEVICE | D3DCREATE_SOFTWARE_VERTEXPROCESSING;
        s_d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hMainWnd, BehaviorFlags, &s_presentParams, &s_d3dDevice);
    }
}

void initialiseCEGUIResources()
{
    CEGUI::DefaultResourceProvider* rp = (CEGUI::DefaultResourceProvider*)CEGUI::System::getSingleton().getResourceProvider();
    rp->setResourceGroupDirectory("schemes", "datafiles/schemes/");
    rp->setResourceGroupDirectory("imagesets", "datafiles/imagesets");
    rp->setResourceGroupDirectory("fonts", "datafiles/fonts");
    rp->setResourceGroupDirectory("looknfeel", "datafiles/looknfeel");
 
    CEGUI::Scheme::setDefaultResourceGroup("schemes");
    CEGUI::Imageset::setDefaultResourceGroup("imagesets");
    CEGUI::Font::setDefaultResourceGroup("fonts");
    CEGUI::WidgetLookManager::setDefaultResourceGroup("looknfeel");

    // setup default group for validation schemas
    CEGUI::XMLParser* parser = CEGUI::System::getSingleton().getXMLParser();
    if (parser->isPropertyPresent("SchemaDefaultResourceGroup"))
        parser->setProperty("SchemaDefaultResourceGroup", "schemas");
}

bool handleLoadURL(const CEGUI::EventArgs&)
{
    WebWindow* webWindow = (WebWindow*)CEGUI::WindowManager::getSingleton().getWindow("webWindow");
    CEGUI::Window* addressBar = CEGUI::WindowManager::getSingleton().getWindow("addressBar");
    webWindow->loadURL(addressBar->getText().c_str());
    return true;
}

void onURLChanged(const wkeClientHandler* clientHandler, const wkeString url)
{
    CEGUI::Window* addressBar = CEGUI::WindowManager::getSingleton().getWindow("addressBar");
    addressBar->setText(wkeToString(url));
}

const float addressBarHeight = 30.f;
void initCEGUI()
{
    CEGUI::System::setDefaultXMLParserName("ExpatParser");
    CEGUI::Direct3D9Renderer::bootstrapSystem(s_d3dDevice);
    initialiseCEGUIResources();

    // register the new window type
    CEGUI::WindowFactoryManager::getSingleton().addFactory<CEGUI::TplWindowFactory<WebWindow> >();

    CEGUI::WindowManager& winMgr = CEGUI::WindowManager::getSingleton();
    CEGUI::Window* root = winMgr.createWindow("DefaultWindow");
    CEGUI::System::getSingleton().setGUISheet(root);

    CEGUI::SchemeManager::getSingleton().create("TaharezLook.scheme");
    
    CEGUI::Window* addressBar = winMgr.createWindow("TaharezLook/Editbox", "addressBar");
    root->addChildWindow(addressBar);
    addressBar->setPosition(CEGUI::UVector2(cegui_absdim(0), cegui_absdim(0)));
    addressBar->setSize(CEGUI::UVector2(cegui_reldim(1.0), cegui_absdim(30)));
    addressBar->subscribeEvent(CEGUI::Editbox::EventTextAccepted, CEGUI::Event::Subscriber(&handleLoadURL));

    WebWindow* webWindow = (WebWindow*)winMgr.createWindow("WebWindow", "webWindow");
    root->addChildWindow(webWindow);
    webWindow->setPosition(CEGUI::UVector2(cegui_absdim(0), cegui_absdim(addressBarHeight)));

    memset(&s_clientHandler, 0, sizeof(s_clientHandler));
    s_clientHandler.onURLChanged = onURLChanged;
    webWindow->setClientHandler(&s_clientHandler);

    webWindow->loadFile("datafiles/html/mac-osx-lion.html");

    RECT rect;
    GetClientRect(hMainWnd, &rect);
    float h = rect.bottom - rect.top - addressBarHeight;
    webWindow->setSize(CEGUI::UVector2(cegui_reldim(1.0), cegui_absdim(h)));
}

void ResetDevice()
{
    CEGUI::WindowManager& winMgr = CEGUI::WindowManager::getSingleton();
    CEGUI::Direct3D9Renderer* d3d_renderer =
        static_cast<CEGUI::Direct3D9Renderer*>(CEGUI::System::getSingleton().getRenderer());

    s_presentParams.BackBufferWidth = 0;
    s_presentParams.BackBufferHeight = 0;
    d3d_renderer->preD3DReset();

    WebWindow* walk = WebWindow::first();
    while (walk)
    {
        walk->preD3DReset();
        walk = walk->next();
    }

    if (D3D_OK == s_d3dDevice->Reset(&s_presentParams))
    {
        d3d_renderer->postD3DReset();

        walk = WebWindow::first();
        while (walk)
        {
            walk->postD3DReset();
            walk = walk->next();
        }
    }
}

bool LostDeviceRestore()
{
    HRESULT hr = s_d3dDevice->TestCooperativeLevel();
    if (hr == D3DERR_DEVICELOST)
        return false;

    if (hr == D3DERR_DEVICENOTRESET)
    {
        ResetDevice();
    }

    return true;
}

void updateCEGUI()
{
    static DWORD s_lastFrameTime = 0;
    DWORD thisTime = GetTickCount();
    float elapsed = float(thisTime - s_lastFrameTime);
    s_lastFrameTime = thisTime;

    CEGUI::System& guiSystem = CEGUI::System::getSingleton();
    guiSystem.injectTimePulse(elapsed/1000.0f);

    if (!LostDeviceRestore())
        return;

    s_d3dDevice->BeginScene();
    s_d3dDevice->Clear(0, 0, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
    guiSystem.renderGUI();
    s_d3dDevice->EndScene();

    s_d3dDevice->Present(0, 0, 0, 0);
}

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT nCmdShow)
{
    wkeInit();

    initWindow(hInst, nCmdShow);
    initRender();
    initCEGUI();

    MSG msg;
    msg.message = WM_NULL;
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else if (!IsIconic(hMainWnd))
        {
            updateCEGUI();
        }
        else
        {
            Sleep(10);
        }
    }


    CEGUI::Direct3D9Renderer::destroySystem();
    s_d3dDevice->Release();
    s_d3d->Release();

    wkeShutdown();
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CHAR:
        CEGUI::System::getSingleton().injectChar((CEGUI::utf32)wParam);
        break;

    case WM_KEYDOWN:
        {
            CEGUI::uint key = translateVirtualKey(wParam);
            if (key)
                CEGUI::System::getSingleton().injectKeyDown(key);
        }
        break;

    case WM_KEYUP:
        {
            CEGUI::uint key = translateVirtualKey(wParam);
            if (key)
                CEGUI::System::getSingleton().injectKeyUp(key);
        }
        break;

    case WM_MOUSEMOVE:
        CEGUI::System::getSingleton().injectMousePosition((float)(LOWORD(lParam)), (float)(HIWORD(lParam)));
        break;

    case WM_LBUTTONDOWN:
        CEGUI::System::getSingleton().injectMouseButtonDown(CEGUI::LeftButton);
        break;

    case WM_LBUTTONUP:
        CEGUI::System::getSingleton().injectMouseButtonUp(CEGUI::LeftButton);
        break;

    case WM_RBUTTONDOWN:
        CEGUI::System::getSingleton().injectMouseButtonDown(CEGUI::RightButton);
        break;

    case WM_RBUTTONUP:
        CEGUI::System::getSingleton().injectMouseButtonUp(CEGUI::RightButton);
        break;

    case WM_MBUTTONDOWN:
        CEGUI::System::getSingleton().injectMouseButtonDown(CEGUI::MiddleButton);
        break;

    case WM_MBUTTONUP:
        CEGUI::System::getSingleton().injectMouseButtonUp(CEGUI::MiddleButton);
        break;

    case 0x020A: // WM_MOUSEWHEEL:
        CEGUI::System::getSingleton().injectMouseWheelChange(static_cast<float>((short)HIWORD(wParam)) / static_cast<float>(120));
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_SIZE:
        {
            if (s_d3dDevice)
            {
                ResetDevice();

                RECT rect;
                GetClientRect(hMainWnd, &rect);
                float h = rect.bottom - rect.top - addressBarHeight;

                CEGUI::Window* webWindow = CEGUI::WindowManager::getSingleton().getWindow("webWindow");
                webWindow->setSize(CEGUI::UVector2(cegui_reldim(1.0), cegui_absdim(h)));
            }
        }
        break;

    case WM_PAINT:
        {
            HDC         hDC;
            PAINTSTRUCT ps;

            hDC = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
        }
        break;

    default:
        return(DefWindowProc(hWnd, message, wParam, lParam));
    }

    return 0;
}