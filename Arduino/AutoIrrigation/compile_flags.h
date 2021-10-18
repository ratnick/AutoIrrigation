#pragma once


//#define COMPILE_SETUP_IRRIGATION
//#define COMPILE_SETUP_DISTANCE_SENSOR
//#define COMPILE_SETUP_SOIL_SENSOR
#define COMPILE_SETUP_DHT11_SENSOR
//#define COMPILE_SETUP_GAS_SENSOR

// general flags used during development and debugging
#define DEBUGLEVEL 5					// Has dual function: 1) serving as default value for Firebase Logging (which can be modified at runtime). 2) Defining debug level on serial port.
#define FORCE_NEW_VALUES false			// [default:false] True will overwrite all values in persistent memory. 
										// To use: Enable once, disable and recompile
//#define USE_DEEP_SLEEP				   // [true] When enabling, connect D0 to RST (on Wemos D1 mini)
//#define RUN_ONCE					   // Debug mode: no looping, just execute once


const String RUNMODE_SOIL = "soil";
const String RUNMODE_DISTANCE = "dist";
const String RUNMODE_WATER = "water";
const String RUNMODE_GAS = "gas";
const String RUNMODE_DHT11 = "Temp+hum";
const String RUNMODE_SENSORTEST = "sensor";
const String RUNMODE_BATTERYTEST = "batt";
const String RUNMODE_HARDWARETEST = "testhw";

// ====================================================
#ifdef COMPILE_SETUP_IRRIGATION
	#define HARDWARE_DESCRIPTION "WeMOS D1 r2, Solar panel, soil sensor"
	const String DefaultRunmode = RUNMODE_SOIL;
	#define DEVICE_ID "#1"
	#define USE_WIFI
	#define USE_FIREBASE
	// Program control:
	#define NBR_OF_LOOPS_BEFORE_SLEEP 2    // [1] How many times will we perform a standard main loop before potentially sleeping
	#define LOOP_DELAY 10                  // [10] secs
	#define TOTAL_SECS_TO_SLEEP 20		   // [20] Default sleep time
	#define DEEP_SLEEP_SOAK_THRESHOLD 120  // [120] if soaking time exceeds this limit, we will use deep sleep instead of delay()
	#define SLEEP_WHEN_LOW_VOLTAGE		   // as the first thing, measure battery voltage. If too low, go immediately to sleep without connecting to wifi.
	#define MEASURE_INTERNAL_VCC         // When enabling, we cannot use analogue reading of sensor. 

	// Development & debugging
	#define SIMULATE_WATERING false        // open the valve in every loop
	// See also SIMULATE_SENSORS in SensorHandler.h

	//Hardware pin configuration on WeMOS D1
	const int MUX_S0 = D8; // S0 = Pin A on schematic
	const int MUX_S1 = D7; // S1 = Pin B on schematic
	const int HUM_SENSOR_PWR_CTRL = D1;
	const int VALVE_CTRL = D5;
	const int ANALOG_INPUT = A0;
	// MUX channels X0, X1, X2... on schematic
	const int CHANNEL_HUM = 0;
	const int CHANNEL_BATT = 1;
	const int CHANNEL_WATER = 2;

	// Not relevant for this device but still needed for compilation
	const int CHANNEL_TEMPERATURE = 2;
	const int DHT11_SIGNAL_PIN = D6;

#endif //COMPILE_SETUP_IRRIGATION

	// ====================================================
#ifdef COMPILE_SETUP_DISTANCE_SENSOR
	#define USE_DISTANCE_SENSOR 
	const String DefaultRunmode = RUNMODE_DISTANCE;
	//const String DefaultRunmode = RUNMODE_SENSORTEST;
	#define HARDWARE_DESCRIPTION "WeMOS D1 r2, Distance, solar"
	#define DEVICE_ID "Dist_#1"
	#define USE_WIFI
	#define USE_FIREBASE
	#define NBR_OF_LOOPS_BEFORE_SLEEP 1    // [1] How many times will we perform a standard main loop before potentially sleeping
	#define LOOP_DELAY 2                   // [10] secs
	#define TOTAL_SECS_TO_SLEEP 5		   // [20] Default sleep time
	#define SLEEP_WHEN_LOW_VOLTAGE		   // as the first thing, measure battery voltage. If too low, go immediately to sleep without connecting to wifi.

	// Not relevant for this device but still needed for compilation
	#define DEEP_SLEEP_SOAK_THRESHOLD 120  // [120] if soaking time exceeds this limit, we will use deep sleep instead of delay()
	#define SIMULATE_WATERING true

	//Hardware pin configuration on WeMOS D1
	const int MUX_S0 = D8; // S0 = Pin A on schematic
	const int MUX_S1 = D7; // S1 = Pin B on schematic
	const int HUM_SENSOR_PWR_CTRL = D1;
	const int VALVE_CTRL = D5;
	const int ANALOG_INPUT = A0;
	// MUX channels X0, X1, X2... on schematic
	const int CHANNEL_HUM = 0;
	const int CHANNEL_BATT = 1;
	const int CHANNEL_WATER = 2;
	const int CHANNEL_TEMPERATURE = 2;
#endif  //COMPILE_SETUP_DISTANCE_SENSOR

#ifdef COMPILE_SETUP_SOIL_SENSOR
	const String DefaultRunmode = RUNMODE_SOIL;
	#define HARDWARE_DESCRIPTION "WeMOS D1 r2, DHT11, 5V"
	#define DEVICE_ID "Temp_#8"
	#define USE_WIFI
	#define USE_FIREBASE
	#define NBR_OF_LOOPS_BEFORE_SLEEP 1    // [1] How many times will we perform a standard main loop before potentially sleeping
	#define LOOP_DELAY 3                   // [10] secs
	#define TOTAL_SECS_TO_SLEEP 20		   // [20] Default sleep time

	// Not relevant for this device but still needed for compilation
	#define DEEP_SLEEP_SOAK_THRESHOLD 120  // [120] if soaking time exceeds this limit, we will use deep sleep instead of delay()
	#define SIMULATE_WATERING true

	//Hardware pin configuration on WeMOS D1
	const int MUX_S0 = D8; // S0 = Pin A on schematic
	const int MUX_S1 = D7; // S1 = Pin B on schematic
	const int HUM_SENSOR_PWR_CTRL = D1;
	const int VALVE_CTRL = D5;
	const int ANALOG_INPUT = A0;
	// MUX channels X0, X1, X2... on schematic
	const int CHANNEL_HUM = 0;
	const int CHANNEL_BATT = 1;
	const int CHANNEL_WATER = 2;
	const int CHANNEL_TEMPERATURE = 2;
#endif  //COMPILE_SETUP_SOIL_SENSOR


// ====================================================
#ifdef COMPILE_SETUP_DHT11_SENSOR
	#define USE_DHT11_SENSOR						// temperature, hum (defined by RUNMODE)
	const String DefaultRunmode = "Temp+hum";
	#define HARDWARE_DESCRIPTION "WeMOS D1 r2, DHT11, 5V"
	#define DEVICE_ID "Temp_#7"
	#define USE_WIFI
	#define USE_FIREBASE
	#define NBR_OF_LOOPS_BEFORE_SLEEP 1    // [1] How many times will we perform a standard main loop before potentially sleeping
	#define LOOP_DELAY 3                   // [10] secs
	#define TOTAL_SECS_TO_SLEEP 20		   // [20] Default sleep time
	//#define SLEEP_WHEN_LOW_VOLTAGE		   // as the first thing, measure battery voltage. If too low, go immediately to sleep without connecting to wifi.


	// Not relevant for this device but still needed for compilation
	#define DEEP_SLEEP_SOAK_THRESHOLD 120  // [120] if soaking time exceeds this limit, we will use deep sleep instead of delay()
	#define SIMULATE_WATERING false        
	const int DHT11_SIGNAL_PIN = D6;

	//Hardware pin configuration on WeMOS D1
	const int MUX_S0 = D8; // S0 = Pin A on schematic
	const int MUX_S1 = D7; // S1 = Pin B on schematic
	const int HUM_SENSOR_PWR_CTRL = D1;
	const int VALVE_CTRL = D5;
	const int ANALOG_INPUT = A0;
	// MUX channels X0, X1, X2... on schematic
	const int CHANNEL_HUM = 0;
	const int CHANNEL_BATT = 1;
	const int CHANNEL_WATER = 2;
	const int CHANNEL_TEMPERATURE = 2;
#endif  //COMPILE_SETUP_DHT11_SENSOR


// ====================================================
#ifdef COMPILE_SETUP_GAS_SENSOR
	#define USE_GAS_SENSOR 
	#define HARDWARE_DESCRIPTION "WeMOS D1 r2, CO-sensor, 5V"
	const String DefaultRunmode = RUNMODE_GAS;
	#define DEVICE_ID "#10"
	#define USE_WIFI
	#define USE_FIREBASE

	// Program control:
	#define NBR_OF_LOOPS_BEFORE_SLEEP 2    // [1] How many times will we perform a standard main loop before potentially sleeping
	#define LOOP_DELAY 10                  // [10] secs
	#define TOTAL_SECS_TO_SLEEP 20		   // [20] Default sleep time
	#define DEEP_SLEEP_SOAK_THRESHOLD 120  // [120] if soaking time exceeds this limit, we will use deep sleep instead of delay()
	//#define SLEEP_WHEN_LOW_VOLTAGE		   // as the first thing, measure battery voltage. If too low, go immediately to sleep without connecting to wifi.
	//#define MEASURE_INTERNAL_VCC         // When enabling, we cannot use analogue reading of sensor. 

	//Hardware pin configuration on WeMOS D1
	const int MUX_S0 = D8; // S0 = Pin A on schematic
	const int MUX_S1 = D7; // S1 = Pin B on schematic
	const int HUM_SENSOR_PWR_CTRL = D1;
	const int VALVE_CTRL = D5;
	const int ANALOG_INPUT = A0;
	// MUX channels X0, X1, X2... on schematic
	const int CHANNEL_HUM = 0;
	const int CHANNEL_BATT = 1;
	const int CHANNEL_WATER = 2;
	const int CHANNEL_TEMPERATURE = 2;


	// Not relevant for this device but still needed for compilation
	#define SIMULATE_WATERING false        // open the valve in every loop
	const int DHT11_SIGNAL_PIN = D6;


#endif //COMPILE_SETUP_GAS_SENSOR



// Old, unused compile flags (not sure if they work anymore)
// Shich cloud to use
//#define USE_GOOGLE_CLOUD
//#define USE_AZURE
