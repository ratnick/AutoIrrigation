// SensorHandler.h

#ifndef _SENSORHANDLER_h
#define _SENSORHANDLER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#define SIMULATE_SENSORS false

class SensorHandlerClass
{
 protected:

 public:
	 enum SensorType {
		 SoilHumiditySensor,
		 WaterSensor,
		 GasSensor,
		 ExternalVoltMeter
	 };

	 void init(int _pinNbr, char _sensorType[], char _metricUnit[], boolean _isBooleanSensor, float _minConvertedValue, float _maxConvertedValue);
	 float ReadAnalogueValue();
	boolean ReadDigitalValue();

	int pinNbr;
	char sensorType[10];
	char metricUnit[10];  
	boolean isAnalogue;
	float minConvertedValue;
	float maxConvertedValue;

 private:
};

extern SensorHandlerClass SensorHandler;

#endif

