#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

#define FREQUENCY 4.0f
#define DELTA_T (1.0 / FREQUENCY) //ms

void timer_interrupt_init();
void toogle_freq_test_pin();

#endif

