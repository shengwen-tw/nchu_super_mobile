#include <Arduino.h>

#define SPEED(x) (42 * x)

void freq_test_pin_init()
{
  // Enable pin 13 with direct port manipulation
  REG_PIOB_PER = 1 << 27;
  REG_PIOB_OER = 1 << 27;
  REG_PIOB_OWER = 1 << 27;
}

void timer_interrupt_init()
{
    pmc_set_writeprotect(false);       // disable write protection for pmc registers /**always the first thing to do*/

    /* turn on the timer clock in the power management controller */
    pmc_enable_periph_clk(ID_TC0);     // enable peripheral clock TC0
    NVIC_EnableIRQ(TC0_IRQn); //Enable interrupt in NVIC before starting interrupt
    REG_TC0_CMR0 = TC_CMR_WAVSEL_UP_RC | TC_CMR_TCCLKS_TIMER_CLOCK1; //Wavesel on rising edge with timer clock1 (42MHz)
    REG_TC0_IER0 = TC_IER_CPCS; //Enabel RC compare interrupt
    REG_TC0_RC0 = SPEED(10000);  //100hz

    REG_TC0_CCR0 = TC_CCR_SWTRG | TC_CCR_CLKEN; //Start timer

    freq_test_pin_init();
}

void toogle_freq_test_pin()
{
  REG_PIOB_ODSR = (~(REG_PIOB_ODSR & PIO_PB27)) & PIO_PB27;
}

void TC0_Handler()
{
  REG_TC0_SR0; //Clear timer counter

  toogle_freq_test_pin();
}
