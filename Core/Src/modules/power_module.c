#include "headers/power_module.h"
#include "gpio.h"

#define EMA_ALPHA 0.3 // Fattore di smoothing per la media mobile

void PowerModule_init(power_module_t *power_module, ADC_HandleTypeDef *hadc){
	power_module->warning_limit = WARNING_LIMIT; // Imposta il limite di avviso
	power_module->stop_limit = STOP_LIMIT; // Imposta il limite di stop
	power_module->hadc = *hadc; // Initialize ADC handle

	HAL_ADC_Start(&power_module->hadc);  // Avvia manualmente
	if (HAL_ADC_PollForConversion(&power_module->hadc, 10) == HAL_OK) {  // Timeout corto
		power_module->voltage = HAL_ADC_GetValue(&power_module->hadc) / SLOPE - OFFSET;
	}
}

void pm_update_data(power_module_t *power_module){
	HAL_ADC_Start(&power_module->hadc);  // Avvia manualmente
	if (HAL_ADC_PollForConversion(&power_module->hadc, 10) == HAL_OK) {  // Timeout corto
		power_module->voltage = power_module->voltage * (1 - EMA_ALPHA) + (HAL_ADC_GetValue(&power_module->hadc) / SLOPE - OFFSET) * EMA_ALPHA; // Media mobile
	}
	HAL_ADC_Stop(&power_module->hadc);  // Ferma l’ADC dopo la conversione
}

