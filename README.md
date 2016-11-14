# EasyLogger

---

# 1. 介绍

[EasyLogger](https://github.com/armink/EasyLogger) 是一款超轻量级(ROM<1.6K, RAM<0.3K)、高性能的 C/C++ 日志库，非常适合对资源敏感的软件项目，例如： IoT 产品、可穿戴设备、智能家居等等。相比 log4c、zlog 这些知名的 C/C++ 日志库， EasyLogger 的功能更加简单，提供给用户的接口更少，但上手会很快，更多实用功能支持以插件形式进行动态扩展。

## 1.1 主要特性

- 支持用户自定义输出方式（例如：终端、文件、数据库、串口、485、Flash...）；
- 日志内容可包含级别、时间戳、线程信息、进程信息等；
- 日志输出被设计为线程安全的方式，并支持 **异步输出** 及 **缓冲输出** 模式；
- 支持多种操作系统（[RT-Thread](http://www.rt-thread.org/)、UCOS、Linux、Windows...），也支持裸机平台；
- 日志支持 **RAW格式** ；
- 支持按 **标签**  、 **级别** 、 **关键词** 进行动态过滤；
- 各级别日志支持不同颜色显示
- 扩展性强，支持以插件形式扩展新功能。

> 名词解释：
1、RAW格式：未经过格式化的原始日志。
2、标签：在软件中可以按照文件、模块、功能等方面，对需要打印的日志设定标签，实现日志分类。

## 1.2 插件

- 1、Flash：使用 [EasyFlash](https://github.com/armink/EasyFlash) 库提供的Flash操作接口，无需文件系统，直接将日志存储在 Flash 中。
- 2、File（正在开发）：支持文件转档、软件运行时动态加载配置文件等与文件日志输出相关功能。
- 3、敬请期待……

## 1.3 Star & Fork

后续我还会提供更多插件。也非常欢迎大家设计、开发更多实用插件和功能，一起来完善 EasyLogger  **([Github](https://github.com/armink/EasyLogger)|[OSChina](http://git.oschina.net/armink/EasyLogger)|[Coding](https://coding.net/u/armink/p/EasyLogger/git))** 。如果觉得这个开源项目很赞，可以点击[项目主页](https://github.com/armink/EasyLogger) 右上角的 **Star** ，同时把它推荐给更多有需要的朋友。

# 2. 使用

### 2.1 参数配置

EasyLogger 拥有过滤方式、输出格式、输出开关这些属性。

- 过滤方式支持按照标签、级别、关键词进行过滤；
- 可以动态的开启/关闭日志的输出；
- 可设定动态和静态的输出级别（静态：一级开关，通过宏定义；动态：二级开关，通过API接口）。

> 注：目前参数配置及输出方式都是单例模式，即全局只支持一种配置方式。此模式下，软件会较为简单，但是无法支持复杂的输出方式。

### 2.2 输出级别

参考 Android Logcat ，级别最高为 0(Assert) ，最低为 5(Verbose) 。

```
0.[A]：断言(Assert)
1.[E]：错误(Error)
2.[W]：警告(Warn)
3.[I]：信息(Info)
4.[D]：调试(Debug)
5.[V]：详细(Verbose)
```

#### 2.2.1 输出缤纷多彩的日志

各个级别日志默认颜色效果如下。用户也可以根据自己的喜好，在 `elog_cfg.h` 对各个级别日志的颜色及字体风格进行单独设置。

![TextColor](https://raw.githubusercontent.com/armink/EasyLogger/master/docs/zh/images/TextColor.png)

### 2.3 输出过滤

支持按照 **级别、标签及关键词** 进行过滤。日志内容较多时，使用过滤功能可以更快定位日志，保证日志的可读性。更多的过滤功能设置方法及细节请阅读[`\docs\zh\api\kernel.md`](https://github.com/armink/EasyLogger/blob/master/docs/zh/api/kernel.md)文档

### 2.4 输出格式

输出格式支持：级别、时间、标签、进程信息、线程信息、文件路径、行号、方法名。每种优先级别可以独立设置输出格式。

> 注：默认为 **RAW格式**，RAW格式日志不支持标签过滤

### 2.5 输出方式

通过用户的移植，可以支持任何一种输出方式。只不过对于某种输出方式可能引入的新功能，需要通过插件实现，例如：文件转存，检索Flash日志等等。后期会有更多的插件开源出来。下面简单对比下部分输出方式使用场景：

- 终端：方便用户动态查看，不具有存储功能；
- 文件与Flash：都具有存储功能，用户可以查看历史日志。但是文件方式需要文件系统的支持，而Flash方式更加适合应用在无文件系统的小型嵌入式设备中。

### 2.6 Demo

### 2.6.1 核心功能

下图为在终端中输入命令来控制日志的输出及过滤器的设置，更加直观的展示了 EasyLogger 核心功能。

- Demo 路径：[`\demo\os\rt-thread\stm32f10x\`](https://github.com/armink/EasyLogger/tree/master/demo/os/rt-thread/stm32f10x)
- API 文档：[`\docs\zh\api\kernel.md`](https://github.com/armink/EasyLogger/blob/master/docs/zh/api/kernel.md)
- 移植文档：[`\docs\zh\port\kernel.md`](https://github.com/armink/EasyLogger/blob/master/docs/zh/port/kernel.md)

![easylogger](https://raw.githubusercontent.com/armink/EasyLogger/master/docs/zh/images/EasyLoggerDemo.gif)

### 2.6.2 Flash Log（将日志保存到 Flash 中）

下图过程为通过控制台输出日志，并将输出的日志存储到 Flash 中。重启再读取上次保存的日志，最后清空 Flash 日志。

- Demo 路径：[`\demo\os\rt-thread\stm32f10x\`](https://github.com/armink/EasyLogger/tree/master/demo/os/rt-thread/stm32f10x)
- API 文档：[`\docs\zh\api\flash.md`](https://github.com/armink/EasyLogger/blob/master/docs/zh/api/flash.md)
- 移植文档：[`\docs\zh\port\flash.md`](https://github.com/armink/EasyLogger/blob/master/docs/zh/port/flash.md)

![FlashLog](https://raw.githubusercontent.com/armink/EasyLogger/master/docs/zh/images/LogDemo.gif)

# 3. 文档

具体内容参考[`\docs\zh\`](https://github.com/armink/EasyLogger/tree/master/docs/zh)下的文件。务必保证在 **阅读文档** 后再移植使用。

# 4. 后期

- 1、~~Flash存储：在[EasyFlash](https://github.com/armink/EasyFlash)中增加日志存储、读取功能，让EasyLogger与其无缝对接。使日志可以更加容易的存储在 **非文件系统** 中，~~并具有历史日检索的功能；
- 2、配置文件：文件系统下的配置文件；
- 3、文件转档：文件系统下支持文件按容量转档，按时间区分；
- 4、日志助手：开发跨平台的日志助手，兼容Linux、Windows、Mac系统，打开助手即可查看、过滤（支持正则表达式）、排序、保存日志等。前端：[HTML5](https://zh.wikipedia.org/wiki/HTML5) + [Bootstrap](https://github.com/twbs/bootstrap) + [AngularJS](https://angularjs.org/) + [NW.js](http://www.oschina.net/p/nwjs)，后端：[Rust](https://github.com/rust-lang/rust) + [iron](https://github.com/iron/iron) + [rust-websocket](https://github.com/cyderize/rust-websocket) + [serial-rs](https://github.com/dcuddeback/serial-rs)；
- 5、~~异步输出：目前日志输出与用户代码之间是同步的方式，这种方式虽然软件简单，也不存在日志覆盖的问题。但在输出速度较低的平台下，会由于增加日志功能，而降低软件运行速度。所以后期会增加 **异步输出** 方式，关键字过滤也可以放到异步输出中去；~~
- 6、Arduino：增加Arduino lib，并提供其Demo；

# 5. 许可

MIT Copyright (c) armink.ztl@gmail.com
