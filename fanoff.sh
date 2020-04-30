#!/bin/sh

# Make sure the pin number matches the fan pin in pimount.h!

gpio -g mode 18 out
gpio -g write 18 0
