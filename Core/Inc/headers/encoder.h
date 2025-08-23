#include "tim.h"

#define RCF 0.00076699 // Radians Conversion Factor	2*pi/PPR		PPR=CPR*RES		2048*4=8192
#define WHEEL_RADIUS 0.0625 // meters

typedef struct{
	uint8_t direction;
	float displacement;
	float speed;
	TIM_TypeDef *tim;
	int8_t direction_invert;
	float position;
} encoder_t;

void Encoder_init(encoder_t *encoder, TIM_HandleTypeDef *em_tim, TIM_HandleTypeDef *s_tim, int8_t direction_invert);

void Encoder_read(encoder_t *encoder);

extern float SAMPLING_PERIOD;
extern uint32_t HCLK;
