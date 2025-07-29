#include "headers/stepper.h"
#include <stdio.h>

#define ALPHA .3

void speed_control(stepper_t *stepper){
	static float smooth_setpoint = 0;
	smooth_setpoint = ALPHA*smooth_setpoint+(1-ALPHA)*stepper->setpoint_speed;

	float e = smooth_setpoint + (stepper->encoder->speed/WHEEL_RADIUS); // Segno invertito
	float delta_f = e/ANGLE_STEP;
	float old_f = (stepper->encoder->speed/WHEEL_RADIUS) / ANGLE_STEP;
	float f = delta_f - old_f;

	// period = (1+ARR)*(1+PSC)/HCLK;
	stepper->tim->ARR = (1/f)*HCLK-1;
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
}
