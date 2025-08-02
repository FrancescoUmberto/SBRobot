#include "headers/imu.h"

#define IMU_EMA_ALPHA 0.04762		// Alpha = 1 / (1 + 20)

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
	imu->wx = 0.0f;
	imu->wy = 0.0f;
	imu->wz = 0.0f;

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
	// Convert raw data to float values (/16834.0f for accelerometer -> in g, /131.0f for gyroscope)
	imu->ax = imu->ax * (1-IMU_EMA_ALPHA) + ((imu->pData[0] << 8) | imu->pData[1]) / 16384.0f * 9.81f * IMU_EMA_ALPHA;
	imu->ay = imu->ay * (1-IMU_EMA_ALPHA) + ((imu->pData[2] << 8) | imu->pData[3]) / 16384.0f * 9.81f * IMU_EMA_ALPHA;
	imu->az = imu->az * (1-IMU_EMA_ALPHA) + ((imu->pData[4] << 8) | imu->pData[5])/ 16384.0f * 9.81f*IMU_EMA_ALPHA;
	imu->wx = imu->wx * (1-IMU_EMA_ALPHA) + ((imu->pData[8] << 8) | imu->pData[9]) / 131.0f * IMU_EMA_ALPHA;
	imu->wy = imu->wy * (1-IMU_EMA_ALPHA) + ((imu->pData[10] << 8) | imu->pData[11]) / 131.0f * IMU_EMA_ALPHA;
	imu->wz = imu->wz * (1-IMU_EMA_ALPHA) + ((imu->pData[12] << 8) | imu->pData[13]) / 131.0f * IMU_EMA_ALPHA;
}
