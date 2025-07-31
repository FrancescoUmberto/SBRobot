#include <stdint.h>
#include "stm32f4xx_hal.h"

#define WARNING_LIMIT 22 // Max value acceptable for battery's voltage
#define STOP_LIMIT 20 // Min value acceptable for battery's voltage
#define SLOPE 114.07
#define OFFSET 0.06708161655124

typedef struct {
  float voltage;
  float warning_limit;
  float stop_limit;
  ADC_HandleTypeDef hadc; // Handle for ADC
} power_module_t;

void PowerModule_init(power_module_t *power_module, ADC_HandleTypeDef *hadc);

void pm_update_data(power_module_t *power_module);
