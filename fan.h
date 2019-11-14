/*
  fan.h
*/

#ifndef _FAN_H_
#define _FAN_H_

/*
  Temperatures are in degrees centigrade.  Below low, the fan will
  stay off.  Above high, the fan will run at full speed.  Between, the
  fan speed will vary -- hopefully somewhat linearly -- between off
  and full speed.

  Hardware PWM is used to control fan speed, so make sure the pin
  selected supports hardware PWM on the Pi model used!

  'bias' sets the minimum duty cycle that spins the fan.  For the
  cheap fans I tried, this was around 200,000.  So, instead of the
  full range offered by pigpio (0...1,000,000), use bias...1,000,000
  instead.
*/

struct fan_params {
	int pin;
	int bias;
	int high;
	int low;
};

void *fan(void *);

#endif	/* _FAN_H_ */
