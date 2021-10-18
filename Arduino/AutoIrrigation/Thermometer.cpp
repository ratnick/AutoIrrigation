// 
// 
// 

#include "Thermometer.h"
#include "AnalogMux.h"
#include "PersistentMemory.h"

#define NBR_OF_SAMPLES 1  // how many samples to average over

// init called during setup(), i.e. after deep sleep and boot
void ThermometerClass::init(int _pinNbr, char _name[], int _muxChannel, SensorHandlerClass::SensorType _sensorType)
{
	strcpy(name, _name);
	pinNbr = _pinNbr;
	muxChannel = _muxChannel;

	SensorHandlerClass::SensorType sensorType = _sensorType;
	lastAnalogueReadingTemp = 0.0;
	lastAnalogueReadingHumidity = 0.0;

	if (this->sensorType == SensorHandlerClass::Thermometer) {
		pinMode(pinNbr, INPUT);
	}
	else {
		dht.setup(_pinNbr, DHTesp::AUTO_DETECT); // Connect DHT sensor to GPIO DIGITAL_IN_PIN
	}
	LogLinef(3, __FUNCTION__, "MUX channel:%d analog pin:%d   name:%s  ", muxChannel, pinNbr, name);
}

void  ThermometerClass::AddTelemetryJson(FirebaseJson* json) {
	float val = this->ReadTemperature();
	if (!isnan(val)) {
		this->tempTm.temperature = (double)val;
		LogLinef(4, __FUNCTION__, " temp raw val=%f    tempTm.temperature=%f", val, tempTm.temperature);
		json->add("Temp", this->tempTm.temperature);

		this->tempTm.lastAnalogueReadingTemp = (int)this->GetlastAnalogueReadingTemperature();
		json->add("lastAnalog", this->tempTm.lastAnalogueReadingTemp);

		val = this->ReadHumidity();
		if (!isnan(val)) {
			this->tempTm.humidity = (double)val;
			json->add("Hum", this->tempTm.humidity);

			this->tempTm.lastAnalogueReadingHumidity = (int)this->GetlastAnalogueReadingHumidity();
			json->add("lastAnalog", this->tempTm.lastAnalogueReadingHumidity);
		}
		else {
			LogLine(0, __FUNCTION__, "NO DHT SENSOR READING HUMIDITY ONLY. Is it connected at all?");
		}

	}
	else {
		LogLine(0, __FUNCTION__, "NO DHT SENSOR READING. Is it connected at all?");
	}
}

float ConvertDHT22NegativeReading(float in) {
	LogLinef(3, __FUNCTION__, "in=%f", in);
	if (in < 0) {
		return -(in + 3276.8);

	}
	else {
		return in;
	}
}

float ThermometerClass::ReadTemperature() {
	float res = 0.0;
	float sumRes;
	int bufSize = NBR_OF_SAMPLES;  // average over this amount of samples

	if (this->sensorType == SensorHandlerClass::Thermometer) {
		LogLinef(3, __FUNCTION__, "READING TEMPERATURE FROM analog MUX channel %d", muxChannel);
		AnalogMux.OpenChannel(muxChannel);
		res = analogRead(pinNbr);
		AnalogMux.CloseMUXpwr();
	}
	else {
		LogLinef(3, __FUNCTION__, "READING TEMPERATURE FROM digital thermometer pin %d", this->pinNbr);
		sumRes = 0.0;

		for (int i = 0; i < bufSize; i++) {
			delayNonBlocking(dht.getMinimumSamplingPeriod() * 3);
			sumRes += ConvertDHT22NegativeReading(dht.getTemperature());
		}
		res = sumRes / bufSize;
	}

	LogLinef(2, __FUNCTION__, "temperature = %f ", res);
	this->lastAnalogueReadingTemp = res;
	return (this->lastAnalogueReadingTemp);
}

float ThermometerClass::ReadHumidity() {
	float res = 0.0;
	float sumRes;
	int bufSize = NBR_OF_SAMPLES;  // average over this amount of samples

	if (this->sensorType == SensorHandlerClass::Thermometer) {
	}
	else {
		LogLinef(3, __FUNCTION__, "READING HUMIDITY FROM digital thermometer pin %d", this->pinNbr);
		sumRes = 0.0;
		for (int i = 0; i < bufSize; i++) {
			delayNonBlocking(dht.getMinimumSamplingPeriod() * 3);
			sumRes += dht.getHumidity();
		}
		res = sumRes / bufSize;
	}
	LogLinef(1, __FUNCTION__, "humidity = %f pct", (double) res);
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

