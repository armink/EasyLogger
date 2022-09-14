# NUCLEO-G070RB(基于STM32G070RBT6) FreeRTOS demo

## 1、简介

使用STM32CubeMX创建FreeRTOS工程，实现异步log输出的demo

## 2、使用方法

导入STM32CubeIDE，连接开发板下载使用。使用NUCLEO自带的STLinkV2-1的串口，配置115200 8 1 N，可观察到3种不同等级和颜色的HelloWorld。

## 3、文件（夹）说明

* `Components/EasyLogger_Port`文件夹下为移植参考适配文件。
* `Core/Src/main.c`添加了串口发送完成回调函数和elog初始化。
* `Core/Src/app_freertos.c`中默认线程每间隔1秒发送一次log。

## 4、其他说明

* 对于NUCLEO-G070RB的默认Cube工程，大致进行了如下修改：修改了系统时钟为64M；修改系统时钟源为TIM1，开启FreeRTOS，添加了log异步输出线程，添加了信号量，使能`      USE_NEWLIB_REENTRANT  `选项；串口数据字节改为8，添加串口发送DMA，打开串口中断。
* 对于异步输出的线程（本demo中为`elog`线程）和需要使用elog的线程，推荐stack大小大于256word，使用Cube默认的128word可能会造成栈溢出。
* 需要打开串口2的中断，而不仅仅是默认打开的DMA中断，才能在DMA传输完成后正常进入`HAL_UART_TxCpltCallback()`回调函数。