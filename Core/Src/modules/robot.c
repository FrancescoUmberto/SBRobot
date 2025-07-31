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

	encoder_init(&encoder_l, &htim3, &htim7, -1);
	stepper_init(&stepper_l, &htim5, TIM_CHANNEL_1, &encoder_l, GPIOA, GPIO_PIN_4);

	encoder_init(&encoder_r, &htim4, &htim7, 1);
	stepper_init(&stepper_r, &htim2, TIM_CHANNEL_2, &encoder_r, GPIOB, GPIO_PIN_0);

	// IMU Init to do
	PowerModule_init(&power_module, &hadc1);

	MAX72_init();
}
