#include "tim.h"
#include "encoder.h"

#define ANGLE_STEP 0.000981748	// MOTOR_STEP / MICRO_STEP		MOTOR_STEP = 1.8°	MICRO_STEP = 32
// M2_PUL PA1
// M2_DIR PB0

typedef struct{
	float angle_step;
	TIM_TypeDef *tim;
	float setpoint_speed;
	encoder_t *encoder;
	float frequency;
} stepper_t;


void set_speed(stepper_t *stepper, float speed);

void speed_control(stepper_t *stepper);

void stepper_init(stepper_t *stepper, TIM_HandleTypeDef *htim, encoder_t *encoder);
