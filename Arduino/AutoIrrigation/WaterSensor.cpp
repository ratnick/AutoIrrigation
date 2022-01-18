// 
// 
// 

#include "globals.h"
#include "WaterSensor.h"
#include "AnalogMux.h"
#include "LogLib.h"

void WaterSensorClass::init(int _pinNbr, char _name[], int _muxChannel, SensorHandlerClass::SensorType _sensorType, int _humLimit)
{
	strcpy(name, _name);
	pinNbr = _pinNbr;
	muxChannel = _muxChannel;
	sensorType = _sensorType;
	lastAnalogueReadingWater = 0.0;
	humLimitPct = _humLimit;
	humLimit = WATER_VALUE + humLimitPct * (DRY_VALUE - WATER_VALUE) / 100.0;

	pinMode(pinNbr, INPUT);
//	LogLine(2, __FUNCTION__, "MUX channel:" + String(muxChannel) + " analog pin:" + String(pinNbr) + " name:" + String(name));
}

void WaterSensorClass::SethumLimitPct(int _humLimitPct) {
	this->humLimitPct = _humLimitPct;
	this->humLimit = this->WATER_VALUE + this->humLimitPct * (DRY_VALUE - WATER_VALUE) / 100.0;
}

float WaterSensorClass::ReadSensor() {
	int raw = 0;
	float res = 0;

//	LogLine(4, __FUNCTION__, "READING HUMIDITY FROM analog MUX channel " + String(muxChannel));
	delay(500);  // allow voltage to settle after valve open
	AnalogMux.OpenChannel(muxChannel);
	for (int i = 0; i < 8; i++) {
		delay(50);  
		raw = analogRead(pinNbr);
		res += raw;
	}
	AnalogMux.CloseMUXpwr();
	res /= 8;
//	LogLine(2, __FUNCTION__, "Value: " + String(res));
	return res;
}

float WaterSensorClass::GetHumidityPct() {
	float hum;
	hum = ((DRY_VALUE - this->lastAnalogueReadingWater) / (DRY_VALUE - WATER_VALUE) * 100.0);
	if (hum < 0) { hum = 0; }
	if (hum > 100) { hum = 100; }
/*	LogLine(3, __FUNCTION__,
		"  water=" + String(WATER_VALUE) +
		"  lastAna=" + String(this->lastAnalogueReadingWater) +
		"  dry=" + String(DRY_VALUE) +
		"  lastAna-water=" + String(this->lastAnalogueReadingWater - WATER_VALUE) +
		"  dry-water=" + String(DRY_VALUE - WATER_VALUE) +
		"  hum=" + String(hum) +
		"  humLimPct=" + String(this->humLimitPct));
*/	return hum;
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
				if (GetHumidityPct() >= this->humLimitPct) {
					LogLine(3, __FUNCTION__, "  Return WET");
					val = WET;
				}
				else {
					LogLine(3, __FUNCTION__, "  Return DRY");
				}
				break;
			case SensorHandlerClass::WaterSensor:
				LogLine(0, __FUNCTION__, "*** ERROR: WaterSensor type is not yet implemented");
				break;
		}
	}
	return val;
}

float WaterSensorClass::GetlastAnalogueReadingWater() {
	return this->lastAnalogueReadingWater;
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

