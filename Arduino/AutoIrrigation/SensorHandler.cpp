// 
// 
// 

#include "SensorHandler.h"


void SensorHandlerClass::init(int _pinNbr, char _sensorType[], char _metricUnit[], boolean _isAnalogue, float _minConvertedValue, float _maxConvertedValue)
{
	pinNbr = _pinNbr;
	strcpy(sensorType, _sensorType);
	strcpy(metricUnit, _metricUnit);
	isAnalogue = _isAnalogue;
	minConvertedValue = _minConvertedValue;
	maxConvertedValue = _maxConvertedValue;
	pinMode(pinNbr, INPUT);

}

float SensorHandlerClass::ReadAnalogueValue() {
	if (!SIMULATE_SENSORS) {
		if (isAnalogue) {
		}
		else {
			return 0.0;
			Serial.println("*** ERROR in SensorHandlerClass::ReadAnalogueValue");
		}
	}
	else {
		return 0.78;
	}
}

boolean SensorHandlerClass::ReadDigitalValue() {
	if (!SIMULATE_SENSORS) {
		if (!isAnalogue) {
		}
		else {
			Serial.println("*** ERROR in SensorHandlerClass::ReadDigitalValue");
			return false;
		}
	}
	else {
		return true;
	}
}

SensorHandlerClass SensorHandler;

