#include "headers/robot.h"
#include "tim.h"
#include "adc.h"

encoder_t encoder_r;
stepper_t stepper_r;

encoder_t encoder_l;
stepper_t stepper_l;

imu_t imu;
power_module_t power_module;

void Robot_init(){
	HAL_TIM_Base_Start_IT(&htim6);						// Display timer (0.1MHz)
	HAL_TIM_Base_Start_IT(&htim7);						// Timeline
	HAL_TIM_Base_Start_IT(&htim10);						// Stepper timer
	HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);		// Encoder right
	HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);		// Encoder left
	HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_2);			// Stepper left
	HAL_TIM_PWM_Start(&htim5,TIM_CHANNEL_1);			// Stepper right

	// IMU Init to do
	IMU_Init(&imu, &hi2c1, MPU_6050_ADDR); // Initialize IMU with I2C handler, address, and buffer size

	Encoder_init(&encoder_l, &htim3, &htim7, -1);
	Stepper_init(&stepper_l, &htim5, TIM_CHANNEL_1, &encoder_l, GPIOA, GPIO_PIN_4);

	Encoder_init(&encoder_r, &htim4, &htim7, 1);
	Stepper_init(&stepper_r, &htim2, TIM_CHANNEL_2, &encoder_r, GPIOB, GPIO_PIN_0);

	PowerModule_init(&power_module, &hadc1);

	MAX72_init(&display);
}
