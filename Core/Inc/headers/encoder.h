#include "tim.h"


typedef struct{
	uint8_t direction;
	float displacement;
	uint32_t old_CNT;
	float speed;
	TIM_TypeDef *tim;
} encoder_t;

void encoder_init(encoder_t *encoder, TIM_HandleTypeDef *em_tim, TIM_HandleTypeDef *s_tim);

void update_data(encoder_t *encoder);

extern float SAMPLING_PERIOD;
