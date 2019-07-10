EESchema Schematic File Version 4
LIBS:Main Board-cache
EELAYER 26 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 2
Title "pimount"
Date "2019-03-27"
Rev "1.0"
Comp ""
Comment1 "https://github.com/eruditelite/pimount.git"
Comment2 "release_1.0"
Comment3 ""
Comment4 ""
$EndDescr
Text Notes 650  7600 0    50   ~ 0
ID_SD and ID_SC PINS:\nThese pins are reserved for HAT ID EEPROM.\n\nAt boot time this I2C interface will be\ninterrogated to look for an EEPROM\nthat identifes the attached board and\nallows automagic setup of the GPIOs\n(and optionally, Linux drivers).\n\nDO NOT USE these pins for anything other\nthan attaching an I2C ID EEPROM. Leave\nunconnected if ID EEPROM not required.
$Comp
L Main-Board-rescue:Mounting_Hole-Mechanical MK1
U 1 1 5834FB2E
P 3000 7200
F 0 "MK1" H 3100 7246 50  0000 L CNN
F 1 "M2.5" H 3100 7155 50  0000 L CNN
F 2 "MountingHole:MountingHole_2.7mm_M2.5" H 3000 7200 60  0001 C CNN
F 3 "" H 3000 7200 60  0001 C CNN
	1    3000 7200
	1    0    0    -1  
$EndComp
$Comp
L Main-Board-rescue:Mounting_Hole-Mechanical MK3
U 1 1 5834FBEF
P 3450 7200
F 0 "MK3" H 3550 7246 50  0000 L CNN
F 1 "M2.5" H 3550 7155 50  0000 L CNN
F 2 "MountingHole:MountingHole_2.7mm_M2.5" H 3450 7200 60  0001 C CNN
F 3 "" H 3450 7200 60  0001 C CNN
	1    3450 7200
	1    0    0    -1  
$EndComp
$Comp
L Main-Board-rescue:Mounting_Hole-Mechanical MK2
U 1 1 5834FC19
P 3000 7400
F 0 "MK2" H 3100 7446 50  0000 L CNN
F 1 "M2.5" H 3100 7355 50  0000 L CNN
F 2 "MountingHole:MountingHole_2.7mm_M2.5" H 3000 7400 60  0001 C CNN
F 3 "" H 3000 7400 60  0001 C CNN
	1    3000 7400
	1    0    0    -1  
$EndComp
$Comp
L Main-Board-rescue:Mounting_Hole-Mechanical MK4
U 1 1 5834FC4F
P 3450 7400
F 0 "MK4" H 3550 7446 50  0000 L CNN
F 1 "M2.5" H 3550 7355 50  0000 L CNN
F 2 "MountingHole:MountingHole_2.7mm_M2.5" H 3450 7400 60  0001 C CNN
F 3 "" H 3450 7400 60  0001 C CNN
	1    3450 7400
	1    0    0    -1  
$EndComp
Text Notes 3000 7050 0    50   ~ 0
Mounting Holes
$Comp
L pimount:ELEGOO U2
U 1 1 5C9935DF
P 4850 3500
F 0 "U2" H 4600 3000 50  0000 C CNN
F 1 "RA Driver" H 5000 3000 50  0000 C CNN
F 2 "pimount:ELEGOO" H 4850 3500 50  0001 C CNN
F 3 "" H 4850 3500 50  0001 C CNN
	1    4850 3500
	-1   0    0    -1  
$EndComp
Text Label 8550 3800 2    50   ~ 0
GPIO21(SPI1_SCK)
Wire Wire Line
	7500 3800 8550 3800
$Comp
L Connector_Generic:Conn_02x20_Odd_Even P1
U 1 1 59AD464A
P 7200 2800
F 0 "P1" H 7250 3917 50  0000 C CNN
F 1 "Conn_02x20_Odd_Even" H 7250 3826 50  0000 C CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_2x20_P2.54mm_Vertical" H 2350 1850 50  0001 C CNN
F 3 "" H 2350 1850 50  0001 C CNN
	1    7200 2800
	1    0    0    -1  
$EndComp
Text Label 8550 2200 2    50   ~ 0
GPIO14(TXD0)
Text Label 8550 2300 2    50   ~ 0
GPIO15(RXD0)
Text Label 8550 2400 2    50   ~ 0
GPIO18(GEN1)(PWM0)
Text Label 8550 2600 2    50   ~ 0
GPIO23(GEN4)
Text Label 8550 2700 2    50   ~ 0
GPIO24(GEN5)
Text Label 8550 2900 2    50   ~ 0
GPIO25(GEN6)
Text Label 8550 3000 2    50   ~ 0
GPIO8(SPI0_CE_N)
Text Label 8550 3100 2    50   ~ 0
GPIO7(SPI1_CE_N)
Text Label 8550 3200 2    50   ~ 0
ID_SC
Text Label 8550 3400 2    50   ~ 0
GPIO12(PWM0)
Text Label 8550 3600 2    50   ~ 0
GPIO16
Text Label 8550 3700 2    50   ~ 0
GPIO20(SPI1_MOSI)
Text Label 5850 3700 0    50   ~ 0
GPIO26
Text Label 5850 3600 0    50   ~ 0
GPIO19(SPI1_MISO)
Text Label 5850 3500 0    50   ~ 0
GPIO13(PWM1)
Text Label 5850 3400 0    50   ~ 0
GPIO6
Text Label 5850 3300 0    50   ~ 0
GPIO5
Text Label 5850 3200 0    50   ~ 0
ID_SD
Text Label 5850 3000 0    50   ~ 0
GPIO11(SPI0_SCK)
Text Label 5850 2900 0    50   ~ 0
GPIO9(SPI0_MISO)
Text Label 5850 2800 0    50   ~ 0
GPIO10(SPI0_MOSI)
Text Label 5850 2600 0    50   ~ 0
GPIO22(GEN3)
Text Label 5850 2500 0    50   ~ 0
GPIO27(GEN2)
Text Label 5850 2400 0    50   ~ 0
GPIO17(GEN0)
Text Label 5850 2200 0    50   ~ 0
GPIO4(GCLK)
Text Label 5850 2100 0    50   ~ 0
GPIO3(SCL1)
Text Label 5850 2000 0    50   ~ 0
GPIO2(SDA1)
Wire Wire Line
	7500 3400 7650 3400
Wire Wire Line
	7500 2700 8550 2700
Wire Wire Line
	7500 2600 8550 2600
Wire Wire Line
	7500 3000 8550 3000
Wire Wire Line
	7500 3200 8550 3200
Wire Wire Line
	7500 3100 8550 3100
Wire Wire Line
	7500 3700 8550 3700
Wire Wire Line
	7500 3600 8550 3600
Wire Wire Line
	6900 2300 7000 2300
$Comp
L power:+3.3V #PWR08
U 1 1 580C1BC1
P 6800 1750
F 0 "#PWR08" H 6800 1600 50  0001 C CNN
F 1 "+3.3V" H 6800 1890 50  0000 C CNN
F 2 "" H 6800 1750 50  0000 C CNN
F 3 "" H 6800 1750 50  0000 C CNN
	1    6800 1750
	1    0    0    -1  
$EndComp
Wire Wire Line
	6800 2700 7000 2700
$Comp
L power:GND #PWR09
U 1 1 580C1E01
P 6900 3950
F 0 "#PWR09" H 6900 3700 50  0001 C CNN
F 1 "GND" H 6900 3800 50  0000 C CNN
F 2 "" H 6900 3950 50  0000 C CNN
F 3 "" H 6900 3950 50  0000 C CNN
	1    6900 3950
	1    0    0    -1  
$EndComp
Wire Wire Line
	7500 2300 8550 2300
Wire Wire Line
	7500 2200 8550 2200
$Comp
L pimount:ELEGOO U1
U 1 1 5C994D26
P 4850 2200
F 0 "U1" H 4600 1700 50  0000 C CNN
F 1 "ELEGOO" H 5000 1700 50  0000 C CNN
F 2 "pimount:ELEGOO" H 4850 2200 50  0001 C CNN
F 3 "" H 4850 2200 50  0001 C CNN
	1    4850 2200
	-1   0    0    -1  
$EndComp
Wire Wire Line
	4450 2000 3500 2000
Wire Wire Line
	3500 2000 3500 3300
Wire Wire Line
	4450 3300 3500 3300
Connection ~ 3500 3300
Wire Wire Line
	3500 3300 3500 5000
Wire Wire Line
	3350 4700 3350 3200
Wire Wire Line
	3350 1900 4450 1900
Wire Wire Line
	4450 3200 3350 3200
Connection ~ 3350 3200
Wire Wire Line
	3350 3200 3350 1900
Wire Wire Line
	4450 2600 4300 2600
Wire Wire Line
	4300 2600 4300 3900
Wire Wire Line
	4300 3900 4450 3900
Wire Wire Line
	4300 3900 4300 4300
Connection ~ 4300 3900
$Comp
L power:GND #PWR05
U 1 1 5CAF3F3F
P 4300 4300
F 0 "#PWR05" H 4300 4050 50  0001 C CNN
F 1 "GND" H 4305 4127 50  0000 C CNN
F 2 "" H 4300 4300 50  0001 C CNN
F 3 "" H 4300 4300 50  0001 C CNN
	1    4300 4300
	1    0    0    -1  
$EndComp
Wire Wire Line
	4450 3800 4200 3800
Wire Wire Line
	4200 3800 4200 2500
Wire Wire Line
	4200 2500 4450 2500
Wire Wire Line
	4200 2500 4200 1700
Connection ~ 4200 2500
$Comp
L power:+3.3V #PWR04
U 1 1 5CB03AEF
P 4200 1700
F 0 "#PWR04" H 4200 1550 50  0001 C CNN
F 1 "+3.3V" H 4215 1873 50  0000 C CNN
F 2 "" H 4200 1700 50  0001 C CNN
F 3 "" H 4200 1700 50  0001 C CNN
	1    4200 1700
	1    0    0    -1  
$EndComp
$Comp
L Device:CP1 C1
U 1 1 5CAA3488
P 3000 4850
F 0 "C1" H 3115 4896 50  0000 L CNN
F 1 "220uF" H 3115 4805 50  0000 L CNN
F 2 "Capacitor_THT:CP_Radial_D6.3mm_P2.50mm" H 3000 4850 50  0001 C CNN
F 3 "~" H 3000 4850 50  0001 C CNN
	1    3000 4850
	1    0    0    -1  
$EndComp
Wire Wire Line
	3000 4700 3350 4700
Wire Wire Line
	3000 5000 3500 5000
$Comp
L Connector:Conn_01x04_Female J1
U 1 1 5CB0FE20
P 2450 2300
F 0 "J1" H 2342 1875 50  0000 C CNN
F 1 "DEC Motor" H 2342 1966 50  0000 C CNN
F 2 "Connector_JST:JST_PH_B4B-PH-K_1x04_P2.00mm_Vertical" H 2450 2300 50  0001 C CNN
F 3 "~" H 2450 2300 50  0001 C CNN
	1    2450 2300
	-1   0    0    1   
$EndComp
Wire Wire Line
	2650 2100 4450 2100
Wire Wire Line
	2650 2200 4450 2200
Wire Wire Line
	2650 2300 4450 2300
Wire Wire Line
	2650 2400 4450 2400
$Comp
L Connector:Conn_01x04_Female J2
U 1 1 5CB27E10
P 2450 3600
F 0 "J2" H 2342 3175 50  0000 C CNN
F 1 "RA Motor" H 2342 3266 50  0000 C CNN
F 2 "Connector_JST:JST_PH_B4B-PH-K_1x04_P2.00mm_Vertical" H 2450 3600 50  0001 C CNN
F 3 "~" H 2450 3600 50  0001 C CNN
	1    2450 3600
	-1   0    0    1   
$EndComp
Wire Wire Line
	2650 3400 4450 3400
Wire Wire Line
	2650 3500 4450 3500
Wire Wire Line
	2650 3600 4450 3600
Wire Wire Line
	2650 3700 4450 3700
NoConn ~ 7000 1900
NoConn ~ 7000 3800
NoConn ~ 7500 2800
NoConn ~ 7500 2500
NoConn ~ 7500 3300
NoConn ~ 7500 3500
Wire Wire Line
	6800 1750 6800 2700
NoConn ~ 7500 2100
Wire Wire Line
	6900 2300 6900 3100
Wire Wire Line
	7500 1900 7550 1900
Wire Wire Line
	7850 1900 7850 1750
$Comp
L power:+5V #PWR010
U 1 1 5CF6643F
P 7850 1750
F 0 "#PWR010" H 7850 1600 50  0001 C CNN
F 1 "+5V" H 7865 1923 50  0000 C CNN
F 2 "" H 7850 1750 50  0001 C CNN
F 3 "" H 7850 1750 50  0001 C CNN
	1    7850 1750
	1    0    0    -1  
$EndComp
NoConn ~ 7500 3800
NoConn ~ 7500 3700
NoConn ~ 7500 3600
NoConn ~ 7500 3200
NoConn ~ 7500 3100
NoConn ~ 7500 3000
NoConn ~ 7500 2700
NoConn ~ 7500 2600
NoConn ~ 7500 2200
NoConn ~ 7500 2300
NoConn ~ 7500 2400
NoConn ~ 7500 3400
NoConn ~ 5250 1900
NoConn ~ 5250 2200
NoConn ~ 5250 3200
NoConn ~ 5250 3500
$Sheet
S 4750 5700 850  800 
U 5C9A8F22
F0 "Joystick Input" 50
F1 "Joystick Input.sch" 50
F2 "5V" I L 4750 5850 50 
F3 "GND" I L 4750 6350 50 
F4 "3.3V" I L 4750 6100 50 
F5 "Z" O R 5600 5850 50 
F6 "Xa" O R 5600 6300 50 
F7 "Xb" O R 5600 6400 50 
F8 "Ya" O R 5600 6150 50 
F9 "Yb" O R 5600 6050 50 
$EndSheet
Wire Wire Line
	4750 6350 4450 6350
Wire Wire Line
	4450 6350 4450 6700
$Comp
L power:GND #PWR07
U 1 1 5C9C2D75
P 4450 6700
F 0 "#PWR07" H 4450 6450 50  0001 C CNN
F 1 "GND" H 4455 6527 50  0000 C CNN
F 2 "" H 4450 6700 50  0001 C CNN
F 3 "" H 4450 6700 50  0001 C CNN
	1    4450 6700
	1    0    0    -1  
$EndComp
Wire Wire Line
	4750 5850 4450 5850
Wire Wire Line
	4450 5850 4450 5450
$Comp
L power:+5V #PWR06
U 1 1 5C9C5B5C
P 4450 5450
F 0 "#PWR06" H 4450 5300 50  0001 C CNN
F 1 "+5V" H 4465 5623 50  0000 C CNN
F 2 "" H 4450 5450 50  0001 C CNN
F 3 "" H 4450 5450 50  0001 C CNN
	1    4450 5450
	1    0    0    -1  
$EndComp
Wire Wire Line
	4750 6100 4150 6100
Wire Wire Line
	4150 6100 4150 5450
$Comp
L power:+3.3V #PWR03
U 1 1 5C9C97CF
P 4150 5450
F 0 "#PWR03" H 4150 5300 50  0001 C CNN
F 1 "+3.3V" H 4165 5623 50  0000 C CNN
F 2 "" H 4150 5450 50  0001 C CNN
F 3 "" H 4150 5450 50  0001 C CNN
	1    4150 5450
	1    0    0    -1  
$EndComp
Wire Wire Line
	3650 2800 3650 5150
Wire Wire Line
	3650 5150 5750 5150
Wire Wire Line
	5750 5150 5750 5850
Wire Wire Line
	5750 5850 5600 5850
Wire Wire Line
	3650 2800 7000 2800
Wire Wire Line
	7000 2900 3750 2900
Wire Wire Line
	3750 2900 3750 5050
Wire Wire Line
	3750 5050 5850 5050
Wire Wire Line
	5850 5050 5850 6050
Wire Wire Line
	5600 6050 5850 6050
Wire Wire Line
	3850 4950 5950 4950
Wire Wire Line
	5950 4950 5950 6150
Wire Wire Line
	5950 6150 5600 6150
Wire Wire Line
	3850 3000 7000 3000
Wire Wire Line
	3850 3000 3850 4950
Wire Wire Line
	7500 2900 8550 2900
Wire Wire Line
	5750 2600 5750 3050
Wire Wire Line
	5750 3050 3950 3050
Wire Wire Line
	3950 3050 3950 4850
Wire Wire Line
	3950 4850 6050 4850
Wire Wire Line
	6050 4850 6050 6300
Wire Wire Line
	6050 6300 5600 6300
Wire Wire Line
	5600 6400 6150 6400
Wire Wire Line
	6150 4750 6150 6400
NoConn ~ 7500 2900
Wire Wire Line
	5750 2600 7000 2600
Wire Wire Line
	7000 3100 6900 3100
Connection ~ 6900 3100
Wire Wire Line
	6900 3100 6900 3950
$Comp
L pimount:BuckConverter U4
U 1 1 5CFD9B09
P 9700 4200
F 0 "U4" V 10200 3850 50  0000 L CNN
F 1 "BuckConverter" V 10200 4200 50  0000 L CNN
F 2 "pimount:BuckConverter" H 9700 4200 50  0001 C CNN
F 3 "" H 9700 4200 50  0001 C CNN
	1    9700 4200
	0    -1   1    0   
$EndComp
$Comp
L pimount:BuckConverter U3
U 1 1 5CFD9C57
P 9250 5400
F 0 "U3" V 9750 5100 50  0000 R CNN
F 1 "BuckConverter" V 9750 5900 50  0000 R CNN
F 2 "pimount:BuckConverter" H 9250 5400 50  0001 C CNN
F 3 "" H 9250 5400 50  0001 C CNN
	1    9250 5400
	0    -1   1    0   
$EndComp
$Comp
L Connector:Conn_01x02_Female J3
U 1 1 5CFE0810
P 10850 5500
F 0 "J3" H 10742 5175 50  0000 C CNN
F 1 "Battery" H 10742 5266 50  0000 C CNN
F 2 "Connector_JST:JST_PH_B2B-PH-K_1x02_P2.00mm_Vertical" H 10850 5500 50  0001 C CNN
F 3 "~" H 10850 5500 50  0001 C CNN
	1    10850 5500
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR013
U 1 1 5D05FD36
P 10100 6050
F 0 "#PWR013" H 10100 5800 50  0001 C CNN
F 1 "GND" H 10105 5877 50  0000 C CNN
F 2 "" H 10100 6050 50  0001 C CNN
F 3 "" H 10100 6050 50  0001 C CNN
	1    10100 6050
	1    0    0    -1  
$EndComp
Wire Wire Line
	10200 4000 10500 4000
Wire Wire Line
	10500 4000 10500 5200
Wire Wire Line
	10500 5500 10650 5500
Wire Wire Line
	9750 5200 10500 5200
Connection ~ 10500 5200
Wire Wire Line
	10500 5200 10500 5500
Wire Wire Line
	10200 4400 10350 4400
Wire Wire Line
	10350 4400 10350 4850
Wire Wire Line
	10350 5900 10100 5900
Wire Wire Line
	10100 5900 10100 6000
Wire Wire Line
	9750 5600 10350 5600
Connection ~ 10350 5600
Wire Wire Line
	10350 5600 10350 5900
Wire Wire Line
	10650 5600 10350 5600
Wire Wire Line
	8750 5600 8500 5600
Wire Wire Line
	8500 5600 8500 6000
Wire Wire Line
	8500 6000 10100 6000
Connection ~ 10100 6000
Wire Wire Line
	10100 6000 10100 6050
Connection ~ 10350 4850
Wire Wire Line
	10350 4850 10350 5600
$Comp
L power:+5V #PWR011
U 1 1 5D097ABF
P 8500 5000
F 0 "#PWR011" H 8500 4850 50  0001 C CNN
F 1 "+5V" H 8515 5173 50  0000 C CNN
F 2 "" H 8500 5000 50  0001 C CNN
F 3 "" H 8500 5000 50  0001 C CNN
	1    8500 5000
	1    0    0    -1  
$EndComp
Wire Wire Line
	8500 5000 8500 5200
Wire Wire Line
	8500 5200 8750 5200
Wire Wire Line
	8900 3800 8900 4000
Wire Wire Line
	8900 4000 9200 4000
Wire Wire Line
	8900 4400 8900 4850
Wire Wire Line
	8900 4400 9200 4400
Wire Wire Line
	8900 4850 10350 4850
$Comp
L power:GND #PWR02
U 1 1 5D0B7C63
P 3000 5200
F 0 "#PWR02" H 3000 4950 50  0001 C CNN
F 1 "GND" H 3005 5027 50  0000 C CNN
F 2 "" H 3000 5200 50  0001 C CNN
F 3 "" H 3000 5200 50  0001 C CNN
	1    3000 5200
	1    0    0    -1  
$EndComp
Wire Wire Line
	3000 5000 3000 5200
Connection ~ 3000 5000
Wire Wire Line
	3000 4500 3000 4700
Connection ~ 3000 4700
Wire Wire Line
	7500 2000 7550 2000
Wire Wire Line
	7550 2000 7550 1900
Connection ~ 7550 1900
Wire Wire Line
	7550 1900 7850 1900
Wire Wire Line
	5600 2500 7000 2500
Wire Wire Line
	5600 2600 5600 2500
Wire Wire Line
	5250 2600 5600 2600
Wire Wire Line
	5500 2400 7000 2400
Wire Wire Line
	5500 2500 5500 2400
Wire Wire Line
	5250 2500 5500 2500
Wire Wire Line
	5400 2200 7000 2200
Wire Wire Line
	5400 2400 5250 2400
Wire Wire Line
	5400 2200 5400 2300
Wire Wire Line
	5250 2100 7000 2100
Wire Wire Line
	5250 2000 7000 2000
Wire Wire Line
	5250 3300 7000 3300
Wire Wire Line
	5250 3400 7000 3400
Wire Wire Line
	5250 3700 5350 3700
Wire Wire Line
	5350 3700 5350 3600
Wire Wire Line
	5350 3500 7000 3500
Wire Wire Line
	5250 3800 5400 3800
Wire Wire Line
	5400 3800 5400 3600
Wire Wire Line
	5400 3600 7000 3600
Wire Wire Line
	5250 3900 5450 3900
Wire Wire Line
	5450 3900 5450 3700
Wire Wire Line
	5450 3700 7000 3700
Text Notes 1200 2350 0    50   ~ 0
RJ9\nPin 1 goes to 1A\nPin 2 goes to 2B\nPin 3 goes to 2A\nPin 4 goes to 1B
Text Notes 1200 3600 0    50   ~ 0
RJ10\nPin 1 goes to 1A\nPin 2 goes to 2B\nPin 3 goes to 2A\nPin 4 goes to 1B
Wire Wire Line
	5250 3600 5350 3600
Connection ~ 5350 3600
Wire Wire Line
	5350 3600 5350 3500
Wire Wire Line
	5250 2300 5400 2300
Connection ~ 5400 2300
Wire Wire Line
	5400 2300 5400 2400
Wire Wire Line
	5850 3200 7000 3200
Wire Wire Line
	7650 3400 7650 4750
Wire Wire Line
	7650 4750 6150 4750
Connection ~ 7650 3400
Wire Wire Line
	7650 3400 8550 3400
$Comp
L power:+10V #PWR012
U 1 1 5D25EF9D
P 8900 3800
F 0 "#PWR012" H 8900 3650 50  0001 C CNN
F 1 "+10V" H 8915 3973 50  0000 C CNN
F 2 "" H 8900 3800 50  0001 C CNN
F 3 "" H 8900 3800 50  0001 C CNN
	1    8900 3800
	1    0    0    -1  
$EndComp
$Comp
L power:+10V #PWR01
U 1 1 5D25F09C
P 3000 4500
F 0 "#PWR01" H 3000 4350 50  0001 C CNN
F 1 "+10V" H 3015 4673 50  0000 C CNN
F 2 "" H 3000 4500 50  0001 C CNN
F 3 "" H 3000 4500 50  0001 C CNN
	1    3000 4500
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x02_Female J6
U 1 1 5D25F838
P 10800 1250
F 0 "J6" H 10692 925 50  0000 C CNN
F 1 "Fan" H 10692 1016 50  0000 C CNN
F 2 "Connector_JST:JST_PH_B2B-PH-K_1x02_P2.00mm_Vertical" H 10800 1250 50  0001 C CNN
F 3 "~" H 10800 1250 50  0001 C CNN
	1    10800 1250
	1    0    0    -1  
$EndComp
$Comp
L Transistor_BJT:S8050 Q6
U 1 1 5D25FF21
P 9950 1800
F 0 "Q6" H 10141 1846 50  0000 L CNN
F 1 "S8050" H 10141 1755 50  0000 L CNN
F 2 "Package_TO_SOT_THT:TO-92_Inline" H 10150 1725 50  0001 L CIN
F 3 "http://www.unisonic.com.tw/datasheet/S8050.pdf" H 9950 1800 50  0001 L CNN
	1    9950 1800
	1    0    0    -1  
$EndComp
$Comp
L Diode:1N4148 D1
U 1 1 5D260018
P 10050 1300
F 0 "D1" V 10004 1379 50  0000 L CNN
F 1 "1N4148" V 10095 1379 50  0000 L CNN
F 2 "Diode_THT:D_DO-35_SOD27_P2.54mm_Vertical_KathodeUp" H 10050 1125 50  0001 C CNN
F 3 "http://www.nxp.com/documents/data_sheet/1N4148_1N4448.pdf" H 10050 1300 50  0001 C CNN
	1    10050 1300
	0    1    1    0   
$EndComp
$Comp
L Device:R R13
U 1 1 5D2600A5
P 9450 1800
F 0 "R13" V 9243 1800 50  0000 C CNN
F 1 "1K" V 9334 1800 50  0000 C CNN
F 2 "Resistor_THT:R_Axial_DIN0204_L3.6mm_D1.6mm_P1.90mm_Vertical" V 9380 1800 50  0001 C CNN
F 3 "~" H 9450 1800 50  0001 C CNN
	1    9450 1800
	0    1    1    0   
$EndComp
$Comp
L power:+5V #PWR016
U 1 1 5D265707
P 10050 1000
F 0 "#PWR016" H 10050 850 50  0001 C CNN
F 1 "+5V" H 10065 1173 50  0000 C CNN
F 2 "" H 10050 1000 50  0001 C CNN
F 3 "" H 10050 1000 50  0001 C CNN
	1    10050 1000
	1    0    0    -1  
$EndComp
Wire Wire Line
	10050 1000 10050 1150
Wire Wire Line
	10050 1150 10600 1150
Wire Wire Line
	10600 1150 10600 1250
Connection ~ 10050 1150
$Comp
L power:GND #PWR017
U 1 1 5D2705BD
P 10050 2150
F 0 "#PWR017" H 10050 1900 50  0001 C CNN
F 1 "GND" H 10055 1977 50  0000 C CNN
F 2 "" H 10050 2150 50  0001 C CNN
F 3 "" H 10050 2150 50  0001 C CNN
	1    10050 2150
	1    0    0    -1  
$EndComp
Wire Wire Line
	10050 1450 10050 1600
Wire Wire Line
	10050 2000 10050 2150
Wire Wire Line
	10050 1450 10600 1450
Wire Wire Line
	10600 1450 10600 1350
Connection ~ 10050 1450
Wire Wire Line
	9600 1800 9750 1800
Wire Wire Line
	9300 1800 8800 1800
Wire Wire Line
	8800 1800 8800 2400
Wire Wire Line
	7500 2400 8800 2400
$EndSCHEMATC
