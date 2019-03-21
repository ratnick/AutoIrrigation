// 
// 
// 

#include "globals.h"
#include "WaterSensor.h"

void WaterSensorClass::init(int _pinNbr, char _name[], SensorHandlerClass::SensorType _sensorType)
{
	strcpy(name, _name);
	pinNbr = _pinNbr;
	sensorType = _sensorType;
	lastAnalogueReading = 0.0;

	pinMode(pinNbr, INPUT);
	Serial.printf("\nWaterSensorClass::init: pin:%d name:%s\n", pinNbr, name);
}

boolean WaterSensorClass::CheckIfWater() {
	float res;
	if (!SIMULATE_SENSORS) {
		res = analogRead(pinNbr);
		lastAnalogueReading = res; 
		Serial.print("Reading: "); Serial.println(res);
		switch (sensorType) {

		case SensorHandlerClass::SoilHumiditySensor:
				if (res < HUMIDITY_THRESHOLD) {
					Serial.println("  Return NO_WATER");
					return NO_WATER;
				}
				else {
					Serial.println("  Return IS_WATER");
					return IS_WATER;
				}
				break;

		case SensorHandlerClass::WaterSensor:
				Serial.println("*** ERROR: WaterSensor type is not yet implemented");
				break;
		}
	} else {
		return NO_WATER;
	}
}

WaterSensorClass WaterSensor;

