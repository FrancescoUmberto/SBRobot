
#include "imu.h"
#include "encoder.h"
#include "stepper.h"
#include "display.h"

typedef struct {
	imu_t *imu; // 5060
	encoder_t *encoder_l; // AMT10
	encoder_t *encoder_r; // AMT10
	stepper_t *stepper_l;
	stepper_t *stepper_r;
} robot_t;

extern encoder_t encoder2;
extern stepper_t stepper2;

void Robot_init();
