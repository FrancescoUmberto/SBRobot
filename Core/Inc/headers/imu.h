#include <stdint.h>
#include "i2c.h"

#define IMU_BUFFER_SIZE 14 			// Size of the buffer for received data
#define MPU_6050_ADDR 0x68 << 1 	// I2C address for IMU
#define IMU_BASE_ACCEL_ADDR 0x3B     // Base address for reading IMU data
#define SLEEP_ADDR 0x6B
#define SMPRT_DIV_ADDR 0x19
#define GYRO_CONFIG_ADDR 0x1B
#define ACCEL_CONFIG_ADDR 0x1C
#define WHO_AM_I_ADDR 0x75 			// Register to check if the IMU is connected
#define CONFIG_ADDR 0x1A 			// Register for configuring the IMU

typedef struct{
	I2C_HandleTypeDef *hi2c;
	uint16_t address;
	volatile uint8_t pData[IMU_BUFFER_SIZE];
	float ax;
	float ay;
	float az;
	float wx;
	float wy;
	float wz;

	float angle;
	uint32_t last_computation_time;

	float az_bias; // Bias for accelerometer z-axis
	uint8_t calibration_mode; // 1 for calibration mode, 0 for normal operation
} imu_t;


uint8_t IMU_Init(imu_t *imu, I2C_HandleTypeDef *hi2c, uint16_t address);

void IMU_ReadData(imu_t *imu);

void IMU_Compute_Data(imu_t *imu);




