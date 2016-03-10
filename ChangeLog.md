#### 2016-03-10 23:29 优化 wkexe、增加命令行直接加载 HTML 文本支持。
* 重构 wkexe 中一部分代码。
* 修改 wkexe 行为，若未在命令行指定参数，使用默认参数启动 about:blank 页面。
* 修改 wkexe 对于`html`参数的理解，现在可支持直接传递 html 内容，如：
```html
wkexe --html="<script>alert('Hello World!')</script>"
wkexe --html="<span>Hello World!</span>"
```
* 增加 wkexe 对于 cookie 文件路径的自定义支持，使用参数`cookie`设置路径。
* 修改 wkeLoadW、wkeLoad 以支持`about:blank`地址。

#### 2016-03-10 22:14 增加 cookie 操作接口。
* 增加 wkeSetCookie 设置指定 wkeWebView 的 cookie。
* 修改 wkeConfigure、wkeInitializeEx 的参数结构体 wkeSettings，增加指定全局 cookie 文件路径的成员。

#### 2016-03-10 21:24 增加查找插件时同时在工作目录的 plugins 目录下查找。
* 增加查找插件时同时在工作目录的 plugins 目录下查找，之前只在程序所在目录查找。

#### 2016-03-09 09:09 增加 Document/todo.md 记录平时积累的需求、问题。
* 增加 Document/todo.md 记录平时积累的需求、问题。

#### 2015-12-30 00:23 修正 wkeWebWindow 网页中编辑框输入法不跟随光标的问题。
* 在 WM_IME_STARTCOMPOSITION 添加 ImmSetCompositionWindow 调用，以修正 wkeWebWindow 网页中编辑框输入法不跟随光标的问题。

#### 2015-12-25 23:44 增加 wkeOnDocumentReady 回调参数，以区分 iframe 的加载成功事件。
* 增加 FrameLoaderClient::dispatchDidFinishDocumentLoad 参数，使 wkeOnDocumentReady 有回调参数，以区分 iframe 的加载成功事件。

#### 2015-12-25 23:44 实现几个底层回调，使 window.close、window.blur、window.focus 生效。
* 实现 ChromeClient::unfocus、ChromeClient::focus、ChromeClient::closeWindowSoon，使 window.close、window.blur、window.focus 生效。

#### 2015-12-25 20:59 修正 wkeBrowser 中拦截 _blank 导致无法打开链接的问题。
* 修正 wkeBrowser 中拦截 _blank 导致无法打开链接的问题。

#### 2015-12-24 20:27 临时解决 wke 的 vs2010 工程 Debug 模式链接失败的问题。
* 修改 vs2010 工程文件，调度模式时忽略默认库 msvcrt.lib;LIBCMT.lib。这只是临时解决问题，应该工程配置某个地方直接或间接引入了这两个 release 模式下的库。

#### 2015-12-23 12:47 修改插件查找机制支持非 np 开头的插件。
* 修改 PluginDatabaseWin.cpp::PluginDatabase::getPluginPathsInDirectories，以支持加载 plugins 目录下非 np 开头的插件。

#### 2015-12-20 19:10 修改 wkeOnCreateView 函数。
* 修改 wkeOnCreateView 函数参数，使用结构体 wkeNewViewInfo 传递，并增加 target 信息。
* 修改 wkeOnCreateView 函数行为，若返回 NULL，则不继续浏览该地址。、
* 在 wkeBrowser 当中增加 wkeOnCreateView 测试。

#### 2015-12-20 15:37 去掉 wke 接口定义的平台依赖。
* 去掉 wke.h 当中对 <windows.h>、HWND、HDC 等的直接引用，接口上去掉平台依赖。

#### 2015-12-20 15:37 明确定义 wke 接口的调用约定为 cdecl。
* 定义调用约定 WKE_CALL = cdecl。
* 每个函数声明前明确加上调用约定，以防止当使用不同的调用约定时出现错误。

#### 2015-12-19 23:03 修正 wke 无法正确加载 windowed 插件的问题。
* 增加 wkeSetHostWindow，若使用 windowed 插件，必须先调用此函数，以免插件找不到容器容器。
* 增加 wkeSetHostWindow，若使用 windowed 插件，必须先调用此函数，以免插件找不到容器容器。
* 修改 WebCore::PluginView::platformStart 函数，若找不到插件容器窗口，则使用桌面为容器。
* 在 wkeBrowser 当中增加 wkeSetHostWindow 调用，以支持 windowed 插件，使用 VLC 插件测试通过。
* 在 使用 wkeCreateWindow 返回的 webView 增加自动的 wkeSetHostWindow 调用，以支持 windowed 插件。

#### 2015-12-19 17:58 wkeBrowser 中增加使用 jsFunction 绑定函数的示例。
* wkeBrowser 中增加使用 jsFunction 绑定函数的示例。

#### 2015-12-16 16:38 增加控制台消息回调。
* 增加控制台消息回调功能，通过 wkeOnConsoleMessage 设置回调函数。
* wkeBrowser 当中增加控制台消息回调功能测试。

#### 2015-11-27 21:56 提交编译好的二进制文件。
* 提交编译好的二进制文件。

#### 2015-11-26 20:23 修正 wke 中 wkeSimpleStringT 拷贝宽字符串只拷一半的问题。
* 之前 wke 中的 wkeSimpleStringT 有拷贝宽字符串只拷一半的问题，导致 wkeToTempStringW 不正常。

#### 2015-11-26 10:26 增加 vs2010.bat 等批处理用于打开工程，避免找不到 nmake 的情况。
* 在使用 vs2010 或 vs2013 编译时可能会出现找不到 namke 的情况，因此增加 vs2010.bat 等批处理，其会先设置环境变量再打开工程。

#### 2015-11-26 10:07 修正 wkeBrowser 的 _WIN32_WINNT 导致找不到符号的问题。
* 修正 wkeBrowser 的 _WIN32_WINNT 导致找不到 IDC_HAND 宏的问题，将其设置为最低 0x501（Windows XP）即可。

#### 2015-11-25 22:28 增加基于 vs2008 工程文件 的 vs2005 工程文件。
* 增加基于 vs2008 工程文件 的 vs2005 工程文件。
* 修改 wkeBrowser 中 _WIN32_WINNT 定义，以支持 vs2005 编译。
* 去掉 wkeBrowser 当中对 Direct3D 的引用，以支持 vs2005 编译。

#### 2015-11-25 21:20 增加基于 BrentHuang 完成的 vs2013 工程文件。
* 增加基于 BrentHuang（群里光 光）完成的 vs2013 工程文件，感谢 BrentHuang 的贡献。
* 使用 BrentHuang 提供的修改过的几个源代码以支持 vs2013 中编译，我另外修改了几处以兼容 vs2008 和 vs2010。

#### 2015-11-25 13:03 增加 wkeLoad/wkeLoadW 智能加载接口。
* 增加 wkeLoad/wkeLoadW 智能加载接口，可分析出参数的类型是 URL 、HTML 或 文件路径，判断顺序如下：
 - 若参数中明确指定了 https://、http://、file:///，则按 URL 加载。
 - 若参数中包含 “<” 和 “>” 两个 HTML 中的特殊字符（同时这两个字符不可能是路径字符），按照 HTML 加载。
 - 若参数中不包含 “:” 字符则按照相对路径处理，依次在当前工作目录、EXE所在目录查找将其补全为完整路径。
 - 若完整路径指向的文件确实存在，则将其按照文件执行加载。
 - 若以上判断全部失败，则当作 URL 加载。
* 在 wkeBrowser 中将加载函数改为 wkeLoadW，可通过运行 wkeBrowser [加载参数] 直接进行测试。

#### 2015-11-24 23:51 修改 ChangeLog 更新日志的路径。
* 去掉 ChangeLog.txt。
* 将 Document/history.md 移至根目录下 ChangeLog.md。

#### 2015-11-24 23:38 去掉 wke 中对 std::string 的引用。
* 去掉 wke 中 jsBind.cpp 对 std::string 的引用，自己实现了一个简单的字符串缓存对象。

#### 2015-11-24 23:35 修正 wkeBrowser 无法访问百度的问题。
* 修正 wkeBrowser 中因在 wkeOnNavigation 中禁用百度，导致无法访问百度的问题。

#### 2015-11-24 23:31 增加 BrentHuang 完成的 vs2010 的工程文件。
* 增加 BrentHuang（群里光 光）完成的 vs2010 的工程文件，感谢 BrentHuang 的贡献。

#### 2015-10-06 14:18 增加基于 wkeWebView 的带窗口、消息处理、绘制的窗口/控件接口。
* 增加基于 wkeWebView 的带窗口、消息处理、绘制的窗口/控件，接口：
 - 使用 wkeCreateWindow 创建窗口或控件，当前支持普通弹出窗口、分层窗口（用于透明）、控件。
 - 使用 wkeShowWindow 显示、隐藏窗口或控件。
 - 使用 wkeEnableWindow 禁用、启用窗口或控件。
 - 使用 wkeMoveWindow/wkeMoveToCenter 移动窗口或控件。
 - 使用 wkeResizeWindow 设置窗口或控件的大小。
 - 使用 wkeSetWindowTitle 设置窗口标题。

* 增加了一个新的项目 wkexe，作为三个新项目（wkeClient、wkeSample）中第一个。此项目的功能：
 - 创建类似于 cmd.exe 之于 cmd、bat 文件的 WebApp 的执行器，加载不同的 html 文件执行任务。
 - 演示项目，用于演示 wke 接口的作用。

* 为了与 wkeCreateWindow 等一系列针对窗口的操作进行区分，修改新页面回调接口名称：
 - 更名 wkeOnNewWindow 为 wkeOnCreateView
 - 更名 wkeNewWindowCallback 为 wkeCreateViewCallback

* 去掉了在 wkeCreateWebView、wkeDestroyWebView 中 s_webViews 相关的东西。
* 修改 alert、confirm 弹出框的默认标题为 "wke" 。

#### 2015-10-05 01:30 增加地址加载、文档解析状态的回调接口。
* 增加地址加载、文档解析状态的回调接口：
 - wkeOnLoadingFinish 设置地址加载成功、失败、取消状态的回调函数。
 - wkeOnDocumentReady 设置文档解析成功的回调函数。

#### 2015-10-05 00:56 修正 wkeNavigationType 的值。
* 在 wke 中将 WebCore::NavigationType 转换为 wkeNavigationType 时使用了直接强制转换，因此需要保证两个 enum 定义的值完全一致。

#### 2015-10-04 22:27 实现打开新窗口的控制回调 。
* 实现打开新窗口的控制回调，使用 wkeOnNewWindow 设置回调函数。
* 在 demo/bin/file.html 当中增加 wkeOnNewWindow 的测试数据（wkeBrowser中暂未增加）。

#### 2015-10-04 17:11 修改实现代码中所有成员变量以 `m_` 开头 。
* 修改实现代码中所有成员变量的命令以 `m_` 开头（与 WebKit 内核保持一致）。

#### 2015-10-04 16:11 实现打开链接的控制回调。
* 实现打开链接的控制回调，使用 wkeOnNavigation 设置回调函数。
* 在 wkeBrowser 当中增加 wkeOnNavigation 的测试。

#### 2015-09-30 17:21 实现 alert、confirm、prompt 回调。
* 实现 alert、confirm、prompt 回调，并支持自定义实现，接口：
 - wkeOnAlertBox
 - wkeOnConfirmBox
 - wkeOnPromptBox

* 修正 wkeGetStringW 复制字符串截断的问题。
* wkeBrowser 中增加 wkeOnTitleChanged 和 wkeOnURLChanged 回调的测试。

#### 2015-09-30 16:36 规范 wke 回调接口。
* 修改所有回调函数类型名称为 `事件名称 + Callback` 的形式：
    - 更名 wkeOnTitleChanged 为 wkeTitleChangedCallback
    - 更名 wkeOnURLChanged 为 wkeURLChangedCallback
    - 更名 wkeOnPaintUpdated 为 wkePaintUpdatedCallback

* 去掉 wkeSetHandler、wkeGetHandler，所有回调定义单独的 wkeOn + 事件名称 形式的设置函数：
    - wkeOnTitleChanged
    - wkeOnURLChanged
    - wkeOnPaintUpdated

* 修改所有回调函数的回调参数，去掉 wkeClientHandler，增加 void* param。
* 去掉 wkeClientHandler 结构。


#### 2015-09-29 23:07 修正去掉 StringTable 依赖之后的编译问题。
* 修正去掉 StringTable 依赖之后的编译问题。

#### 2015-09-29 15:32 去掉 wke 项目中的 StringTable 依赖。
* 去掉 wke 项目中的 StringTable 依赖，因 StringTable 实际上全局字符串池，且只在 wkeFinalize 时才释放，若字符串函数（比如jsToString）调用过多，会导致内存只增不减。
* 修改 jsToString 和 jsToStringW 的实现，不依赖 StringTable，返回 static 字符串，因此其返回值会被下次调用冲掉，在使用不能保存字符串指针，应该立即保存到字符串数组或 std::string、std::wstring 当中再使用。
* 更名 jsToString 为 jsToTempString，更名 jsToStringW 为 jsToTempStringW，因其返回值会被下次调用冲掉。

#### 2015-09-29 09:10 增加两个 C 导出函数 wkeRepaintIfNeed 和 wkeGetViewDC 。
* 增加两个 C 导出函数 wkeRepaintIfNeed 和 wkeGetViewDC 。
* 修正 wkeSetHandler 和 wkeGetHandler 的 const 问题。

#### 2015-09-29 02:20 去掉 IWebView 接口。
* 因 IWebView* 纯虚函数的调用方式，与 C 接口没有大的区别，择其一即可。而 C 接口可兼容大多数的开发语言，所以保留 C 接口。不再支持 IWebView* 接口。
* 在 C++ 模式下导出符号增加 extern "C" 。

#### 2015-09-29 01:58 修改 wkeString 字符串相关接口。
* 修改 wkeString 的类型定义，使在 C 下更安全，在 C++ 下更方便。
* 更名 wkeToString 为 wkeGetString，更名 wkeToStringW 为 wkeGetStringW，使其符合 wke 的命令习惯 。
* 增加 wkeSetString 和 wkeSetStringW 。

#### 2015-09-29 01:39 整理 wke 内部结构。
* 整理 wke 内部的结构，将所有的 *.inl 文件使用 .h 和 .cpp 进行分离。
* 在 wke.h 当中包含上 windows.h，因其需要 HDC 的定义。

#### 2015-09-29 22:17 提交 Release 版的目标文件。
* 上次提交误提交了 Debug 版的 wke.dll，改为提交 Release 版的。

#### 2015-09-29 22:10 修改 wkeBrowser 对 wke 的调用为 C 接口调用。
* 修改 wkeBrowser 对 wke 的调用为 C 接口调用，因 IWebView* 纯虚函数的调用方式，与 C 接口没有大的区别，择其一即可。而容 C 接口可兼容大多数的开发语言，所以保留 C 接口，为抛弃 C++ 接口做准备。
* 修正 wkeGetCookie、wkeGetCookieW 函数没有 wkeWebView 参数的 BUG。
* 更名 wkeGetCaret 为 wkeGetCaretRect 。
* 修改 wkeBrowser，默认关闭代理（取消定义宏 WKE_BROWSER_USE_LOCAL_PROXY）。

#### 2015-09-29 22：01 修改 wke 实现中用到的 IWebView 为 CWebView。
* 修改实现中（如 wkeChromeClient、wkeFrameLoaderClient 等）用到的 IWebView 为 CWebView，因为实现需要知道 IWebView 之外的、具体实现相关的信息，用 IWebView 不合适。
* 此修改为后续的实现作准备。

#### 2015-09-29 21:06 合并 zhanjx1314 实现的设置 UserAgent 的功能。
* 合并 zhanjx1314（群里(__少） 实现的设置 UserAgent 的功能。
* 在 webBrowser 当中测试了设置 UserAgent 的功能。

#### 2015-09-23 02:02 修正 JavaScriptCore 当中脚本错误导致后面调用逻辑异常。
JSValueToXXXXX 和 JSObjectXXXX 函数族中的某些函数在转换/构造变量之后，会判断当前若有JS错误，则返回非法值。但这个错误有可能不是本次转换/构造中发生的，是由前次不相干的操作，比如某次脚本调用中的语法错误，此时返回非法值，则是不正确的。而且返回值不应该在调用完来修正，应该由被调用者来修正。

#### 2015-09-18 20:28 增加代理类型 socks4、socks4a、socks5、socks5h 的支持。
* 增加代理类型 socks4、socks4a、socks5、socks5 h的支持，接口不用修改。

#### 2015-09-18 20:01 增加全局配置函数，当前只支持代理配置。
* 增加全局配置函数 wkeConfigure，可在初始化之后执行全局性的配置操作。
* 修改 wkeBrowser 使用 wkeConfigure 进行全局性配置。

#### 2015-09-17 01:28 增加初始化时的全局配置功能，当前只支持代理配置。
* 增加初始化函数 wkeInitializeEx，可在初始化时同时进行一些全局性地配置 。
* 通过 wkeInitializeEx 增加代理配置功能，目前仅支持 HTTP 代理。
* 修改 wkeBrowser 使用 wkeInitializeEx 进行初始化，并增加代理测试。

#### 2015-09-07 00:23 两处小调整。
* 修改 readme.md 。
* 提交 release 版本的 demo/bin/wke.dll 。

#### 2015-09-04 20:40 修改项目 readme 文件。
* 增加文件 readme.md 描述项目相关信息，并在末尾附原项目 readme.txt 的内容。
* 删除原项目 readme.txt 。

#### 2015-09-04 19:56 实现 <input type="file"/> 文件选择功能。
* 实现 ChromeClient::runOpenPanel 函数，打开文件对话框，支持 multipe 属性多选文件。可通过 demo/bin/file.html 和 Document/demo/file.asp 测试。

#### 2015-09-04 19:53 增加从命令行提供的URL进行加载的功能。
* webBrowser 增加命令行支持，若有命令行参数，则将其作为URL加载。

#### 2015-09-03 10:37 增加项目规范、根据规范调整命名。
* 编写项目规范，位于 Document/rules.md，合作提交的朋友请按规范编写文档和代码。目前此规范还不完善，后续继续添加。
* 根据 Document/rules.md 中的命名规范，以及使语法更正确、含义更清晰，作了以下命名调整：
    - 更名 wkeClientHandler 为 wkeViewHandler（因这里的 "Client" 一词实际上是 WebKit 内部实现相关的东西，在接口中不需要）。
    - 更名 wkeSetClientHandler 为 wkeSetHandler （去掉 Client）。
    - 更名 IWebView::setClientHandler 为 IWebView::setHandler （去掉 Client）。
    - 更名 IWebView::getClientHandler 为 IWebView::handler （去掉 Client、成员函数获取属性不加 get）。
    - 更名 IWebView::cookieEnabled 为 IWebView::isCookieEnabled （获取是否属性前面加 is）。
    - 更名 IWebView::getCaret 为 IWebView::caretRect （成员函数获取属性不加 get、使含义更明确）。
    - 更名 IWebView::getViewDC 为 IWebView::viewDC （成员函数获取属性不加 get）。
    - 更名 IWebView::transparent 为 IWebView::isTransparent （获取是否属性前面加 is）。
    - 更名 IWebView::awaken 为 IWebView::wake（执行操作的函数使用动词）。
    - 更名 IWebView::contentsHeight 为 IWebView::contentHeight（content 即可代表所有内容，无需复数）。
    - 更名 IWebView::contentsWidth 为 IWebView::contentWidth（content 即可代表所有内容，无需复数）。
    - 更名 IWebView::contextMenuEvent 为 IWebView::fireContextMenuEvent（使含义更准确）。
    - 更名 IWebView::copy 为 IWebView::editorCopy （使含义更准确）。
    - 更名 IWebView::cut 为 IWebView::editorCut （使含义更准确）。
    - 更名 IWebView::delete_ 为 IWebView::editorDelete （使含义更准确）。
    - 更名 IWebView::focus 为 IWebView::setFocus （使含义更准确）。
    - 更名 IWebView::isLoadComplete 为 isLoadingCompleted （语法更准确、统一）。
    - 更名 IWebView::isLoaded 为 isLoadingSucceeded （语法更准确、统一）。
    - 更名 IWebView::isLoadFailed 为 isLoadingFailed （语法更准确、统一）。
    - 更名 IWebView::keyDown 为 IWebView::fireKeyDownEvent（使含义更准确）。
    - 更名 IWebView::keyPress 为 IWebView::fireKeyPressEvent（使含义更准确）。
    - 更名 IWebView::keyUp 为 IWebView::fireKeyUpEvent（使含义更准确）。
    - 更名 IWebView::mouseEvent 为 IWebView::fireMouseEvent（使含义更准确）。
    - 更名 IWebView::mouseWheel 为 IWebView::fireMouseWheelEvent（使含义更准确）。
    - 更名 IWebView::paste 为 IWebView::editorPaste （使含义更准确）。
    - 更名 IWebView::selectAll 为 IWebView::editorSelectAll  （使含义更准确）。
    - 更名 IWebView::tick 为 IWebView::IWebView::repaintIfNeeded（使含义更准确）。
    - 更名 IWebView::unfocus 为 IWebView::killFocus（使含义更准确） 。
    - 更名 jsObjectData 为 jsData（Object 函数统一不加 Object）。
    - 更名 jsObjectGetData 为 jsGetData（Object 函数统一不加 Object）。
    - 更名 ON_PAINT_UPDATED 为 wkeOnPaintUpdated（类型用大驼峰命名法）。
    - 更名 ON_TITLE_CHANGED 为 wkeOnTitleChanged（类型用大驼峰命名法）。
    - 更名 ON_URL_CHANGED 为 wkeOnUrlChanged（类型用大驼峰命名法）。
    - 更名 wkeInit 为 wkeInitialize （使含义更准确）。
    - 更名 wkeShutdown 为 wkeFinalize （使含义更准确）。
    - 更名 wkeVersion 为 wkeGetVersion（非成员函数获取数据加 get）。
    - 更名 wkeVersionString 为 wkeGetVersionString（非成员函数获取数据加 get）。
    - 更名 wkeTitle 为 wkeGetTitle，wkeTitleW 为 wkeGetTitleW（非成员函数获取数据加 get）。
    - 更名 wkeWidth 为 wkdGetWidth（非成员函数获取数据加 get）。
    - 更名 wkeHeight 为 wkdGetHeight（非成员函数获取数据加 get）。	
    - 更名 wkeWebViewName 为 wkeGetName（非成员函数获取数据加 get、WebView 相关操作函数命名统一不加 WebView）。
    - 更名 wkeZoomFactor 为 wkeGetZoomFactor（非成员函数获取数据加 get）。
    - 更名 wkeAwaken 为 wkeWake（wake 动词含义更准确）。
    - 更名 wkeContentsWidth 为 wkdGetContentWidth（content 即可代表所有内容无需复数、非成员函数获取数据加 get）。
    - 更名 wkeContentsHeight 为 wkeGetContentHeight（content 即可代表所有内容无需复数、非成员函数获取数据加 get）。
    - 更名 wkeSelectAll 为 wkeEditorSelectAll （使含义更准确）。
    - 更名 wkeCopy 为 wkeEditorCopy （使含义更准确）。
    - 更名 wkeCut 为 wkeEditorCut （使含义更准确）。
    - 更名 wkePaste 为 wkeEditorPaste （使含义更准确）。
    - 更名 wkeFocus 为 wkeSetFocus（使含义更准确）。
    - 更名 wkeUnfocus 为 wkeKillFocus（使含义更准确）。
    - 更名 wkeGlobalExec 为 wkeGetGlobalExec（非成员函数获取数据加 get）。
    - 更名 wkeIsLoadComplete 为 wkeIsLoadingCompleted  （语法更准确、统一）。
    - 更名 wkeIsLoaded 为 wkeIsLoadingSucceeded  （语法更准确、统一）。
    - 更名 wkeIsLoadFailed 为 wkeIsLoadingFailed  （语法更准确、统一）。
    - 更名 wkeMediaVolume 为 wkeGetMediaVolume（非成员函数获取数据加 get）。
    - 更名 wkeSetWebViewName 为 wkeSetName（WebView 相关操作函数命名统一不加 WebView）。
    - 更名 wkeCookieEnabled 为 wkeIsCookieEnabled（获取是否数据是前面加 is）。
    - 更名 wkeGetCaret 为 wkeGetCaretRect （使含义更明确）。

#### 2015-09-03 10:17 将回调全部集中到接口 wkeClientHandler 当中。
* 去掉 wkeBufHandler，将功能合并到 wkeClientHandler 当中。
* wkeClientHandler 中增加一个回调参数，以传递自定义参数。
* 去掉 wkeClientHandler 的 const 属性，避免在回调处理中要需要非 const 访问时导致问题。


#### 2015-08-31 02:13 修改 Document/history.md 的标题级别。 …
每条记录的标题修改为四级标题，因三级在浏览器中查看太大。


#### 2015-08-31 02:13 修改 Document/history.md 的记录格式。 …
每条记录不再使用严格的 markdown 格式，改为直接使用提交日志的文本，但增加 markdown 三级标题。


#### 2015-08-30 23:24 合并 zhanjx1314 实现的 POST 方式加载页面的功能。 …
合并 zhanjx1314 实现的 wkePostURL 函数，该函数可以 POST 方式加载页面。


#### 2015-08-30 22:59 合并 zhanjx1314 实现的获取 cookie 的功能。 …
合并 zhanjx1314 实现的获取 cookie 的功能（wchar_t* 版本），并在其基础上增加了一个 utf8* 版本的重载，另外在 webBrowser 中增加一个菜单项测试此功能。


#### 2015-08-30 02:00 调整 Document/history.md 标题级别（之前太大）。


#### 2015-08-30 01:40 修正 wkeLoadURL 加载中文路径 HTML 文件失败的问题。 …
wkeLoadURL 调用 `KURL::KURL(const KURL& base, const String& relative, const TextEncoding&)` 构造 KURL ，第二个参数传入的是 const char* 类型的 UTF-8 字符串以构造一个 WTF::String 对象，然而 WTF::String::String(const char*) 是把字符串按照 latin1 处理的，正确应该使用 WTF::String::fromUTF8 显式从 UTF-8 构造。可以用 wkeBrowser 加载 demo/bin/中文.html 测试。


#### 2015-08-30 01:36 增加 Document/utility.md 记录一些零碎信息。


#### 2015-08-30 01:23 修正 HTML 中图片地址包含中文时无法显示的问题。 …
WebCore::Document::completeURL 当中构造 KURL 对象时，会调用 encodeRelativeString 将所有 URL（包括本地路径）转换为 UTF-8 编码，导致文件打开失败，显然在 Windows 上路径应该是 GBK。可用 wkeBrowser 加载 demo/bin/english.html 测试。


#### 2015-08-30 01:26 调整 Document/history.md 记录日志的顺序，按时间从新到旧。


#### 2015-08-30 01:19 增加 webBrowser 拖放加载 HTML 文件的功能。


#### 2015-08-29 23:56 修正 jsSet、jsGet 始终访问的是第一次调用时的属性的问题。 …
JSC::Identifier 内部以 JSC::UString 保存属性名称，而 JSC::UString 内部把 char* 类型字符串的指针值作为其 hash 值，虽然在一个字符串内存上设置了与前次不同的字符串内容，但因为使用了同一内存地址（指针相同）， hash 值相同，会认为是同一个字符串。


#### 2015-08-29 22:14 修改 Javascript 绑定相关的接口。 …
* 修改 jsObject、jsArray 函数名为 jsEmptyObject、jsEmptyArray，使其名称更符合实际功能。
* 去掉 jsFunction 函数。
* 增加新的 jsObject、jsFunction 函数用于实现对象和函数的绑定，针对单独页面的绑定。
* 增加 Document 目录，用于存放一些说明性的东西。


#### 2015-08-29 21:22 合并从 SOUI 仓库获取的修改。 …
感谢 SOUI 的作者，其主要增加了两个功能：
* wkePaint 拷贝页面的内容图像到指定内存。
* wkeBufHandler 当页面的内容图像更新时回调通知。


#### 2015-08-29 21:16:57 忽略 build 目录（中间目录）。

2011-09-07
        version 1.00 released 