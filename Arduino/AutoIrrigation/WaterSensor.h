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

	 void init(int _pinNbr, char _name[], int _muxChannel, SensorHandlerClass::SensorType _sensorType, int _humLimit);
	 boolean CheckIfWater();
	 float GetlastAnalogueReadingWater();
	 float GetHumidityPct();
	 float ReadSensor();
	 float TestSensor();
	 void SethumLimitPct(int _humLimitPct);

private:
	const float DRY_VALUE = 930.0; // above this value we have detected water
	const float WATER_VALUE = 490.0; // above this value we have detected water
	float humLimitPct;
	float humLimit;
	int muxChannel;
	int pinNbr;
	float lastAnalogueReadingWater;
	boolean lastDigitalReading;

};

extern WaterSensorClass WaterSensor;

#endif

