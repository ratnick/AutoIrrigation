#include "DistanceSensor.h"
#include "AnalogMux.h"

void DistanceSensorClass::init(int _pinNbr, char _name[], int _muxChannel, SensorHandlerClass::SensorType _sensorType, int _distLimit)
{
	strcpy(name, _name);
	pinNbr = _pinNbr;
	muxChannel = _muxChannel;
	sensorType = _sensorType;
	lastAnalogueReadingDist = 0.0;
	distLimitCentimeter = _distLimit;

	pinMode(pinNbr, INPUT);
	L::LogLinef(2, __FUNCTION__, "MUX channel:%d   analog pin:%d   name:%s   _distLimit=%d    _sensorType=%d", muxChannel, pinNbr, name, _distLimit, this->sensorType);
}

void DistanceSensorClass::AddTelemetryJson(FirebaseJson* json) {
	//this->soilTm.distCm = (double)this->GetDistanceCentimeter()/1024.0*100;
	this->soilTm.distCm = (double)this->lastAnalogueReadingDist / 1024.0 * 100.0;
	json->add("Hum", this->soilTm.distCm);
}

void DistanceSensorClass::SetdistLimitPct(int _distLimitCentimeter) {
	this->distLimitCentimeter = _distLimitCentimeter;
}

float DistanceSensorClass::ReadSensor() {
	float raw = 0.0;
	float res = 0;
	int AVG_COUNT;
	if (sensorType == SensorHandlerClass::DistanceSensor) {
		AVG_COUNT = 5;
	}
	else {
		AVG_COUNT = 3;
	}

	L::LogLinef(3, __FUNCTION__, "READING DISTANCE FROM analog MUX channel %d pin %d", muxChannel, this->pinNbr);
	AnalogMux.OpenChannel(muxChannel);
	delayNonBlocking(100);
	for (int i = 0; i < AVG_COUNT; i++) {
		delayNonBlocking(50);
		raw = analogRead(this->pinNbr);
		L::LogLinef(5, __FUNCTION__, "Value: %f", raw);
		res += raw;
	}
	//	delayNonBlocking(100);
	AnalogMux.CloseMUXpwr();
	res /= AVG_COUNT;
	this->lastAnalogueReadingDist = res;

	L::LogLinef(2, __FUNCTION__, "Distance raw value: %f", res);
	return res;
}

float DistanceSensorClass::GetDistanceCentimeter() {

	float adjFactor = 1.8;   // rough measure
	float cm = 0;
	switch (this->sensorType) {
		case SensorHandlerClass::DistanceSensor:
			L::LogLinef(5, __FUNCTION__, "lastAnalogueReadingDist Value: %f", this->lastAnalogueReadingDist);
/*			cm = adjFactor * pow(3027.4 / this->lastAnalogueReadingDist, 1.2134); //convert readings to distance(cm)
			if (cm < 10) { cm = 10; }
			if (cm > 120) { cm = 120; }
*/
			cm = this->lastAnalogueReadingDist;
			break;
		default:
			L::LogLine(1, __FUNCTION__, "unexpected sensor type");
			break;
	}

	L::LogLinef(3, __FUNCTION__, "  raw=%f  cm=%f", this->lastAnalogueReadingDist, cm);
	return cm;
}

boolean DistanceSensorClass::CheckIfWater() {
	float res = 0;
	boolean val;

	val = DRY;
	if (!SIMULATE_SENSORS) {
		res = this->ReadSensor();
		this->lastAnalogueReadingDist = res;
		if (GetDistanceCentimeter() <= this->distLimitCentimeter) {   // the greater distance, the less water
			L::LogLine(3, __FUNCTION__, "  Soil or reservoir is WET");
			val = WET;
		}
		else {
			L::LogLine(3, __FUNCTION__, "  Soil or reservoir is DRY");
			val = DRY;
		}
	}
	return val;
}

float DistanceSensorClass::GetlastAnalogueReadingDist() {
	return this->lastAnalogueReadingDist;
}

float DistanceSensorClass::TestSensor() {
	
	float raw = 0.0;
	float res = 0;
	int AVG_COUNT;
	if (sensorType == SensorHandlerClass::DistanceSensor) {
		AVG_COUNT = 5;
	}
	else {
		AVG_COUNT = 3;
	}

	L::LogLinef(3, __FUNCTION__, "READING DISTANCE FROM analog MUX channel %d pin %d", muxChannel, this->pinNbr);
	AnalogMux.OpenChannel(muxChannel);
	delayNonBlocking(100);
	for (int i = 0; i < AVG_COUNT; i++) {
		delayNonBlocking(50);
		raw = analogRead(this->pinNbr);
		L::LogLinef(5, __FUNCTION__, "Value: %f", raw);
		res += raw;
	}
//	delayNonBlocking(100);
	AnalogMux.CloseMUXpwr();
	res /= AVG_COUNT;
	this->lastAnalogueReadingDist = res;
	L::LogLinef(1, __FUNCTION__, "Raw average Value: %f", res);
	return res;

}




DistanceSensorClass DistanceSensor;

