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