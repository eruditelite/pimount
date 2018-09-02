/*
  pins.h
*/

#ifndef _PINS_H_
#define _PINS_H_

int pins_set_mode(int, unsigned, unsigned);
int pins_set_pull_up_down(int, unsigned, unsigned);
int pins_set_glitch_filter(int, unsigned, unsigned);
int pins_gpio_write(int, unsigned, unsigned);
int pins_callback(int, unsigned, unsigned,
		  void (*)(int, unsigned, unsigned, unsigned));

#endif	/* _PINS_H_ */
