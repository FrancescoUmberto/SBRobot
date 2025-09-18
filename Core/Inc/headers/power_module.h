#include <stdint.h>
#include "stm32f4xx_hal.h"

#define WARNING_LIMIT 22 // Max value acceptable for battery's voltage
#define STOP_LIMIT 20 // Min value acceptable for battery's voltage
#define SLOPE 114.07
#define OFFSET 0.06708161655124
#define ACCUMULATOR_SIZE 20 // Size of the accumulator for averaging

typedef struct {
  float voltage;
  float warning_limit;
  float stop_limit;
  uint8_t warning_issued; // Flag to indicate if a warning has been issued
  uint8_t stop_issued; // Flag to indicate if a stop has been issued
  float accumulator[ACCUMULATOR_SIZE]; // Array for storing voltage samples
  uint8_t accumulator_idx; // Current index in the accumulator
  ADC_HandleTypeDef *hadc; // Handle for ADC
} power_module_t;

void PowerModule_init(power_module_t *power_module, ADC_HandleTypeDef *hadc);

void PowerModule_read_data(power_module_t *power_module);
void PowerModule_update_data(power_module_t *power_module);
