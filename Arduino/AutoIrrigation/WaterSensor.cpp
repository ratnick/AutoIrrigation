// 
// 
// 

#include "globals.h"
#include "WaterSensor.h"
#include "AnalogMux.h"
#include "LogLib.h"

void WaterSensorClass::init(int _pinNbr, char _name[], int _muxChannel, SensorHandlerClass::SensorType _sensorType)
{
	strcpy(name, _name);
	pinNbr = _pinNbr;
	muxChannel = _muxChannel;
	sensorType = _sensorType;
	lastAnalogueReadingWater = 0.0;

	pinMode(pinNbr, INPUT);
	LogLine(2, __FUNCTION__, "MUX channel:" + String(muxChannel) + " analog pin:" + String(pinNbr) + " name:" + String(name));
}

float WaterSensorClass::ReadSensor() {
	int raw = 0;
	float res = 0;

	LogLine(4, __FUNCTION__, "READING HUMIDITY FROM analog MUX channel " + String(muxChannel));
	AnalogMux.OpenChannel(muxChannel);
	for (int i = 0; i < 8; i++) {
		raw = analogRead(pinNbr);
		LogLine(4, __FUNCTION__, "RAW HUM: " + String(raw));
		res += raw;
	}
	AnalogMux.CloseMUXpwr();
	res /= 8;
	LogLine(2, __FUNCTION__, "Value: " + String(res));
	return res;
}

boolean WaterSensorClass::CheckIfWater() {
	float res = 0;
	int raw = 0;
	boolean val;

	val = DRY;
	if (!SIMULATE_SENSORS) {
		res = this->ReadSensor();
		this->lastAnalogueReadingWater = res;
		switch (sensorType) {
			case SensorHandlerClass::SoilHumiditySensor:
				if (res > HUMIDITY_THRESHOLD) {
					Serial.println("  Return DRY");
				}
				else {
					Serial.println("  Return WET");
					val = WET;
				}
				break;
			case SensorHandlerClass::WaterSensor:
				Serial.println("*** ERROR: WaterSensor type is not yet implemented");
				break;
		}
	}
	return val;
}

float WaterSensorClass::GetlastAnalogueReadingWater() {
	return this->lastAnalogueReadingWater;
}

float WaterSensorClass::GetHumidity() {
	LogLine(1, __FUNCTION__, "values: " + String(WATER_VALUE) + "  " + String(this->lastAnalogueReadingWater) + 
							 "  " + String(DRY_VALUE) + "  " + String(this->lastAnalogueReadingWater - WATER_VALUE) +
							 "  " + String(DRY_VALUE - WATER_VALUE));
	return ((DRY_VALUE - this->lastAnalogueReadingWater) / (DRY_VALUE - WATER_VALUE) * 100.0);
}

float WaterSensorClass::TestSensor() {
	float raw;
	for (int i = 0; i < 1; i++) {
		raw = this->ReadSensor();
		this->lastAnalogueReadingWater = raw;
	}
	return raw;
}


WaterSensorClass WaterSensor;

