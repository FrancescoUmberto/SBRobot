#include "headers/stepper.h"
#include <stdio.h>

float err = 0;

void speed_control(stepper_t *stepper){
	update_data(stepper->encoder);

	float e = stepper->setpoint_speed - (-stepper->encoder->speed); // SEGNO INVERTITO
	float delta_f = e / ANGLE_STEP ;

	stepper->frequency += delta_f;
	err = e;

	// period = (1+ARR)*(1+PSC)/HCLK;
	stepper->tim->ARR = (1/stepper->frequency)*HCLK-1;
	*stepper->CCR = (stepper->tim->ARR+1)/2;
	stepper->tim->EGR = TIM_EGR_UG;
}

void set_speed(stepper_t *stepper, float speed){
	stepper->setpoint_speed = speed;
}

void stepper_init(stepper_t *stepper, TIM_HandleTypeDef *htim, uint32_t tim_channel, encoder_t *encoder, uint8_t direction_invert){
	stepper->angle_step = ANGLE_STEP;
	stepper->tim = htim->Instance;
	stepper->direction_invert = direction_invert;
	switch (tim_channel){
	case TIM_CHANNEL_1:
		stepper->CCR = &stepper->tim->CCR1;
		break;
	case TIM_CHANNEL_2:
		stepper->CCR = &stepper->tim->CCR2;
		break;
	case TIM_CHANNEL_3:
		stepper->CCR = &stepper->tim->CCR3;
		break;
	case TIM_CHANNEL_4:
		stepper->CCR = &stepper->tim->CCR4;
		break;
	default:
		stepper->CCR = NULL;
		break;
	}

	if (stepper->CCR != NULL)
		*stepper->CCR = 0; // Disattiva il PWM (duty cycle 0)

	stepper->encoder = encoder;
	stepper->setpoint_speed = 0;
}

