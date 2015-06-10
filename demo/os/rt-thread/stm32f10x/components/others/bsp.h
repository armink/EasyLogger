
#ifndef  BSP_PRESENT
#define  BSP_PRESENT

/*
*********************************************************************************************************
*                                                 EXTERNS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                              INCLUDE FILES
*********************************************************************************************************
*/



#include <stm32f10x_conf.h>
#include <rthw.h>
#include <rtthread.h>
#include "usart.h"

/*
*********************************************************************************************************
*                                                 DEFINES
*********************************************************************************************************
*/
/* board configuration */
// <o> SDCard Driver <1=>SDIO sdcard <0=>SPI MMC card
// 	<i>Default: 1
#define STM32_USE_SDIO			0

/* whether use board external SRAM memory */
// <e>Use external SRAM memory on the board
// 	<i>Enable External SRAM memory
#define STM32_EXT_SRAM          0
//	<o>Begin Address of External SRAM
//		<i>Default: 0x68000000
#define STM32_EXT_SRAM_BEGIN    0x68000000 /* the begining address of external SRAM */
//	<o>End Address of External SRAM
//		<i>Default: 0x68080000
#define STM32_EXT_SRAM_END      0x68080000 /* the end address of external SRAM */
// </e>

// <o> Internal SRAM memory size[Kbytes] <8-64>
//	<i>Default: 64
#define STM32_SRAM_SIZE         64
#define STM32_SRAM_END          (SRAM_BASE + STM32_SRAM_SIZE * 1024)

#define VECT_TAB_FLASH                                     /* use Flash to store vector table */

/* RT_USING_UART */
#define RT_USING_UART1
#define RT_UART_RX_BUFFER_SIZE	64

#define LED_RUN_ON              GPIO_SetBits  (GPIOA,GPIO_Pin_10)  	   //RUN
#define LED_RUN_OFF             GPIO_ResetBits(GPIOA,GPIO_Pin_10) 	   //RUN

/*********************************************************************************************************/
/**                                                 MACRO'S												 */
/***********************************************************************************************************/


/***********************************************************************************************************/
/*                                               DATA TYPES												 */
/***********************************************************************************************************/


/**********************************************************************************************************
*                                            GLOBAL VARIABLES
**********************************************************************************************************/




/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void BSP_Init(void);

void rt_hw_board_init(void);
void IWDG_Feed(void);
void rt_hw_timer_handler(void);

/*
*********************************************************************************************************
*                                           INTERRUPT SERVICES
*********************************************************************************************************
*/


#endif                                                          /* End of module include.                               */
