#include "headers/robot.h"
#include "tim.h"
#include "adc.h"
#include <math.h>
#include <string.h>
#include <stdio.h>
#include "stm32f4xx_hal.h"

#define FLASH_START_ADDR   0x08060000

encoder_t encoder_r;
stepper_t stepper_r;

encoder_t encoder_l;
stepper_t stepper_l;

imu_t imu;
power_module_t power_module;
pid_t pid;

float alpha = 0.05f;
float js_x = 0.0f, js_y = 0.0f;

#define I2C_SCL_GPIO_Port GPIOB
#define I2C_SCL_Pin GPIO_PIN_8
#define I2C_SDA_GPIO_Port GPIOB
#define I2C_SDA_Pin GPIO_PIN_9
#define WHEEL_AXIS_MIDPOINT 132.5 // mm

// quando HAL_I2C_Master_Receive_DMA() fallisce per colpa del bus bloccato (es. I2C_FLAG_BUSY sempre attivo),
// l’unico rimedio affidabile è resettare completamente il periferico I²C
static void I2C1_BusRecovery(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // 1) Disabilita I2C
    __HAL_I2C_DISABLE(&hi2c1);
    __HAL_RCC_I2C1_FORCE_RESET();
    __HAL_RCC_I2C1_RELEASE_RESET();

    // 2) Configura SCL e SDA come GPIO open-drain con pull-up interne
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Pin = I2C_SCL_Pin | I2C_SDA_Pin;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // 3) Se SDA rimane bassa, genera fino a 9 clock manuali su SCL
    for (int i = 0; i < 9 && HAL_GPIO_ReadPin(I2C_SDA_GPIO_Port, I2C_SDA_Pin) == GPIO_PIN_RESET; i++)
    {
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
    GPIO_InitStruct.Pull = GPIO_NOPULL; // pull-up interne già attive
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Pin = I2C_SCL_Pin | I2C_SDA_Pin;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // 6) Ri-inizializza l’I2C
    MX_I2C1_Init();
}

void Robot_init(robot_t *robot)
{

    HAL_TIM_Base_Start_IT(&htim6);                     // Display timer (0.1MHz)
    HAL_TIM_Base_Start_IT(&htim7);                     // Timeline
    HAL_TIM_Base_Start_IT(&htim10);                    // Stepper timer
    HAL_TIM_Encoder_Start_IT(&htim3, TIM_CHANNEL_ALL); // Encoder right
    HAL_TIM_OC_Start_IT(&htim3, TIM_CHANNEL_3);
    HAL_TIM_Encoder_Start_IT(&htim4, TIM_CHANNEL_ALL); // Encoder left
    HAL_TIM_OC_Start_IT(&htim4, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2); // Stepper left
    HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_1); // Stepper right

    HAL_TIM_Base_Start(&htim8); // Virtual timer overflow timer
    HAL_TIM_Base_Start(&htim1); // Microsecond timer

    MAX72_init(&display);

    while (!IMU_Init(&imu, &hi2c1, MPU_6050_ADDR))
    {
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
}

static float Load_BaseAngleOrDefault(void) {
    uint32_t raw = *(uint32_t*)FLASH_START_ADDR;

    // se il valore non è mai stato scritto, Flash contiene 0xFFFFFFFF
    if(raw == 0xFFFFFFFF) {
        return 1.0f; // default
    } else {
        float angle;
        memcpy(&angle, &raw, sizeof(float));
        return angle;
    }
}

void Save_BaseAngle(pid_t *pid) {
    HAL_FLASH_Unlock();

    // Cancella la pagina prima di scrivere
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t SectorError;

    EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3;
    EraseInitStruct.Sector        = FLASH_SECTOR_7;
    EraseInitStruct.NbSectors     = 1;

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK) {
        // gestione errore
    }

    // Scrivi il float reinterpretando i bit come uint32_t
    uint32_t data;
    memcpy(&data, &pid->base_angle_sp, sizeof(float));

    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_START_ADDR, data) != HAL_OK) {
        // gestione errore
    }

    HAL_FLASH_Lock();
}

void Robot_read_serial_msg(robot_t *robot, char *msg)
{
	pid_t *pid = robot->pid;
    static float last_base_angle_stick_val = 0.0f;
    uint8_t base_angle_config = 0;

    // Pulizia del buffer: rimuove eventuali spazi/residui
    char clean_msg[14] = {0}; // 13 caratteri + \0
    strncpy(clean_msg, msg, 13);
    clean_msg[13] = '\0';

    // Parsing robusto con sscanf
    int parsed = sscanf(clean_msg, "%f;%f;%hhu", &js_x, &js_y, &base_angle_config);
    if (parsed != 3)
    {
        // Pacchetto incompleto o malformato, esci
        return;
    }

    // Gestione base angle mode
    if (base_angle_config != pid->base_angle_config)
    {
        pid->base_angle_config = base_angle_config;
        last_base_angle_stick_val = 0.0f;
        pid->js_speed_sp = 0.0f; // Reset joystick offset when switching modes
        pid->js_speed = 0.0f;
        pid->js_multiplier_sp = 1.0f; // Reset speed multiplier when switching modes
        pid->js_multiplier = 1.0f;

        static display_data_t base_angle_data = {NULL, PRINT_FLOAT, FLOAT, DISPLAY_TYPE_FLOAT, 2};
        base_angle_data.data = &pid->base_angle_sp;

        if (base_angle_config)
        {
            MAX72_Add_Data(&display, &base_angle_data);
            MAX72_Stop_Changing_Data(&display, 0); // Stop changing data to always show base angle
            while (display.data[display.current_index].data != &pid->base_angle_sp)
            {
                MAX72_Change_Data(&display, 1); // Force change to base angle display
            }
        }
        else
        {
            MAX72_Remove_Data(&display, &base_angle_data);
            MAX72_Resume_Changing_Data(&display, 1); // Resume changing data
        }
    }

    if (base_angle_config)
    {
        pid->js_multiplier_sp = 1.0f; // Fixed speed multiplier in base angle mode
        if (fabs(js_y) > last_base_angle_stick_val && fabs(js_y) >= 0.1f)
        {
            pid->base_angle_sp += js_y * 0.02f; // Map joystick Y to base angle setpoint
            if (pid->base_angle_sp > pid->max_angle_offset)
                pid->base_angle_sp = pid->max_angle_offset;
            else if (pid->base_angle_sp < -pid->max_angle_offset)
                pid->base_angle_sp = -pid->max_angle_offset;
        }
        last_base_angle_stick_val = fabs(js_y);
    }
    else
    {
        pid->js_multiplier_sp = js_x > 0.0f ? 1 - js_x : -1 - js_x; // Map joystick X to speed multiplier setpoint
        pid->js_speed_sp = js_y * ((js_y > 0)/3.0f + 1.0f ) * pid->max_speed;          // Map joystick Y to speed setpoint
    }
}

void PID_Init(pid_t *pid)
{
    pid->Kp = -0.85f;
    pid->Ki = -4.8f;
    pid->Kd = -0.004f;

    pid->base_angle_sp = 0.1f;//Load_BaseAngleOrDefault();

    pid->js_speed_sp = 0.0f;
    pid->js_speed = 0.0f;

    pid->js_multiplier = 1.0f;
    pid->js_multiplier_sp = 1.0f;

    pid->Kp_speed = 0.73f;
	pid->Ki_speed = 0.0f;
	pid->Kd_speed = 0.0045f;

    pid->speed_sp = 0.0f; // Do not change, change via joystick

    pid->max_angle_offset = 2.0f;
    pid->max_speed = 5.0f; // r/s

    pid->angle_sp = 0.0f; // Do not change, it is only for CubeMonitor

    pid->active = 0;
    pid->base_angle_config = 0;
    PID_Reset(pid);
}

static void PID_Angle_Update(pid_t *pid)
{
    pid->speed_sp = alpha * pid->js_speed_sp + (1.0f - alpha) * pid->speed_sp;

    float speed_err = pid->speed_sp - (encoder_r.speed + encoder_l.speed) / 2.0f;

    pid->integral_speed_err += speed_err * SAMPLING_PERIOD;
    float derivative_speed_err = (speed_err - pid->last_speed_err) / SAMPLING_PERIOD;

    float angle_offset = pid->Kp_speed * speed_err + pid->Ki_speed * pid->integral_speed_err + pid->Kd_speed * derivative_speed_err;

    if (angle_offset > pid->max_angle_offset)
        angle_offset = pid->max_angle_offset;
    else if (angle_offset < -pid->max_angle_offset)
        angle_offset = -pid->max_angle_offset;

    // pid->js_speed = alpha * pid->js_angle_offset_sp + (1.0f - alpha) * pid->js_speed;

    pid->angle_sp = pid->base_angle_sp + angle_offset /*+ pid->js_speed*/;

    pid->last_speed_err = speed_err;
}

static void Differential_Drive_Kinematics(pid_t *pid, float speed_setpoint)
{
    /*
    Differential Drive Kinematics

    Distance from the midpoint of the wheels axis to the ICC
    R = (l/2) * ((V_l + V_r)/(V_r - V_l)) 

    omega = (V_r - V_l)/l must be the same for both wheels

    V_r = omega*(R + l/2)
    V_l = omega*(R - l/2)

    Forward Kinematics:
    - Instantaneous Center of Curvature (ICC): 
        ICC = [x - R * sin(theta), y + R * cos(theta)]
    - At the time t+dt the pose will be:
        [x', y', theta'] =
            [cos(omega*dt), -sin(omega*dt), 0; sin(omega*dt), cos(omega*dt), 0; 0, 0, 1] *
            [x - ICC_x, y - ICC_y, theta] + [ICC_x, ICC_y, omega*dt]
    */

	float l = WHEEL_AXIS_MIDPOINT;
	float dt = SAMPLING_PERIOD;

	float V = js_y * pid->js_multiplier;
	float omega = -js_x * 0.05f;

	float V_r, V_l;

	if (fabsf(omega) < 1e-6f) {
		// Solo avanzamento
		V_r = V;
		V_l = V;
	}
	else if (fabsf(V) < 1e-6f) {
		// Solo rotazione
		V_r =  omega * (l / 2.0f);
		V_l = -omega * (l / 2.0f);
	}
	else {
        omega *= (js_y < -0.05f ? -1.0f : 1.0f); // Inverti il verso di rotazione se si va indietro
        
		// Avanzamento + rotazione
		float R = V / omega;
		// controllo che R non sia assurdo
		if (fabsf(R) > 1e6f || isnan(R) || isinf(R)) {
			V_r = V;
			V_l = V;
		} else {
			V_r = omega * (R + l / 2.0f);
			V_l = omega * (R - l / 2.0f);
		}
	}

	float V_l_cmd = V_l + speed_setpoint;
	float V_r_cmd = V_r + speed_setpoint;
	// protezione contro valori non validi
	if (isnan(V_l_cmd) || isinf(V_l_cmd)) V_l_cmd = 0.0f;
	if (isnan(V_r_cmd) || isinf(V_r_cmd)) V_r_cmd = 0.0f;

	// saturazione con limite hardware
	if (V_l_cmd >  MAX_CTRL_FREQUENCY) V_l_cmd =  MAX_CTRL_FREQUENCY;
	if (V_l_cmd < -MAX_CTRL_FREQUENCY) V_l_cmd = -MAX_CTRL_FREQUENCY;
	if (V_r_cmd >  MAX_CTRL_FREQUENCY) V_r_cmd =  MAX_CTRL_FREQUENCY;
	if (V_r_cmd < -MAX_CTRL_FREQUENCY) V_r_cmd = -MAX_CTRL_FREQUENCY;

	set_speed(&stepper_l, V_l_cmd);
	set_speed(&stepper_r, V_r_cmd);
}

static void PID_Speed_Update(pid_t *pid)
{
    float error = pid->angle_sp - imu.angle;

    if (fabs(error) > 30.0f) {
        set_speed(&stepper_l, 0.0f);
        set_speed(&stepper_r, 0.0f);
        PID_Reset(pid);
    } else {
        pid->integral_error += error * SAMPLING_PERIOD;
        float derivative_error = (error - pid->last_error) / SAMPLING_PERIOD;

        float speed_setpoint = pid->Kp * error +
                               pid->Ki * pid->integral_error +
                               pid->Kd * derivative_error;

        pid->js_multiplier = alpha * pid->js_multiplier_sp + (1.0f - alpha) * pid->js_multiplier;

		if (pid->js_multiplier > 0.95f) {
			set_speed(&stepper_l, speed_setpoint);
			set_speed(&stepper_r, speed_setpoint);
		} else {
			Differential_Drive_Kinematics(pid, speed_setpoint);
		}
    }
    pid->last_error = error;
}

static void PID_Set_Param(pid_t *pid){
	if (fabs(pid->js_speed) > 0.5f && !pid->base_angle_config) {
		pid->Kp = -0.3f;
		pid->Ki = -4.8f;
		pid->Kd = -0.004f;

		pid->Kp_speed = 4.0f;
		pid->Ki_speed = 0.0f;
		pid->Kd_speed = 0.5f;
	} else {
		pid->Kp = -0.85f;
		pid->Ki = -4.8f;
		pid->Kd = -0.004f;

		pid->Kp_speed = 0.73f;
		pid->Ki_speed = 0.0f;
		pid->Kd_speed = 0.0045f;
	}
}

void PID_Update(pid_t *pid)
{
	PID_Set_Param(pid);
    PID_Angle_Update(pid);
    PID_Speed_Update(pid);
}

void PID_Reset(pid_t *pid)
{
    pid->integral_error = 0.0f;
    pid->integral_speed_err = 0.0f;
    pid->last_error = 0.0f;
    pid->last_speed_err = 0.0f;

    pid->js_speed_sp = 0.0f;
    pid->js_speed = 0.0f;
    pid->js_multiplier_sp = 1.0f;
    pid->js_multiplier = 1.0f;
}


