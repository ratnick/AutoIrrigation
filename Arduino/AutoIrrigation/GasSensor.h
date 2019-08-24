// GasSensor.h

#ifndef _GASSENSOR_h
#define _GASSENSOR_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "SensorHandler.h"

class GasSensorClass
{
 public:
	 char name[15];
	 SensorHandlerClass::SensorType sensorType;

	 void init(int _pinNbr, char _name[], int _muxChannel, SensorHandlerClass::SensorType _sensorType);
	 float GetlastPPM();
	 float ReadSensor();
	 float ConvertToPPM(float rawVal);
	 float TestSensor();

private:
	int muxChannel;
	int pinNbr;
	float lastAnalogueReading; 
	float lastPPM;
	boolean lastDigitalReading;

};

extern GasSensorClass GasSensor;

#endif

