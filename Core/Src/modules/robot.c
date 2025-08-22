#include "headers/robot.h"
#include "tim.h"
#include "adc.h"
#include <math.h>

encoder_t encoder_r;
stepper_t stepper_r;

encoder_t encoder_l;
stepper_t stepper_l;

imu_t imu;
power_module_t power_module;
pid_t pid;

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

    PID_Init(&pid);
}

float integral_error = 0.0f;
float last_error = 0.0f;
float derivative_error = 0.0f;
float last_speed_error = 0.0f;
float derivative_speed_error = 0.0f;

void PID_Init(pid_t *pid){
	pid->Kp = -3.1f;
	pid->Ki = -0.08f;
	pid->Kd = -0.011f;
    pid->Vp = 0.0f;
    pid->Vi = 0.0f;
    pid->Vd = 0.0f;
	pid->angle_setpoint = -0.5f;
    pid->speed_setpoint = 0.0f;
}

void PID_Update(pid_t *pid) {
	float error = pid->angle_setpoint - imu.angle;
    float speed_error = pid->speed_setpoint - imu.wy; // Use angular velocity for speed error

    // Stop the motors if imu.angle is greater than 10 degrees
    if (fabs(error) > 20.0f) {
        set_speed(&stepper_l, 0.0f);
        set_speed(&stepper_r, 0.0f);
        return;
    }
	integral_error += error;
	derivative_error = (error - last_error)/SAMPLING_PERIOD;
	derivative_speed_error = (speed_error - last_speed_error)/SAMPLING_PERIOD;

	float propotional_component = pid->Kp * error;

	float integral_component = pid->Ki * integral_error;

	float derivative_component = pid->Kd * derivative_error;
//
//	float vp_component = pid->Vp * speed_error * (1<= .5/fabs(imu.angle) ? 1 : .5/fabs(imu.angle));
//
//	float vd_component = pid->Vd * derivative_speed_error;

	float speed_setpoint = propotional_component + integral_component + derivative_component; // + vp_component + vd_component;

	set_speed(&stepper_l, speed_setpoint);
    set_speed(&stepper_r, speed_setpoint);

	last_error = error;
	last_speed_error = speed_error;
}
