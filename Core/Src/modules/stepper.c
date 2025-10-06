#include "headers/stepper.h"
#include <stdio.h>
#include <math.h>

float freq = 0.0f; // Frequency in Hz

void Stepper_SpeedControl(stepper_t *stepper){
	/**
	 * @brief Control the speed of the stepper motor.
	 * 
	 * @param stepper Pointer to the stepper_t structure
	 */
	Encoder_Read(stepper->encoder);

	float e = stepper->setpoint_speed - stepper->encoder->speed;

	// stepper->frequency += e / ANGLE_STEP; // Integral only controller

	stepper->frequency += e * AEP + stepper->last_error * LEP; // PI controller
	stepper->last_error = e;



	uint8_t sign = stepper->frequency > 0;

	freq = stepper->frequency; // Store frequency for debugging
	//	stepper->frequency = 255;

	if (fabs(stepper->frequency) > MAX_CTRL_FREQUENCY) {
		stepper->frequency = MAX_CTRL_FREQUENCY * (sign?1:-1); // Limit frequency to 9000 Hz
		stepper->last_error = (sign && stepper->last_error>0) || (!sign && stepper->last_error<0) ? 0 : stepper->last_error; // Reset last error if the direction has changed
	} else if (fabs(stepper->frequency) < 20) {
		stepper->frequency = 0; // Stop the motor if frequency is too low
	}

	HAL_GPIO_WritePin(stepper->DIR_PORT, stepper->DIR_PIN, (stepper->encoder->direction_invert > 0 ? sign : !sign));

	stepper->tim->ARR = fabs(1/stepper->frequency)*HCLK-1; 	// period = (1+ARR)*(1+PSC)/HCLK;
	*stepper->CCR = (stepper->tim->ARR+1)/2; 				// 50% duty cycle
	stepper->tim->EGR = TIM_EGR_UG; 						// Force update of the timer registers
}

void Stepper_SetSpeed(stepper_t *stepper, float speed){
	/**
	 * @brief Set the desired speed for the stepper motor.
	 * 
	 * @param stepper Pointer to the stepper_t structure
	 * @param speed The desired speed in Hz
	 */
	stepper->setpoint_speed = speed;
}

void Stepper_Init(stepper_t *stepper, TIM_HandleTypeDef *htim, uint32_t tim_channel,
	encoder_t *encoder, GPIO_TypeDef *DIR_PORT, uint16_t DIR_PIN){
	/**
	 * @brief Initialize the stepper motor.
	 * 
	 * @param stepper Pointer to the stepper_t structure
	 * @param htim Pointer to the TIM handle
	 * @param tim_channel The TIM channel to use
	 * @param encoder Pointer to the associated encoder_t structure
	 * @param DIR_PORT GPIO port for the direction pin
	 * @param DIR_PIN GPIO pin for the direction pin
	 */
	stepper->angle_step = ANGLE_STEP;
	stepper->tim = htim->Instance;
	stepper->DIR_PORT = DIR_PORT;
	stepper->DIR_PIN = DIR_PIN;
	switch (tim_channel){
	case TIM_CHANNEL_1:
		stepper->CCR = &stepper->tim->CCR1;
		break;
	case TIM_CHANNEL_2:
		stepper->CCR = &stepper->tim->CCR2;
		break;
	case TIM_CHANNEL_3:
		stepper->CCR = &stepper->tim->CCR3;
		break;
	case TIM_CHANNEL_4:
		stepper->CCR = &stepper->tim->CCR4;
		break;
	default:
		stepper->CCR = NULL;
		break;
	}

	if (stepper->CCR != NULL)
		*stepper->CCR = 0;

	stepper->encoder = encoder;
	stepper->setpoint_speed = 0;
	stepper->frequency = 0;
	stepper->last_error = 0;
}

