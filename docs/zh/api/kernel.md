# EasyLogger 核心功能 API 说明

---

所有核心功能API接口都在[`\easylogger\inc\elog.h`](https://github.com/armink/EasyLogger/blob/master/easylogger/inc/elog.h)中声明。以下内容较多，可以使用 **CTRL+F** 搜索。

> 建议：点击项目主页 https://github.com/armink/EasyLogger 右上角 **Watch & Star**，这样项目有更新时，会及时以邮件形式通知你。

## 1、用户使用接口

### 1.1 初始化

初始化的 EasyLogger 的核心功能，初始化后才可以使用下面的API。

```
ElogErrCode elog_init(void)
```

### 1.2 启动

**注意**：在初始化完成后，必须调用启动方法，日志才会被输出。

```
void elog_start(void)
```

### 1.3 输出日志

所有日志的级别关系大小如下：

```
级别 标识 描述
0    [A]  断言(Assert)
1    [E]  错误(Error)
2    [W]  警告(Warn)
3    [I]  信息(Info)
4    [D]  调试(Debug)
5    [V]  详细(Verbose)
```

#### 1.3.1 输出基本日志

所有级别的日志输出方法如下，每种级别都有两种简化方式，用户可以自行选择。

```c
#define elog_assert(tag, ...) 
#define elog_a(tag, ...) //简化方式1，每次需填写 LOG_TAG
#define log_a(...)       //简化方式2，LOG_TAG 已经在文件顶部定义，使用前无需填写 LOG_TAG

#define elog_error(tag, ...)
#define elog_e(tag, ...)
#define log_a(...)

#define elog_warn(tag, ...)
#define elog_w(tag, ...)
#define log_a(...)

#define elog_info(tag, ...)
#define elog_i(tag, ...)
#define log_a(...)

#define elog_debug(tag, ...)
#define elog_d(tag, ...)
#define log_a(...)

#define elog_verbose(tag, ...)
#define elog_v(tag, ...)
#define log_a(...)
```

|参数                                    |描述|
|:-----                                  |:----|
|tag                                     |日志标签|
|...                                     |不定参格式，与`printf`入参一致，放入将要输出日志|

**技巧一** ：对于每个源代码文件，可以在引用 `elog.h` 上方，根据模块的不同功能，定义不同的日志标签，如下所示，这样既可直接使用 `log_x` 这类无需输入标签的简化方式 API 。

```c
//WiFi 协议处理(位于 /wifi/proto.c 源代码文件)
#define LOG_TAG    "wifi.proto"

#include <elog.h>

log_e("我是 wifi.proto 日志");
```

```C
//WiFi 数据打包处理(位于 /wifi/package.c 源代码文件)
#define LOG_TAG    "wifi.package"

#include <elog.h>

log_w("我是 wifi.package 日志");
```

```C
//CAN 命令解析(位于 /can/disp.c 源代码文件)
#define LOG_TAG    "can.disp"

#include <elog.h>

log_w("我是 can.disp 日志");
```

**技巧二** ：为了实现按照模块、子模块作用域来限制日志输出级别的功能，可以按照下面的方式，在模块的头文件中定义以下宏定义：

```C
/**
 * Log default configuration for EasyLogger.
 * NOTE: Must defined before including the <elog.h>
 */
#if !defined(LOG_TAG)
    #define LOG_TAG                    "xx"
#endif
#undef LOG_LVL
#if defined(XX_LOG_LVL)
    #define LOG_LVL                    XX_LOG_LVL
#endif
```

XX 是模块名称的缩写，该段内容务必定义在 `elog.h` 之前，否则失效；这样做的 **好处** 是，如果模块内的源文件没有定义 TAG ，则会自动引用该段内容中的定义的 TAG 。同时可以在 头文件中可以配置 `XX_LOG_LVL` ，这样只会输出比这个优先级高或相等级别的日志。当然 XX_LOG_LVL 这个宏也可以不定义，此时会输出全部级别的日志，定义为 ASSERT 级别，就只剩断言信息了。
此时我们就能够实现 **源文件->子模块->模块->EasyLogger全局** 对于其中任何环节的日志配置及控制。调试时想要查看其中任何环节的日志，或者调整其中的某个环节日志级别，都会非常轻松，极大的提高了调试的灵活性及效率。

#### 1.3.2 输出 RAW 格式日志

```
void elog_raw(const char *format, ...)
```
|参数                                    |描述|
|:-----                                  |:----|
|format                                  |样式，类似`printf`首个入参|
|...                                     |不定参|

### 1.4 断言

#### 1.4.1 使用断言

EasyLogger自带的断言，可以直接用户软件，在断言表达式不成立后会输出断言信息并保持`while(1)`，或者执行断言钩子方法，钩子方法的设定参考 [`elog_assert_set_hook`](#114-设置断言钩子方法)。

```
#define ELOG_ASSERT(EXPR)
#define assert(EXPR)   //简化形式
```

|参数                                    |描述|
|:-----                                  |:----|
|EXPR                                    |表达式|

#### 1.4.2 设置断言钩子方法

默认断言钩子方法为空，设置断言钩子方法后。当断言`ELOG_ASSERT(EXPR)`中的条件不满足时，会自动执行断言钩子方法。断言钩子方法定义及使用可以参考上一章节的例子。

```c
void elog_assert_set_hook(void (*hook)(const char* expr, const char* func, size_t line))
```

|参数                                    |描述|
|:-----                                  |:----|
|hook                                    |断言钩子方法|

### 1.5 日志输出控制

#### 1.5.1 使能/失能日志输出

```
void elog_set_output_enabled(bool enabled)
```
|参数                                    |描述|
|:-----                                  |:----|
|enabled                                 |true: 使能，false: 失能|

#### 1.5.2 获取日志使能状态

```
bool elog_get_output_enabled(void)
```

#### 1.5.3 使能/失能日志输出锁

默认为使能状态，当系统或MCU进入异常后，需要输出异常日志时，就必须失能日志输出锁，来保证异常日志能够被正常输出。

```
void elog_output_lock_enabled(bool enabled)
```

|参数                                    |描述|
|:-----                                  |:----|
|enabled                                 |true: 使能，false: 失能|

例子：

```c
/* EasyLogger断言钩子方法 */
static void elog_user_assert_hook(const char* ex, const char* func, size_t line) {
    /* 失能异步输出方式（异步输出模块自带方法） */
    elog_async_enabled(false);
    /* 失能日志输出锁 */
    elog_output_lock_enabled(false);
    /* 失能 EasyLogger 的 Flash 插件自带同步锁（Flash 插件自带方法） */
    elog_flash_lock_enabled(false);
    /* 输出断言信息 */
    elog_a("elog", "(%s) has assert failed at %s:%ld.\n", ex, func, line);
    /* 将缓冲区中所有日志保存至 Flash （Flash 插件自带方法） */
    elog_flash_flush();
    while(1);
}
```

### 1.6 日志格式及样式

#### 1.6.1 设置日志格式

每种级别可对应一种日志输出格式，日志的输出内容位置顺序固定，只可定义开启或关闭某子内容。可设置的日志子内容包括：级别、标签、时间、进程信息、线程信息、文件路径、行号、方法名。

> 注：默认为 RAW格式

```
void elog_set_fmt(uint8_t level, size_t set)
```

|参数                                    |描述|
|:-----                                  |:----|
|level                                   |级别|
|set                                     |格式集合|

例子：

```c
/* 断言：输出所有内容 */
elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);
/* 错误：输出级别、标签和时间 */
elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
/* 警告：输出级别、标签和时间 */
elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
/* 信息：输出级别、标签和时间 */
elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
/* 调试：输出除了方法名之外的所有内容 */
elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_ALL & ~ELOG_FMT_FUNC);
/* 详细：输出除了方法名之外的所有内容 */
elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL & ~ELOG_FMT_FUNC);
```

#### 1.6.2 使能/失能日志颜色

日志颜色功能是将各个级别日志按照颜色进行区分，默认颜色功能是关闭的。日志的颜色修改方法详见《EasyLogger 移植说明》中的 `设置参数` 章节。

```
void elog_set_text_color_enabled(bool enabled)
```

|参数                                    |描述|
|:-----                                  |:----|
|enabled                                 |true: 使能，false: 失能|

#### 1.6.3 查找日志级别

在日志中查找该日志的级别。查找成功则返回其日志级别，查找失败则返回 -1 。

> **注意** ：使用此功能时，请务必保证所有级别的设定的日志格式里均已开启输出日志级别功能，否则会断言错误。

```
int8_t elog_find_lvl(const char *log)
```

|参数                                    |描述|
|:-----                                  |:----|
|log                                     |待查找的日志缓冲区|

#### 1.6.4 查找日志标签

在日志中查找该日志的标签。查找成功则返回其日志标签及标签长度，查找失败则返回 NULL 。

> **注意** ：使用此功能时，首先请务必保证该级别对应的设定日志格式中，已开启输出日志标签功能，否则会断言错误。其次需保证设定日志标签中 **不能包含空格** ，否则会查询失败。

```
const char *elog_find_tag(const char *log, uint8_t lvl, size_t *tag_len)
```

|参数                                    |描述|
|:-----                                  |:----|
|log                                     |待查找的日志缓冲区|
|lvl                                     |待查找日志的级别|
|tag_len                                 |查找到的标签长度|

### 1.7 过滤日志

#### 1.7.1 设置过滤级别

默认过滤级别为5(详细)，用户可以任意设置。在设置高优先级后，低优先级的日志将不会输出。例如：设置当前过滤的优先级为3(警告)，则只会输出优先级别为警告、错误、断言的日志。

```
void elog_set_filter_lvl(uint8_t level)
```

|参数                                    |描述|
|:-----                                  |:----|
|level                                   |级别|

#### 1.7.2 设置过滤标签

默认过滤标签为空字符串(`""`)，即不过滤。当前输出日志的标签会与过滤标签做字符串匹配，日志的标签包含过滤标签，则该输出该日志。例如：设置过滤标签为WiFi，则系统中包含WiFi字样标签的（WiFi.Bsp、WiFi.Protocol、Setting.Wifi）日志都会被输出。

>注：RAW格式日志不支持标签过滤

```
void elog_set_filter_tag(const char *tag)
```

|参数                                    |描述|
|:-----                                  |:----|
|tag                                     |标签|

#### 1.7.3 设置过滤关键词

默认过滤关键词为空字符串("")，即不过滤。检索当前输出日志中是否包含该关键词，包含则允许输出。

> 注：对于配置较低的MCU建议不开启关键词过滤（默认为不过滤），增加关键字过滤将会在很大程度上减低日志的输出效率。实际上当需要实时查看日志时，过滤关键词功能交给上位机做会更轻松，所以后期的跨平台日志助手开发完成后，就无需该功能。

```
void elog_set_filter_kw(const char *keyword)
```

|参数                                    |描述|
|:-----                                  |:----|
|keyword                                 |关键词|

#### 1.7.4 设置过滤器

设置过滤器后，只输出符合过滤要求的日志。所有参数设置方法，可以参考上述3个章节。

```
void elog_set_filter(uint8_t level, const char *tag, const char *keyword)
```

|参数                                    |描述|
|:-----                                  |:----|
|level                                   |级别|
|tag                                     |标签|
|keyword                                 |关键词|

### 1.8 缓冲输出模式

#### 1.8.1 使能/失能缓冲输出模式

```
void elog_buf_enabled(bool enabled)
```
|参数                                    |描述|
|:-----                                  |:----|
|enabled                                 |true: 使能，false: 失能|


#### 1.8.2 将缓冲区中的日志全部输出

在缓冲输出模式下，执行此方法可以将缓冲区中的日志全部输出干净。

```
void elog_flush(void)
```

### 1.9 异步输出模式

#### 1.9.1 使能/失能异步输出模式

```
void elog_async_enabled(bool enabled)
```
|参数                                    |描述|
|:-----                                  |:----|
|enabled                                 |true: 使能，false: 失能|

#### 1.9.2 在异步输出模式下获取日志

在异步输出模式下，如果用户没有启动 pthread 库，此时需要启用额外线程来实现日志的异步输出功能。使用此方法即可获取到异步输出缓冲区中的指定长度的日志。如果设定日志长度小于日志缓冲区中已存在日志长度，将只会返回已存在日志长度。

```C
size_t elog_async_get_log(char *log, size_t size)
```

|参数                                    |描述|
|:-----                                  |:----|
|log                                     |取出的日志内容|
|size                                    |待取出的日志大小|

#### 1.9.3 在异步输出模式下获取行日志（以换行符结尾）

异步模式下获取行日志与 1.9.2 中的直接获取日志功能类似，只不过这里所获取到的日志内容，必须为 **行日志** （以换行符结尾）格式，为后续的日志按行分析功能提供便利。如果设定日志长度小于日志缓冲区中已存在日志长度，将只会返回日志缓冲区中行日志的长度。如果缓冲区中不存在行日志，将不能保证返回的日志格式是行日志。

```C
size_t elog_async_get_line_log(char *log, size_t size)
```

|参数                                    |描述|
|:-----                                  |:----|
|log                                     |取出的行日志内容|
|size                                    |待取出的行日志大小|

## 2、配置

参照 《EasyLogger 移植说明》（[`\docs\zh\port\kernel.md`](https://github.com/armink/EasyLogger/blob/master/docs/zh/port/kernel.md)）中的 `设置参数` 章节
