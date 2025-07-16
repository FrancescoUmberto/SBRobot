#include "tim.h"


typedef struct{
	uint8_t direction;
	uint32_t displacement;
	TIM_TypeDef *tim;
} encoder_t;


uint32_t get_speed(); // needs to be defined

uint32_t get_displacement();

void update_direction(encoder_t *encoder);
