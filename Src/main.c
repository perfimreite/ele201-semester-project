/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdbool.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

typedef struct {
  uint32_t start;
  uint32_t end;
} Interval;

typedef struct {
  Interval intervals[MAX_INTERVALS];
  uint32_t count;
  uint32_t index;
} Recording;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

TIM_HandleTypeDef htim1;

/* USER CODE BEGIN PV */

Recording recordings[KEYS_COUNT] = {0};
bool recording_active[KEYS_COUNT] = {0};

GPIO_TypeDef *ports[KEYS_COUNT] = {IN_C4_GPIO_Port, IN_D4_GPIO_Port, IN_E4_GPIO_Port, IN_F4_GPIO_Port, IN_G4_GPIO_Port, IN_A4_GPIO_Port, IN_B4_GPIO_Port};
uint16_t pins[KEYS_COUNT] = {IN_C4_Pin, IN_D4_Pin, IN_E4_Pin, IN_F4_Pin, IN_G4_Pin, IN_A4_Pin, IN_B4_Pin};
uint16_t notes[KEYS_COUNT] = {NOTE_C4, NOTE_D4, NOTE_E4, NOTE_F4, NOTE_G4, NOTE_A4, NOTE_B4};

uint16_t arr = 0;
uint16_t pwm = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void compute_arr_and_pwm(uint16_t *arr, uint16_t *pwm, uint16_t note);

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */

  for (int i = 0; i < KEYS_COUNT; i++) {
    recordings[i] = (Recording){0};
  }

  uint32_t record_start = 0; 
  uint32_t play_start = 0; 

  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    // Check if recording should start
    if (/*HAL_GPIO_ReadPin(IN_RECORD_GPIO_Port, IN_RECORD_Pin) &&*/ record_start == 0 && play_start == 0) {
      for (int i = 0; i < KEYS_COUNT; i++) {
        recordings[i] = (Recording){0};
        recording_active[i] = false;
      }
      record_start = HAL_GetTick();
    }

    // *** RECORD ***
    if (record_start > 0) {
      uint32_t elapsed = HAL_GetTick() - record_start;

      for (int i = 0; i < KEYS_COUNT; i++) {
        Recording *r = &recordings[i];

        bool on = HAL_GPIO_ReadPin(ports[i], pins[i]);

        if (r->count < MAX_INTERVALS) {
          if (!recording_active[i] && on) {
            r->intervals[r->count].start = elapsed;
            recording_active[i] = true;
          } else if (recording_active[i] && !on) {
            r->intervals[r->count].end = elapsed;
            r->count += 1;
            recording_active[i] = false;
          }
        }
      }

      // End recording
      if (elapsed >= RECORD_TIME) {
        for (int i = 0; i < KEYS_COUNT; i++) {
          Recording *r = &recordings[i];

          if (recording_active[i] && r->count < MAX_INTERVALS) {
            r->intervals[r->count].end = RECORD_TIME;
            r->count += 1;
          }

          recording_active[i] = false;
          r->index = 0;
        }

        record_start = 0;
        play_start = HAL_GetTick();
      }
    } 
    // *** PLAY RECORDING ***
    else if (play_start > 0) {
      uint32_t elapsed = HAL_GetTick() - play_start;

      // Play notes
      // If multiple keys are pressed the ones on earlier index have 'play priority'
      bool key_was_pressed = false;
      for (int i = 0; i < KEYS_COUNT && !key_was_pressed; i++) {
        Recording *r = &recordings[i];

        // Update recording index for the current key
        while (r->index < r->count && elapsed >= r->intervals[r->index].end) {
          r->index += 1;
        }

        if (r->index < r->count && elapsed >= r->intervals[r->index].start && elapsed < r->intervals[r->index].end) {
           compute_arr_and_pwm(&arr, &pwm, notes[i]);
          __HAL_TIM_SET_AUTORELOAD(&htim1, arr);
          __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pwm);

          key_was_pressed = true;
        }
      }

      // Play no noise if none of the keys were pressed
      if (!key_was_pressed) {
         compute_arr_and_pwm(&arr, &pwm, NOTE_NULL);
         __HAL_TIM_SET_AUTORELOAD(&htim1, arr);
         __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pwm);
      }

      // End playing
      if (elapsed >= RECORD_TIME) {
         // Reset recordings
        for (int i = 0; i < KEYS_COUNT; i++) {
          recordings[i] = (Recording){0};
        }

        play_start = 0;
      }
    }
    // *** NORMAL ***
     else {
      // Play notes
      // If multiple keys are pressed the ones on earlier index have 'play priority'
      bool key_was_pressed = false;
      for (int i = 0; i < KEYS_COUNT && !key_was_pressed; i++) {
        bool on = HAL_GPIO_ReadPin(ports[i], pins[i]);
        if (on) {
          compute_arr_and_pwm(&arr, &pwm, notes[i]);
          __HAL_TIM_SET_AUTORELOAD(&htim1, arr);
          __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pwm);

          key_was_pressed = true;
        }
      }

      // Play no noise if none of the keys were pressed
      if (!key_was_pressed) {
         compute_arr_and_pwm(&arr, &pwm, NOTE_NULL);
         __HAL_TIM_SET_AUTORELOAD(&htim1, arr);
         __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pwm);
      }
    }
  }

  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 108;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 108-1;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 3822;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.BreakFilter = 0;
  sBreakDeadTimeConfig.Break2State = TIM_BREAK2_DISABLE;
  sBreakDeadTimeConfig.Break2Polarity = TIM_BREAK2POLARITY_HIGH;
  sBreakDeadTimeConfig.Break2Filter = 0;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();

  /*Configure GPIO pin : IN_C4_Pin */
  GPIO_InitStruct.Pin = IN_C4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(IN_C4_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : IN_D4_Pin IN_A4_Pin IN_B4_Pin */
  GPIO_InitStruct.Pin = IN_D4_Pin|IN_A4_Pin|IN_B4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : IN_F4_Pin IN_G4_Pin IN_E4_Pin */
  GPIO_InitStruct.Pin = IN_F4_Pin|IN_G4_Pin|IN_E4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void compute_arr_and_pwm(uint16_t *arr, uint16_t *pwm, uint16_t note)
{
  if (note == 0) {
    *pwm = 0;
    return;
  }
  *arr = note == (uint16_t)((TIM1_CLOCK_HZ / ((htim1.Init.Prescaler + 1.0f) * note)) - 1);
  *pwm = ((*arr) + 1) / 2;
}

/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
