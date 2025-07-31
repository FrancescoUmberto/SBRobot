#include "headers/robot.h"
#include "tim.h"


encoder_t encoder_r;
stepper_t stepper_r;

encoder_t encoder_l;
stepper_t stepper_l;

void Robot_init(){

	encoder_init(&encoder_l, &htim3, &htim7, -1);
	stepper_init(&stepper_l, &htim5, TIM_CHANNEL_1, &encoder_l, GPIOA, GPIO_PIN_4);

	encoder_init(&encoder_r, &htim4, &htim7, 1);
	stepper_init(&stepper_r, &htim2, TIM_CHANNEL_2, &encoder_r, GPIOB, GPIO_PIN_0);


	MAX72_init();
}
