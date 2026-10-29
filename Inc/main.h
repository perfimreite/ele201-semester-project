/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define IN_C4_Pin GPIO_PIN_0
#define IN_C4_GPIO_Port GPIOA
#define IN_D4_Pin GPIO_PIN_0
#define IN_D4_GPIO_Port GPIOB
#define PWM_Pin GPIO_PIN_9
#define PWM_GPIO_Port GPIOE
#define IN_F4_Pin GPIO_PIN_14
#define IN_F4_GPIO_Port GPIOE
#define IN_G4_Pin GPIO_PIN_15
#define IN_G4_GPIO_Port GPIOE
#define IN_A4_Pin GPIO_PIN_10
#define IN_A4_GPIO_Port GPIOB
#define IN_B4_Pin GPIO_PIN_11
#define IN_B4_GPIO_Port GPIOB
#define IN_E4_Pin GPIO_PIN_0
#define IN_E4_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

#define KEYS_COUNT 7
#define RECORD_TIME 5000
#define MAX_INTERVALS 64
#define TIM1_CLOCK_HZ (108 * 1e6) // 108 MHz

#define NOTE_NULL 0
#define NOTE_C4 262
#define NOTE_D4 294
#define NOTE_E4 330
#define NOTE_F4 349
#define NOTE_G4 392
#define NOTE_A4 440
#define NOTE_B4 494
#define NOTE_C5 523

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
