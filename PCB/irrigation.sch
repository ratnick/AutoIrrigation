EESchema Schematic File Version 4
LIBS:irrigation-cache
EELAYER 26 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "Irrigation"
Date "2019-02-08"
Rev "1.00"
Comp ""
Comment1 "with battery monitor and 5V/12V option for valve"
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Transistor_BJT:TIP120 Q1
U 1 1 5C4F21EB
P 5850 4200
F 0 "Q1" H 6057 4246 50  0001 L CNN
F 1 "TIP120" H 6057 4200 50  0000 L CNN
F 2 "footprint:TIP120_v4" H 6050 4125 50  0001 L CIN
F 3 "http://www.fairchildsemi.com/ds/TI/TIP120.pdf" H 5850 4200 50  0001 L CNN
	1    5850 4200
	-1   0    0    1   
$EndComp
$Comp
L Device:R R1
U 1 1 5C4F22D3
P 6600 3950
F 0 "R1" H 6670 3950 50  0000 L CNN
F 1 "5k" H 6670 3905 50  0000 L TNN
F 2 "Resistor_THT:R_Axial_DIN0204_L3.6mm_D1.6mm_P5.08mm_Horizontal" V 6530 3950 50  0001 C CNN
F 3 "~" H 6600 3950 50  0001 C CNN
	1    6600 3950
	1    0    0    -1  
$EndComp
$Comp
L Switch:SW_SPST SW1
U 1 1 5C4F30A2
P 4700 1300
F 0 "SW1" H 4700 1535 50  0001 C CNN
F 1 "ON/OFF" H 4700 1443 50  0000 C CNN
F 2 "footprint:2p_connector_v2" H 4700 1300 50  0001 C CNN
F 3 "" H 4700 1300 50  0001 C CNN
	1    4700 1300
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x02_Female J4
U 1 1 5C4F9672
P 9250 5050
F 0 "J4" H 9277 5026 50  0000 L CNN
F 1 "Soil sensor" H 9277 4935 50  0000 L CNN
F 2 "footprint:2p_connector_v2" H 9250 5050 50  0001 C CNN
F 3 "~" H 9250 5050 50  0001 C CNN
	1    9250 5050
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x02_Female J2
U 1 1 5C4F9789
P 2250 1550
F 0 "J2" H 2277 1526 50  0000 L CNN
F 1 "Solar panel" H 2277 1435 50  0000 L CNN
F 2 "footprint:2p_connector_v2" H 2250 1550 50  0001 C CNN
F 3 "~" H 2250 1550 50  0001 C CNN
	1    2250 1550
	-1   0    0    1   
$EndComp
$Comp
L Connector:Conn_01x02_Female J3
U 1 1 5C4F97F0
P 5950 4650
F 0 "J3" H 5977 4626 50  0000 L CNN
F 1 "Valve" H 5977 4535 50  0000 L CNN
F 2 "footprint:2p_connector_v2" H 5950 4650 50  0001 C CNN
F 3 "~" H 5950 4650 50  0001 C CNN
	1    5950 4650
	1    0    0    -1  
$EndComp
$Comp
L wemos_mini:WeMos_mini U3
U 1 1 5C4F9AC3
P 5900 2950
F 0 "U3" H 5900 3693 60  0001 C CNN
F 1 "WeMos_mini" H 5900 3481 60  0000 C CNN
F 2 "wemos_d1_mini:D1_mini_board_v4" H 6450 2250 60  0001 C CNN
F 3 "http://www.wemos.cc/Products/d1_mini.html" H 5900 3481 60  0001 C CNN
	1    5900 2950
	1    0    0    -1  
$EndComp
$Comp
L Device:R R2
U 1 1 5C4F9DC6
P 2750 3500
F 0 "R2" H 2820 3500 50  0000 L CNN
F 1 "100k" H 2820 3455 50  0000 L TNN
F 2 "Resistor_THT:R_Axial_DIN0204_L3.6mm_D1.6mm_P5.08mm_Horizontal" V 2680 3500 50  0001 C CNN
F 3 "~" H 2750 3500 50  0001 C CNN
	1    2750 3500
	1    0    0    -1  
$EndComp
$Comp
L NNR-lib:SOIL_SENSOR SS1
U 1 1 5C4FA314
P 8500 3800
F 0 "SS1" H 9028 3146 47  0001 L CNN
F 1 "SOIL_SENSOR" H 9028 3100 50  0000 L CNN
F 2 "footprint:Soil sensor_v3" H 9950 3750 50  0001 C CNN
F 3 "https://www.aliexpress.com/item/Free-Shipping-Soil-Hygrometer-Humidity-Detection-Module-Moisture-Water-Sensor-Soil-moisture-for-Arduino/32704803481.html?spm=2114.search0104.3.16.6368597e5Z19nX&transAbTest=ae803_4&ws_ab_test=searchweb0_0%2Csearchweb201602_2_10065_10068_319_317_10696_453_10084_454_10083_433_10618_431_10304_10307_10820_10821_537_10302_536_10902_10059_10884_10887_100031_321_322_10103%2Csearchweb201603_51%2CppcSwitch_0&algo_pvid=aab9002f-3c39-4bfa-ba02-40efef501c31&algo_expid=aab9002f-3c39-4bfa-ba02-40efef501c31-2" H 9950 3750 50  0001 C CNN
	1    8500 3800
	1    0    0    -1  
$EndComp
Wire Wire Line
	8700 5050 8700 5150
Wire Wire Line
	2550 1550 2450 1550
Wire Wire Line
	3450 1300 2550 1300
Wire Wire Line
	2550 1300 2550 1450
Wire Wire Line
	2550 1450 2450 1450
Wire Wire Line
	5150 1300 5150 2600
Wire Wire Line
	6600 3800 6600 3000
Wire Wire Line
	6600 3000 6400 3000
Wire Wire Line
	6600 4200 6600 4100
Wire Wire Line
	5750 4400 5750 4650
Wire Wire Line
	6400 3100 6500 3100
Wire Wire Line
	6500 3100 6500 3300
Wire Wire Line
	6500 3300 6400 3300
$Comp
L power:GND #PWR0105
U 1 1 5C512636
P 2100 2200
F 0 "#PWR0105" H 2100 1950 50  0001 C CNN
F 1 "GND" H 2105 2027 50  0000 C CNN
F 2 "" H 2100 2200 50  0001 C CNN
F 3 "" H 2100 2200 50  0001 C CNN
	1    2100 2200
	1    0    0    -1  
$EndComp
Wire Wire Line
	2100 2200 2100 2150
Wire Wire Line
	7450 5250 2750 5250
Wire Wire Line
	2750 3650 2750 5250
$Comp
L Connector_Generic:Conn_01x02 J1
U 1 1 5C554B8B
P 2300 3050
F 0 "J1" H 2220 2725 50  0000 C CNN
F 1 "To Battery" H 2220 2816 50  0000 C CNN
F 2 "footprint:2p_connector_v2" H 2300 3050 50  0001 C CNN
F 3 "~" H 2300 3050 50  0001 C CNN
	1    2300 3050
	-1   0    0    1   
$EndComp
Wire Wire Line
	2750 2200 2750 3050
Wire Wire Line
	2500 3050 2600 3050
Wire Wire Line
	2600 3050 2750 3050
Connection ~ 2750 3050
Wire Wire Line
	2750 3050 2750 3350
Wire Wire Line
	8800 5050 9050 5050
Wire Wire Line
	9050 5150 8700 5150
$Comp
L power:PWR_FLAG #FLG0101
U 1 1 5C580511
P 2550 1200
F 0 "#FLG0101" H 2550 1275 50  0001 C CNN
F 1 "PWR_FLAG" H 2550 1374 50  0000 C CNN
F 2 "" H 2550 1200 50  0001 C CNN
F 3 "~" H 2550 1200 50  0001 C CNN
	1    2550 1200
	1    0    0    -1  
$EndComp
Wire Wire Line
	2550 1200 2550 1300
Connection ~ 2550 1300
$Comp
L power:PWR_FLAG #FLG0102
U 1 1 5C5831E2
P 2600 3150
F 0 "#FLG0102" H 2600 3225 50  0001 C CNN
F 1 "PWR_FLAG" H 2600 3323 50  0000 C CNN
F 2 "" H 2600 3150 50  0001 C CNN
F 3 "~" H 2600 3150 50  0001 C CNN
	1    2600 3150
	-1   0    0    1   
$EndComp
Wire Wire Line
	2500 2100 4550 2100
Wire Wire Line
	4550 2100 4550 1700
Wire Wire Line
	4600 2200 4600 1400
Wire Wire Line
	4600 1400 4350 1400
Wire Wire Line
	2750 2200 4600 2200
Wire Wire Line
	4350 1300 4500 1300
$Comp
L power:PWR_FLAG #FLG0103
U 1 1 5C594544
P 4900 1050
F 0 "#FLG0103" H 4900 1125 50  0001 C CNN
F 1 "PWR_FLAG" H 4900 1224 50  0000 C CNN
F 2 "" H 4900 1050 50  0001 C CNN
F 3 "~" H 4900 1050 50  0001 C CNN
	1    4900 1050
	1    0    0    -1  
$EndComp
Wire Wire Line
	5150 2600 5400 2600
Wire Wire Line
	4350 1700 4550 1700
$Comp
L power:GND #PWR0101
U 1 1 5C5C5510
P 8800 3950
F 0 "#PWR0101" H 8800 3700 50  0001 C CNN
F 1 "GND" H 8805 3777 50  0000 C CNN
F 2 "" H 8800 3950 50  0001 C CNN
F 3 "" H 8800 3950 50  0001 C CNN
	1    8800 3950
	-1   0    0    1   
$EndComp
$Comp
L NNR-lib:GND_FLAG #G0104
U 1 1 5C5C7A00
P 1900 2450
F 0 "#G0104" H 1900 2450 50  0001 C CNN
F 1 "GND_FLAG" H 1900 2450 50  0001 C CNN
F 2 "" H 1900 2450 50  0001 C CNN
F 3 "" H 1900 2450 50  0001 C CNN
	1    1900 2450
	1    0    0    -1  
$EndComp
Wire Wire Line
	2100 2150 1900 2150
$Comp
L power:GND #PWR0102
U 1 1 5C5CB3A0
P 3150 2400
F 0 "#PWR0102" H 3150 2150 50  0001 C CNN
F 1 "GND" H 3155 2227 50  0000 C CNN
F 2 "" H 3150 2400 50  0001 C CNN
F 3 "" H 3150 2400 50  0001 C CNN
	1    3150 2400
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR0103
U 1 1 5C5CB3E8
P 5750 4000
F 0 "#PWR0103" H 5750 3750 50  0001 C CNN
F 1 "GND" H 5755 3827 50  0000 C CNN
F 2 "" H 5750 4000 50  0001 C CNN
F 3 "" H 5750 4000 50  0001 C CNN
	1    5750 4000
	-1   0    0    1   
$EndComp
Wire Wire Line
	2500 2150 2500 2950
Wire Wire Line
	2500 2100 2500 2150
Connection ~ 2500 2150
Wire Wire Line
	2100 2150 2500 2150
Connection ~ 2100 2150
Wire Wire Line
	6400 2600 8900 2600
Wire Wire Line
	8900 2600 8900 3950
Wire Wire Line
	6400 3200 7350 3200
Wire Wire Line
	8600 2950 7450 2950
Wire Wire Line
	8600 2950 8600 3950
Wire Wire Line
	7450 5250 7450 3450
Wire Wire Line
	4900 2400 4900 1300
Wire Wire Line
	4900 1050 4900 1300
Connection ~ 4900 1300
Wire Wire Line
	4900 1300 5000 1300
Connection ~ 5000 1300
Wire Wire Line
	5000 1300 5150 1300
Wire Wire Line
	5750 4750 5550 4750
Wire Wire Line
	4900 3500 4900 5000
Wire Wire Line
	4900 5000 5450 5000
Wire Wire Line
	5000 4500 5450 4500
Wire Wire Line
	5000 1300 5000 4500
Wire Wire Line
	6050 4200 6600 4200
Connection ~ 2600 3050
Wire Wire Line
	2600 3150 2600 3050
Wire Wire Line
	3150 2400 3350 2400
Wire Wire Line
	4900 2400 4300 2400
Wire Wire Line
	2550 1550 2550 1800
Wire Wire Line
	2550 1800 3100 1800
$Comp
L power:GND #PWR0104
U 1 1 5C52E918
P 5400 2700
F 0 "#PWR0104" H 5400 2450 50  0001 C CNN
F 1 "GND" H 5405 2527 50  0000 C CNN
F 2 "" H 5400 2700 50  0001 C CNN
F 3 "" H 5400 2700 50  0001 C CNN
	1    5400 2700
	0    1    1    0   
$EndComp
$Comp
L Device:Jumper_NC_Dual JP2
U 1 1 5C52F931
P 5450 4750
F 0 "JP2" V 5496 4852 50  0000 L CNN
F 1 "5V/12V Valve" V 5405 4852 50  0000 L CNN
F 2 "footprint:3p_switch_v2" H 5450 4750 50  0001 C CNN
F 3 "~" H 5450 4750 50  0001 C CNN
	1    5450 4750
	0    -1   -1   0   
$EndComp
$Comp
L Device:Jumper_NC_Dual JP1
U 1 1 5C52F9B9
P 7450 3200
F 0 "JP1" V 7404 3302 50  0000 L CNN
F 1 "Humidity/BatteryVolt" V 7495 3302 50  0000 L CNN
F 2 "footprint:3p_switch_v2" H 7450 3200 50  0001 C CNN
F 3 "~" H 7450 3200 50  0001 C CNN
	1    7450 3200
	0    1    1    0   
$EndComp
$Comp
L NNR-lib:5-12V_DCDC_Converter U1
U 1 1 5C53EC4A
P 3700 2600
F 0 "U1" H 3825 3025 50  0000 C CNN
F 1 "5-12V_DCDC_Converter" H 3825 2934 50  0000 C CNN
F 2 "footprint:5-12V_DCDCv4" H 3800 2450 50  0001 C CNN
F 3 "" H 3800 2450 50  0001 C CNN
	1    3700 2600
	1    0    0    -1  
$EndComp
Wire Wire Line
	3350 3500 3350 2400
Connection ~ 3350 2400
Wire Wire Line
	4300 3500 4900 3500
$Comp
L NNR-lib:TP4056 U6
U 1 1 5C4FADB2
P 3850 1550
F 0 "U6" H 3900 2193 60  0001 C CNN
F 1 "TP4056" H 3900 1981 60  0000 C CNN
F 2 "" H 3850 1450 60  0001 C CNN
F 3 "" H 3900 1981 60  0001 C CNN
	1    3850 1550
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0106
U 1 1 5C5D972A
P 3100 1850
F 0 "#PWR0106" H 3100 1600 50  0001 C CNN
F 1 "GND" H 3105 1677 50  0000 C CNN
F 2 "" H 3100 1850 50  0001 C CNN
F 3 "" H 3100 1850 50  0001 C CNN
	1    3100 1850
	1    0    0    -1  
$EndComp
Wire Wire Line
	3100 1850 3100 1800
Connection ~ 3100 1800
Wire Wire Line
	3100 1800 3450 1800
$Comp
L power:GND #PWR0107
U 1 1 5C5DB796
P 4350 1850
F 0 "#PWR0107" H 4350 1600 50  0001 C CNN
F 1 "GND" H 4355 1677 50  0000 C CNN
F 2 "" H 4350 1850 50  0001 C CNN
F 3 "" H 4350 1850 50  0001 C CNN
	1    4350 1850
	1    0    0    -1  
$EndComp
Wire Wire Line
	4350 1850 4350 1800
$EndSCHEMATC
