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
    ascension motor uses a RJ11.  Instead of creating a driver, I use
    a pre-made driver based on the A4988.  The version I used is the
    'ELEGOO Stepstick Stepper Motor Driver Module A4988' -- others are
    available, but KiCAD footprints must be created.  Here's how the
    wiring works.
	
  * I "reverse engineered" TrueTrack to figure out how to drive the
    stepper motors.  See appendix below...

### References ###

Others have modified the system as well. See, for example,

  * [PIC 16F627 Based EQ-5 / CG-5 Dual Axis Hand Controller]<http://telescope.marford.me.uk/Projects/PICcontroller.htm>
  * [THE EQMOD PROJECT]<http://eq-mod.sourceforge.net/>
  * [AstroEQ]<https://www.astroeq.co.uk/tutorials.php?link=custommotors>
  * [Motor]<https://www.digikey.com/catalog/en/partgroup/pm-series/536>
  * [Motor]<https://www.cloudynights.com/topic/631615-want-to-make-diy-controller-for-my-cg-4-dual-axis-motors/>
  * 

### Stepper Motor Wiring and Current Limit ###

Set the voltage (potentiometer to ground) to 125 mV.  Motor voltage
should be 10 V.

## Setup ##



## Local OLED Display ##

The plan is to use a white display with red film...

The part is IZOKEE 0.96'' I2C IIC 12864 128X64 Pixel OLED LCD Display
Shield Board Module SSD1306

First unit test... The part is IZOKEE 0.96'' I2C IIC 12864 128X64
Pixel OLED LCD Display

"The original Pi" (not sure about this, but it seems correct) used
i2c-0.  After that, i2c-1 is used (pins bcm2 and bcm3 instead of bcm0
and bcm1).  Trying to use i2c-0 is fraught with difficulties.  Using
i2c-1.

## Local Control ##

Local control uses a USB game controller (SNES style).  Don't use
iBuffalo, as there are phantom button presses!  Currently using
innext.

## Web User Interface ##

### Why ###

Allows basic control from any device with a web server.

### How ###

See https://github.com/Mjrovai/RPi-Flask-WebServer for an example
setup. That's the skeleton.

## INDI Driver ##

The most straight-forward way to support guiding seems to be with an
INDI driver.  Start with the 'drivers/telescope/telescope_simulator.*'
files in INDI.

### Install ###

Use the latest version of the INDI library by installing in /usr/local
as follows.

    sudo apt-get -y install libnova-dev libcfitsio-dev libusb-1.0-0-dev \
    	zlib1g-dev libgsl-dev build-essential cmake git libjpeg-dev \
    	libcurl4-gnutls-dev libtheora-dev libfftw3-dev
    
    git clone https://github.com/indilib/indi.git
    cd indi
    INDI_CLONE=$(pwd)
    <currently at 4543974f on master or v1.8.5>
    BUILD_DIRECTORY=<wherever you want to build>
    cd $BUILD_DIRECTORY
    cmake --DCMAKE_INSTALL_PREFIX=/usr $INDI_CLONE
    make -j4
    sudo make install

### NOTES ###

1. Use 'indiprop' to test.
   * https://github.com/aaronevers/indiprop.git
2. Adding things to the driver.
   * Add private variables in indi/pimount.h.
   * A *Vector* is required for the client to use the property.
   * For a single thing (number, string, etc.) create an array with
     one element, just creating a single element will NOT work
     (INumber ANewNumber[1], not INumber ANewNumber).

## lin_guider ##

wget https://sourceforge.net/projects/linguider/files/4.2.0/lin_guider-4.2.0.tar.bz2/download
mv download lin_guider-4.2.0.tar.bz2
tar xf lin_guider-4.2.0.tar.bz2
cd lin_guider_pack/lin_guider
sudo apt-get install libusb-1.0-0-dev libqt4-dev libftdi-dev fxload
./configure
make

## ASI SDK ##

sudo cp include/ASICamera2.h /usr/local/include
sudo chmod 644 /usr/local/include/ASICamera2.h

## systemd ##

As everyone uses it now...

make install

Then, pimount and pimount-indi will start at boot.  Use 'systemctl
...' to control pimount and pimount-indi as expected.

## Post Mortems ##

### release_1.3 ###

### release_1.2 ###

  * Circuit clean up -- the ELEGOO stepper motor pin size was too small etc.
  * Build updates.
  * Add fan control.
  * Update the board and silk screen.

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

## Issues ##

### USB Controller Only Works if Connected when the Program Starts ###
