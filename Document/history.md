#### 2015-08-30 22:59 合并[zhanjx1314](https://github.com/zhanjx1314/oos)实现的获取cookie的功能。
合并[zhanjx1314](https://github.com/zhanjx1314/oos)实现的获取cookie的功能（`wchar_t*`版本），并在其基础上增加了一个`utf8*`版本的重载，另外在webBrowser中增加一个菜单项测试此功能。

#### 2015-08-30 02:00 调整`Document/history.md`标题级别（之前太大）。

#### 2015-08-30 01:40 修正`CWebView::loadURL`加载中文路径的HTML文件失败的问题。
`CWebView::loadURL`调用`KURL::KURL(const KURL& base, const String& relative, const TextEncoding&)`构造`KURL`，第二个参数传入的是`const char*`类型的UTF-8字符串以构造一个`String`对象，然而`String::String(const char*)`是把字符串按照latin1处理的，正确应该使用`String::fromUTF8`显式从UTF-8构造。可以用wkeBrowser加载`demo/bin/中文.html`测试。

#### 2015-08-30 01:36 增加`Document/utility.md`记录一些零碎信息。

#### 2015-08-30 01:23 修正HTML中图片地址包含中文时无法显示的问题。
`WebCore::Document::completeURL`当中构造`KURL`对象时，会调用`encodeRelativeString`将所有URL（包括本地路径）转换为UTF-8编码，导致文件打开失败，显然在Windows上路径应该是GBK。可用wkeBrowser加载`demo/bin/english.html`测试。

#### 2015-08-30 01:26 调整`Document/history.md`日志排列顺序按时间从新到旧。

#### 2015-08-30 01:19 增加webBrowser拖放加载HTML文件的功能。

#### 2015-08-29 23:56 修正`jsSet`/`jsGet`多次调用不同属性却始终获取/设置的是第一次调用时的属性的问题。
`JSC::Identifier`内部以`JSC::UString`保存属性名称，而`JSC::UString`内部把`char*`类型字符串的指针值作为其hash值，虽然在一个字符串内存上设置了与前次不同的字符串内容，但因为使用了同一内存地址（指针相同），hash值相同，会认为是同一个字符串。

#### 2015-08-29 22:14 增删、修改JS绑定相关的接口，增加`Document/说明.txt`。
1. 修改`jsObject`、`jsArray`函数名为`jsEmptyObject`、`jsEmptyArray`，意义更明确。
2. 去掉`jsFunction`函数。
3. 增加新的`jsObject`、`jsFunction`函数用于实现对象和函数的绑定，针对单独的页面绑定。
4. 增加`Document`目录，用于存放一些说明性的东西。
