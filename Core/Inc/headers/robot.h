#include "imu.h"
#include "button.h"
#include "stepper.h"
#include "display.h"
#include "power_module.h"
#include "stm32f4xx_hal.h"

typedef struct {
	imu_t *imu;
	encoder_t *encoder_l;
	encoder_t *encoder_r;
	stepper_t *stepper_l;
	stepper_t *stepper_r;
	power_module_t *power_module;
} robot_t;

extern encoder_t encoder_r;
extern stepper_t stepper_r;

extern encoder_t encoder_l;
extern stepper_t stepper_l;

extern imu_t imu;
extern power_module_t power_module;

void Robot_init();

void Robot_balancing_control();
