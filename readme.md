
项目介绍
---------

wke 是一个基于 WebKit 精简的浏览器引擎，原项目地址 <https://github.com/BlzFans/wke>。原作者已停止更新，本人出于技术研究的目的 fork 过来，业余时间继续维护，既学习 WebKit 的相关技术，也实验将 web 技术用于桌面UI。如果你对本项目感兴趣，欢迎请关注并随时提出你的宝贵意见。如果你也是一名 WebKit 的研究者、开发者，也欢迎你通过本项目来分享你宝贵的经验。


注意事项
---------

为了使本项目长期良好地发展下去，所有的开发维护都需要在准确细致的规范下进行。本人拍拍脑袋先写了一个简单的[规范文档](https://github.com/cexer/wke/blob/master/Document/rules.md)，目前远不够完善，但希望能投石引玉，吸引到大家的宝贝意见。

若你想快速浏览一下本项目完整的修改历史，可以浏览[提交日志](https://github.com/cexer/wke/commits/master)。也可以查看[修改记录](https://github.com/cexer/wke/blob/master/Document/history.md)了解最近的修改。

若你有任何与项目相关的问题或建议，请联系本人邮箱 <cexer@qq.com>，或者加入QQ群 *178168957* 和大家一起讨论。


FAQ
---------

- [Q1. 编译出错解决办法](#Q1)
- [Q2. 为什么解决方案中 wke 项目没有设置为依赖 WebCore 项目](#Q2)


### <a name="Q1"></a>Q1. 编译出错解决办法。

**若出现编译出错，首先按照以下步骤操作， 99% 可以解决问题：**

1. 从 [本项目](https://www.github.com/cexer/wke) 下载源码的 ZIP 包并解压项目目录。
2. 根据需要，选择执行项目根目录的 vs2008.bat、vs2010.bat、vs2015.bat，该批处理会设置好依赖环境并用 VS 打开解决方案。
3. 在解决方案中，首先编译 WebCore 项目，成功之后再编译 wkeBrowser、wkexe 项目。

出错原因，主要有两个：

**1. 脑残的 git 客户端 autocrlf 功能的问题：**

JavaScriptCore、WebCore 项目编译时需要利用 perl、python 等脚本来从模板生成源码文件、从IDL生成接口文件等，为方便起见，wke 项目自带一个精简的 cygwin 项目，且已经预置了 perl、python 等软件，而 cygwin 版本的 perl、python 和 linux 版本的一样，只支持读取包含 linux 风格换行符（`\n`）的文件，这没有什么问题，因为 wke 项目中所有的模板文件、IDL文件都是 linux 格式的换行符。**然而！！！**，自作聪明的 git 客户端默认会开启一个脑残的功能 `autocrlf`，这个功能开启之后，使用 git 客户端拉取代码的时候，会将所有源码文件、模板文件、IDL文件的换行符强行修改为当前系统的换行符（在 windows 上即 `\r\n`），导致 perl、python 等软件无法正常读取文件，无法生成那一堆源码文件、接口文件…………，而下载 ZIP 包可以绕过 git 客户端的脑残功能。

**2. 智障的 VS2010 - VS2015 找不到自己 nmake 问题：**

如上所述 JavaScriptCore、WebCore 项目编译时需要利用 perl、python 等脚本来生成一堆文件，而 perl、python 是VS通过自家的 nmake 调起来的。然而患了肥胖症兼老年痴呆的 VS2010 - VS2015，默认找不到 自家的 nmake，必须在启动VS之前调用一下 vsvars32.bat 设置环境变量。项目根目录 vs2010.bat、vs2015.bat 等文件的作用其实就是这个。VS2008 倒是没有这个问题，但为统一起见，也在根目录下放了一个 vs2008.bat。

执行以上操作若仍未解决问题，请加群 [WebUI:wke/cef/mshtml/miniblink](http://shang.qq.com/wpa/qunwpa?idkey=ebe64628d19349ed41e2e77b6dfdb4bdd11fc1cb46f6721f24de1a674bd3bf9b) 反馈，内有大神。

### <a name="Q2"></a>Q2. 为什么解决方案中 wke 项目没有设置为依赖 WebCore 项目？

如 [编译出错解决办法](#Q1) 中的原因所描述的，因 JavaScriptCore、WebCore 等项目编译前会有很多的脚本调用，不论项目是否已经编译成功过，这些调用每次都会执行一遍，很费时间，而实际上 WebCore、JavaScriptCore 这两个项目在开发当中，是极少会有修改真正需要重新编译的。如果把 wke 设置为依赖 WebCore，那么每次 wke 编译都会导致 WebCore、JavaScriptCore 项目进行脚本调用，花费不必要的时间，因此折衷处理：设置 wke 不依赖 WebCore，在项目初始编译或者 WebCore 真正有修改时时，手动编译一次 WebCore，然后绝大多时间编译都不用花费不必要的脚本调用时间了。

