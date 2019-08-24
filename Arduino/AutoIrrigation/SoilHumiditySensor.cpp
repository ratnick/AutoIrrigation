// 
// 
// 

#include "globals.h"
#include "SoilHumiditySensor.h"
#include "AnalogMux.h"
#include "LogLib.h"

void SoilHumiditySensorClass::init(int _pinNbr, char _name[], int _muxChannel, SensorHandlerClass::SensorType _sensorType, int _humLimit)
{
	strcpy(name, _name);
	pinNbr = _pinNbr;
	muxChannel = _muxChannel;
	sensorType = _sensorType;
	lastAnalogueReadingWater = 0.0;
	humLimitPct = _humLimit;
	humLimit = WET_VALUE_HUMIDITY + humLimitPct * (DRY_VALUE_HUMIDITY - WET_VALUE_HUMIDITY) / 100.0;

	pinMode(pinNbr, INPUT);
	LogLinef(2, __FUNCTION__, "MUX channel:%d   analog pin:%d   name:%s", muxChannel, pinNbr, name);
}

void SoilHumiditySensorClass::SethumLimitPct(int _humLimitPct) {
	this->humLimitPct = _humLimitPct;
	this->humLimit = this->WET_VALUE_HUMIDITY + this->humLimitPct * (DRY_VALUE_HUMIDITY - WET_VALUE_HUMIDITY) / 100.0;
}

float SoilHumiditySensorClass::ReadSensor() {
	int raw = 0;
	float res = 0;
	int AVG_COUNT;
	if (sensorType == SensorHandlerClass::SoilHumiditySensor) {
		AVG_COUNT = 8;
	}
	else {
		AVG_COUNT = 8;
	}

	LogLinef(4, __FUNCTION__, "READING HUMIDITY FROM analog MUX channel %d", muxChannel);
	delay(500);  // allow voltage to settle after valve open
	AnalogMux.OpenChannel(muxChannel);
	for (int i = 0; i < AVG_COUNT; i++) {
		delay(50);  
		raw = analogRead(pinNbr);
		res += raw;
	}
	AnalogMux.CloseMUXpwr();
	res /= AVG_COUNT;

	LogLinef(1, __FUNCTION__, "Value: %d" , res);
	return res;
}

float SoilHumiditySensorClass::GetHumidityPct() {
	float hum;

	switch (sensorType) {
		case SensorHandlerClass::SoilHumiditySensor:
			hum = ((DRY_VALUE_HUMIDITY - this->lastAnalogueReadingWater) / (DRY_VALUE_HUMIDITY - WET_VALUE_HUMIDITY) * 100.0);
			break;
		case SensorHandlerClass::WaterSensor:
			hum = ((DRY_VALUE_WATER - this->lastAnalogueReadingWater) / (DRY_VALUE_WATER - WET_VALUE_WATER) * 100.0);
			break;
		break;
	}

	if (hum < 0) { hum = 0; }
	if (hum > 100) { hum = 100; }

	LogLinef(1, __FUNCTION__,
		"  water=%d  lastAna=%d  dry=%d  lastAna-water=%d  dry-water=%d  hum=%d  humLimPct=%d", 
		WET_VALUE_HUMIDITY,
		this->lastAnalogueReadingWater,
		DRY_VALUE_HUMIDITY,
		(this->lastAnalogueReadingWater - WET_VALUE_HUMIDITY),
		(DRY_VALUE_HUMIDITY - WET_VALUE_HUMIDITY),
		hum,
		this->humLimitPct); 
	return hum;
}

boolean SoilHumiditySensorClass::CheckIfWater() {
	float res = 0;
	int raw = 0;
	boolean val;

	val = DRY;
	if (!SIMULATE_SENSORS) {
		res = this->ReadSensor();
		this->lastAnalogueReadingWater = res;
		if (GetHumidityPct() >= this->humLimitPct) {
			LogLine(3, __FUNCTION__, "  Soil or reservoir is WET");
			val = WET;
		}
		else {
			LogLine(3, __FUNCTION__, "  Soil or reservoir is DRY");
			val = DRY;
		}
	}
	return val;
}

float SoilHumiditySensorClass::GetlastAnalogueReadingWater() {
	return this->lastAnalogueReadingWater;
}

float SoilHumiditySensorClass::TestSensor() {
	float raw;
	for (int i = 0; i < 1; i++) {
		raw = this->ReadSensor();
		this->lastAnalogueReadingWater = raw;
	}
	return raw;
}


SoilHumiditySensorClass SoilHumiditySensor;

