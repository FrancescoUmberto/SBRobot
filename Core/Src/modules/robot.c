#include "headers/robot.h"
#include "tim.h"


encoder_t encoder_r;
stepper_t stepper_r;

encoder_t encoder_l;
stepper_t stepper_l;

void Robot_init(){

	encoder_init(&encoder_r, &htim3, &htim7);
	stepper_init(&stepper_r, &htim5, TIM_CHANNEL_1, &encoder_r, -1);

	encoder_init(&encoder_l, &htim4, &htim7);
	stepper_init(&stepper_l, &htim2, TIM_CHANNEL_2, &encoder_l, 1);


	MAX72_init();
}
