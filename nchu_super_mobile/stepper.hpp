#ifndef __STEPPER_H__
#define __STEPPER_H__

#define STEPPER_RESOLUTION 0.9
#define STEPPER_SINGLE_STEP (1 / STEPPER_RESOLUTION)

enum StepperDirection {STEPPER_CW, STEPPER_CCW};

void stepper_init();

#endif
