#include "headers/robot.h"
#include "tim.h"
#include "adc.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

encoder_t encoder_r;
stepper_t stepper_r;

encoder_t encoder_l;
stepper_t stepper_l;

imu_t imu;
power_module_t power_module;
pid_t pid;

float max_angle_offset = 3.0f;
float alpha = 0.05f;

#define I2C_SCL_GPIO_Port   GPIOB
#define I2C_SCL_Pin         GPIO_PIN_8
#define I2C_SDA_GPIO_Port   GPIOB
#define I2C_SDA_Pin         GPIO_PIN_9

// quando HAL_I2C_Master_Receive_DMA() fallisce per colpa del bus bloccato (es. I2C_FLAG_BUSY sempre attivo),
// l’unico rimedio affidabile è resettare completamente il periferico I²C
static void I2C1_BusRecovery(void) {
    GPIO_InitTypeDef  GPIO_InitStruct = {0};

    // 1) Disabilita I2C
    __HAL_I2C_DISABLE(&hi2c1);
    __HAL_RCC_I2C1_FORCE_RESET();
    __HAL_RCC_I2C1_RELEASE_RESET();

    // 2) Configura SCL e SDA come GPIO open-drain con pull-up interne
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull  = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Pin   = I2C_SCL_Pin | I2C_SDA_Pin;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // 3) Se SDA rimane bassa, genera fino a 9 clock manuali su SCL
    for (int i = 0; i < 9 && HAL_GPIO_ReadPin(I2C_SDA_GPIO_Port, I2C_SDA_Pin) == GPIO_PIN_RESET; i++) {
        HAL_GPIO_WritePin(I2C_SCL_GPIO_Port, I2C_SCL_Pin, GPIO_PIN_RESET);
        HAL_Delay(1);
        HAL_GPIO_WritePin(I2C_SCL_GPIO_Port, I2C_SCL_Pin, GPIO_PIN_SET);
        HAL_Delay(1);
    }

    // 4) Genera un STOP: SDA da bassa → alta mentre SCL alto
    HAL_GPIO_WritePin(I2C_SDA_GPIO_Port, I2C_SDA_Pin, GPIO_PIN_RESET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(I2C_SCL_GPIO_Port, I2C_SCL_Pin, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(I2C_SDA_GPIO_Port, I2C_SDA_Pin, GPIO_PIN_SET);
    HAL_Delay(1);

    // 5) Ripristina AF I2C su SCL e SDA
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;        // pull-up interne già attive
    GPIO_InitStruct.Speed= GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Pin  = I2C_SCL_Pin | I2C_SDA_Pin;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // 6) Ri-inizializza l’I2C
    MX_I2C1_Init();
}


void Robot_init(robot_t *robot) {

	HAL_TIM_Base_Start_IT(&htim6);						// Display timer (0.1MHz)
	HAL_TIM_Base_Start_IT(&htim7);						// Timeline
	HAL_TIM_Base_Start_IT(&htim10);						// Stepper timer
	HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);		// Encoder right
	HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);		// Encoder left
	HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_2);			// Stepper left
	HAL_TIM_PWM_Start(&htim5,TIM_CHANNEL_1);			// Stepper right

	MAX72_init(&display);

	while(!IMU_Init(&imu, &hi2c1, MPU_6050_ADDR)){
		MAX72_Print_String("I2C", NO_SETTINGS);
		I2C1_BusRecovery(); // Attempt to recover I2C bus if IMU init fails
	}
	MAX72_Clear();
    robot->imu = &imu;

	Encoder_init(&encoder_l, &htim3, &htim7, -1);
    robot->encoder_l = &encoder_l;
	Stepper_init(&stepper_l, &htim5, TIM_CHANNEL_1, &encoder_l, GPIOA, GPIO_PIN_4);
    robot->stepper_l = &stepper_l;
	Encoder_init(&encoder_r, &htim4, &htim7, 1);
    robot->encoder_r = &encoder_r;
	Stepper_init(&stepper_r, &htim2, TIM_CHANNEL_2, &encoder_r, GPIOB, GPIO_PIN_0);
    robot->stepper_r = &stepper_r;

	PowerModule_init(&power_module, &hadc1);
	robot->power_module = &power_module;

    PID_Init(&pid);
    robot->pid = &pid;

    robot->base_angle_config = 0;
}



void Robot_read_serial_msg(robot_t *robot, char *msg) {
    static float last_base_angle_stick_val = 0.0f;
    float js_x = 0.0f, js_y = 0.0f;
    uint8_t base_angle_config = 0;

    // Pulizia del buffer: rimuove eventuali spazi/residui
    char clean_msg[14] = {0}; // 13 caratteri + \0
    strncpy(clean_msg, msg, 13);
    clean_msg[14] = '\0';

    // Parsing robusto con sscanf
    int parsed = sscanf(clean_msg, "%f;%f;%hhu", &js_x, &js_y, &base_angle_config);
    if (parsed != 3) {
        // Pacchetto incompleto o malformato, esci
        return;
    }

    // Gestione base angle mode
    if (base_angle_config != robot->base_angle_config) {
        robot->base_angle_config = base_angle_config;
        last_base_angle_stick_val = 0.0f;
        robot->pid->js_angle_offset_sp = 0.0f; // Reset joystick offset when switching modes
        robot->pid->js_angle_offset = 0.0f;
        robot->pid->js_multiplier_sp = 1.0f; // Reset speed multiplier when switching modes
        robot->pid->js_multiplier = 1.0f;

    	static display_data_t base_angle_data = {NULL, PRINT_FLOAT, FLOAT, DISPLAY_TYPE_FLOAT, 2};
    	base_angle_data.data = &robot->pid->base_angle_sp;

        if (base_angle_config) {
			MAX72_Add_Data(&display, &base_angle_data);
			MAX72_Stop_Changing_Data(&display, 0); // Stop changing data to always show base angle
			while (display.data[display.current_index].data != &robot->pid->base_angle_sp) {
				MAX72_Change_Data(&display, 1); // Force change to base angle display
			}
		} else {
			MAX72_Remove_Data(&display, &base_angle_data);
			MAX72_Resume_Changing_Data(&display, 1); // Resume changing data
		}
    }

    if (base_angle_config) {
    	robot->pid->js_multiplier_sp = 1.0f; // Fixed speed multiplier in base angle mode
        if (fabs(js_y) > last_base_angle_stick_val && fabs(js_y) >= 0.1f) {
            robot->pid->base_angle_sp += js_y * 0.02f; // Map joystick Y to base angle setpoint
            if (robot->pid->base_angle_sp > robot->pid->max_angle_offset)
                robot->pid->base_angle_sp = robot->pid->max_angle_offset;
            else if (robot->pid->base_angle_sp < -robot->pid->max_angle_offset)
                robot->pid->base_angle_sp = -robot->pid->max_angle_offset;
        }
        last_base_angle_stick_val = fabs(js_y);
    } else {
    	robot->pid->js_multiplier_sp = js_x > 0.0f ? 1-js_x: -1-js_x; // 0.5 to 1.0 with sign
        robot->pid->js_angle_offset_sp = js_y * max_angle_offset; // Map joystick Y to speed setpoint
    }
}


void PID_Init(pid_t *pid){
	pid->Kp = -2.0f;
	pid->Ki = -20.0f;
	pid->Kd = -0.06f;

	pid->base_angle_sp = 0.75f;

	pid->js_angle_offset_sp = 0.0f;
	pid->js_angle_offset = 0.0f;

	pid->js_multiplier = 1.0f;
	pid->js_multiplier_sp = 1.0f;

    pid->Kp_speed = 0.4f;
    pid->Ki_speed = 0.0f;
    pid->Kd_speed = 0.0008f;

    pid->speed_sp = 0.0f; // Do not change, change via joystick

    pid->max_angle_offset = 2.0f;
    pid->angle_sp = 0.0f; // Do not change, it is only for CubeMonitor

    pid->active = 0;
    PID_Reset(pid);
}

void PID_Update(pid_t *pid) {
    float speed_err = pid->speed_sp - (encoder_r.speed + encoder_l.speed)/2.0f;

    pid->integral_speed_err += speed_err * SAMPLING_PERIOD;
    float derivative_speed_err = (speed_err - pid->last_speed_err)/SAMPLING_PERIOD;

    float angle_offset = pid->Kp_speed * speed_err + pid->Ki_speed * pid->integral_speed_err + pid->Kd_speed * derivative_speed_err;

    if (angle_offset > pid->max_angle_offset) angle_offset = pid->max_angle_offset;
    else if (angle_offset < -pid->max_angle_offset) angle_offset = -pid->max_angle_offset;

    pid->js_angle_offset = alpha * pid->js_angle_offset_sp + (1.0f - alpha) * pid->js_angle_offset;

    pid->angle_sp = pid->base_angle_sp + angle_offset + pid->js_angle_offset;

	float error = pid->angle_sp - imu.angle;

	pid->integral_error += error * SAMPLING_PERIOD;
	float derivative_error = (error - pid->last_error)/SAMPLING_PERIOD;

	float speed_setpoint = pid->Kp * error + 
                            pid->Ki * pid->integral_error +
                            pid->Kd * derivative_error;

	pid->js_multiplier = alpha * pid->js_multiplier_sp + (1.0f - alpha) * pid->js_multiplier;

    if (fabs(error) > 20.0f) {
        set_speed(&stepper_l, 0.0f);
        set_speed(&stepper_r, 0.0f);
        PID_Reset(pid);
    }else {
		if(pid->js_multiplier > 0.9f || pid->js_multiplier < -0.9f){
			set_speed(&stepper_l, speed_setpoint);
			set_speed(&stepper_r, speed_setpoint);
		} else if (pid->js_multiplier > 0.0f) {
			set_speed(&stepper_l, speed_setpoint);
			set_speed(&stepper_r, speed_setpoint * (pid->js_multiplier/2 + (fabs(pid->js_angle_offset) < 0.3f ? 0 : 0.5)));
		} else {
			set_speed(&stepper_l, speed_setpoint * (-pid->js_multiplier/2 + (fabs(pid->js_angle_offset) < 0.3f ? 0 : 0.5)));
			set_speed(&stepper_r, speed_setpoint);
		}
    }

    pid->last_speed_err = speed_err;
	pid->last_error = error;
}

void PID_Reset(pid_t *pid) {
    pid->integral_error = 0.0f;
    pid->integral_speed_err = 0.0f;
    pid->last_error = 0.0f;
    pid->last_speed_err = 0.0f;

    pid->js_angle_offset_sp = 0.0f;
    pid->js_angle_offset = 0.0f;
    pid->js_multiplier_sp = 1.0f;
    pid->js_multiplier = 1.0f;
}
