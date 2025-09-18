#include "headers/button.h"
#include "headers/robot.h"

static uint32_t last_debounce_time = 0; // Timestamp of the last button press

void on_click(){
	if ((HAL_GetTick() - last_debounce_time) > DEBOUNCE_DELAY){
		last_debounce_time = HAL_GetTick();

		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_9);

		pid.active ^= 1;
		if (pid.active) {
			PID_Reset(&pid); // Reset PID before starting control to avoid spikes
		} else {
			set_speed(&stepper_l, 0);
			set_speed(&stepper_r, 0);
			Save_BaseAngle(&pid); // Save the current base angle to flash
		}
	}
}
