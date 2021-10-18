// 
// 
// 

#include "SensorHandler.h"


void SensorHandlerClass::init(int _pinNbr, SensorType _sensorType, char _metricUnit[], boolean _isAnalogue, float _minConvertedValue, float _maxConvertedValue)
{
}

float ReadSensor() {
	return 0.77;
}

float TestSensor() {
	return 0.78;
}

float SensorHandlerClass::ReadAnalogueValue() {
	return 0.79;
}

boolean SensorHandlerClass::ReadDigitalValue() {
	return false;
}

SensorHandlerClass SensorHandler;

