/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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


volatile uint8_t motor_phase;
uint32_t adc_value;
uint32_t U_period;
uint8_t esc_mode;

void SystemClock_Config(void);
static void MX_HRTIM1_Init(void);

uint8_t endl[] = "\r\n";
uint8_t st[]= "10";

int main(void)
{
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

  /* System interrupt init*/
  NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

  /** Disable the internal Pull-Up in Dead Battery pins of UCPD peripheral */
  LL_PWR_DisableUCPDDeadBattery();

  /* Configure the system clock */
  SystemClock_Config();

  /* Zainicjalizuj peryferia */
  GPIO_Init();
  MX_HRTIM1_Init();
 // ADC2_Init();
 // Activate_ADC2();
  ADC1_Init();
  Activate_ADC1();
  USART2_Init();
  /* 			*/


  /* Zmiana rozdzielczości SysTick */
  LL_InitTick(HCLK_frequency, 100000);


  /* Sprawdź odczyty z czujników Halla */
  motor_phase = Detect_Hall_State();

  /* Jeśli czujniki są niepodłączone */
  if(motor_phase == 10)
	  esc_mode = FEEDBACK_MODE_BEMF;
  else
	  esc_mode = FEEDBACK_MODE_HALL;

  if(esc_mode % FEEDBACK_MODE == FEEDBACK_MODE_BEMF)
  {
	  BEMF_Init();
  }
  if(esc_mode % FEEDBACK_MODE == FEEDBACK_MODE_HALL)
  {
	  HALL_Init();
  }

  while (1)
  {
  }

}

/* Konfiguracja zegarów systemowych */
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_4);
  while(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_4)
  {
  }
  LL_PWR_EnableRange1BoostMode();
  LL_RCC_HSE_Enable();
   /* Wait till HSE is ready */
  while(LL_RCC_HSE_IsReady() != 1)
  {
  }

  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_6, 80, LL_RCC_PLLR_DIV_2);
  LL_RCC_PLL_EnableDomain_SYS();
  LL_RCC_PLL_Enable();
   /* Wait till PLL is ready */
  while(LL_RCC_PLL_IsReady() != 1)
  {
  }

  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_2);
   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {
  }

  /* Insure 1us transition state at intermediate medium speed clock*/
  for (__IO uint32_t i = (160 >> 1); i !=0; i--);

  /* Set AHB prescaler*/
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

  LL_Init1msTick(160000000);

  LL_SetSystemCoreClock(160000000);
}

/* Inicjalizacja HRTIM1 */
static void MX_HRTIM1_Init(void)
{

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_HRTIM1);

  LL_HRTIM_ConfigDLLCalibration(HRTIM1, LL_HRTIM_DLLCALIBRATION_MODE_CONTINUOUS, LL_HRTIM_DLLCALIBRATION_RATE_3);

  /* Poll for DLL end of calibration */
#if (USE_TIMEOUT == 1)
  uint32_t Timeout = 10; /* Timeout Initialization */
#endif  /*USE_TIMEOUT*/

  while(LL_HRTIM_IsActiveFlag_DLLRDY(HRTIM1) == RESET){
#if (USE_TIMEOUT == 1)
    if (LL_SYSTICK_IsActiveCounterFlag())  /* Check Systick counter flag to decrement the time-out value */
    {
        if(Timeout-- == 0)
        {
          Error_Handler();  /* error management */
        }
    }
#endif  /* USE_TIMEOUT */
  }

  LL_HRTIM_TIM_SetPrescaler(HRTIM1, LL_HRTIM_TIMER_MASTER, LL_HRTIM_PRESCALERRATIO_MUL8);
  LL_HRTIM_TIM_SetCounterMode(HRTIM1, LL_HRTIM_TIMER_MASTER, LL_HRTIM_MODE_CONTINUOUS);
  LL_HRTIM_TIM_SetPeriod(HRTIM1, LL_HRTIM_TIMER_MASTER, TIM_M_PERIOD);
  LL_HRTIM_TIM_SetRepetition(HRTIM1, LL_HRTIM_TIMER_MASTER, 0x00);
  LL_HRTIM_TIM_DisableHalfMode(HRTIM1, LL_HRTIM_TIMER_MASTER);
  LL_HRTIM_TIM_SetInterleavedMode(HRTIM1, LL_HRTIM_TIMER_MASTER, LL_HRTIM_INTERLEAVED_MODE_DISABLED);
  LL_HRTIM_TIM_DisableStartOnSync(HRTIM1, LL_HRTIM_TIMER_MASTER);
  LL_HRTIM_TIM_DisableResetOnSync(HRTIM1, LL_HRTIM_TIMER_MASTER);
  LL_HRTIM_TIM_SetDACTrig(HRTIM1, LL_HRTIM_TIMER_MASTER, LL_HRTIM_DACTRIG_NONE);
  LL_HRTIM_TIM_DisablePreload(HRTIM1, LL_HRTIM_TIMER_MASTER);
  LL_HRTIM_TIM_SetUpdateGating(HRTIM1, LL_HRTIM_TIMER_MASTER, LL_HRTIM_UPDATEGATING_INDEPENDENT);
  LL_HRTIM_TIM_SetUpdateTrig(HRTIM1, LL_HRTIM_TIMER_MASTER, LL_HRTIM_UPDATETRIG_NONE);
  LL_HRTIM_TIM_SetBurstModeOption(HRTIM1, LL_HRTIM_TIMER_MASTER, LL_HRTIM_BURSTMODE_MAINTAINCLOCK);
  LL_HRTIM_ForceUpdate(HRTIM1, LL_HRTIM_TIMER_MASTER);
  LL_HRTIM_TIM_SetCompare1(HRTIM1, LL_HRTIM_TIMER_MASTER, TIM_M_PERIOD * TIM_M_DUTY_CYCLE_STARTUP);

  /* Poll for DLL end of calibration */
#if (USE_TIMEOUT == 1)
  uint32_t Timeout = 10; /* Timeout Initialization */
#endif  /*USE_TIMEOUT*/

  while(LL_HRTIM_IsActiveFlag_DLLRDY(HRTIM1) == RESET){
#if (USE_TIMEOUT == 1)
    if (LL_SYSTICK_IsActiveCounterFlag())  /* Check Systick counter flag to decrement the time-out value */
    {
        if(Timeout-- == 0)
        {
          Error_Handler();  /* error management */
        }
    }
#endif  /* USE_TIMEOUT */
  }

  LL_HRTIM_TIM_SetPrescaler(HRTIM1, LL_HRTIM_TIMER_A, LL_HRTIM_PRESCALERRATIO_MUL32);
  LL_HRTIM_TIM_SetCounterMode(HRTIM1, LL_HRTIM_TIMER_A, LL_HRTIM_MODE_CONTINUOUS);
  LL_HRTIM_TIM_SetPeriod(HRTIM1, LL_HRTIM_TIMER_A, 0xFFDF);
  LL_HRTIM_TIM_SetRepetition(HRTIM1, LL_HRTIM_TIMER_A, 0x00);
  LL_HRTIM_TIM_SetUpdateGating(HRTIM1, LL_HRTIM_TIMER_A, LL_HRTIM_UPDATEGATING_INDEPENDENT);
  LL_HRTIM_TIM_SetCountingMode(HRTIM1, LL_HRTIM_TIMER_A, LL_HRTIM_COUNTING_MODE_UP);
  LL_HRTIM_TIM_SetDACTrig(HRTIM1, LL_HRTIM_TIMER_A, LL_HRTIM_DACTRIG_NONE);
  LL_HRTIM_TIM_DisableHalfMode(HRTIM1, LL_HRTIM_TIMER_A);
  LL_HRTIM_TIM_SetInterleavedMode(HRTIM1, LL_HRTIM_TIMER_A, LL_HRTIM_INTERLEAVED_MODE_DISABLED);
  LL_HRTIM_TIM_DisableStartOnSync(HRTIM1, LL_HRTIM_TIMER_A);
  LL_HRTIM_TIM_DisableResetOnSync(HRTIM1, LL_HRTIM_TIMER_A);
  LL_HRTIM_TIM_DisablePreload(HRTIM1, LL_HRTIM_TIMER_A);
  LL_HRTIM_TIM_SetUpdateTrig(HRTIM1, LL_HRTIM_TIMER_A, LL_HRTIM_UPDATETRIG_NONE|LL_HRTIM_UPDATETRIG_NONE);
  LL_HRTIM_TIM_SetResetTrig(HRTIM1, LL_HRTIM_TIMER_A, LL_HRTIM_RESETTRIG_NONE);
  LL_HRTIM_TIM_DisablePushPullMode(HRTIM1, LL_HRTIM_TIMER_A);
  LL_HRTIM_TIM_DisableDeadTime(HRTIM1, LL_HRTIM_TIMER_A);
  LL_HRTIM_TIM_SetBurstModeOption(HRTIM1, LL_HRTIM_TIMER_A, LL_HRTIM_BURSTMODE_MAINTAINCLOCK);
  LL_HRTIM_ForceUpdate(HRTIM1, LL_HRTIM_TIMER_A);
  LL_HRTIM_TIM_DisableResyncUpdate(HRTIM1, LL_HRTIM_TIMER_A);
  LL_HRTIM_OUT_SetPolarity(HRTIM1, HRTIMER_A_H, LL_HRTIM_OUT_POSITIVE_POLARITY);
  LL_HRTIM_OUT_SetOutputSetSrc(HRTIM1, HRTIMER_A_H, LL_HRTIM_OUTPUTSET_MASTERPER);
  LL_HRTIM_OUT_SetOutputResetSrc(HRTIM1, HRTIMER_A_H, LL_HRTIM_OUTPUTRESET_MASTERCMP1);
  LL_HRTIM_OUT_SetIdleMode(HRTIM1, HRTIMER_A_H, LL_HRTIM_OUT_NO_IDLE);
  LL_HRTIM_OUT_SetIdleLevel(HRTIM1, HRTIMER_A_H, LL_HRTIM_OUT_IDLELEVEL_INACTIVE);
  LL_HRTIM_OUT_SetFaultState(HRTIM1, HRTIMER_A_H, LL_HRTIM_OUT_FAULTSTATE_NO_ACTION);
  LL_HRTIM_OUT_SetChopperMode(HRTIM1, HRTIMER_A_H, LL_HRTIM_OUT_CHOPPERMODE_DISABLED);
  LL_HRTIM_OUT_SetPolarity(HRTIM1, HRTIMER_A_L, LL_HRTIM_OUT_POSITIVE_POLARITY);
  LL_HRTIM_OUT_SetOutputSetSrc(HRTIM1, HRTIMER_A_L, LL_HRTIM_OUTPUTSET_MASTERPER);
  LL_HRTIM_OUT_SetOutputResetSrc(HRTIM1, HRTIMER_A_L, LL_HRTIM_OUTPUTRESET_NONE);
  LL_HRTIM_OUT_SetIdleMode(HRTIM1, HRTIMER_A_L, LL_HRTIM_OUT_NO_IDLE);
  LL_HRTIM_OUT_SetIdleLevel(HRTIM1, HRTIMER_A_L, LL_HRTIM_OUT_IDLELEVEL_INACTIVE);
  LL_HRTIM_OUT_SetFaultState(HRTIM1, HRTIMER_A_L, LL_HRTIM_OUT_FAULTSTATE_NO_ACTION);
  LL_HRTIM_OUT_SetChopperMode(HRTIM1, HRTIMER_A_L, LL_HRTIM_OUT_CHOPPERMODE_DISABLED);

  /* Poll for DLL end of calibration */
#if (USE_TIMEOUT == 1)
  uint32_t Timeout = 10; /* Timeout Initialization */
#endif  /*USE_TIMEOUT*/

  while(LL_HRTIM_IsActiveFlag_DLLRDY(HRTIM1) == RESET){
#if (USE_TIMEOUT == 1)
    if (LL_SYSTICK_IsActiveCounterFlag())  /* Check Systick counter flag to decrement the time-out value */
    {
        if(Timeout-- == 0)
        {
          Error_Handler();  /* error management */
        }
    }
#endif  /* USE_TIMEOUT */
  }

  LL_HRTIM_TIM_SetPrescaler(HRTIM1, LL_HRTIM_TIMER_B, LL_HRTIM_PRESCALERRATIO_MUL32);
  LL_HRTIM_TIM_SetCounterMode(HRTIM1, LL_HRTIM_TIMER_B, LL_HRTIM_MODE_CONTINUOUS);
  LL_HRTIM_TIM_SetPeriod(HRTIM1, LL_HRTIM_TIMER_B, 0xFFDF);
  LL_HRTIM_TIM_SetRepetition(HRTIM1, LL_HRTIM_TIMER_B, 0x00);
  LL_HRTIM_TIM_SetUpdateGating(HRTIM1, LL_HRTIM_TIMER_B, LL_HRTIM_UPDATEGATING_INDEPENDENT);
  LL_HRTIM_TIM_SetCountingMode(HRTIM1, LL_HRTIM_TIMER_B, LL_HRTIM_COUNTING_MODE_UP);
  LL_HRTIM_TIM_SetDACTrig(HRTIM1, LL_HRTIM_TIMER_B, LL_HRTIM_DACTRIG_NONE);
  LL_HRTIM_TIM_DisableHalfMode(HRTIM1, LL_HRTIM_TIMER_B);
  LL_HRTIM_TIM_SetInterleavedMode(HRTIM1, LL_HRTIM_TIMER_B, LL_HRTIM_INTERLEAVED_MODE_DISABLED);
  LL_HRTIM_TIM_DisableStartOnSync(HRTIM1, LL_HRTIM_TIMER_B);
  LL_HRTIM_TIM_DisableResetOnSync(HRTIM1, LL_HRTIM_TIMER_B);
  LL_HRTIM_TIM_DisablePreload(HRTIM1, LL_HRTIM_TIMER_B);
  LL_HRTIM_TIM_SetUpdateTrig(HRTIM1, LL_HRTIM_TIMER_B, LL_HRTIM_UPDATETRIG_NONE|LL_HRTIM_UPDATETRIG_NONE);
  LL_HRTIM_TIM_SetResetTrig(HRTIM1, LL_HRTIM_TIMER_B, LL_HRTIM_RESETTRIG_NONE);
  LL_HRTIM_TIM_DisablePushPullMode(HRTIM1, LL_HRTIM_TIMER_B);
  LL_HRTIM_TIM_DisableDeadTime(HRTIM1, LL_HRTIM_TIMER_B);
  LL_HRTIM_TIM_SetBurstModeOption(HRTIM1, LL_HRTIM_TIMER_B, LL_HRTIM_BURSTMODE_MAINTAINCLOCK);
  LL_HRTIM_ForceUpdate(HRTIM1, LL_HRTIM_TIMER_B);
  LL_HRTIM_TIM_DisableResyncUpdate(HRTIM1, LL_HRTIM_TIMER_B);
  LL_HRTIM_OUT_SetPolarity(HRTIM1, HRTIMER_B_H, LL_HRTIM_OUT_POSITIVE_POLARITY);
  LL_HRTIM_OUT_SetOutputSetSrc(HRTIM1, HRTIMER_B_H, LL_HRTIM_OUTPUTSET_MASTERPER);
  LL_HRTIM_OUT_SetOutputResetSrc(HRTIM1, HRTIMER_B_H, LL_HRTIM_OUTPUTRESET_MASTERCMP1);
  LL_HRTIM_OUT_SetIdleMode(HRTIM1, HRTIMER_B_H, LL_HRTIM_OUT_NO_IDLE);
  LL_HRTIM_OUT_SetIdleLevel(HRTIM1, HRTIMER_B_H, LL_HRTIM_OUT_IDLELEVEL_INACTIVE);
  LL_HRTIM_OUT_SetFaultState(HRTIM1, HRTIMER_B_H, LL_HRTIM_OUT_FAULTSTATE_NO_ACTION);
  LL_HRTIM_OUT_SetChopperMode(HRTIM1, HRTIMER_B_H, LL_HRTIM_OUT_CHOPPERMODE_DISABLED);
  LL_HRTIM_OUT_SetPolarity(HRTIM1, HRTIMER_B_L, LL_HRTIM_OUT_POSITIVE_POLARITY);
  LL_HRTIM_OUT_SetOutputSetSrc(HRTIM1, HRTIMER_B_L, LL_HRTIM_OUTPUTSET_MASTERPER);
  LL_HRTIM_OUT_SetOutputResetSrc(HRTIM1, HRTIMER_B_L, LL_HRTIM_OUTPUTRESET_NONE);
  LL_HRTIM_OUT_SetIdleMode(HRTIM1, HRTIMER_B_L, LL_HRTIM_OUT_NO_IDLE);
  LL_HRTIM_OUT_SetIdleLevel(HRTIM1, HRTIMER_B_L, LL_HRTIM_OUT_IDLELEVEL_INACTIVE);
  LL_HRTIM_OUT_SetFaultState(HRTIM1, HRTIMER_B_L, LL_HRTIM_OUT_FAULTSTATE_NO_ACTION);
  LL_HRTIM_OUT_SetChopperMode(HRTIM1, HRTIMER_B_L, LL_HRTIM_OUT_CHOPPERMODE_DISABLED);

  /* Poll for DLL end of calibration */
#if (USE_TIMEOUT == 1)
  uint32_t Timeout = 10; /* Timeout Initialization */
#endif  /*USE_TIMEOUT*/

  while(LL_HRTIM_IsActiveFlag_DLLRDY(HRTIM1) == RESET){
#if (USE_TIMEOUT == 1)
    if (LL_SYSTICK_IsActiveCounterFlag())  /* Check Systick counter flag to decrement the time-out value */
    {
        if(Timeout-- == 0)
        {
          Error_Handler();  /* error management */
        }
    }
#endif  /* USE_TIMEOUT */
  }

  LL_HRTIM_TIM_SetPrescaler(HRTIM1, LL_HRTIM_TIMER_C, LL_HRTIM_PRESCALERRATIO_MUL32);
  LL_HRTIM_TIM_SetCounterMode(HRTIM1, LL_HRTIM_TIMER_C, LL_HRTIM_MODE_CONTINUOUS);
  LL_HRTIM_TIM_SetPeriod(HRTIM1, LL_HRTIM_TIMER_C, 0xFFDF);
  LL_HRTIM_TIM_SetRepetition(HRTIM1, LL_HRTIM_TIMER_C, 0x00);
  LL_HRTIM_TIM_SetUpdateGating(HRTIM1, LL_HRTIM_TIMER_C, LL_HRTIM_UPDATEGATING_INDEPENDENT);
  LL_HRTIM_TIM_SetCountingMode(HRTIM1, LL_HRTIM_TIMER_C, LL_HRTIM_COUNTING_MODE_UP);
  LL_HRTIM_TIM_SetDACTrig(HRTIM1, LL_HRTIM_TIMER_C, LL_HRTIM_DACTRIG_NONE);
  LL_HRTIM_TIM_DisableHalfMode(HRTIM1, LL_HRTIM_TIMER_C);
  LL_HRTIM_TIM_SetInterleavedMode(HRTIM1, LL_HRTIM_TIMER_C, LL_HRTIM_INTERLEAVED_MODE_DISABLED);
  LL_HRTIM_TIM_DisableStartOnSync(HRTIM1, LL_HRTIM_TIMER_C);
  LL_HRTIM_TIM_DisableResetOnSync(HRTIM1, LL_HRTIM_TIMER_C);
  LL_HRTIM_TIM_DisablePreload(HRTIM1, LL_HRTIM_TIMER_C);
  LL_HRTIM_TIM_SetUpdateTrig(HRTIM1, LL_HRTIM_TIMER_C, LL_HRTIM_UPDATETRIG_NONE|LL_HRTIM_UPDATETRIG_NONE);
  LL_HRTIM_TIM_SetResetTrig(HRTIM1, LL_HRTIM_TIMER_C, LL_HRTIM_RESETTRIG_NONE);
  LL_HRTIM_TIM_DisablePushPullMode(HRTIM1, LL_HRTIM_TIMER_C);
  LL_HRTIM_TIM_DisableDeadTime(HRTIM1, LL_HRTIM_TIMER_C);
  LL_HRTIM_TIM_SetBurstModeOption(HRTIM1, LL_HRTIM_TIMER_C, LL_HRTIM_BURSTMODE_MAINTAINCLOCK);
  LL_HRTIM_ForceUpdate(HRTIM1, LL_HRTIM_TIMER_C);
  LL_HRTIM_TIM_DisableResyncUpdate(HRTIM1, LL_HRTIM_TIMER_C);
  LL_HRTIM_OUT_SetPolarity(HRTIM1, HRTIMER_C_H, LL_HRTIM_OUT_POSITIVE_POLARITY);
  LL_HRTIM_OUT_SetOutputSetSrc(HRTIM1, HRTIMER_C_H, LL_HRTIM_OUTPUTSET_MASTERPER);
  LL_HRTIM_OUT_SetOutputResetSrc(HRTIM1, HRTIMER_C_H, LL_HRTIM_OUTPUTRESET_MASTERCMP1);
  LL_HRTIM_OUT_SetIdleMode(HRTIM1, HRTIMER_C_H, LL_HRTIM_OUT_NO_IDLE);
  LL_HRTIM_OUT_SetIdleLevel(HRTIM1, HRTIMER_C_H, LL_HRTIM_OUT_IDLELEVEL_INACTIVE);
  LL_HRTIM_OUT_SetFaultState(HRTIM1, HRTIMER_C_H, LL_HRTIM_OUT_FAULTSTATE_NO_ACTION);
  LL_HRTIM_OUT_SetChopperMode(HRTIM1, HRTIMER_C_H, LL_HRTIM_OUT_CHOPPERMODE_DISABLED);
  LL_HRTIM_OUT_SetPolarity(HRTIM1, HRTIMER_C_L, LL_HRTIM_OUT_POSITIVE_POLARITY);
  LL_HRTIM_OUT_SetOutputSetSrc(HRTIM1, HRTIMER_C_L, LL_HRTIM_OUTPUTSET_MASTERPER);
  LL_HRTIM_OUT_SetOutputResetSrc(HRTIM1, HRTIMER_C_L, LL_HRTIM_OUTPUTRESET_NONE);
  LL_HRTIM_OUT_SetIdleMode(HRTIM1, HRTIMER_C_L, LL_HRTIM_OUT_NO_IDLE);
  LL_HRTIM_OUT_SetIdleLevel(HRTIM1, HRTIMER_C_L, LL_HRTIM_OUT_IDLELEVEL_INACTIVE);
  LL_HRTIM_OUT_SetFaultState(HRTIM1, HRTIMER_C_L, LL_HRTIM_OUT_FAULTSTATE_NO_ACTION);
  LL_HRTIM_OUT_SetChopperMode(HRTIM1, HRTIMER_C_L, LL_HRTIM_OUT_CHOPPERMODE_DISABLED);

  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOB);
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);
  /**HRTIM1 GPIO Configuration
  PB12   ------> HRTIM1_CHC1
  PB13   ------> HRTIM1_CHC2
  PA8   ------> HRTIM1_CHA1
  PA9   ------> HRTIM1_CHA2
  PA10   ------> HRTIM1_CHB1
  PA11   ------> HRTIM1_CHB2
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_12;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_13;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_13;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_13;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_8;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_13;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_9;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_13;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_10;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_13;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_11;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_13;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  LL_HRTIM_DisableOutput(HRTIM1, HRTIMER_A_H  + HRTIMER_B_H + HRTIMER_C_H);
  LL_HRTIM_DisableOutput(HRTIM1, HRTIMER_A_L  + HRTIMER_B_L + HRTIMER_C_L);

  LL_HRTIM_TIM_CounterEnable(HRTIM1, LL_HRTIM_TIMER_MASTER + LL_HRTIM_TIMER_A + LL_HRTIM_TIMER_B + LL_HRTIM_TIMER_C);
}

/* Inicjalizacja GPIO */
void GPIO_Init(void)
{

	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

   LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOC);
   LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOF);
   LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);
   LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOB);

	// Comparators
	GPIO_InitStruct.Pin = LL_GPIO_PIN_11|LL_GPIO_PIN_14|LL_GPIO_PIN_15;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	// PIN na ESP
	GPIO_InitStruct.Pin = LL_GPIO_PIN_9;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_DOWN;
	LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	//ADC1
	GPIO_InitStruct.Pin = LL_GPIO_PIN_2;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	//ADC2
	GPIO_InitStruct.Pin = LL_GPIO_PIN_5;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = LL_GPIO_PIN_7;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = LL_GPIO_PIN_2;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	// ENABLE
	GPIO_InitStruct.Pin = LL_GPIO_PIN_4;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	// HALL
	GPIO_InitStruct.Pin = LL_GPIO_PIN_5|LL_GPIO_PIN_6|LL_GPIO_PIN_7;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/* Inicjalizacja ADC1 */
void ADC1_Init(void)
{
    LL_ADC_InitTypeDef ADC_InitStruct = {0};
    LL_ADC_REG_InitTypeDef ADC_REG_InitStruct = {0};
    LL_ADC_CommonInitTypeDef ADC_CommonInitStruct = {0};

    LL_RCC_SetADCClockSource(LL_RCC_ADC12_CLKSOURCE_SYSCLK);

	LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_ADC12 );

	/** Common config */
    ADC_InitStruct.Resolution = LL_ADC_RESOLUTION_12B;
    ADC_InitStruct.DataAlignment = LL_ADC_DATA_ALIGN_RIGHT;
    ADC_InitStruct.LowPowerMode = LL_ADC_LP_MODE_NONE;
	LL_ADC_Init(ADC1, &ADC_InitStruct);

	ADC_REG_InitStruct.TriggerSource = LL_ADC_REG_TRIG_SOFTWARE;
	ADC_REG_InitStruct.SequencerLength = LL_ADC_REG_SEQ_SCAN_DISABLE;
	ADC_REG_InitStruct.SequencerDiscont = LL_ADC_REG_SEQ_DISCONT_DISABLE;
	ADC_REG_InitStruct.ContinuousMode = LL_ADC_REG_CONV_CONTINUOUS;
	ADC_REG_InitStruct.DMATransfer = LL_ADC_REG_DMA_TRANSFER_NONE;
	ADC_REG_InitStruct.Overrun = LL_ADC_REG_OVR_DATA_OVERWRITTEN;
    LL_ADC_REG_Init(ADC1, &ADC_REG_InitStruct);

    ADC_CommonInitStruct.CommonClock = LL_ADC_CLOCK_SYNC_PCLK_DIV4;
    ADC_CommonInitStruct.Multimode = LL_ADC_MULTI_INDEPENDENT;
    LL_ADC_CommonInit(__LL_ADC_COMMON_INSTANCE(ADC1), &ADC_CommonInitStruct);

	/* Disable ADC deep power down (enabled by default after reset state) */
	LL_ADC_DisableDeepPowerDown(ADC1);
	/* Enable ADC internal voltage regulator */
	LL_ADC_EnableInternalRegulator(ADC1);

    uint32_t wait_loop_index;
	wait_loop_index = ((LL_ADC_DELAY_INTERNAL_REGUL_STAB_US * (SystemCoreClock / (100000 * 2))) / 10);
	while(wait_loop_index != 0)
	{
	  wait_loop_index--;
	}

	/** Configure Regular Channel*/
	LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_1, LL_ADC_CHANNEL_3);
	LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_3, LL_ADC_SAMPLINGTIME_247CYCLES_5);
	LL_ADC_SetChannelSingleDiff(ADC1, LL_ADC_CHANNEL_3, LL_ADC_SINGLE_ENDED);
}

/* Inicjalizacja ADC2 - nieużywany*/
void ADC2_Init(void)
{
    LL_ADC_InitTypeDef ADC_InitStruct = {0};
    LL_ADC_REG_InitTypeDef ADC_REG_InitStruct = {0};
    LL_ADC_CommonInitTypeDef ADC_CommonInitStruct = {0};

    LL_RCC_SetADCClockSource(LL_RCC_ADC12_CLKSOURCE_SYSCLK);

	LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_ADC12 );

	/** Common config */
    ADC_InitStruct.Resolution = LL_ADC_RESOLUTION_12B;
    ADC_InitStruct.DataAlignment = LL_ADC_DATA_ALIGN_RIGHT;
    ADC_InitStruct.LowPowerMode = LL_ADC_LP_MODE_NONE;
	LL_ADC_Init(ADC2, &ADC_InitStruct);

	ADC_REG_InitStruct.TriggerSource = LL_ADC_REG_TRIG_SOFTWARE;
	ADC_REG_InitStruct.SequencerLength = LL_ADC_REG_SEQ_SCAN_DISABLE;
	ADC_REG_InitStruct.SequencerDiscont = LL_ADC_REG_SEQ_DISCONT_DISABLE;
	ADC_REG_InitStruct.ContinuousMode = LL_ADC_REG_CONV_CONTINUOUS;
	ADC_REG_InitStruct.DMATransfer = LL_ADC_REG_DMA_TRANSFER_NONE;
	ADC_REG_InitStruct.Overrun = LL_ADC_REG_OVR_DATA_OVERWRITTEN;
    LL_ADC_REG_Init(ADC2, &ADC_REG_InitStruct);

    ADC_CommonInitStruct.CommonClock = LL_ADC_CLOCK_SYNC_PCLK_DIV4;
    ADC_CommonInitStruct.Multimode = LL_ADC_MULTI_INDEPENDENT;
    LL_ADC_CommonInit(__LL_ADC_COMMON_INSTANCE(ADC2), &ADC_CommonInitStruct);

	/* Disable ADC deep power down (enabled by default after reset state) */
	LL_ADC_DisableDeepPowerDown(ADC2);
	/* Enable ADC internal voltage regulator */
	LL_ADC_EnableInternalRegulator(ADC2);

    uint32_t wait_loop_index;
	wait_loop_index = ((LL_ADC_DELAY_INTERNAL_REGUL_STAB_US * (SystemCoreClock / (100000 * 2))) / 10);
	while(wait_loop_index != 0)
	{
	  wait_loop_index--;
	}

	/** Configure Regular Channel*/
	LL_ADC_REG_SetSequencerRanks(ADC2, LL_ADC_REG_RANK_1, LL_ADC_CHANNEL_4|LL_ADC_CHANNEL_12|LL_ADC_CHANNEL_13);
	LL_ADC_SetChannelSamplingTime(ADC2, LL_ADC_CHANNEL_4|LL_ADC_CHANNEL_12|LL_ADC_CHANNEL_13, LL_ADC_SAMPLINGTIME_247CYCLES_5);
	LL_ADC_SetChannelSingleDiff(ADC2, LL_ADC_CHANNEL_4|LL_ADC_CHANNEL_12|LL_ADC_CHANNEL_13, LL_ADC_SINGLE_ENDED);
}

/* Aktywacja ADC1 */
void Activate_ADC1(void)
{
	 LL_ADC_Enable(ADC1);
	 while (LL_ADC_IsActiveFlag_ADRDY(ADC1) == 0){}

	 LL_ADC_REG_StartConversion(ADC1);
}

/* Aktywacja ADC2 - nieużywany */
void Activate_ADC2(void)
{
	 LL_ADC_Enable(ADC2);
	 while (LL_ADC_IsActiveFlag_ADRDY(ADC2) == 0){}

	 LL_ADC_REG_StartConversion(ADC2);
}

/* Inicjalizacja TIM5 */
void TIM5_Init(void)
{
  LL_TIM_InitTypeDef TIM_InitStruct = {0};
  LL_TIM_OC_InitTypeDef TIM_OC_InitStruct = {0};
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM5);

  TIM_InitStruct.Prescaler = TIM5_PRESCALE-1;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = TIM5_PERIOD;
  LL_TIM_Init(TIM5, &TIM_InitStruct);

  LL_TIM_EnableARRPreload(TIM5);
  LL_TIM_OC_EnablePreload(TIM5, LL_TIM_CHANNEL_CH1);

  TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_PWM1;
  TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;
  TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;
  TIM_OC_InitStruct.CompareValue = TIM5_PERIOD-1;
  TIM_OC_InitStruct.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
  LL_TIM_OC_Init(TIM5, LL_TIM_CHANNEL_CH1, &TIM_OC_InitStruct);
  LL_TIM_OC_DisableFast(TIM5, LL_TIM_CHANNEL_CH1);
  LL_TIM_SetTriggerOutput(TIM5, LL_TIM_TRGO_RESET);
  LL_TIM_DisableMasterSlaveMode(TIM5);

  LL_TIM_EnableIT_CC1(TIM5);

  NVIC_SetPriority(TIM5_IRQn,8);

  /* Enable output channel 1 */
  LL_TIM_CC_EnableChannel(TIM5, LL_TIM_CHANNEL_CH1);

  /* Enable counter */
  LL_TIM_EnableCounter(TIM5);

  /* Force update generation */
  LL_TIM_GenerateEvent_UPDATE(TIM5);
}

/* Inicjalizacja USART2 */
void USART2_Init(void)
{
	  LL_USART_InitTypeDef USART_InitStruct = {0};
	  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

	  LL_RCC_SetUSARTClockSource(LL_RCC_USART2_CLKSOURCE_PCLK1);

	  /* Peripheral clock enable */
	  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);

	  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOB);
	  /**USART2 GPIO Configuration
	  PB3   ------> USART2_TX
	  PB4   ------> USART2_RX
	  */
	  GPIO_InitStruct.Pin = LL_GPIO_PIN_3;
	  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
	  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
	  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	  GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
	  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	  GPIO_InitStruct.Pin = LL_GPIO_PIN_4;
	  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
	  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
	  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	  GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
	  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	  USART_InitStruct.PrescalerValue = LL_USART_PRESCALER_DIV1;
	  USART_InitStruct.BaudRate = 115200;
	  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
	  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
	  USART_InitStruct.Parity = LL_USART_PARITY_NONE;
	  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
	  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
	  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
	  LL_USART_Init(USART2, &USART_InitStruct);
	  LL_USART_SetTXFIFOThreshold(USART2, LL_USART_FIFOTHRESHOLD_1_8);
	  LL_USART_SetRXFIFOThreshold(USART2, LL_USART_FIFOTHRESHOLD_1_8);
	  LL_USART_DisableFIFO(USART2);
	  LL_USART_ConfigAsyncMode(USART2);

	  LL_USART_Enable(USART2);

	  /* Polling USART2 initialisation */
	  while((!(LL_USART_IsActiveFlag_TEACK(USART2))) || (!(LL_USART_IsActiveFlag_REACK(USART2))))
	  {
	  }
}

/* Konfiguracja przerwań EXTI dla trybu BEMF */
void EXTI_Init_BEMF(void)
{
	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

	// Setting pin sources of EXTI lines
	LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTB, LL_SYSCFG_EXTI_LINE11);
	LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTB, LL_SYSCFG_EXTI_LINE14);
	LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTB, LL_SYSCFG_EXTI_LINE15);

	// Set edge detection to Rising and Falling edge
	LL_EXTI_EnableRisingTrig_0_31(LL_EXTI_LINE_11|LL_EXTI_LINE_14|LL_EXTI_LINE_15 );
	LL_EXTI_EnableFallingTrig_0_31(LL_EXTI_LINE_11|LL_EXTI_LINE_14|LL_EXTI_LINE_15);

	//Enable Interrupts on lines: 11, 14, 15
	LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_11|LL_EXTI_LINE_14|LL_EXTI_LINE_15);

	NVIC_SetPriority(EXTI15_10_IRQn,9);
}

/* Wystartuj silnik w trybie BEMF */
void BEMF_Init(void)
{
	//Inicjalizacja peryferiów
	EXTI_Init_BEMF();
	TIM5_Init();

	// Czas poszczególnych komutacji rozruchu
	uint32_t i;

	// Wyrównaj silnik
	LL_HRTIM_EnableOutput(HRTIM1, HRTIMER_C_H + HRTIMER_A_L  + HRTIMER_B_L);

	i = 5000; // i= 1 ~10us
	LL_mDelay(i);

	LL_HRTIM_DisableOutput(HRTIM1, HRTIMER_C_H);
	LL_HRTIM_DisableOutput(HRTIM1, HRTIMER_A_L  + HRTIMER_B_L);

	LL_mDelay(i*5);

	// Start silnika
	motor_phase = 0;
	i = 1000;
	while(i > 300)
	{
		motor_phase = Phase_Commutation(motor_phase);
		LL_mDelay(i);

		i = i * 98/100;
	}
	LL_HRTIM_DisableOutput(HRTIM1, HRTIMER_A_H  + HRTIMER_B_H + HRTIMER_C_H);
	LL_HRTIM_DisableOutput(HRTIM1, HRTIMER_A_L  + HRTIMER_B_L + HRTIMER_C_L);

	LL_TIM_GenerateEvent_UPDATE(TIM5);

	// Dalsze działanie oparte na detekcji BEMF
	NVIC_EnableIRQ(TIM5_IRQn);
	NVIC_EnableIRQ(EXTI15_10_IRQn);
}

/* Konfiguracja przerwań EXTI dla trybu HALL */
void EXTI_Init_HALL(void)
{
	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

	// Setting pin sources of EXTI lines
	LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTB, LL_SYSCFG_EXTI_LINE5);
	LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTB, LL_SYSCFG_EXTI_LINE6);
	LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTB, LL_SYSCFG_EXTI_LINE7);

	// Set edge detection to Rising and Falling edge
	LL_EXTI_EnableRisingTrig_0_31(LL_EXTI_LINE_5|LL_EXTI_LINE_6|LL_EXTI_LINE_7 );
	LL_EXTI_EnableFallingTrig_0_31(LL_EXTI_LINE_5|LL_EXTI_LINE_6|LL_EXTI_LINE_7);

	//Enable Interrupts on lines: 5, 6, 7
	LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_5|LL_EXTI_LINE_6|LL_EXTI_LINE_7);

	NVIC_SetPriority(EXTI9_5_IRQn,9);
	//NVIC_EnableIRQ(EXTI9_5_IRQn);
}

/* Wystartuj silnik w trybie HALL */
void HALL_Init(void)
{

	//Inicjalizacja peryferiów
	EXTI_Init_HALL();
	TIM5_Init();

	//Pierwsza komutacja
	motor_phase = Phase_Commutation(motor_phase);

	//Dalsze działanie oparte na detekcji HALL
	NVIC_EnableIRQ(EXTI9_5_IRQn);
}

/* Określ położenie silnika na podstawie czujników Halla */
uint8_t Detect_Hall_State(void)
{
 uint8_t Motor_position =0;
 uint8_t motor_phase    =0;

 Motor_position = (GPIOB->IDR & EXTI5_7_MASK) >> 5;
 switch(Motor_position)
 	{
 		case HALL_STEP_1:
 			motor_phase = 0;
 			break;

 		case HALL_STEP_2:
 			motor_phase = 1;
 			break;

 		case HALL_STEP_3:
 			motor_phase = 2;
 			break;

 		case HALL_STEP_4:
 			motor_phase = 3;
 			break;

 		case HALL_STEP_5:
 			motor_phase = 4;
 			break;

 		case HALL_STEP_6:
 			motor_phase = 5;
 			break;
 		default:
 			motor_phase = 10;
 			break;
 	}

 return motor_phase;
}

/* Komutacja faz silnika*/
uint8_t Phase_Commutation(uint8_t Phase)
{
	switch(Phase)
	{
		case 0:
			 LL_HRTIM_DisableOutput(HRTIM1, HRTIMER_C_H);
			 LL_HRTIM_EnableOutput(HRTIM1, HRTIMER_A_H + HRTIMER_B_L);
			break;

		case 1:
			 LL_HRTIM_DisableOutput(HRTIM1, HRTIMER_B_L);
			 LL_HRTIM_EnableOutput(HRTIM1, HRTIMER_A_H + HRTIMER_C_L);
			break;

		case 2:
			 LL_HRTIM_DisableOutput(HRTIM1, HRTIMER_A_H);
			 LL_HRTIM_EnableOutput(HRTIM1, HRTIMER_B_H + HRTIMER_C_L);
			break;

		case 3:
			 LL_HRTIM_DisableOutput(HRTIM1, HRTIMER_C_L);
			 LL_HRTIM_EnableOutput(HRTIM1, HRTIMER_B_H + HRTIMER_A_L);
			break;

		case 4:
			 LL_HRTIM_DisableOutput(HRTIM1, HRTIMER_B_H);
			 LL_HRTIM_EnableOutput(HRTIM1, HRTIMER_C_H + HRTIMER_A_L);
			break;

		case 5:
			 LL_HRTIM_DisableOutput(HRTIM1, HRTIMER_A_L);
			 LL_HRTIM_EnableOutput(HRTIM1, HRTIMER_C_H + HRTIMER_B_L);
			break;
	}
	Phase++;
	Phase %= 6;

	return Phase;
}


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

#ifdef  USE_FULL_ASSERT
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
