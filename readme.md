
项目介绍
---------

wke 是一个基于 WebKit 精简的浏览器引擎，原项目地址 <https://github.com/BlzFans/wke>。原作者已停止更新，本人出于技术研究的目的 fork 过来，业余时间继续维护，既学习 WebKit 的相关技术，也实验将 web 技术用于桌面UI。如果你对本项目感兴趣，欢迎请关注并随时提出你的宝贵意见。如果你也是一名 WebKit 的研究者、开发者，也欢迎你通过本项目来分享你宝贵的经验。

注意事项
---------

为了使本项目长期良好地发展下去，所有的开发维护都需要在准确细致的规范下进行。本人拍拍脑袋先写了一个简单的[规范文档](https://github.com/cexer/wke/blob/master/Document/rules.md)，目前远不够完善，但希望能投石引玉，吸引到大家的宝贝意见。

若你想快速浏览一下本项目完整的修改历史，可以浏览[提交日志](https://github.com/cexer/wke/commits/master)。也可以查看[修改记录](https://github.com/cexer/wke/blob/master/Document/history.md)了解最近的修改。

若你有任何与项目相关的问题或建议，请联系本人邮箱 <cexer@qq.com>，或者加入QQ群 178168957 和大家一起讨论。

**待完善**


附原项目的 `readme.txt` 文件
----------------------------

Web and Flash Embedded in 3D games, based on WebKit




Building wke

VS2005:
1 Install Visual Studio 2005 SP1
2 Install the Windows SDK 6.0
3 Open the vs2005/wke.sln solution file in Visual Studio and build the solution. 
  This can take from 30 minutes to 1 hours. More likely 40 minutes.

VS2008:
1 Install Visual Studio 2008 SP1
2 Install the Windows SDK 6.0 or newer
3 Open the vs2008/wke.sln solution file in Visual Studio and build the solution. 
  This can take from 30 minutes to 1 hours. More likely 40 minutes.

Run wke
The wkeBrowser.exe executable can be found at demo/bin/wkeBrowser.exe


Common Build Errors:

1. Cygwin exception::handle: Exception: STATUS_ACCESS_VIOLATION

   Close the Visual Studio, then relaunch and rebuild. This usually fixes the problem.