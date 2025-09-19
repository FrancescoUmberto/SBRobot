#include <stdint.h>
#include "stm32f4xx_hal.h"

#define WARNING_LIMIT 22 // Maximum acceptable battery voltage
#define STOP_LIMIT 20 // Minimum acceptable battery voltage
#define SLOPE 114.07
#define OFFSET 0.06708161655124
#define ACCUMULATOR_SIZE 20 // Number of samples in the averaging buffer

typedef struct {
  float voltage; // Current voltage reading
  uint8_t alert_issued; // Flag to indicate if an alert has been issued
  float accumulator[ACCUMULATOR_SIZE]; // Array for storing voltage samples
  uint8_t accumulator_idx; // Current index in the accumulator
  ADC_HandleTypeDef *hadc; // Handle for ADC
} power_module_t;

void PowerModule_Init(power_module_t *power_module, ADC_HandleTypeDef *hadc);

void PowerModule_ReadData(power_module_t *power_module);
void PowerModule_UpdateData(power_module_t *power_module);
