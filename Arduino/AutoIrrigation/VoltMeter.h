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
	 SensorHandlerClass::SensorType sensorType;

	 void init(int _pinNbr, char _name[], int _muxChannel, SensorHandlerClass::SensorType _sensorType, float _lastSummarizedReading);
	 float ReadVoltage();
	 float GetlastAnalogueReadingVoltage();
	 float GetlastSummarizedReading();
	 float lastSummarizedReading;
	 float TestSensor();

private:
	int muxChannel;
	int pinNbr;
	char name[15];
	static const int bufSize = 10;
	float readBuffer[bufSize];
	int currentCnt = 0;
	float lastAnalogueReadingVoltage;

};

extern VoltMeterClass VoltMeter;

#endif

