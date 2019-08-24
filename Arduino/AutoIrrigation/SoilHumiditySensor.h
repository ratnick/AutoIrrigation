// SoilHumiditySensor.h

#ifndef _SOILHUMIDITYSENSOR_h
#define _SOILHUMIDITYSENSOR_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "SensorHandler.h"

class SoilHumiditySensorClass
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
	const float DRY_VALUE_HUMIDITY = 930.0; // above this value we have detected water
	const float WET_VALUE_HUMIDITY = 490.0; // above this value we have detected water
	const float DRY_VALUE_WATER =   0.0; // above this value we have detected water
	const float WET_VALUE_WATER = 900.0; // above this value we have detected water
	float humLimitPct;
	float humLimit;
	int muxChannel;
	int pinNbr;
	float lastAnalogueReadingWater;
	boolean lastDigitalReading;

};

extern SoilHumiditySensorClass SoilHumiditySensor;

#endif

