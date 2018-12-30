
#include "app.h"
#include "cmdline.h"
#include "path.h"

#include <wke.h>
#include <stdio.h>
#include <stdlib.h>



BOOL FixupHtmlUrl(Application* app)
{
    LPWSTR html = app->options.html;
    if (!html || html[0] == 0)
        return FALSE;

    // 包含 : 说明是完整的路径
    if (wcsstr(html, L":"))
        return TRUE;

    // 若不是完整URL，补完生成新的地址
    html = FixupHtmlFilePath(html);
    if (html != app->options.html)
    {
        free(app->options.html);
        app->options.html = html;
    }

    return TRUE;
}

BOOL ProcessOptions(Application* app)
{
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    ParseOptions(&app->options, argc, argv);
    LocalFree(argv);

    return TRUE;
}

void FillDefultOptions(Application* app)
{
    app->options.cookiePath[0] = 0;
    app->options.showHelp = false;
    app->options.transparent = false;
}

void FillDefaultUrl(Application* app)
{
    app->options.html = (WCHAR*)malloc(100);
    wcscpy(app->options.html, L"about:blank");
}

// 回调：点击了关闭、返回 true 将销毁窗口，返回 false 什么都不做。
bool HandleWindowClosing(wkeWebView* webWindow, void* param)
{
    Application* app = (Application*)param;
    return IDYES == MessageBoxW(NULL, L"确定要退出程序吗？", L"wkexe", MB_YESNO|MB_ICONQUESTION);
}

// 回调：窗口已销毁
void HandleWindowDestroy(wkeWebView* webWindow, void* param)
{
    Application* app = (Application*)param;
    app->window = NULL;
    PostQuitMessage(0);
}

// 回调：文档加载成功
void HandleDocumentReady(wkeWebView* webWindow, void* param, const wkeDocumentReadyInfo* info)
{
    //主页面加载成功(非iframe)
    if (info->frameJSState == info->mainFrameJSState)
        wkeShowWindow(webWindow, TRUE);
}

// 回调：页面标题改变
void HandleTitleChanged(wkeWebView* webWindow, void* param, const wkeString* title)
{
    wkeSetWindowTitleW(webWindow, wkeGetStringW(title));
}

// 回调：创建新的页面，比如说调用了 window.open 或者点击了 <a target="_blank" .../>
wkeWebView* HandleCreateView(wkeWebView* webWindow, void* param, const wkeNewViewInfo* info)
{
    wkeWebView* newWindow = wkeCreateWebWindow(WKE_WINDOW_TYPE_POPUP, NULL, info->x, info->y, info->width, info->height);
    wkeShowWindow(newWindow, SW_SHOW);
    return newWindow;
}

// 创建主页面窗口
BOOL CreateWebWindow(Application* app)
{
    //wkeSettings settings;
    //settings.proxy.type = WKE_PROXY_HTTP;
    //settings.proxy.hostname = "127.0.0.1:8888";
    //wkeConfigure(&settings);

    if (app->options.transparent)
        app->window = wkeCreateWebWindow(WKE_WINDOW_TYPE_TRANSPARENT, NULL, 0, 0, 640, 480);
    else
        app->window = wkeCreateWebWindow(WKE_WINDOW_TYPE_POPUP, NULL, 0, 0, 640, 480);

    if (!app->window)
        return FALSE;

    wkeSetUserAgentW(app->window, app->options.userAgent);
    //wkeSetProxy(app->window, app->options.userAgent);

    wkeOnWindowClosing(app->window, HandleWindowClosing, app);
    wkeOnWindowDestroy(app->window, HandleWindowDestroy, app);
    wkeOnDocumentReady(app->window, HandleDocumentReady, app);
    wkeOnTitleChanged(app->window, HandleTitleChanged, app);
    wkeOnCreateView(app->window, HandleCreateView, app);

    wkeMoveToCenter(app->window);
    wkeLoadW(app->window, app->options.html);

    return TRUE;
}

void PrintHelpAndQuit(Application* app)
{
    PrintHelp();
    PostQuitMessage(0);
}

void RunMessageLoop(Application* app)
{
    wkeRunMessageLoop(NULL);
}

void RunApplication(Application* app)
{
    memset(app, 0, sizeof(Application));

    if (!ProcessOptions(app))
        FillDefultOptions(app);

    if (!FixupHtmlUrl(app))
        FillDefaultUrl(app);

    wkeInitialize();
    do
    {
        if (!CreateWebWindow(app))
        {
            PrintHelpAndQuit(app);
            break;
        }

        RunMessageLoop(app);
    }
    while (0);
    wkeFinalize();
}

void QuitApplication(Application* app)
{
    FreeOptions(&app->options);

    if (app->window)
    {
        wkeDestroyWebWindow(app->window);
        app->window = NULL;
    }
}
