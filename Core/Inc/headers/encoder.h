#include "tim.h"
#include "arm_math.h" // CMSIS DSP Library

#define RCF 0.00076699 // Radians Conversion Factor	2*pi/PPR		PPR=CPR*RES		2048*4=8192
#define WHEEL_RADIUS 0.0625 // meters

#define N_SAMPLES 6
#define POLY_ORDER 2
#define N_COEFF (POLY_ORDER + 1)

typedef struct{
	uint8_t direction;
	float displacement;
	float speed;
	TIM_TypeDef *tim;
	int8_t direction_invert;
	float32_t position;
	uint32_t timestamps[N_SAMPLES];
	float32_t positions[N_SAMPLES];
	uint8_t vec_index;
	float polynomial[N_COEFF];
	int32_t t_ref;
	float32_t old_displacement;
} encoder_t;

void Encoder_init(encoder_t *encoder, TIM_HandleTypeDef *em_tim, TIM_HandleTypeDef *s_tim, int8_t direction_invert);

void Encoder_read(encoder_t *encoder);

void Encoder_event(encoder_t *encoder);

extern float SAMPLING_PERIOD;
extern uint32_t HCLK;
