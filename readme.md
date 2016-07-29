wke 项目文档
===========================

目录
---------

- [项目介绍](#introduce)
- [编译指南](#build)
- [FAQ](#FAQ)
    - [FAQ1. wke 对 HTML5 的支持如何？](#FAQ1)
    - [FAQ2. 为什么编译出错？](#FAQ2)
    - [FAQ3. 为什么解决方案中 wke 项目没有设置为依赖 WebCore 项目？](#FAQ3)
- [贡献者名单](#contributors)
    - [BlzFans](#blzfans)
    - [Vincent.Lin√](#vincent-lin)
    - [（__少](#shao)
    - [光 光](#guang-guang)
    - [蚂蚁](#ma-yi)
    - [SOUI作者](#flyhigh)
    - [其它贡献者](#others)

项目介绍 <a name="introduce"></a>
---------

wke 是一个基于 WebKit 精简的浏览器引擎，原项目地址 <https://github.com/BlzFans/wke>。原作者已停止更新，本人出于技术研究的目的 fork 过来，业余时间继续维护，既学习 WebKit 的相关技术，也实验将 web 技术用于桌面UI。如果你对本项目感兴趣，欢迎请关注并随时提出你的宝贵意见。如果你也是一名 WebKit 的研究者、开发者，也欢迎你通过本项目来分享你宝贵的经验。

若想快速浏览一下本项目完整的修改历史，可以浏览[提交日志](https://github.com/cexer/wke/commits/master)。也可以查看[修改记录](https://github.com/cexer/wke/blob/master/Document/history.md)了解最近的修改。若有任何与项目相关的问题或建议，欢迎加入QQ群 **178168957** [WebUI:wke/cef/mshtml/miniblink](http://shang.qq.com/wpa/qunwpa?idkey=ebe64628d19349ed41e2e77b6dfdb4bdd11fc1cb46f6721f24de1a674bd3bf9b) 反馈，内有大神。


编译指南 <a name="build"></a>
---------

请严格按照以下步骤进行编译：

1. 从 [本项目（https://www.github.com/cexer/wke）](https://www.github.com/cexer/wke) 下载源码的 **ZIP 包**并解压项目目录。
2. 根据需要选择执行项目根目录的 vs2008.bat、vs2010.bat、vs2015.bat，该批处理会设置好依赖环境并用 VS 打开解决方案。
3. 在解决方案中，首先编译 WebCore 项目，成功之后再编译 wkeBrowser、wkexe 项目。

若不严格按照上述步骤编译，99%的可能性会踩坑翻车。如果想了解为什么必须这样编译，请见 [FAQ2. 为什么编译出错](#FAQ2)。


FAQ <a name="FAQ"></a>
---------

### FAQ1. wke 对 HTML5 的支持如何？<a name="FAQ1"></a>

wke 是由 WebKit 2011 年的版本精简而来，之后一直没有同步 WebKit 更新过，因此 wke 对 HTML5 的支持能力停留在 2011 年，一直未改进。一个更加直观的、然而不是很准确的概念就是 wke 对 HTML5 的支持大约是在 IE9 之上，在 IE10 之下，在 <http://html5test.com> 上跑分为 222 分，远在 CEF 的 530 分之下。因此若是对 HTML5 要求较高的环境，不建议使用 wke，请考虑 CEF 或者正在开发当中的 miniblink 内核。


### FAQ2. 为什么编译出错？<a name="FAQ2"></a>

99%的可能性是由于以下两个原因：

**1. 脑残的 git 客户端 autocrlf 功能的问题：**

JavaScriptCore、WebCore 项目编译时需要利用 perl、python 等脚本来从模板生成源码文件、从IDL生成接口文件等，为方便起见，wke 项目自带一个精简的 cygwin 项目，且已经预置了 perl、python 等软件，而 cygwin 版本的 perl、python 和 linux 版本的一样，只支持读取包含 linux 风格换行符（`\n`）的文件，这没有什么问题，因为 wke 项目中所有的模板文件、IDL文件都是 linux 格式的换行符。**然而！！！**，自作聪明的 git 客户端默认会开启一个脑残的功能 `autocrlf`，这个功能开启之后，使用 git 客户端拉取代码的时候，会将所有源码文件、模板文件、IDL文件的换行符强行修改为当前系统的换行符（在 windows 上即 `\r\n`），导致 perl、python 等软件无法正常读取文件，无法生成那一堆源码文件、接口文件…………，而下载 ZIP 包可以绕过 git 客户端的脑残功能。

如果想详细了解这个问题，请查看链接 [GitHub 第一坑：换行符自动转换](http://blog.jobbole.com/46200/)。

**2. 智障的 VS2010 - VS2015 找不到自己 nmake 问题：**

如上所述 JavaScriptCore、WebCore 项目编译时需要利用 perl、python 等脚本来生成一堆文件，而 perl、python 是VS通过自家的 nmake 调起来的。然而患了肥胖症兼老年痴呆的 VS2010 - VS2015，默认找不到 自家的 nmake，必须在启动VS之前调用一下 vsvars32.bat 设置环境变量。项目根目录 vs2010.bat、vs2015.bat 等文件的作用其实就是这个。VS2008 倒是没有这个问题，但为统一起见，也在根目录下放了一个 vs2008.bat。

执行以上操作若仍未解决问题，请加QQ群 **178168957** [WebUI:wke/cef/mshtml/miniblink](http://shang.qq.com/wpa/qunwpa?idkey=ebe64628d19349ed41e2e77b6dfdb4bdd11fc1cb46f6721f24de1a674bd3bf9b) 反馈。

### FAQ3. 为什么解决方案中 wke 项目没有设置为依赖 WebCore 项目？<a name="FAQ3"></a>

如 [FAQ2. 为什么编译出错](#FAQ2) 中的原因所描述的，因 JavaScriptCore、WebCore 等项目编译前会有很多的脚本调用，不论项目是否已经编译成功过，这些调用每次都会执行一遍，很费时间，而实际上 WebCore、JavaScriptCore 这两个项目在开发当中，是极少会有修改真正需要重新编译的。如果把 wke 设置为依赖 WebCore，那么每次 wke 编译都会导致 WebCore、JavaScriptCore 项目进行脚本调用，花费不必要的时间，因此折衷处理：设置 wke 不依赖 WebCore，在项目初始编译或者 WebCore 真正有修改时时，手动编译一次 WebCore，然后绝大多时间编译都不用花费不必要的脚本调用时间了。


贡献者名单 <a name="contributors"></a>
---------

### BlzFans <a name="blzfans"></a>

`BlzFans` 此位大侠功力深不可测然而为人极低调，将 wke 项目甩给开源界之后，从容离开去实现自己更高的人生价值了，挥一挥衣袖，不带走一声膜拜。感谢他的努力，如今我们除了在他开凿的道路上逶迤前进，也希望有一天他能够再次回到 WebUI 的队伍中来，带我们直接飞往 WebUI 技术的巅峰。

### Vincent.Lin√<a name="vincent-lin"></a>

`Vincent.Lin√` 是促成建立QQ群、建立 github 项目的原老。wke 之前一直没有QQ群和持续维护的开源项目，直到某次偶遇 `Vincent.Lin√` 并和他一起解决了几个 wke 的BUG，才萌生了要建立一个QQ群，新建个开源项目的想法，从此BUG修复、新增功能之类的有了一个分享和积累的仓库，wke 的用户有一个交流的场所。


### （__少 <a name="shao"></a>

`（__少` 也是群的原老之一，为 wke 贡献了不少代码，比如增加了 cookie 操作的接口、增加了 POST 方式加载页面的接口等。作为群管理员（老鸨）之一，任劳任怨，XX如一日每日参与讨论技术问题、闲聊扯谈、为新人答疑解惑。


###  光 光 <a name="guang-guang"></a>

`光 光` 此君也是来无影去无踪的大神。wke 之前只有 vs2005、vs2008 的VS项目文件，经常有人想用 vs2010、vs2015 等版本来编译而不知法，苦得哭爹喊娘，`光 光` 这时候出来为 wke 贡献了 vs2010、vs2013、vs2015 的工程。然后他也神秘消失了。然而也许 vs2017、vs2018 上市，我们为编译问题焦头烂额的时候，他又会踏着七色云彩出来解救众生。


### 蚂蚁 <a name="ma-yi"></a>

`蚂蚁` 老师是如今 wke 群的中坚技术力量，久经杀场，经验丰富，上能架构集群，下能逆向驱动。`蚂蚁` 老师不仅解决了困扰我们已久的 wke 绘制慢的问题（比如某些情况下会导致 codemirror 滚动条卡顿）,并且实现了人民呼声很高的 websocket 功能（现在 wke 也能玩蝌蚪聊天了），功莫大焉。


### SOUI作者 <a name="flyhigh"></a>

`SOUI` 是新一代 DirectUI 界面库，其作者在我的记忆中是最早将 wke 从开源网站中拿到实际的 SOUI 工程中。他为 wke 增强了不少功能、修复了一些 BUG，比如绘制回调、URL改变回调等。也是从他的 SOUI 群里 wke 被越来越多的人知道。我们的这个项目仓库里，就有他最早的贡献，有兴趣的可以加他的 SOUI QQ群 **229313785**。


### 其它贡献者 <a name="others"></a>

其实还有很多其它人，也为 wke 项目做过贡献过，不管是代码上的还是别的方法的的贡献，在此一并感谢。

是每个贡献者的一份力量一起，使 wke 越来越强大！