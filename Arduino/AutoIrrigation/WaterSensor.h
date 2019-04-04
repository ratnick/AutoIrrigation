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
	 int pwrCtrlPin; // if a pin is used to control power relay / optocoupler or similar. Set to zero if unused
	 boolean pwrCtrlPinActive;  // which value is used to activate the relay (true or false)

	 char name[15];
	 SensorHandlerClass::SensorType sensorType;
	 float lastAnalogueReading;
	 boolean lastDigitalReading;

	 void init(int _pinNbr, char _name[], SensorHandlerClass::SensorType _sensorType, int _pwrCtrlPin, boolean _pwrCtrlPinActive);
	 boolean CheckIfWater();

private:

};

extern WaterSensorClass WaterSensor;

#endif

