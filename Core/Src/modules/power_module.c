#include "headers/power_module.h"
#include "headers/display.h"
#include "gpio.h"

void PowerModule_Init(power_module_t *power_module, ADC_HandleTypeDef *hadc){
	/**
	 * @brief Initialize the power module
	 *
	 * @param power_module Pointer to the power_module_t structure
	 * @param hadc Pointer to the ADC handle
	 */

	power_module->alert_issued = 0; 	// Initialize alert flag
	power_module->hadc = hadc; 			// Initialize ADC handle

	HAL_ADC_Start(&power_module->hadc);
	if (HAL_ADC_PollForConversion(&power_module->hadc, 10) == HAL_OK) {
		power_module->voltage = HAL_ADC_GetValue(&power_module->hadc) / SLOPE - OFFSET;
	}
	HAL_ADC_Stop(&power_module->hadc);

	// Initialize the accumulator
	for (int i = 0; i < ACCUMULATOR_SIZE; i++) {
		power_module->accumulator[i] = power_module->voltage; // Initialize with the first read value
	}
	power_module->accumulator_idx = 0; // Initialize the accumulator index
}

void PowerModule_ReadData(power_module_t *power_module){
	/**
	 * @brief Read data from the power module
	 * 
	 * @param power_module Pointer to the power_module_t structure
	 */
	HAL_ADC_Start_IT(&power_module->hadc);
}

void PowerModule_UpdateData(power_module_t *power_module){
	/**
	 * @brief Update the power module data
	 * 
	 * @param power_module Pointer to the power_module_t structure
	 */
	power_module->accumulator[power_module->accumulator_idx] = HAL_ADC_GetValue(&power_module->hadc) / SLOPE - OFFSET; 	// Calculate voltage
	power_module->accumulator_idx = (power_module->accumulator_idx + 1) % ACCUMULATOR_SIZE; 							// Update index cyclically

	// Calculate average of values in accumulator
	float sum = 0.0f;
	for (int i = 0; i < ACCUMULATOR_SIZE; i++) {
		sum += power_module->accumulator[i];
	}
	power_module->voltage = sum / ACCUMULATOR_SIZE; // Update voltage with average	

	// Check limits
	if (power_module->voltage < WARNING_LIMIT) { 			// If voltage is below warning limit
		if (power_module->voltage < STOP_LIMIT) { 			// If voltage is below stop limit
			if (power_module->alert_issued < 2) { 			// If stop alert has not been issued
				MAX72_Stop_Changing_Data(&display, 1); 		// Stop automatic data change
				MAX72_Scroll_Start_IT("Critical Voltage!"); // Start scrolling stop message
				power_module->alert_issued = 2; 			// Set stop alert flag
			}
		} else if (!power_module->alert_issued) { 			// If warning alert has not been issued
			MAX72_Stop_Changing_Data(&display, 1); 			// Stop automatic data change
			MAX72_Scroll_Start_IT("Low Voltage!"); 			// Start scrolling warning message
			power_module->alert_issued = 1; 				// Set warning alert flag
		}

	}
}

