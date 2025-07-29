#define ANGLE_STEP 0.5625	// MOTOR_STEP / MICRO_STEP		MOTOR_STEP = 1.8°	MICRO_STEP = 32

typedef struct{
	float angle_step;
	int b;
} stepper_t;

void speed_control(stepper_t *stepper, float speed);

void stepper_init(stepper_t *stepper);
