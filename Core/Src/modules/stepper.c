#include "headers/stepper.h"
#include <stdio.h>
#include <math.h>

float err = 0;
float freq = 0;

static const float constant = 1020*0.05/2;

void speed_control(stepper_t *stepper){
	Encoder_read(stepper->encoder);

	float e = stepper->setpoint_speed - stepper->encoder->speed;
//	float delta_f = e / ANGLE_STEP ;
//
//	stepper->frequency += delta_f;

	stepper->frequency += 350*e + constant*(e+err);
	uint8_t sign = stepper->frequency > 0;

	freq = stepper->frequency;

	HAL_GPIO_WritePin(stepper->DIR_PORT, stepper->DIR_PIN, (stepper->encoder->direction_invert > 0 ? sign : !sign));

	err = e;

	// period = (1+ARR)*(1+PSC)/HCLK;
	stepper->tim->ARR = fabs(1/stepper->frequency)*HCLK-1;
	*stepper->CCR = (stepper->tim->ARR+1)/2;
	stepper->tim->EGR = TIM_EGR_UG;
}

void set_speed(stepper_t *stepper, float speed){
	stepper->setpoint_speed = speed;
}

void Stepper_init(stepper_t *stepper, TIM_HandleTypeDef *htim, uint32_t tim_channel,
		encoder_t *encoder, GPIO_TypeDef *DIR_PORT, uint16_t DIR_PIN){
	stepper->angle_step = ANGLE_STEP;
	stepper->tim = htim->Instance;
	stepper->DIR_PORT = DIR_PORT;
	stepper->DIR_PIN = DIR_PIN;
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

