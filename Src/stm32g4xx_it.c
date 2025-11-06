/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32g4xx_it.c
  * @brief   Interrupt Service Routines.
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
#include "stm32g4xx_it.h"

extern volatile uint8_t motor_phase;
volatile uint32_t commutation_period;
volatile uint32_t TIM5_period;
extern uint32_t adc_value;
uint32_t Duty_Cycle = TIM_M_DUTY_CYCLE_STARTUP * 10000;

volatile int32_t value_error0 = 0;
volatile int32_t value_error1 = 0;

uint32_t index;
volatile uint32_t set_period; // 10ms
volatile char user_speed;

uint8_t even_commutation_flag;

void PrintInfo(uint8_t *String, uint32_t Size)
{
  uint32_t index = 0;
  uint8_t *pchar = String;

  /* Send characters one per one, until last char to be sent */
  for (index = 0; index < Size; index++)
  {
    /* Wait for TXE flag to be raised */
    while (!LL_USART_IsActiveFlag_TXE(USART2))
    {
    }

    /* Write character in Transmit Data register.
       TXE flag is cleared by writing data in TDR register */
    LL_USART_TransmitData8(USART2, *pchar++);
  }

  /* Wait for TC flag to be raised for last char */
  while (!LL_USART_IsActiveFlag_TC(USART2))
  {
  }
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/

/* USER CODE BEGIN EV */
/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
   while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Prefetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */

  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32G4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32g4xx.s).                    */
/******************************************************************************/

/* USER CODE BEGIN 1 */

// HALL edge detection
void EXTI9_5_IRQHandler(void)
{
	motor_phase = Phase_Commutation(motor_phase);

	// Get period of current motor phase duration
	commutation_period = LL_TIM_GetCounter(TIM5);
	LL_TIM_GenerateEvent_UPDATE(TIM5);

	set_period = 2700 - (ADC1->DR) ;

	value_error1 = value_error0;
	value_error0 = commutation_period - set_period;

	//Duty_Cycle = Duty_Cycle + (int32_t)(PI_K * value_error1);
	/* Regulator PI */
	Duty_Cycle = Duty_Cycle + PI_K*((value_error0 - value_error1) + PI_T/PI_Ti * value_error0);

	uint8_t Duty = Duty_Cycle/100;
	uint8_t r_period = commutation_period/100;


	if(Duty_Cycle>9900)
	{
		Duty_Cycle = 9900;
	}

	LL_HRTIM_TIM_SetCompare1(HRTIM1, LL_HRTIM_TIMER_MASTER, TIM_M_PERIOD * Duty_Cycle/10000);

	//
    char str[3]= "";
    char separator;

    if(even_commutation_flag==0)
    {
    	// Convert duty cycle to string
    	itoa(Duty, str, 10);
    	separator = 'x';
    	even_commutation_flag = 1;
    }
    else
    {
    	// Convert commutation period to string
    	itoa(r_period, str, 10);
    	separator = ' ';
    	even_commutation_flag = 0;
    }
    for (index = 0; index < sizeof(str); index++)
   {
	 /* Wait for TXE flag to be raised */
	 while (!LL_USART_IsActiveFlag_TXE(USART2)){}

	 /* Write character in Transmit Data register.
	    TXE flag is cleared by writing data in TDR register */
	 LL_USART_TransmitData8(USART2, str[index]);
   }
   while (!LL_USART_IsActiveFlag_TXE(USART2)){}
   LL_USART_TransmitData8(USART2, separator);

	LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_5|LL_EXTI_LINE_6|LL_EXTI_LINE_7);
}
// BEMF edge detection (voltage zero-crossing)
void EXTI15_10_IRQHandler(void)
{
	// Get period of current motor phase duration
	commutation_period = LL_TIM_GetCounter(TIM5);

	if(commutation_period>200)
	{
		// Reset TIM5 counter and set CompareCH1 on half period of last motor phase duration
		LL_TIM_GenerateEvent_UPDATE(TIM5);

		TIM5_period = commutation_period/2;
		LL_TIM_OC_SetCompareCH1(TIM5, TIM5_period);

		/* Odczytaj zadaną prędkość */
		set_period = 2700 - (ADC1->DR) ;

		value_error1 = value_error0;
		value_error0 = commutation_period - set_period;

		//Duty_Cycle = Duty_Cycle + (int32_t)(PI_K * value_error1);
		/* Regulator PI */
		Duty_Cycle = Duty_Cycle + PI_K*((value_error0 - value_error1) + PI_T/PI_Ti * value_error0);

		uint8_t Duty = Duty_Cycle/100;
		uint8_t r_period = commutation_period/100;;

		if(Duty_Cycle>9900)
		{
			Duty_Cycle = 99000;
		}


		LL_HRTIM_TIM_SetCompare1(HRTIM1, LL_HRTIM_TIMER_MASTER, TIM_M_PERIOD * Duty_Cycle/10000);

		//
	    char str[3]= "";
	    char separator;

	    if(even_commutation_flag==0)
	    {
	    	/* Konwersja wypełnienia sygnału na string */
	    	itoa(Duty, str, 10);
	    	separator = 'x';
	    	even_commutation_flag = 1;
	    }
	    else
	    {
	    	/* Konwersja okresu komutacji na string */
	    	itoa(r_period, str, 10);
	    	separator = ' ';
	    	even_commutation_flag = 0;
	    }
	    for (index = 0; index < sizeof(str); index++)
	   {
		 /* Wait for TXE flag to be raised */
		 while (!LL_USART_IsActiveFlag_TXE(USART2)){}

		 /* Write character in Transmit Data register.
		    TXE flag is cleared by writing data in TDR register */
		 LL_USART_TransmitData8(USART2, str[index]);
	   }
	   while (!LL_USART_IsActiveFlag_TXE(USART2)){}
	   LL_USART_TransmitData8(USART2, separator);

	}
	LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_11|LL_EXTI_LINE_14|LL_EXTI_LINE_15);
}

/* Komutacja wywołana przez BEMF */
void TIM5_IRQHandler(void)
{
	motor_phase = Phase_Commutation(motor_phase);

	LL_TIM_ClearFlag_CC1(TIM5);
}
