#include "imu.h"
#include "button.h"
#include "stepper.h"
#include "display.h"
#include "power_module.h"
#include "stm32f4xx_hal.h"

#define I2C_SCL_GPIO_Port GPIOB
#define I2C_SCL_Pin GPIO_PIN_8
#define I2C_SDA_GPIO_Port GPIOB
#define I2C_SDA_Pin GPIO_PIN_9

#define TILT_ANGLE_LIMIT 30.0f // degrees

#define WHEEL_AXIS_MIDPOINT 132.5 // mm

#define FLASH_START_ADDR   0x08060000  // Start address of the flash sector to store base angle

typedef struct {
	float Kp, Ki, Kd;
	float base_angle_sp;
	float integral_error, last_error;

	float js_speed_sp, js_speed;

	float js_multiplier, js_multiplier_sp;

	float Kp_speed, Kd_speed;
	float speed_sp;
	float last_speed_err;

	float max_angle_offset, angle_sp, max_speed;

	uint8_t active;

	uint8_t base_angle_config;
} pid_t;

void PID_Init(pid_t *pid);
void PID_Update(pid_t *pid);
void PID_Reset(pid_t *pid);

void PID_ReadSerialMsg(pid_t *pid, char *msg);

void PID_SaveBaseAngle(pid_t *pid);

typedef struct {
	imu_t *imu;
	encoder_t *encoder_l;
	encoder_t *encoder_r;
	stepper_t *stepper_l;
	stepper_t *stepper_r;
	power_module_t *power_module;
	pid_t *pid;
} robot_t;

extern encoder_t encoder_r;
extern stepper_t stepper_r;

extern encoder_t encoder_l;
extern stepper_t stepper_l;

extern imu_t imu;
extern power_module_t power_module;

extern pid_t pid;

void Robot_Init(robot_t *robot);