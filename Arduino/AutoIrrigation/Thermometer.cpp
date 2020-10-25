// 
// 
// 

#include "globals.h"
#include "Thermometer.h"
#include "AnalogMux.h"
#include "LogLib.h"
#include "PersistentMemory.h"

#define NBR_OF_SAMPLES 5  // how many samples to average over

// init called during setup(), i.e. after deep sleep and boot
void ThermometerClass::init(int _pinNbr, char _name[], int _muxChannel, SensorHandlerClass::SensorType _sensorType)
{
	strcpy(name, _name);
	pinNbr = _pinNbr;
	muxChannel = _muxChannel;

	SensorHandlerClass::SensorType sensorType = _sensorType;
	lastAnalogueReadingTemp = 0.0;

	if (this->sensorType == SensorHandlerClass::Thermometer) {
		pinMode(pinNbr, INPUT);
	}
	else {
		dht.setup(_pinNbr, DHTesp::DHT11); // Connect DHT sensor to GPIO DIGITAL_IN_PIN
	}
	LogLinef(3, __FUNCTION__, "MUX channel:%d analog pin:%d   name:%s  ", muxChannel, pinNbr, name);
}

void  ThermometerClass::AddTelemetryJson(FirebaseJson* json) {
	float val = this->ReadTemperature();
	this->tempTm.temperature = (double)val;
	json->add("Temp", this->tempTm.temperature);
	val = this->ReadHumidity();
	this->tempTm.humidity = (double)val;
	json->add("Hum", this->tempTm.humidity);
	this->tempTm.lastAnalogueReadingTemp = (int)this->GetlastAnalogueReadingTemperature();
	json->add("lastAnalog", this->tempTm.lastAnalogueReadingTemp);
	this->tempTm.lastAnalogueReadingHumidity = (int)this->GetlastAnalogueReadingHumidity();
	json->add("lastAnalog", this->tempTm.lastAnalogueReadingHumidity);
}


float ThermometerClass::ReadTemperature() {
	float res = 0.0;

	if (this->sensorType == SensorHandlerClass::Thermometer) {
		LogLinef(3, __FUNCTION__, "READING FROM analog MUX channel %d", muxChannel);
		AnalogMux.OpenChannel(muxChannel);
		res = analogRead(pinNbr);
		AnalogMux.CloseMUXpwr();
	}
	else {
		LogLinef(3, __FUNCTION__, "READING FROM digital thermometer pin %d", this->pinNbr);
		static const int bufSize = NBR_OF_SAMPLES;  // average over this amount of samples
		float readBuffer[bufSize];
		float sumRes = 0.0;

		for (int i = 0; i < bufSize; i++) {
			delay(dht.getMinimumSamplingPeriod() * 3);
			sumRes += dht.getTemperature();
		}
		res = sumRes / bufSize;
	}
	LogLinef(2, __FUNCTION__, "temperature = %f ", res);
	this->lastAnalogueReadingTemp = res;
	return (this->lastAnalogueReadingTemp);
}

float ThermometerClass::ReadHumidity() {
	float res = 0.0;

	if (this->sensorType == SensorHandlerClass::Thermometer) {
	}
	else {
		LogLinef(3, __FUNCTION__, "READING FROM digital thermometer pin %d", this->pinNbr);
		static const int bufSize = NBR_OF_SAMPLES;  // average over this amount of samples
		float readBuffer[bufSize];
		float sumRes = 0.0;
		for (int i = 0; i < bufSize; i++) {
			delay(dht.getMinimumSamplingPeriod() * 3);
			sumRes += dht.getHumidity();
		}
		res = sumRes / bufSize;
	}
	LogLinef(2, __FUNCTION__, "humidity = %f %   ", res);
	this->lastAnalogueReadingHumidity = res;
	return (this->lastAnalogueReadingHumidity);
}


float ThermometerClass::GetlastAnalogueReadingTemperature() {
	return this->lastAnalogueReadingTemp;
}

float ThermometerClass::GetlastAnalogueReadingHumidity() {
	return this->lastAnalogueReadingHumidity;
}

float ThermometerClass::TestSensor() {
	return this->ReadTemperature();
}




ThermometerClass Thermometer;

