# stm32f10x RT-Thread Demo

---

## 1、简介

通过 `app\src\app_task.c` 的 `test_elog()` 方法来测试日志的输出，用户可以接入终端来控制日志的输出与过滤器的设置。

### 1.1、使用方法

打开电脑的终端与Demo的串口1进行连接，串口配置 115200 8 1 N，此时在终端中就可以输入 "2.6 Demo" Gif动画中提到的常用命令，如下：

- 1、elog：使能与失能输出日志。elog on：使能，elog off：失能
- 2、elog_lvl：设置过滤级别(0-5)。
- 3、elog_tag：设置过滤标签，设置后，则只有当日志的标签包含过滤标签时，才会被输出。不带任何参数则清空过滤标签。
- 4、elog_kw：设置过滤关键词，设置后，则只有当日志的 **所有内容** 包含过滤关键词时，才会被输出。不带任何参数则清空过滤关键词。

## 2、文件（夹）说明

`components\easylogger\port\elog_port.c` 移植参考文件

`RVMDK` 下为Keil工程文件

`EWARM` 下为IAR工程文件

## 3、其他功能

- 1、新增 RTT断言及硬件异常的钩子方法，使得系统在出现异常时，错误日志依然可以被输出或保存。参考 `app\src\app_task.c` 中的 `rtt_user_assert_hook` 及 `exception_hook` 方法。
- 2、新增 EasyLogger断言的钩子方法，使得系统在出现异常时，错误日志依然可以被输出或保存。参考 `app\src\app_task.c` 中的 `elog_user_assert_hook` 方法。