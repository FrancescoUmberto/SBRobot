#include "tim.h"
#include "encoder.h"

#define ANGLE_STEP 0.000981748	// MOTOR_STEP / MICRO_STEP		MOTOR_STEP = 1.8°	MICRO_STEP = 32
// M2_PUL PA1
// M2_DIR PB0

typedef struct{
	float last_error;
	float angle_step;
	float setpoint_speed;
	float frequency;
	encoder_t *encoder;
	TIM_TypeDef *tim;
	uint32_t *CCR;
	uint32_t DIR_PORT;
	uint16_t DIR_PIN;
} stepper_t;

void set_speed(stepper_t *stepper, float speed);

void speed_control(stepper_t *stepper);

void Stepper_init(stepper_t *stepper, TIM_HandleTypeDef *htim, uint32_t tim_channel,
		encoder_t *encoder, GPIO_TypeDef *DIR_PORT, uint16_t DIR_PIN);
