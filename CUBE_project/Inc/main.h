/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_ll_hrtim.h"
#include "stm32g4xx_ll_rcc.h"
#include "stm32g4xx_ll_bus.h"
#include "stm32g4xx_ll_crs.h"
#include "stm32g4xx_ll_system.h"
#include "stm32g4xx_ll_exti.h"
#include "stm32g4xx_ll_cortex.h"
#include "stm32g4xx_ll_utils.h"
#include "stm32g4xx_ll_pwr.h"
#include "stm32g4xx_ll_dma.h"
#include "stm32g4xx_ll_gpio.h"

#if defined(USE_FULL_ASSERT)
#include "stm32_assert.h"
#endif /* USE_FULL_ASSERT */

#include "stm32g4xx_ll_adc.h"
#include "stm32g4xx_ll_tim.h"
#include "stm32g4xx_ll_usart.h"


/* Funkcje */
void Error_Handler(void);

void GPIO_Init(void);
void ADC1_Init(void);
void Activate_ADC1(void);
void ADC2_Init(void);
void Activate_ADC2(void);
void TIM5_Init(void);
void USART2_Init(void);

void EXTI_Init_BEMF(void);
void EXTI_Init_HALL(void);
void BEMF_Init(void);
void HALL_Init(void);

uint8_t Detect_Hall_State(void);
uint8_t Phase_Commutation(uint8_t Phase);


/* Definicje*/
#define LED2_Pin LL_GPIO_PIN_5
#define LED2_GPIO_Port GPIOA
#ifndef NVIC_PRIORITYGROUP_0
#define NVIC_PRIORITYGROUP_0         ((uint32_t)0x00000007) /*!< 0 bit  for pre-emption priority,
                                                                 4 bits for subpriority */
#define NVIC_PRIORITYGROUP_1         ((uint32_t)0x00000006) /*!< 1 bit  for pre-emption priority,
                                                                 3 bits for subpriority */
#define NVIC_PRIORITYGROUP_2         ((uint32_t)0x00000005) /*!< 2 bits for pre-emption priority,
                                                                 2 bits for subpriority */
#define NVIC_PRIORITYGROUP_3         ((uint32_t)0x00000004) /*!< 3 bits for pre-emption priority,
                                                                 1 bit  for subpriority */
#define NVIC_PRIORITYGROUP_4         ((uint32_t)0x00000003) /*!< 4 bits for pre-emption priority,
                                                                 0 bit  for subpriority */
#endif

/* System */
#define HCLK_frequency 160000000

/* HRTIMER */
#define HRTIM_INPUT_CLOCK HCLK_frequency

#define TIM_M_PWM_FREQ 20000
#define TIM_M_DUTY_CYCLE_STARTUP 0.15
#define TIM_M_DUTY_CYCLE 0.01
#define TIM_M_PERIOD ((uint16_t)((((uint64_t)HRTIM_INPUT_CLOCK) * 8) / TIM_M_PWM_FREQ))
/*				*/
/* Zamiana oznaczeń wyjść HRTIMER */
#define HRTIMER_A_H LL_HRTIM_OUTPUT_TB1
#define HRTIMER_B_H LL_HRTIM_OUTPUT_TA1
#define HRTIMER_C_H LL_HRTIM_OUTPUT_TC1
#define HRTIMER_A_L LL_HRTIM_OUTPUT_TB2
#define HRTIMER_B_L LL_HRTIM_OUTPUT_TA2
#define HRTIMER_C_L LL_HRTIM_OUTPUT_TC2

/* Maski poszczególnych komutacji */
#define HALL_STEP_1 0x3
#define HALL_STEP_2 0x2
#define HALL_STEP_3 0x6
#define HALL_STEP_4 0x4
#define HALL_STEP_5 0x5
#define HALL_STEP_6 0x1

/* TIM5 */ // Tick = 1us, Max = 40ms
#define TIM5_PERIOD 40000
#define TIM5_PRESCALE 160

/* ESC mode */
#define FEEDBACK_MODE 	    0x02
#define FEEDBACK_MODE_HALL  0x00
#define FEEDBACK_MODE_BEMF  0x01

#define CONTROL_MODE 			   0x02
#define CONTROL_MODE_POTENTIOMETER 0x00
#define CONTROL_MODE_ESP 		   0x02

/* Maska dla czujników Halla */
#define EXTI5_7_MASK 0x07 << 5

/* PI regulator */
#define FREQUENCY_RESOLUTION 1000000
#define PI_K  0.025
#define PI_T  3
#define PI_Ti 2
#define SET_PERIOD 100000 // 5000us - > 5ms -> 200Hz
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
