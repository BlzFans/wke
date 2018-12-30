## 测试绑定对象功能
运行wkeWebBrowser.exe，在地址栏输入`inject`回车，即可注册JS对象`test`，注册后就JS中可访问`test`对象的成员变量`value`和成员函数`msgbox`了。
在地址栏输入`javascript:test.msgbox('1')`测试调用成员函数。
在地址栏输入`javascript:document.write(test.value)`测试访问成员变量。