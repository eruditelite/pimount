/*
  fan.h
*/

#ifndef _FAN_H_
#define _FAN_H_

struct fan_params {
	int pin;
	int pi;
};

void *fan(void *);

#endif	/* _FAN_H_ */
