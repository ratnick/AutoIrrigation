// VoltMeter.h

#ifndef _VOLTMETER_h
#define _VOLTMETER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "SensorHandler.h"

class VoltMeterClass
{
 protected:

 public:
	 int pinNbr;
	 char name[15];
	 SensorHandlerClass::SensorType sensorType;

	 void init(int _pinNbr, char _name[], SensorHandlerClass::SensorType _sensorType, float _lastSummarizedReading);
	 float ReadVoltage();
	 float GetlastAnalogueReading();
	 float GetlastSummarizedReading();
	 float lastAnalogueReading;
	 float lastSummarizedReading;

private:
	static const int bufSize = 10;
	float readBuffer[bufSize];
	int currentCnt = 0;
};

extern VoltMeterClass VoltMeter;

#endif

