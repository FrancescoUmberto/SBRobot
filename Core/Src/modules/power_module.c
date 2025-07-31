#include "headers/power_module.h"
#include "headers/display.h"
#include "gpio.h"

void PowerModule_init(power_module_t *power_module, ADC_HandleTypeDef *hadc){
	power_module->warning_limit = WARNING_LIMIT; // Imposta il limite di avviso
	power_module->stop_limit = STOP_LIMIT; // Imposta il limite di stop
	power_module->warning_issued = 0; // Inizializza il flag di avviso
	power_module->stop_issued = 0; // Inizializza il flag di stop
	power_module->hadc = *hadc; // Initialize ADC handle

	HAL_ADC_Start(&power_module->hadc);  // Avvia manualmente
	if (HAL_ADC_PollForConversion(&power_module->hadc, 10) == HAL_OK) {  // Timeout corto
		power_module->voltage = HAL_ADC_GetValue(&power_module->hadc) / SLOPE - OFFSET;
	}
	HAL_ADC_Stop(&power_module->hadc);  // Ferma l’ADC dopo la conversione

	// Inizializza l'accumulatore
	for (int i = 0; i < ACCUMULATOR_SIZE; i++) {
		power_module->accumulator[i] = power_module->voltage; // Inizializza con il primo valore letto
	}
	power_module->accumulator_idx = 0; // Inizializza l'indice dell'accumulatore
}

void PowerModule_update_data(power_module_t *power_module){
	HAL_ADC_Start(&power_module->hadc);  // Avvia l'ADC
	if (HAL_ADC_PollForConversion(&power_module->hadc, 10) == HAL_OK) {  // Timeout corto
		// Aggiunge il nuovo valore all'accumulatore
		power_module->accumulator[power_module->accumulator_idx] = HAL_ADC_GetValue(&power_module->hadc) / SLOPE - OFFSET; // Calcola la tensione
		power_module->accumulator_idx = (power_module->accumulator_idx + 1) % ACCUMULATOR_SIZE; // Aggiorna l'indice ciclicamente

		// Calcola la media dei valori nell'accumulatore
		float sum = 0.0f;
		for (int i = 0; i < ACCUMULATOR_SIZE; i++) {
			sum += power_module->accumulator[i];
		}
		power_module->voltage = sum / ACCUMULATOR_SIZE; // Aggiorna la tensione con la media

		// Controlla i limiti
		if (power_module->voltage < power_module->warning_limit) {
			if (power_module->voltage < power_module->stop_limit) { // Se la tensione è sotto il limite di stop
				if (!power_module->stop_issued) { // Se non è già stato emesso un stop
					MAX72_Stop_Changing_Data(&display, 1); // Ferma il cambio automatico dei dati
					MAX72_Scroll_Start_IT("Critical Voltage!"); // Avvia lo scrolling del messaggio di stop
					power_module->stop_issued = 1; // Imposta il flag di stop emesso
				}
			} else if (!power_module->warning_issued) { // Se non è già stato emesso un avviso
				MAX72_Stop_Changing_Data(&display, 1); // Ferma il cambio automatico dei dati
				MAX72_Scroll_Start_IT("Low Voltage!"); // Avvia lo scrolling del messaggio di avviso
				power_module->warning_issued = 1; // Imposta il flag di avviso emesso
			}

		}
	}
	HAL_ADC_Stop(&power_module->hadc);  // Ferma l'ADC dopo la conversione
}

