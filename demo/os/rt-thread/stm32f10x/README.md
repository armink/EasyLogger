# stm32f10x RT-Thread demo

---

## 1、简介

通过 `app\src\app_task.c` 的 `test_elog()` 方法来测试日志的输出，默认开启了异步输出模式，用户可以接入终端来控制日志的输出与过滤器的设置。

## 2、使用方法

打开电脑的终端与Demo的串口1进行连接，串口配置 115200 8 1 N，此时在终端中就可以输入 "2.6 Demo" Gif动画中提到的常用命令，如下：

### 2.1 核心功能

- 1、elog：使能与失能输出日志。`elog on`：使能，`elog off`：失能（提示：在日志输出过多，不方便输入命令测试的时候，可以将日志输出先失能）；
- 2、elog_lvl：设置过滤级别(0-5)；
- 3、elog_tag：设置过滤标签，输入 `elog_tag+想要过滤的标签` 后，则只有当日志的标签包含过滤标签时，才会被输出。不带任何参数则清空过滤标签；
- 4、elog_kw：设置过滤关键词，输入 `elog_kw+想要过滤的关键词` 后，则只有当日志的 **所有内容** 包含过滤关键词时，才会被输出。直接输入 `elog_kw` ，此时不带任何参数，将会清空已设置的过滤关键词。

### 2.2 Flash Log（将日志保存到Flash中）

这里我在`components\easylogger\plugins\flash\elog_flash_cfg.h`配置开启缓冲模式，此时只有缓冲区满了才会向Flash中写入。

- 1、elog_flash read：读取存储在Flash中的所有日志；
 - 1.1、elog_flash read xxxx：读取最近保存的xxxx字节大小的日志；
- 2、elog_flash flush：立刻将缓冲区中的所有日志保存至Flash中（注意：只有开启的了缓冲功能才会有效）；
- 3、elog_flash clean：清空Flash中的所有已保存日志，此时缓冲区的日志也将被清空。

## 3、文件（夹）说明

- `components\easylogger\port\elog_port.c` 核心功能移植参考文件
- `components\easylogger\plugins\flash\elog_flash_port.c` Flash Log功能移植参考文件
- `RVMDK` 下为Keil工程文件
- `EWARM` 下为IAR工程文件

## 4、其他功能

- 1、新增 RTT断言及硬件异常的钩子方法，使得系统或MCU在出现异常时，错误日志依然可以被输出同时保存至Flash。参考 `app\src\app_task.c` 中的 `rtt_user_assert_hook` 及 `exception_hook` 方法。
- 2、新增 EasyLogger断言的钩子方法，使得系统在出现异常时，错误日志依然可以被输出同时保存至Flash。参考 `app\src\app_task.c` 中的 `elog_user_assert_hook` 方法。