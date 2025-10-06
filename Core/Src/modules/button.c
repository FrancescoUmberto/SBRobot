#include "headers/button.h"
#include "headers/robot.h"

static uint32_t last_debounce_time = 0; // Timestamp of the last button press

void Button_OnClick(){
	/**
	 * @brief Handle button click event
	 * This function toggles the controller's active state and updates the
	 * display accordingly. It also implements a debounce mechanism to
	 * prevent multiple toggles from a single press.
	 */
	if ((HAL_GetTick() - last_debounce_time) > DEBOUNCE_DELAY){
		last_debounce_time = HAL_GetTick();

		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_9);

		controller.active ^= 1;
		if (controller.active) {
			Controller_Reset(&controller); 			// Reset controller before starting control to avoid spikes
		} else {
			Stepper_SetSpeed(&stepper_l, 0);
			Stepper_SetSpeed(&stepper_r, 0);
			Controller_SaveBaseAngle(&controller); 	// Save the current base angle to flash
		}
	}
}
