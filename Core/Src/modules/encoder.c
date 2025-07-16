#include "headers/encoder.h"

void update_direction(encoder_t *encoder){
	encoder->direction = (encoder->tim->CR1 & TIM_CR1_DIR_Msk) >> TIM_CR1_DIR_Pos;
	return;
}
