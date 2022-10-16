/**
  ******************************************************************************
  * @file    stm32g0xx_hal_flash_ex.h
  * @author  MCD Application Team
  * @brief   Header file of FLASH HAL Extended module.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file in
  * the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32G0xx_HAL_FLASH_EX_H
#define STM32G0xx_HAL_FLASH_EX_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g0xx_hal_def.h"

/** @addtogroup STM32G0xx_HAL_Driver
  * @{
  */

/** @addtogroup FLASHEx
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/** @defgroup FLASHEx_Exported_Constants FLASH Exported Constants
  * @{
  */
/** @defgroup FLASHEx_Empty_Check FLASHEx Empty Check
  * @{
  */
#define FLASH_PROG_NOT_EMPTY                0x00000000u          /*!< 1st location in Flash is programmed */
#define FLASH_PROG_EMPTY                    FLASH_ACR_PROGEMPTY  /*!< 1st location in Flash is empty */
/**
  * @}
  */
/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
/** @addtogroup FLASHEx_Exported_Functions
  * @{
  */

/* Extended Program operation functions  *************************************/
/** @addtogroup FLASHEx_Exported_Functions_Group1
  * @{
  */
HAL_StatusTypeDef HAL_FLASHEx_Erase(FLASH_EraseInitTypeDef *pEraseInit, uint32_t *PageError);
HAL_StatusTypeDef HAL_FLASHEx_Erase_IT(FLASH_EraseInitTypeDef *pEraseInit);
void              HAL_FLASHEx_EnableDebugger(void);
void              HAL_FLASHEx_DisableDebugger(void);
uint32_t          HAL_FLASHEx_FlashEmptyCheck(void);
void              HAL_FLASHEx_ForceFlashEmpty(uint32_t FlashEmpty);
#if defined(FLASH_SECURABLE_MEMORY_SUPPORT)
void              HAL_FLASHEx_EnableSecMemProtection(uint32_t Banks);
#endif /* FLASH_SECURABLE_MEMORY_SUPPORT */
HAL_StatusTypeDef HAL_FLASHEx_OBProgram(FLASH_OBProgramInitTypeDef *pOBInit);
void              HAL_FLASHEx_OBGetConfig(FLASH_OBProgramInitTypeDef *pOBInit);
/**
  * @}
  */

/**
  * @}
  */

/* Private macros ------------------------------------------------------------*/
/** @defgroup FLASHEx_Private_Constants FLASHEx Private Constants
  * @{
  */
#define FLASH_PCROP_GRANULARITY_OFFSET             9u                                        /*!< FLASH Code Readout Protection granularity offset */
#define FLASH_PCROP_GRANULARITY                    (1UL << FLASH_PCROP_GRANULARITY_OFFSET)   /*!< FLASH Code Readout Protection granularity, 512 Bytes */
/**
  * @}
  */


/** @defgroup FLASHEx_Private_Macros FLASHEx Private Macros
  *  @{
  */
#define IS_FLASH_EMPTY_CHECK(__VALUE__)     (((__VALUE__) == FLASH_PROG_EMPTY) || ((__VALUE__) == FLASH_PROG_NOT_EMPTY))
void              FLASH_PageErase(uint32_t Banks, uint32_t Page);
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* STM32G0xx_HAL_FLASH_EX_H */

