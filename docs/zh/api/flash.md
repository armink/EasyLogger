# EasyLogger Flash 插件 API 说明

---

所有Flash插件功能的API接口都在[`\easylogger\plugins\flash\elog_flash.h`](https://github.com/armink/EasyLogger/blob/master/easylogger/plugins/flash/elog_flash.h)中声明。以下内容较多，可以使用 **CTRL+F** 搜索。

> 建议：点击项目主页 https://github.com/armink/EasyLogger 右上角 **Watch & Star**，这样项目有更新时，会及时以邮件形式通知你。

## 1、用户使用接口

### 1.1 初始化

初始化的EasyLogger的Flash插件，初始化后才可以使用下面的API。

> 注意：插件的初始化必须放在核心功能初始化之后。

```
ElogErrCode elog_flash_init(void)
```

### 1.2 输出Flash中指定位置存储的日志

日志的输出方式取决用户的移植接口`elog_flash_port_putput`的实现，具体参考Flash插件移植说明（[`\docs\zh\port\flash.md`](https://github.com/armink/EasyLogger/blob/master/docs/zh/port/flash.md)）。首页Demo中是输出到控制台的方式。

```
void elog_flash_output(size_t pos, size_t size)
```

|参数                                    |描述|
|:-----                                  |:----|
|pos                                     |日志存储的位置索引（从0开始）|
|size                                    |日志大小，单位：字节|

### 1.3 输出Flash中存储的所有日志

```
void elog_flash_output_all(void)
```

### 1.4 输出Flash中最近存储的日志

```
void elog_flash_output_recent(size_t size)
```

|参数                                    |描述|
|:-----                                  |:----|
|size                                    |日志大小，单位：字节|

### 1.5 往Flash中写入日志

此方法可以放到核心功能的日志输出移植接口`elog_port_output`中，实现所有日志自动保存至Flash中的功能。

```
void elog_flash_write(const char *log, size_t size)
```

|参数                                    |描述|
|:-----                                  |:----|
|log                                     |日志内容|
|size                                    |日志大小，单位：字节|

### 1.6 清空Flash中全部日志

> 注意：如果Flash插件开启缓冲模式，那么缓冲区的日志也将会被清空。

```
void elog_flash_clean(void)
```

### 1.7 使能/失能Flash日志功能锁

默认为使能状态，当系统或MCU进入异常后，需要输出异常日志时，就必须失能Flash日志功能锁，来保证异常日志能够被正常输出。

```
void elog_flash_lock_enabled(bool enabled)
```

|参数                                    |描述|
|:-----                                  |:----|
|enabled                                 |true: 使能，false: 失能|

例子：

```c
/* EasyLogger断言钩子方法 */
static void elog_user_assert_hook(const char* ex, const char* func, size_t line) {
    /* 失能日志输出锁 */
    elog_output_lock_enabled(false);
    /* 失能EasyLogger的Flash插件自带同步锁（Flash插件自带方法） */
    elog_flash_lock_enabled(false);
    /* 输出断言信息 */
    elog_a("elog", "(%s) has assert failed at %s:%ld.\n", ex, func, line);
    /* 将缓冲区中所有日志保存至Flash（Flash插件自带方法） */
    elog_flash_flush();
    while(1);
}
```

### 1.8 将缓冲区中所有日志保存至Flash中

> 注意：只有Flash插件开启缓冲模式时，此功能才可以被使用。非缓冲模式下调用`elog_flash_write()`会立刻保存日志。

```
void elog_flash_flush(void);
```

## 2、配置

参照EasyLogger Flash插件移植说明（[`\docs\zh\port\flash.md`](https://github.com/armink/EasyLogger/blob/master/docs/zh/port/flash.md)）中的 `设置参数` 章节
