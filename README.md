# pimount #

## Overview ##

Telescope mount driver.

## Starting Points and Connections ##

This all started with a SkyView Pro mount and Orion 7832 Dual Axis
TrueTrack Telescope Drive.  This setup works, but there are a number
of issues.

  * The battery pack is not robust.  Using rechargable D cells is
    okay, but it is not ideal for weekend trips etc.
  * There is no "guiding".  I added a port (ST4) that can be used to
    provide guiding, but haven't tried it.  Adding the ST4 port is
    described at http://www.store.shoestringastronomy.com/eq_mod.pdf .

So, this project uses a Raspberry Pi and a custom ciruit to replace
most of the 7832 drive.  The stepper motors are the only part that
isn't replaced.  It is intended to handle drive and autoguide when
complete.  The components are as follow.

  * Use a deep cycle 12 VDC battery.  This can power the Pi, stepper
    motors, other accessories, etc.  Adding a "DC to DC buck
    converter" provides 6 VDC which can be used to power the 7832 if
    desired.  Here's the result.  The paper towel roll segment holds
    the DC to DC buck converter that supplies 6 VDC as expected by
    Orion accessories.

	[Battery](https://www.dropbox.com/s/0trkuq7uk3z1awa/battery.jpg?dl=0)

  * The stepper motors use different sized connectors to avoid
    confusion.  The declination motor uses a RJ9 and the right
    ascension motor uses a RJ10.  Instead of creating a driver, I use
    a pre-made driver based on the A4988.  The version I used is the
    'ELEGOO Stepstick Stepper Motor Driver Module A4988' -- others are
    available, but KiCAD footprints must be created.  Here's how the
    wiring works.

### Stepper Motor Wiring and Current Limit ###

Set the voltage (potentiometer to ground) to 125 mV.  Motor voltage
should be 10 V.

## Post Mortems ##

### release_1.1 ###

  * I hate JST connectors!!!!!  I was finally able to make a few work,
    but it took much too long and there were too many failures (cut
    the wire and restart sort).  Is there another way?
  * Even if I stick with JST connectors, 2.0 mm is too small for the
    power connector.  The insulation doesn't fit in the crimpers.
  * I didn't measure the pins on the ELEGOO stepper motor
    drivers... They don't fit in the default through-hole size.  As I
    created the footprint, the problem would be me.  I think
    un-soldering the driver would be too hard to be reasonable.  I was
    hoping to use a socket, but with the size difference, that may be
    difficult.  At least the size of the holes should be increased!
	  * Fixed (increased the size of the holes) by
	    '4588a09 Update the ELEGOO Stepper Motor Footprint'.
  * As I'm planning on adding a fan for ventilation, there needs to be
    a fan control circuit.
  * Joystick wiring was not described adequately on the schematic.
  * Stepper moter wiring as well.
  * Not sure about the debug headers.  I didn't solder any on, but
    that pads are useful!
  * Default pin assignments are wrong.
