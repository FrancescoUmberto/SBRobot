#include "stm32f4xx_hal.h"
#include "tim.h"
#include "gpio.h"
#include "headers/robot.h"

#define DEBOUNCE_DELAY 100 // ms

static uint32_t last_debounce_time = 0;
// static uint8_t active = 0;

void on_click(){
	if ((HAL_GetTick() - last_debounce_time) > DEBOUNCE_DELAY){
		last_debounce_time = HAL_GetTick();
		// active ^= 1;

		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_9);

		// if(active){
		// 	set_speed(&stepper_l, 3);
		// 	set_speed(&stepper_r, 3);
		// } else{
		// 	set_speed(&stepper_l, 0);
		// 	set_speed(&stepper_r, 0);
		// }

		pid.active ^= 1;
		if (pid.active) {
			PID_Reset(&pid);
		} else {
			set_speed(&stepper_l, 0);
			set_speed(&stepper_r, 0);
		}
	}
}
