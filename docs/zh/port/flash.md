# EasyLogger Flash插件移植说明

---

## 1、准备工作

在使用Flash插件前，保证EasyLogger的核心功能已经在项目中移植成功（[移植文档点这里](https://github.com/armink/EasyLogger/blob/master/docs/zh/port/kernel.md)）。再确认已下载的源码中存在`\easylogger\plugins\flash`文件夹，该文件夹为Flash插件的所有源码。

## 2、导入项目

目前只有STM32F10x平台的RT-Thread系统Demo里面使用了Flash插件，如果你的项目与这个Demo平台一致，则可以先直接跳过2、3、4章节，按照第5章的要求设置参数，并运行、验证Demo。验证通过再按照下面的导入项目要求，将Demo中的移植文件直接导入到项目中即可。

- 1、打开`\easylogger\plugins\flash`文件夹，内部文件结构如下：

|源文件                                 |描述   |
|:------------------------------        |:----- |
|elog_flash.c                           |Flash插件源码|
|elog_flash.h                           |Flash插件头文件|
|elog_flash_cfg.h                       |Flash插件配置头文件|
|elog_flash_port.c                      |Flash插件移植文件|


- 2、将`\easylogger\plugins\flash`下的所有文件拷贝到项目中；
- 3、添加`elog_flash.c`及`elog_flash_port.c`这些文件到项目的编译路径中；
- 4、添加插件所在文件夹到编译的头文件目录列表中；

## 3、移植接口

### 3.1 移植初始化

Flash插件移植初始化。初始化Flash插件移植所需的资源等等。

```C
ElogErrCode elog_flash_port_init(void)
```

### 3.2 Flash中的日志被读取后的输出接口

将日志从Flash中读取后，调用`elog_flash_outout()`、`elog_flash_outout_all()`及`elog_flash_outout_recent()`进行输出展示时，将会调用此移植接口。可以在里面增加输出到终端、网络等功能。

```C
void elog_flash_port_output(const char *log, size_t size)
```

|参数                                    |描述|
|:-----                                  |:----|
|log                                     |日志内容|
|size                                    |日志大小|

例子：
```c
void elog_flash_port_output(const char *log, size_t size) {
    /* output to terminal */
    rt_kprintf("%.*s", size, log);
}

```
### 3.3 资源同步锁-加锁

对日志写入Flash操作进行加锁，保证日志在并发写入时的正确性。有操作系统时可以使用获取信号量来加锁，裸机时可以通过关闭全局中断来加锁。

```C
void elog_flash_port_lock(void)
```

### 3.4 资源同步锁-解锁

与加锁功能对应。

```C
void elog_flash_port_unlock(void)
```
## 4、设置参数

配置时需要修改项目中的`elog_flash_cfg.h`文件，开启、关闭、修改对应的宏即可。

### 4.1 缓冲模式
    
开启后，需要写入Flash的日志会先存储至RAM缓冲区，当缓冲区满时，缓冲区中的所有日志将自动写入Flash。如果关闭，所有日志在输出时会立刻写入Flash。

- 默认状态：开启
- 操作方法：开启、关闭`ELOG_FLASH_USING_BUF_MODE`宏即可

#### 4.1.1 缓冲区大小

当开启缓冲模式后，此配置才会生效，单位：byte。

- 操作方法：修改`ELOG_FLASH_BUF_SIZE`宏对应值即可

## 5、测试验证

每次使用前，务必核心功能都已经初始化完成，再调用`elog_flash_init()`方法对Flash插件进行初始化，保证初始化没问题后，再调用`elog_start()`方法启动EasyLogger，最后就可以使用Flash插件自带的API方法进行测试。如果使用的RT-Thread的Demo，则可以按照[这里的命令要求](https://github.com/armink/EasyLogger/tree/master/demo/os/rt-thread/stm32f10x#22-flash-log将日志保存到flash中)，接上finsh串口，输入finsh命令即可测试。

摘取自STM32平台下RT-Thread Demo中的初始化过程（[点击查看全部](https://github.com/armink/EasyLogger/blob/master/demo/os/rt-thread/stm32f10x/app/src/app_task.c)）：
```c
/* 初始化EasyFlash及EasyLogger */
if ((easyflash_init() == EF_NO_ERR)&&(elog_init() == ELOG_NO_ERR)) {
    /* 设置日志格式 */
    elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL & ~ELOG_FMT_P_INFO);
    elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_ALL & ~(ELOG_FMT_FUNC | ELOG_FMT_P_INFO));
    elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL & ~(ELOG_FMT_FUNC | ELOG_FMT_P_INFO));
    /* 设置EasyLogger的断言钩子方法 */
    elog_assert_set_hook(elog_user_assert_hook);
    /* 初始化EasyLogger的Flash 插件 */
    elog_flash_init();
    /* 启动EasyLogger */
    elog_start();
    /* 设置RT-Thread提供的硬件异常钩子方法 */
    rt_hw_exception_install(exception_hook);
    /* 设置RT-Thread断言钩子方法 */
    rt_assert_set_hook(rtt_user_assert_hook);
}
```
