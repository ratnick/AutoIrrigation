// 
// 
// 

#include "globals.h"
#include "VoltMeter.h"

const float MAX_VOLTAGE = 4.2;
const float factor = MAX_VOLTAGE / 1024.0;  // 3.3 = max voltage on A0, https://arduinodiy.wordpress.com/2016/12/25/monitoring-lipo-battery-voltage-with-wemos-d1-minibattery-shield-and-thingspeak/
//const float factor = MAX_VOLTAGE * ((4.3 + 6.8) / 6.8) / 1023.0;  // 3.2 = max voltage on A0
														  // 4.3 and 6.8 are resistor values

// init called during setup(), i.e. after deep sleep and boot
void VoltMeterClass::init(int _pinNbr, char _name[], SensorHandlerClass::SensorType _sensorType, float _lastSummarizedReading)
{
	strcpy(name, _name);
	int pinNbr = _pinNbr;
	SensorHandlerClass::SensorType sensorType = _sensorType;
	lastAnalogueReading = 0.0;
	currentCnt = 0;
	if (_lastSummarizedReading > 0) {
		lastSummarizedReading = _lastSummarizedReading;
	} else {
		lastSummarizedReading = 0.0;
	}

	pinMode(pinNbr, INPUT);
	Serial.printf("\VoltMeterClass::init: pin:%d name:%s lastRead:%f \n", pinNbr, name, lastSummarizedReading);
	for (int i = 0; i < bufSize; i++) {
		readBuffer[i] = lastSummarizedReading;  //res;
	}
}

float VoltMeterClass::ReadVoltage() {
	float sumRes = 0.0;
	float res = analogRead(pinNbr);
	//Serial.printf("\VoltMeterClass::ReadVoltage: last analogue read:%f\n", res);
	lastAnalogueReading = res;
	res = res * factor;
	readBuffer[currentCnt] = res;
	for (int i=0; i < bufSize; i++) {
		sumRes += readBuffer[i];
	}
	lastSummarizedReading = sumRes / bufSize;

	currentCnt++;
	if (currentCnt >= bufSize) {
		currentCnt = 0;
	}

	return (lastSummarizedReading);
}

float VoltMeterClass::GetlastAnalogueReading() {
	return lastAnalogueReading;
}
float VoltMeterClass::GetlastSummarizedReading() {
	return lastSummarizedReading;
}


VoltMeterClass VoltMeter;

