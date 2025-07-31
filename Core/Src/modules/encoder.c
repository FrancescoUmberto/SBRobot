#include "headers/encoder.h"
#include <stdlib.h>
#include <stdio.h>

float SAMPLING_PERIOD; // seconds
uint32_t HCLK;

static void update_direction(encoder_t *encoder){
	encoder->direction = (encoder->tim->CR1 & TIM_CR1_DIR_Msk) >> TIM_CR1_DIR_Pos;
	return;
}

static void update_displacement(encoder_t *encoder){

	encoder->displacement = ((float)encoder->tim->CNT - 4096) * DCF * encoder->direction_invert;
	encoder->tim->CNT = 4096;
	return;
}

static void compute_speed(encoder_t *encoder){
	encoder->speed = encoder->displacement / SAMPLING_PERIOD;
}

void Encoder_read(encoder_t *encoder){
	update_direction(encoder);
	update_displacement(encoder);
	compute_speed(encoder);
	return;
}

// em_tim : encoder mode timer | s_tim : sampling timer
void Encoder_init(encoder_t *encoder, TIM_HandleTypeDef *em_tim, TIM_HandleTypeDef *s_tim, int8_t direction_invert){
	encoder->tim = em_tim->Instance;
	encoder->tim->CNT = 4096;
	encoder->direction_invert = direction_invert;

	encoder->speed = 0;
	HCLK = HAL_RCC_GetHCLKFreq();
	SAMPLING_PERIOD = (float)(1+s_tim->Instance->ARR)*(1+s_tim->Instance->PSC)/HCLK;
}
