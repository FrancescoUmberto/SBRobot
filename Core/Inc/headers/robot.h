#include "imu.h"
#include "button.h"
#include "stepper.h"
#include "display.h"
#include "power_module.h"
#include "stm32f4xx_hal.h"

typedef struct {
	float Kp, Ki, Kd;
	float base_angle_sp;
	float integral_error, last_error;

	float js_angle_offset_sp, js_angle_offset;

	float js_multiplier, js_multiplier_sp;

	float Kp_speed, Ki_speed, Kd_speed;
	float speed_sp;
	float integral_speed_err, last_speed_err;

	float max_angle_offset, angle_sp;

	uint8_t active;
} pid_t;

void PID_Init(pid_t *pid);
void PID_Update(pid_t *pid);
void PID_Reset(pid_t *pid);

typedef struct {
	imu_t *imu;
	encoder_t *encoder_l;
	encoder_t *encoder_r;
	stepper_t *stepper_l;
	stepper_t *stepper_r;
	power_module_t *power_module;
	pid_t *pid;
	uint8_t base_angle_config;
} robot_t;

extern encoder_t encoder_r;
extern stepper_t stepper_r;

extern encoder_t encoder_l;
extern stepper_t stepper_l;

extern imu_t imu;
extern power_module_t power_module;

extern pid_t pid;

void Robot_init(robot_t *robot);
void Robot_read_serial_msg(robot_t *robot, char *msg);
