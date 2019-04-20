// WaterSensor.h

#ifndef _WATERSENSOR_h
#define _WATERSENSOR_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "SensorHandler.h"

class WaterSensorClass
{
 public:
	 const boolean DRY = false;
	 const boolean WET = true;

	 char name[15];
	 SensorHandlerClass::SensorType sensorType;

	 void init(int _pinNbr, char _name[], int _muxChannel, SensorHandlerClass::SensorType _sensorType);
	 boolean CheckIfWater();
	 float GetlastAnalogueReadingWater();
	 float GetHumidity();
	 float ReadSensor();
	 float TestSensor();

private:
	const float DRY_VALUE = 930.0; // above this value we have detected water
	const float WATER_VALUE = 490.0; // above this value we have detected water
	const float HUMIDITY_THRESHOLD_PCT = 50.0;
	const float HUMIDITY_THRESHOLD = WATER_VALUE + HUMIDITY_THRESHOLD_PCT * (DRY_VALUE - WATER_VALUE) / 100.0;
	int muxChannel;
	int pinNbr;
	float lastAnalogueReadingWater;
	boolean lastDigitalReading;

};

extern WaterSensorClass WaterSensor;

#endif

