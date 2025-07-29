#include "tim.h"

#define DCF 0.00076699 // Degree Conversion Factor	2*pi/PPR		PPR=CPR*RES		2048*4=8192
#define WHEEL_RADIUS 0.0625 // meters

typedef struct{
	uint8_t direction;
	float displacement;
	float speed;
	TIM_TypeDef *tim;
} encoder_t;

void encoder_init(encoder_t *encoder, TIM_HandleTypeDef *em_tim, TIM_HandleTypeDef *s_tim);

void update_data(encoder_t *encoder);

extern float SAMPLING_PERIOD;
extern uint32_t HCLK;
