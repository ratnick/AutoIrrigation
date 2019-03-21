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
 protected:

 public:
	 const boolean NO_WATER = false;
	 const boolean IS_WATER = true;
	 const float HUMIDITY_THRESHOLD = 30; // above this value we have detected water

	 int pinNbr;
	 char name[15];
	 SensorHandlerClass::SensorType sensorType;
	 float lastAnalogueReading;
	 boolean lastDigitalReading;

	 void init(int _pinNbr, char _name[], SensorHandlerClass::SensorType _sensorType);
	 boolean CheckIfWater();

private:

};

extern WaterSensorClass WaterSensor;

#endif

