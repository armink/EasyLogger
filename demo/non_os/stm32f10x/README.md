# stm32f10x 裸机 demo

---

## 1、简介

通过 `app\src\app.c` 的 `test_elog()` 方法来测试日志的输出。

### 1.1、使用方法

打开电脑的终端与demo的串口1进行连接，串口配置 115200 8 1 N，此时可以在终端上看到demo的打印日志

> 注意：对于无法连接终端的用户，也可以使用仿真器与demo平台进行连接，调试观察日志的输出情况。

## 2、文件（夹）说明

`components\easylogger\port\elog_port.c` 移植参考文件

`RVMDK` 下为Keil工程文件

`EWARM` 下为IAR工程文件

## 3、其他功能

可以打开 `app\src\app.c` 中的部分注释，来测试以下功能。

- `elog_set_output_enabled(false);` ：动态使能或失能日志输出
- `elog_set_filter_lvl(ELOG_LVL_WARN);` ：动态设置过滤优先级
- `elog_set_filter_tag("main");` ：动态设置过滤标签
- `elog_set_filter_kw("Hello");` ：动态设置过滤关键词
