// 
// 
// 
 
#include "globals.h"
#include "VoltMeter.h"
#include "AnalogMux.h"
#include "LogLib.h"

const float MAX_VOLTAGE = 4.0;
const float factor = MAX_VOLTAGE / 1024.0;  // 3.3 = max voltage on A0, https://arduinodiy.wordpress.com/2016/12/25/monitoring-lipo-battery-voltage-with-wemos-d1-minibattery-shield-and-thingspeak/
//const float factor = MAX_VOLTAGE * ((4.3 + 6.8) / 6.8) / 1023.0;  // 3.2 = max voltage on A0
														  // 4.3 and 6.8 are resistor values

// init called during setup(), i.e. after deep sleep and boot
void VoltMeterClass::init(int _pinNbr, char _name[], int _muxChannel, SensorHandlerClass::SensorType _sensorType, float _lastSummarizedReading)
{
	strcpy(name, _name);
	pinNbr = _pinNbr;
	muxChannel = _muxChannel;

	SensorHandlerClass::SensorType sensorType = _sensorType;
	lastAnalogueReadingVoltage = 0.0;
	currentCnt = 0;
	if (_lastSummarizedReading > 0) {
		lastSummarizedReading = _lastSummarizedReading;
	} else {
		lastSummarizedReading = 0.0;
	}

	pinMode(pinNbr, INPUT);
	LogLinef(2, __FUNCTION__, "MUX channel:%d analog pin:%d   name:%s  lastSummarizedReading:%f", muxChannel, pinNbr, name, lastSummarizedReading);
	for (int i = 0; i < bufSize; i++) {
		readBuffer[i] = lastSummarizedReading;  //res;
	}
}

float VoltMeterClass::ReadVoltage() {
	float sumRes = 0.0;
	LogLinef(3, __FUNCTION__, "READING FROM analog MUX channel %d", muxChannel);
	AnalogMux.OpenChannel(muxChannel);
	float res = analogRead(pinNbr);
	AnalogMux.CloseMUXpwr();

	LogLinef(2, __FUNCTION__, "Value: %f   converted:%f", res, (res*factor));
	this->lastAnalogueReadingVoltage = res;
	res = res * factor;
	readBuffer[currentCnt] = res;
	for (int i=0; i < bufSize; i++) {
		sumRes += readBuffer[i];
	}
	this->lastSummarizedReading = sumRes / bufSize;

	currentCnt++;
	if (currentCnt >= bufSize) {
		currentCnt = 0;
	}

	return (this->lastSummarizedReading);
}

float VoltMeterClass::GetlastAnalogueReadingVoltage() {
	return this->lastAnalogueReadingVoltage;
}
float VoltMeterClass::GetlastSummarizedReading() {
	return this->lastSummarizedReading;
}

float VoltMeterClass::TestSensor() {
	return this->ReadVoltage();
}


VoltMeterClass VoltMeter;

