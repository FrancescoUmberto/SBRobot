#include "headers/imu.h"
#include <math.h>

// #define IMU_EMA_ALPHA 0.03921568627f		// Alpha = 2 / (1 + 50)	EMA Filter coefficient for 50 samples window
// #define IMU_EMA_ALPHA 0.08695652174		// Alpha = 2 / (1 + 22)	EMA Filter coefficient for 22 samples window
#define IMU_EMA_ALPHA 0.1538461538f	// Alpha = 2 / (1 + 12)	EMA Filter coefficient for 12 samples window

static void IMU_Calibrate(imu_t *imu) {
	imu->az = imu->az * (1-IMU_EMA_ALPHA) + ((int16_t)(imu->pData[4] << 8) | imu->pData[5]) * IMU_EMA_ALPHA;

	if (imu->az > imu->az_bias){
		imu->az_bias = imu->az_bias * (1-IMU_EMA_ALPHA) + imu->az * IMU_EMA_ALPHA; // Update az bias with EMA filter
		imu->angle = 0.0f; // Reset angle to 0 during calibration
	}
	float delta_az = imu->az_bias - imu->az; // Calculate the change in az value

	if(imu->calibration_mode == 1 && delta_az > 500){ // Pitch forward calibration step completed
		imu->calibration_mode++;
		imu->az_bias = imu->az; // Set az bias to the current az value
	} else if(imu->calibration_mode == 2 && delta_az > 300){ // Pitch backward calibration step completed (rising again)
		imu->calibration_mode = 0; // Reset calibration mode
		imu->az_bias = imu->az_bias - 16384.0f; // Adjust az bias to remove the offset
		imu->az= (imu->az - imu->az_bias) / 16384.0f * 9.81f; // Remove the bias from the az value
	}
}

uint8_t IMU_Init(imu_t *imu, I2C_HandleTypeDef *hi2c, uint16_t address) {
	if (__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_BUSY) != RESET) {
		// I2C bus is busy, stop the I2C communication
		return 0;
	}

	imu->hi2c = hi2c;
	imu->address = address;

	imu->ax = 0.0f;
	imu->ay = 0.0f;
	imu->az = 0.0f;
	imu->vx = 0.0f;

	imu->wx = 0.0f;
	imu->wy = 0.0f;
	imu->wz = 0.0f;
	imu->alpha_y = 0.0f; // Initialize alpha_y to 0

	imu->angle = 0.0f;
	imu->last_computation_time = 0;

	imu->az_bias = 0.0f; // Initialize az bias to 0
	imu->calibration_mode = 1; // Calibration mode enabled by default

	uint8_t check;
	HAL_I2C_Mem_Read(hi2c, address, WHO_AM_I_ADDR, I2C_MEMADD_SIZE_8BIT, &check, 1, 1000);
	if (check == 0x68) {
		uint8_t data = 0x00;
		HAL_I2C_Mem_Write(hi2c, address, SLEEP_ADDR, I2C_MEMADD_SIZE_8BIT, &data, 1, 1000);

		data = 0x00;
		HAL_I2C_Mem_Write(hi2c, address, CONFIG_ADDR, I2C_MEMADD_SIZE_8BIT, &data, 1, 1000);
	}
	return 1;
}

void IMU_ReadData(imu_t *imu){
	HAL_I2C_Mem_Read_DMA(imu->hi2c, imu->address, IMU_BASE_ACCEL_ADDR, I2C_MEMADD_SIZE_8BIT, (uint8_t *)imu->pData, IMU_BUFFER_SIZE);
}

void IMU_Compute_Data(imu_t *imu) {
	uint32_t delta_time = HAL_GetTick() - imu->last_computation_time; // Calculate time since last angle update

	imu->ax = (1-IMU_EMA_ALPHA) * imu->ax + IMU_EMA_ALPHA * (((int16_t)(imu->pData[0] << 8) | imu->pData[1]) - 280.591f) / 16384.0f * 9.81f;
	imu->ay = (1-IMU_EMA_ALPHA) * imu->ay + IMU_EMA_ALPHA * ((int16_t)(imu->pData[2] << 8) | imu->pData[3]) / 16384.0f * 9.81f;
	imu->az = (1-IMU_EMA_ALPHA) * imu->az + IMU_EMA_ALPHA * (((int16_t)(imu->pData[4] << 8) | imu->pData[5]) - 1117.735f) / 16384.0f * 9.81f;
//	if (imu->calibration_mode) {
//		IMU_Calibrate(imu); // Call calibration function if in calibration mode
//	} else {
//		// Convert raw data to float values (/16834.0f for accelerometer -> in g, /131.0f for gyroscope)
//		imu->az = (1-IMU_EMA_ALPHA) * imu->az + IMU_EMA_ALPHA * (((int16_t)(imu->pData[4] << 8) | imu->pData[5]) - imu->az_bias)/ 16384.0f * 9.81f;
//	}
	float old_wy = imu->wy;
	imu->wx = (1-IMU_EMA_ALPHA) * imu->wx + IMU_EMA_ALPHA * ((int16_t)(imu->pData[8] << 8) | imu->pData[9]) / 131.0f;
	imu->wy = (1-IMU_EMA_ALPHA) * imu->wy + IMU_EMA_ALPHA * (((int16_t)(imu->pData[10] << 8) | imu->pData[11]) - 393.589f) / 131.0f;
	imu->wz = (1-IMU_EMA_ALPHA) * imu->wz + IMU_EMA_ALPHA * ((int16_t)(imu->pData[12] << 8) | imu->pData[13]) / 131.0f;

	imu->last_computation_time = HAL_GetTick(); // Update last computation time

	imu->vx = imu->vx + imu->ax * (float)delta_time / 1000.0f; // Update velocity based on accelerometer data
	imu->alpha_y = (imu->wy - old_wy) / ((float)delta_time / 1000.0f); // Calculate angular acceleration around y-axis

	if(imu->calibration_mode) {
		imu->angle = -atan2f(imu->ax, imu->az) * 180.0f / M_PI; // Use accelerometer data to compute angle in calibration mode
		imu->calibration_mode = 0; // Reset calibration mode after computing angle
	}else {
		imu->angle = .996f * (imu->angle + imu->wy * (float)delta_time/1000.0f) - .004f * atan2f(imu->ax, imu->az) * 180.0f / M_PI; // Complementary filter to combine gyroscope and accelerometer data
	}
}
