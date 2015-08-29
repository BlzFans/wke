## 2015-08-29 22:14 增删、修改JS绑定相关的接口，增加`Document/说明.txt`。
1. 修改`jsObject`、`jsArray`函数名为`jsEmptyObject`、`jsEmptyArray`，意义更明确。
2. 去掉`jsFunction`函数。
3. 增加新的`jsObject`、`jsFunction`函数用于实现对象和函数的绑定，针对单独的页面绑定。
4. 增加`Document`目录，用于存放一些说明性的东西。

## 2015-08-29 23:56 修正jsSet/jsGet多次调用不同属性却始终获取/设置的是第一次调用时的属性的问题。
`JSC::Identifier`内部以`JSC::UString`保存属性名称，而`JSC::UString`内部把`char*`类型字符串的指针值作为其hash值，虽然在一个字符串内存上设置了与前次不同的字符串内容，但因为使用了同一内存地址（指针相同），hash值相同，会认为是同一个字符串。