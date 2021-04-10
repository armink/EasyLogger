/*
 * File      : usart.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006-2021, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 * 2010-03-29     Bernard      remove interrupt Tx and DMA Rx mode
 * 2013-05-13     aozima       update for kehong-lingtai.
 */

#include "stm32f10x.h"
#include "usart.h"

#include "bsp.h"
#include <rtdevice.h>

/* USART1 */
#define UART1_GPIO_TX        GPIO_Pin_9
#define UART1_GPIO_RX        GPIO_Pin_10
#define UART1_GPIO           GPIOA
/* USART1_REMAP */
#define UART1_GPIO_REMAP_TX  GPIO_Pin_6
#define UART1_GPIO_REMAP_RX  GPIO_Pin_7
#define UART1_GPIO_REMAP     GPIOB

/* USART2 */
#define UART2_GPIO_TX        GPIO_Pin_2
#define UART2_GPIO_RX        GPIO_Pin_3
#define UART2_GPIO           GPIOA

/* USART3_REMAP[1:0] = 00 */
#define UART3_GPIO_TX        GPIO_Pin_10
#define UART3_GPIO_RX        GPIO_Pin_11
#define UART3_GPIO           GPIOB

/* STM32 uart driver */
struct stm32_uart
{
    USART_TypeDef* uart_device;
    IRQn_Type irq;
    /* transmit interrupt type */
    rt_uint32_t tx_irq_type;
};

static rt_err_t stm32_configure(struct rt_serial_device *serial, struct serial_configure *cfg)
{
    struct stm32_uart* uart;
    USART_InitTypeDef USART_InitStructure;

    RT_ASSERT(serial != RT_NULL);
    RT_ASSERT(cfg != RT_NULL);

    uart = (struct stm32_uart *)serial->parent.user_data;

    USART_InitStructure.USART_BaudRate = cfg->baud_rate;

    if (cfg->data_bits == DATA_BITS_8){
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    } else if (cfg->data_bits == DATA_BITS_9) {
        USART_InitStructure.USART_WordLength = USART_WordLength_9b;
    }

    if (cfg->stop_bits == STOP_BITS_1){
        USART_InitStructure.USART_StopBits = USART_StopBits_1;
    } else if (cfg->stop_bits == STOP_BITS_2){
        USART_InitStructure.USART_StopBits = USART_StopBits_2;
    }

    if (cfg->parity == PARITY_NONE){
        USART_InitStructure.USART_Parity = USART_Parity_No;
    } else if (cfg->parity == PARITY_ODD) {
        USART_InitStructure.USART_Parity = USART_Parity_Odd;
    } else if (cfg->parity == PARITY_EVEN) {
        USART_InitStructure.USART_Parity = USART_Parity_Even;
    }

    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(uart->uart_device, &USART_InitStructure);

    /* Enable USART */
    USART_Cmd(uart->uart_device, ENABLE);

    return RT_EOK;
}

static rt_err_t stm32_control(struct rt_serial_device *serial, int cmd, void *arg)
{
    struct stm32_uart* uart;
    rt_uint32_t irq_type = (rt_uint32_t)(arg);

    RT_ASSERT(serial != RT_NULL);
    uart = (struct stm32_uart *)serial->parent.user_data;

    switch (cmd)
    {
        /* disable interrupt */
    case RT_DEVICE_CTRL_CLR_INT:

        if (irq_type == RT_DEVICE_FLAG_INT_RX)
        {
            /* disable rx irq */
            USART_ITConfig(uart->uart_device, USART_IT_RXNE, DISABLE);
        }
        else if (irq_type == RT_DEVICE_FLAG_INT_TX)
        {
            /* disable tx irq */
            USART_ITConfig(uart->uart_device, uart->tx_irq_type, DISABLE);
        }
        else if (irq_type == RT_NULL)
        {
            /* disable irq */
            UART_DISABLE_IRQ(uart->irq);
        }
        break;
        /* enable interrupt */
    case RT_DEVICE_CTRL_SET_INT:
        if (irq_type == RT_DEVICE_FLAG_INT_RX)
        {
            /* enable rx irq */
            USART_ITConfig(uart->uart_device, USART_IT_RXNE, ENABLE);
        }
        else if (irq_type == RT_DEVICE_FLAG_INT_TX)
        {
            /* enable tx irq */
            USART_ITConfig(uart->uart_device, uart->tx_irq_type, ENABLE);
        }
        else if (irq_type == RT_NULL)
        {
            /* enable irq */
            UART_ENABLE_IRQ(uart->irq);
        }
        break;
        /* get interrupt flag */
    case RT_DEVICE_CTRL_GET_INT:
        if (irq_type == RT_DEVICE_FLAG_INT_RX)
        {
            /* return rx irq flag */
            return USART_GetITStatus(uart->uart_device, USART_IT_RXNE);
        }
        else if (irq_type == RT_DEVICE_FLAG_INT_TX)
        {
            /* return tx irq flag */
            return USART_GetITStatus(uart->uart_device, uart->tx_irq_type);
        }
    break;
         /* get USART flag */
    case RT_DEVICE_CTRL_GET_FLAG:
        if (irq_type == RT_DEVICE_FLAG_INT_RX)
        {
            /* return rx irq flag */
            return USART_GetFlagStatus(uart->uart_device, USART_FLAG_RXNE);
        }
        else if (irq_type == RT_DEVICE_FLAG_INT_TX)
        {
            /* return tx flag */
            if (uart->tx_irq_type == USART_IT_TC)
            {
                return USART_GetFlagStatus(uart->uart_device, USART_FLAG_TC);
            }
            else if (uart->tx_irq_type == USART_IT_TXE)
            {
                return USART_GetFlagStatus(uart->uart_device, USART_FLAG_TXE);
            }
        }
        break;
    }
    return RT_EOK;
}

static int stm32_putc(struct rt_serial_device *serial, char c)
{
    struct stm32_uart* uart;

    RT_ASSERT(serial != RT_NULL);
    uart = (struct stm32_uart *)serial->parent.user_data;

    while (!(uart->uart_device->SR & USART_FLAG_TXE));
    uart->uart_device->DR = c;

    return 1;
}

static int stm32_getc(struct rt_serial_device *serial)
{
    int ch;
    struct stm32_uart* uart;

    RT_ASSERT(serial != RT_NULL);
    uart = (struct stm32_uart *)serial->parent.user_data;

    ch = -1;
    if (uart->uart_device->SR & USART_FLAG_RXNE)
    {
        ch = uart->uart_device->DR & 0xff;
    }

    return ch;
}

static const struct rt_uart_ops stm32_uart_ops =
{
    stm32_configure,
    stm32_control,
    stm32_putc,
    stm32_getc,
};

#if defined(RT_USING_UART1) || defined(RT_USING_REMAP_UART1)
/* UART1 device driver structure */
struct stm32_uart uart1 =
{
    USART1,
    USART1_IRQn,
    USART_IT_TC,
};
struct rt_serial_device serial1;

void USART1_IRQHandler(void)
{
    struct stm32_uart* uart;

    uart = &uart1;

    /* enter interrupt */
    rt_interrupt_enter();
    if(USART_GetITStatus(uart->uart_device, USART_IT_RXNE) != RESET)
    {
        rt_hw_serial_isr(&serial1, RT_SERIAL_EVENT_RX_IND);
        /* clear interrupt */
        USART_ClearITPendingBit(uart->uart_device, USART_IT_RXNE);
    }

    if (USART_GetITStatus(uart->uart_device, uart->tx_irq_type) != RESET)
    {
        /* clear interrupt */
        rt_hw_serial_isr(&serial1, RT_SERIAL_EVENT_TX_DONE);
        if (uart->tx_irq_type == USART_IT_TC)
        {
            USART_ClearITPendingBit(uart->uart_device, uart->tx_irq_type);
        }
    }
    if (USART_GetFlagStatus(uart->uart_device, USART_FLAG_ORE) == SET)
    {
        stm32_getc(&serial1);
    }
    /* leave interrupt */
    rt_interrupt_leave();
}
#endif /* RT_USING_UART1 */

#if defined(RT_USING_UART2)
/* UART1 device driver structure */
struct stm32_uart uart2 =
{
    USART2,
    USART2_IRQn,
    USART_IT_TC,
};
struct rt_serial_device serial2;

void USART2_IRQHandler(void)
{
    struct stm32_uart* uart;

    uart = &uart2;

    /* enter interrupt */
    rt_interrupt_enter();
    if(USART_GetITStatus(uart->uart_device, USART_IT_RXNE) != RESET)
    {
        rt_hw_serial_isr(&serial2, RT_SERIAL_EVENT_RX_IND);
        /* clear interrupt */
        USART_ClearITPendingBit(uart->uart_device, USART_IT_RXNE);
    }
    if (USART_GetITStatus(uart->uart_device, uart->tx_irq_type) != RESET)
    {
        /* clear interrupt */
        if (uart->tx_irq_type == USART_IT_TC)
        {
            USART_ClearITPendingBit(uart->uart_device, uart->tx_irq_type);
        }
    }
    if (USART_GetFlagStatus(uart->uart_device, USART_FLAG_ORE) == SET)
    {
        stm32_getc(&serial2);
    }

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif /* RT_USING_UART2 */

#if defined(RT_USING_UART3)
/* UART3 device driver structure */
struct stm32_uart uart3 =
{
    USART3,
    USART3_IRQn,
    USART_IT_TC,
};
struct rt_serial_device serial3;

void USART3_IRQHandler(void)
{
    struct stm32_uart* uart;

    uart = &uart3;

    /* enter interrupt */
    rt_interrupt_enter();
    if(USART_GetITStatus(uart->uart_device, USART_IT_RXNE) != RESET)
    {
        rt_hw_serial_isr(&serial3, RT_SERIAL_EVENT_RX_IND);
        /* clear interrupt */
        USART_ClearITPendingBit(uart->uart_device, USART_IT_RXNE);
    }
    if (USART_GetITStatus(uart->uart_device, uart->tx_irq_type) != RESET)
    {
        /* clear interrupt */
        if (uart->tx_irq_type == USART_IT_TC)
        {
            USART_ClearITPendingBit(uart->uart_device, uart->tx_irq_type);
        }
    }
    if (USART_GetFlagStatus(uart->uart_device, USART_FLAG_ORE) == SET)
    {
        stm32_getc(&serial3);
    }

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif /* RT_USING_UART3 */

static void RCC_Configuration(void)
{
#if defined(RT_USING_UART1)
    /* Enable UART GPIO clocks */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    /* Enable UART clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
#elif defined(RT_USING_REMAP_UART1)
    /* Enable UART GPIO clocks */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    /* Enable UART clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
#endif

#ifdef RT_USING_UART2
    /* Enable UART GPIO clocks */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    /* Enable UART clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
#endif /* RT_USING_UART2 */

#ifdef RT_USING_UART3
    /* Enable UART GPIO clocks */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    /* Enable UART clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
#endif /* RT_USING_UART3 */
}

static void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;

#if defined(RT_USING_UART1)
    /* Configure USART Rx/tx PIN */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = UART1_GPIO_RX;
    GPIO_Init(UART1_GPIO, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = UART1_GPIO_TX;
    GPIO_Init(UART1_GPIO, &GPIO_InitStructure);
#elif defined(RT_USING_REMAP_UART1)
    /* Configure USART Remap Rx/tx PIN */
    GPIO_PinRemapConfig(GPIO_Remap_USART1,ENABLE);
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = UART1_GPIO_REMAP_RX;
    GPIO_Init(UART1_GPIO_REMAP, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = UART1_GPIO_REMAP_TX;
    GPIO_Init(UART1_GPIO_REMAP, &GPIO_InitStructure);
#endif /* RT_USING_UART1 */

#ifdef RT_USING_UART2
    /* Configure USART Rx/tx PIN */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = UART2_GPIO_RX;
    GPIO_Init(UART2_GPIO, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = UART2_GPIO_TX;
    GPIO_Init(UART2_GPIO, &GPIO_InitStructure);
#endif /* RT_USING_UART2 */

#ifdef RT_USING_UART3
    /* Configure USART Rx/tx PIN */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = UART3_GPIO_RX;
    GPIO_Init(UART3_GPIO, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = UART3_GPIO_TX;
    GPIO_Init(UART3_GPIO, &GPIO_InitStructure);
#endif /* RT_USING_UART3 */
}

static void NVIC_Configuration(struct stm32_uart* uart)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Enable the USART1 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = uart->irq;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void rt_hw_usart_init(void)
{
    struct stm32_uart* uart;
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;

    RCC_Configuration();
    GPIO_Configuration();

#if defined(RT_USING_UART1) || defined(RT_USING_REMAP_UART1)
    uart = &uart1;
    config.baud_rate = BAUD_RATE_115200;

    serial1.ops    = &stm32_uart_ops;
    serial1.config = config;

    NVIC_Configuration(&uart1);

    /* register UART1 device */
    rt_hw_serial_register(&serial1, "uart1",
                          RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX ,
                          uart);
#endif /* RT_USING_UART1 */

#ifdef RT_USING_UART2
    uart = &uart2;

    config.baud_rate = BAUD_RATE_115200;
    serial2.ops    = &stm32_uart_ops;
    serial2.config = config;

    NVIC_Configuration(&uart2);

    /* register UART1 device */
    rt_hw_serial_register(&serial2, "uart2",
                          RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX,
                          uart);
#endif /* RT_USING_UART2 */

#ifdef RT_USING_UART3
    uart = &uart3;

    config.baud_rate = BAUD_RATE_115200;

    serial3.ops    = &stm32_uart_ops;
    serial3.config = config;

    NVIC_Configuration(&uart3);

    /* register UART1 device */
    rt_hw_serial_register(&serial3, "uart3",
                          RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX,
                          uart);
#endif /* RT_USING_UART3 */
}
