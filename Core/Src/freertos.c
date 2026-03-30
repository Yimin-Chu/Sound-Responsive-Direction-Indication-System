/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "usb_otg.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "ssd1306.h"
#include "mic_audio.h"
#include "oled_ui.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for micTask */
osThreadId_t micTaskHandle;
const osThreadAttr_t micTask_attributes = {
  .name = "micTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for motorTask */
osThreadId_t motorTaskHandle;
const osThreadAttr_t motorTask_attributes = {
  .name = "motorTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for oledTask */
osThreadId_t oledTaskHandle;
const osThreadAttr_t oledTask_attributes = {
  .name = "oledTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for micMutex */
osMutexId_t micMutexHandle;
const osMutexAttr_t micMutex_attributes = {
  .name = "micMutex"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

extern mic_result_t mic;
extern uint32_t adc_dual_buf[MIC_BUF_SAMPLES];
extern volatile uint8_t adc_ready;

void StartMicTask(void *argument);
void StartMotorTask(void *argument);
void StartOledTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* creation of micMutex */
  micMutexHandle = osMutexNew(&micMutex_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of micTask */
  micTaskHandle = osThreadNew(StartMicTask, NULL, &micTask_attributes);

  /* creation of motorTask */
  motorTaskHandle = osThreadNew(StartMotorTask, NULL, &motorTask_attributes);

  /* creation of oledTask */
  oledTaskHandle = osThreadNew(StartOledTask, NULL, &oledTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartMicTask */
/**
  * @brief  Function implementing the micTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartMicTask */
void StartMicTask(void *argument)
{
  /* USER CODE BEGIN StartMicTask */
  for(;;)
  {
	  if (adc_ready)
	  {
	        adc_ready = 0;
	        if (osMutexAcquire(micMutexHandle, 10) == osOK)
	        {
	            mic_process(adc_dual_buf, MIC_BUF_SAMPLES, &mic);
	            osMutexRelease(micMutexHandle);
	        }
	  }
	  osDelay(1); // Small sleep to let other tasks run
	}
  /* USER CODE END StartMicTask */
}

/* USER CODE BEGIN Header_StartMotorTask */
/**
* @brief Function implementing the motorTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartMotorTask */
void StartMotorTask(void *argument)
{
  for(;;)
  {
	  if (osMutexAcquire(micMutexHandle, 10) == osOK)
	  {
	          mic_direction_t direc = mic.direction;
	          osMutexRelease(micMutexHandle);
	          switch(direc)
	          {
	              case MIC_DIR_LEFT:
	            	  set_servo_pulse(2000);
	            	  break;
	              case MIC_DIR_RIGHT:
	            	  set_servo_pulse(1000);
	            	  break;
	              case MIC_DIR_CENTER:
	            	  set_servo_pulse(1500);
	            	  break;
	              default:
	            	  break;
	          }
	      }
	      osDelay(20); // Servos update every 20ms
  }
  /* USER CODE END StartMotorTask */
}

/* USER CODE BEGIN Header_StartOledTask */
/**
* @brief Function implementing the oledTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartOledTask */
void StartOledTask(void *argument)
{
  /* USER CODE BEGIN StartOledTask */
  /* Infinite loop */
  for(;;)
  {
	  if (osMutexAcquire(micMutexHandle, 10) == osOK)
	  {
	          oled_draw_dual(&mic);
	          char msg[64];
	          sprintf(msg, "L:%3u R:%3u D:%+4d %s\r\n", mic.left_rms, mic.right_rms, mic.diff, mic_dir_str(mic.direction));
	          HAL_UART_Transmit(&huart3, (uint8_t *)msg, strlen(msg), 100);

	          osMutexRelease(micMutexHandle);
	      }
	      osDelay(500); // Replaces your 500ms if() check
  }
  /* USER CODE END StartOledTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

