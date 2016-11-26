# EasyLogger 核心功能移植说明

---

## 1、下载源码

[点击此链接](https://github.com/armink/EasyLogger/archive/master.zip)即可直接下载位于Github上的源码。软件版本号位于 `\easylogger\inc\elog.h` 的 `ELOG_SW_VERSION` 宏定义。当然你也可以选择使用Git工具直接克隆到本地。

> 建议：点击项目主页 https://github.com/armink/EasyLogger 右上角 **Watch & Star**，这样项目有更新时，会及时以邮件形式通知你。

如果Github下载太慢，也可以点击项目位于的国内仓库下载的链接([OSChina](https://git.oschina.net/Armink/EasyLogger/repository/archive?ref=master)|[Coding](https://coding.net/u/armink/p/EasyLogger/git/archive/master))。

## 2、导入项目

在导入到项目前，先打开[`\demo\`](https://github.com/armink/EasyLogger/tree/master/demo)文件夹，检查下有没有与项目平台一致的Demo。如果有则先直接跳过2、3、4章节，按照第5章的要求设置参数，并运行、验证Demo。验证通过再按照下面的导入项目要求，将Demo中的移植文件直接导入到项目中即可。

- 1、先解压下载好的源码包，文件的目录结构大致如下：

|源文件                                 |描述   |
|:------------------------------        |:----- |
|\easylogger\src\elog.c                 |核心功能源码|
|\easylogger\src\elog_async.c           |核心功能异步输出模式源码|
|\easylogger\src\elog_buf.c             |核心功能缓冲输出模式源码|
|\easylogger\src\elog_utils.c           |EasyLogger常用小工具|
|\easylogger\port\elog_port.c           |不同平台下的EasyLogger移植接口|
|\easylogger\plugins\                   |插件源码目录|
|\docs\zh\                              |所有中文文档目录|
|\demo\non_os\stm32f10x\                |stm32f10x裸机的 demo|
|\demo\os\linux\                        |linux平台 demo|
|\demo\os\windows\                      |windows平台 demo|
|\demo\os\rt-thread\stm32f10x\          |stm32f10x基于[RT-Thread](http://www.rt-thread.org/)的demo（包含Flash插件demo）|


- 2、将`\easylogger\`（里面包含`inc`、`src`及`port`的那个）文件夹拷贝到项目中；
- 3、添加`\easylogger\src\elog.c`、`\easylogger\src\elog_utils.c`及`\easylogger\port\elog_port.c`这些文件到项目的编译路径中（elog_async.c 及 elog_buf.c 视情况选择性添加）；
- 4、添加`\easylogger\inc\`文件夹到编译的头文件目录列表中；

## 3、移植接口

### 3.1 移植初始化

EasyLogger移植初始化。初始化EasyLogger移植所需的资源等等。

```C
ElogErrCode elog_port_init(void)
```

### 3.2 日志输出接口

日志最终输出的末端接口。可以在里面增加输出到终端、输出到文件、输出到Flash等方法。

```C
void elog_port_output(const char *log, size_t size)
```

|参数                                    |描述|
|:-----                                  |:----|
|log                                     |日志内容|
|size                                    |日志大小|

例子：
```c
void elog_port_output(const char *log, size_t size) {
    /* output to terminal */
    printf("%.*s", size, log);
    /* output to flash */
    elog_flash_write(log, size);
}
```

### 3.3 对日志输出加锁

对日志输出方法进行加锁，保证日志在并发输出时的正确性。有操作系统时可以使用获取信号量来加锁，裸机时可以通过关闭全局中断来加锁。

```C
void elog_port_output_lock(void)
```

### 3.4 对日志输出解锁

与日志输出加锁功能对应。

```C
void elog_port_output_unlock(void)
```

### 3.5 获取当前时间

返回当前时间，将会显示在日志中。

```C
const char *elog_port_get_time(void)
```

### 3.6 获取进程信息

返回进程信息，将会显示在日志中。（没有则可以返回 `""` ）

```C
const char *elog_port_get_p_info(void)
```

### 3.7 获取线程信息

返回线程信息，将会显示在日志中。（没有则可以返回 `""`）

```C
const char *elog_port_get_t_info(void)
```

## 4、设置参数

配置时需要修改项目中的`elog_cfg.h`文件，开启、关闭、修改对应的宏即可。

### 4.1 输出开关
    
开启后，日志才会被输出。如果关闭，所有日志输出代码都将会被替换为空。

- 操作方法：开启、关闭`ELOG_OUTPUT_ENABLE`宏即可

### 4.2 输出级别

此方法为静态设置输出级别。设置后，比当前输出级别低的日志输出代码将会被替换为空。

输出控制级别： `ELOG_OUTPUT_ENABLE（总开关） > ELOG_OUTPUT_LVL（静态） > elog_set_filter（动态）`

- 操作方法：修改`ELOG_OUTPUT_LVL`宏对应值即可

可选的设置级别如下：
```
/* output log's level */
#define ELOG_LVL_ASSERT                      0
#define ELOG_LVL_ERROR                       1
#define ELOG_LVL_WARN                        2
#define ELOG_LVL_INFO                        3
#define ELOG_LVL_DEBUG                       4
#define ELOG_LVL_VERBOSE                     5
```
### 4.3 断言开关

开启后，将会启动断言检查功能。如果关闭，所有断言检查代码都将会被替换为空。

- 操作方法：开启、关闭`ELOG_ASSERT_ENABLE`宏即可

### 4.4 每行日志的缓冲区大小

该配置决定了日志一行最多输出多少字符，单位：byte。

- 操作方法：修改`ELOG_LINE_BUF_SIZE`宏对应值即可

### 4.5 行号最大长度

建议设置`5`较为合适，用户可以根据自己的文件行号最大值进行设置，例如最大行号为：`9999`，则可以设置行号最大长度为`4`

- 操作方法：修改`ELOG_LINE_NUM_MAX_LEN`宏对应值即可

### 4.6 过滤标签最大长度

日志中标签内容及用户设置过滤标签的最大长度，单位：byte。

- 操作方法：修改`ELOG_FILTER_TAG_MAX_LEN`宏对应值即可

### 4.7 过滤关键词最大长度

用户可设置过滤关键字的最大长度，单位：byte。

- 操作方法：修改`ELOG_FILTER_KW_MAX_LEN`宏对应值即可

### 4.8 换行符

用户可以根据自己的使用场景自定义换行符，例如：`"\r\n"`，`"\n"`

- 操作方法：修改`ELOG_NEWLINE_SIGN`宏对应值即可

### 4.9 颜色

> **注意** ：启用颜色功能需先定义 `ELOG_COLOR_ENABLE`

每个级别的日志均有默认颜色。如果想修改，请先查看在 `elog.c` 的头部定义的各种颜色及字体风格，这里以修改 `VERBOSE` 级别日志来举例：

首先选择前景色为白色，再选择背景色为黑色，最后字体风格为粗体

那么最终的配置如下：

```
#define ELOG_COLOR_VERBOSE         (F_WHITE B_BLACK S_BOLD)
```

- 操作方法：增加并修改`ELOG_COLOR_VERBOSE`宏对应值即可，其他级别日志颜色的修改以此类推

### 4.10 异步输出模式

开启异步输出模式后，将会提升用户应用程序的执行效率。应用程序在进行日志输出时，无需等待日志彻底输出完成，即可直接返回。

- 操作方法：开启、关闭`ELOG_ASYNC_OUTPUT_ENABLE`宏即可

#### 4.10.1 异步输出模式缓冲区大小

- 默认大小：`(ELOG_LINE_BUF_SIZE * 10)` ，不定义此宏，将会自动按照默认值设置
- 操作方法：修改`ELOG_ASYNC_OUTPUT_BUF_SIZE`宏对应值即可

#### 4.10.2 异步按行输出日志

由于异步输出方式内部拥有缓冲区，所以直接输出缓冲区中积累的日志时，日志移植输出方法 (`elog_port_output`) 输出的日志将不会按照 **行日志** （以换行符结尾）的格式进行输出。这使得无法在移植输出方法中完成日志的分析处理。开启此功能后，将会最大限度保证移植输出方法每次输出的日志格式都为行日志。

- 操作方法：开启、关闭`ELOG_ASYNC_LINE_OUTPUT`宏即可

#### 4.10.3 启用 pthread 库

异步输出模式默认是使用 POSIX 的 pthread 库来实现，用户的平台如果支持 pthread ，则可以开启此宏。对于一些缺少 pthread 的支持平台，可以关闭此宏，参考 `elog_async.c` 中关于日志异步输出线程的实现方式，自己动手实现此功能。

- 操作方法：开启、关闭`ELOG_ASYNC_OUTPUT_USING_PTHREAD`宏即可

### 4.11 缓冲输出模式

开启缓冲输出模式后，如果缓冲区不满，用户线程在进行日志输出时，无需等待日志彻底输出完成，即可直接返回。但当日志缓冲区满以后，将会占用用户线程，自动将缓冲区中的日志全部输出干净。同时用户也可以在非日志输出线程，通过定时等机制使用 `void elog_flush(void)` 将缓冲区中的日志输出干净。

- 操作方法：开启、关闭`ELOG_BUFF_OUTPUT_ENABLE`宏即可

#### 4.11.1 缓冲输出模式缓冲区大小

- 默认大小：`(ELOG_LINE_BUF_SIZE * 10)` ，不定义此宏，将会自动按照默认值设置
- 操作方法：修改`ELOG_BUF_OUTPUT_BUF_SIZE`宏对应值即可

## 5、测试验证

如果`\demo\`文件夹下有与项目平台一致的Demo，则直接编译运行，观察测试结果即可。无需关注下面的步骤。

每次使用前，务必先执行`elog_init()`方法对EasyLogger库进行初始化，保证初始化没问题后，再设置输出格式、过滤级别、断言钩子等，最后记得调用`elog_start()`方法启动EasyLogger，否则EasyLogger将不会开始工作。启动后接上终端就即可日志的输出信息，可以参考并运行这里的[日志测试函数](https://github.com/armink/EasyLogger/blob/master/demo/os/windows/main.c#L76-L88)。如果出现错误或断言，需根据提示信息检查移植配置及接口。

下面为常见初始化方式（[点击查看源码](https://github.com/armink/EasyLogger/blob/master/demo/os/windows/main.c)）
```c
/* close printf buffer */
setbuf(stdout, NULL);
/* initialize EasyLogger */
elog_init();
/* set EasyLogger log format */
elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);
elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_ALL & ~ELOG_FMT_FUNC);
elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL & ~ELOG_FMT_FUNC);
/* start EasyLogger */
elog_start();
```