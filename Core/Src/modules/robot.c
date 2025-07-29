#include "headers/robot.h"
#include "tim.h"

encoder_t encoder2;
stepper_t stepper2;

void Robot_init(){
	encoder_init(&encoder2,&htim4,&htim7);

	MAX72_init();										// Display Init
}
