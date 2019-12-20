/*
  pins.h
*/

#ifndef _PINS_H_
#define _PINS_H_

#include <pigpio.h>

int pins_set_mode(unsigned, unsigned);
int pins_set_pull_up_down(unsigned, unsigned);
int pins_gpio_read(unsigned);
int pins_gpio_write(unsigned, unsigned);
int pins_isr(unsigned, unsigned, int, gpioISRFunc_t);

#endif	/* _PINS_H_ */
