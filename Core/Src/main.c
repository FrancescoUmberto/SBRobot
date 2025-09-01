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
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "stm32f4xx.h"
#include "headers/robot.h"
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

/* USER CODE BEGIN PV */
static uint8_t tim6_update_cnt = 0;
static uint8_t IMU_Rx_Cplt = 0; // Flag to indicate that IMU data has been received

uint8_t rx_byte;
static uint8_t rx_index = 0;
static char js_buffer[15];
static uint8_t js_msg_ready = 0;

robot_t robot;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void transmit_IMU_data(){
	  char msg[80];
	  static uint8_t calibration_step_shown = 0;
	  switch (imu.calibration_mode) {
		  case 1: // Calibration mode
	//	    			  MAX72_Print_String("Pitch forward", NO_SETTINGS);
			  snprintf(msg, sizeof(msg), "Pitch forward\n");
			break;
		  case 2: // Calibration mode
			  //	    			  MAX72_Print_String("Pitch backward", NO_SETTINGS);
			  snprintf(msg, sizeof(msg), "Pitch backward\n");
			break;
	  }
	  if (calibration_step_shown < imu.calibration_mode) {
		  HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
		  calibration_step_shown = imu.calibration_mode; // Update the last shown step
	  }

	  // print az imu
	  if (imu.calibration_mode){
		  snprintf(msg, sizeof(msg), "Az: %.3f	Bias: %.3f	ax: %.3f	wy: %.3f\n", imu.az, imu.az_bias,imu.ax,imu.wy);
	  } else {
		  snprintf(msg, sizeof(msg), "Deg: %.3f ° - wy: %.3f - ax: %.3f - az: %.3f - az_bias: %.3f\n", imu.angle, imu.wy, imu.ax,imu.az,imu.az_bias);
	  }

	  HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

static void show_calibration_messages(){
		  static uint8_t calibration_step_shown = 0;
		  static char msg[20];
		  switch (imu.calibration_mode) {
			  case 1: // Calibration mode
		//	    			  MAX72_Print_String("Pitch forward", NO_SETTINGS);
				  snprintf(msg, sizeof(msg), "Pitch forward\n");
				break;
			  case 2: // Calibration mode
				  //	    			  MAX72_Print_String("Pitch backward", NO_SETTINGS);
				  snprintf(msg, sizeof(msg), "Pitch backward\n");
				break;
		  }
		  if (calibration_step_shown < imu.calibration_mode) {
			  MAX72_Stop_Changing_Data(&display, 1);
			  MAX72_Scroll_Start_IT(msg);
			  calibration_step_shown = imu.calibration_mode; // Update the last shown step
		  } else if (calibration_step_shown>imu.calibration_mode) {
			  // Reset the calibration step shown when calibration is complete
			  calibration_step_shown = 0;
			  MAX72_Resume_Changing_Data(&display,1);
		  }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

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
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_TIM4_Init();
  MX_TIM6_Init();
  MX_SPI2_Init();
  MX_TIM2_Init();
  MX_TIM7_Init();
  MX_TIM3_Init();
  MX_TIM5_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_TIM10_Init();
  MX_USART6_UART_Init();
  MX_TIM11_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  HAL_Delay(50);
  Robot_init(&robot);

//  char str[] = "SBRobot";
//  display_data_t data = {str, PRINT_SCROLL, NO_SETTINGS, DISPLAY_TYPE_STRING, 0};
//  MAX72_Add_Data(&display, &data);

//  display_data_t data2 = {&encoder_l.speed, PRINT_FLOAT, MINIDIGITS, DISPLAY_TYPE_FLOAT, 3};
//  MAX72_Add_Data(&display, &data2);

  display_data_t data3 = {&imu.angle, PRINT_FLOAT, FLOAT, DISPLAY_TYPE_FLOAT, 2};
  MAX72_Add_Data(&display, &data3);

//  display_data_t data4 = {&power_module.voltage, PRINT_FLOAT, NO_SETTINGS, DISPLAY_TYPE_FLOAT, 2};
//  MAX72_Add_Data(&display, &data4);

  HAL_UART_Receive_DMA(&huart6, (uint8_t*)js_buffer, 14);
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	  if (IMU_Rx_Cplt) {
		  IMU_Rx_Cplt = 0; // Reset flag
		  IMU_Compute_Data(&imu); // Process received data
	  }

	  if (js_msg_ready) {
		  js_msg_ready = 0; // Reset flag

//		  if (huart6.ErrorCode != HAL_UART_ERROR_NONE) {
//		      uint32_t err = huart6.ErrorCode;
//
//		      if (err & HAL_UART_ERROR_PE)  printf("UART ERROR: Parity\n");
//		      if (err & HAL_UART_ERROR_NE)  printf("UART ERROR: Noise\n");
//		      if (err & HAL_UART_ERROR_FE)  printf("UART ERROR: Framing\n");
//		      if (err & HAL_UART_ERROR_ORE) printf("UART ERROR: Overrun\n");
//
//		      // Pulisci i flag
//		      __HAL_UART_CLEAR_PEFLAG(&huart6);
//		      __HAL_UART_CLEAR_FEFLAG(&huart6);
//		      __HAL_UART_CLEAR_NEFLAG(&huart6);
//		      __HAL_UART_CLEAR_OREFLAG(&huart6);
//
//		      // Azzeriamo anche ErrorCode nella struct
//		      huart6.ErrorCode = HAL_UART_ERROR_NONE;
//		  }
		  Robot_read_serial_msg(&robot, js_buffer);
	  }

	  static uint8_t last_cnt = 255;
	  if (last_cnt != tim6_update_cnt) { // Update every 100ms
	      last_cnt = tim6_update_cnt;

	      //TODO Activate
//	      PowerModule_update_data(&power_module);

	      MAX72_Update_Data(&display);
	      if (tim6_update_cnt % 5 == 0) { // Update every 500ms
	    	  // Display refresh data


	    	  // Send IMU data via UART for debugging
//	    	  transmit_IMU_data();
//			  show_calibration_messages();

	    	  if (tim6_update_cnt % 10 == 0) { // Every 1 second
	    		  MAX72_Change_Data(&display,0);
	    	  }
	      }

	      MAX72_Scroll_Process(); // Process scrolling text
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
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if (htim->Instance == TIM6){
		tim6_update_cnt++;
		if (tim6_update_cnt == 250){
			tim6_update_cnt = 0;
		}
	} else if (htim->Instance == TIM7) {
    if (pid.active){
      PID_Update(&pid);
    }

		speed_control(&stepper_r);
		speed_control(&stepper_l);
	} else if (htim->Instance == TIM10){
		// Read from IMU
		IMU_ReadData(&imu);
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if(GPIO_Pin == GPIO_PIN_5) {
    on_click();
  }
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c) {
	if (hi2c == imu.hi2c) {
		// Data received from IMU, process it
		IMU_Rx_Cplt = 1; // Set flag to indicate data is ready
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART6) {

			js_buffer[14] = '\0';     // chiudi stringa
			js_msg_ready = 1;         // segnala che il messaggio è pronto
	}
}

 int __io_putchar(int ch){
 	ITM_SendChar(ch);
 	return ch;
 }
/* USER CODE END 4 */

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
