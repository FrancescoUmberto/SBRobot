#include "headers/encoder.h"
#include <stdlib.h>
#include <stdio.h>

#define DCF 0.00076699 // Degree Conversion Factor	2*pi/PPR		PPR=CPR*RES		2048*4=8192
#define WHEEL_RADIUS 0.0625 // meters

float SAMPLING_PERIOD; // seconds

static void update_direction(encoder_t *encoder){
	encoder->direction = (encoder->tim->CR1 & TIM_CR1_DIR_Msk) >> TIM_CR1_DIR_Pos;
	return;
}

static void update_displacement(encoder_t *encoder){
	int delta_CNT = (int)encoder->tim->CNT - (int)encoder->old_CNT;
	if (abs(delta_CNT) > 7000) {
		delta_CNT = encoder->tim->CNT + encoder->old_CNT - 8190;
	}
	encoder->displacement = (float)delta_CNT * DCF * WHEEL_RADIUS;
	encoder->old_CNT = encoder->tim->CNT;
	return;
}

static void compute_speed(encoder_t *encoder){
	encoder->speed = encoder->displacement / SAMPLING_PERIOD;
}

void update_data(encoder_t *encoder){
	update_direction(encoder);
	update_displacement(encoder);
	compute_speed(encoder);
	return;
}

// em_tim : encoder mode timer | s_tim : sampling timer
void encoder_init(encoder_t *encoder, TIM_HandleTypeDef *em_tim, TIM_HandleTypeDef *s_tim){
	encoder->tim = em_tim->Instance;
	encoder->tim->CNT = 4096;

	SAMPLING_PERIOD = (float)(1+s_tim->Instance->ARR)*(1+s_tim->Instance->PSC)/HAL_RCC_GetHCLKFreq();
}
