// 
// 
// 

#include "globals.h"
#include "WaterSensor.h"
#include "LogLib.h"

void WaterSensorClass::init(int _pinNbr, char _name[], SensorHandlerClass::SensorType _sensorType, int _pwrCtrlPin, boolean _pwrCtrlPinActive)
{
	strcpy(name, _name);
	pinNbr = _pinNbr;
	pwrCtrlPin = _pwrCtrlPin;
	sensorType = _sensorType;
	lastAnalogueReading = 0.0;

	pinMode(pinNbr, INPUT);
	if (pwrCtrlPin > 0) {
		pinMode(pwrCtrlPin, OUTPUT);
		pwrCtrlPinActive = _pwrCtrlPinActive;
		digitalWrite(pwrCtrlPin, !pwrCtrlPinActive);
	}
	Serial.printf("\nWaterSensorClass::init: pin:%d name:%s\n", pinNbr, name);
}

boolean WaterSensorClass::CheckIfWater() {
	float res;
	boolean val;

	val = NO_WATER;
	if (!SIMULATE_SENSORS) {

		if (pwrCtrlPin > 0) {
			LogLine(3, __FUNCTION__, "activate");
			digitalWrite(pwrCtrlPin, pwrCtrlPinActive);
			delay(300);
		}

		res = analogRead(pinNbr);
		lastAnalogueReading = res; 
		Serial.print("Reading: "); Serial.println(res);
		switch (sensorType) {

		case SensorHandlerClass::SoilHumiditySensor:
				if (res < HUMIDITY_THRESHOLD) {
					Serial.println("  Return NO_WATER");
				}
				else {
					Serial.println("  Return IS_WATER");
					val = IS_WATER;
				}
				break;

		case SensorHandlerClass::WaterSensor:
				Serial.println("*** ERROR: WaterSensor type is not yet implemented");
				break;
		}

		if (pwrCtrlPin > 0) {
			digitalWrite(pwrCtrlPin, !pwrCtrlPinActive);
			LogLine(3, __FUNCTION__, "de-activate");
		}

	}
	return val;
}

WaterSensorClass WaterSensor;

