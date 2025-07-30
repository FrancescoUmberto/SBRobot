#include "headers/stepper.h"
#include <stdio.h>

float err = 0;

void speed_control(stepper_t *stepper){
	update_data(stepper->encoder);

	float e = stepper->setpoint_speed - (-stepper->encoder->speed); // SEGNO INVERTITO
	float delta_f = e / ANGLE_STEP ;


	stepper->frequency += delta_f;

	stepper->old_speed = stepper->encoder->speed;

	err = e;

	// period = (1+ARR)*(1+PSC)/HCLK;
	stepper->tim->ARR = (1/stepper->frequency)*HCLK-1;
	stepper->tim->CCR2 = (stepper->tim->ARR+1)/2;
	stepper->tim->EGR = TIM_EGR_UG;
}

void set_speed(stepper_t *stepper, float speed){
	stepper->setpoint_speed = speed;
}

void stepper_init(stepper_t *stepper, TIM_HandleTypeDef *htim, encoder_t *encoder){
	stepper->angle_step = ANGLE_STEP;
	stepper->tim = htim->Instance;
	stepper->tim->CCR2 = 0; // Motori spenti
	stepper->encoder = encoder;
	stepper->setpoint_speed = 0;
	stepper->old_speed = stepper->encoder->speed;
}
