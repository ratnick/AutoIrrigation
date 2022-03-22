// This "sensor" is special, because it's located at another device (co_monitor).
// What this file does is establish serial comm to that device.
// 

#include "GasSensor.h"
#include "serialPortHandler.h"


void GasSensorClass::init(int _pinNbr, char _name[], int _muxChannel, SensorHandlerClass::SensorType _sensorType)
{
	// For serial comm to other Arduino:
	SetupSerialPort();

	strcpy(name, _name);
	pinNbr = _pinNbr;
	muxChannel = _muxChannel;
	sensorType = _sensorType;
	lastAnalogueReading = 0.0;

	pinMode(pinNbr, INPUT);
	//log	L::LogLine(2, __FUNCTION__, "MUX channel:" + String(muxChannel) + " analog pin:" + String(pinNbr) + " name:" + String(name));
}

const int jsonMaxSize = 500;
char jsonStr[jsonMaxSize];

boolean GasSensorClass::GetTelemetryJson(FirebaseJson *json) {
	FirebaseJsonData jsonData;

	L::LogLine(4, __FUNCTION__, "begin");
	if (ReadSerialJsonOnce(jsonStr, jsonMaxSize)) {
		L::LogLine(4, __FUNCTION__, "fetched from serial port");
		L::LogLine(4, __FUNCTION__, jsonStr);

		//	Serial.println(jsonStr);
		json->setJsonData(jsonStr);

		// assign internal variables
		json->get(jsonData, "cur_ppm");		this->gasTm.cur_CO_ppm_measurement = jsonData.doubleValue;
		json->get(jsonData, "last_ppm");	this->gasTm.last_CO_ppm_measurement = jsonData.doubleValue;
		json->get(jsonData, "sens_val");	this->gasTm.sens_val = jsonData.doubleValue;
		json->get(jsonData, "phase");		this->gasTm.phase = jsonData.intValue;
		L::LogLine(4, __FUNCTION__, "finished");
		return true;
	}
	L::LogLine(2, __FUNCTION__, "no telemetry received from device via serial port");
	return false;
}

float GasSensorClass::ConvertToPPM(float rawVal) {

	float RS_gas = 0;
	float ratio = 0;
	float sensorVolt = 0;
	float sensorVolt_5Vnorm = 0;
	float R0 = 340000;
	float MIN_V = 0.08;
	float MAX_V = 0.4;

	sensorVolt = (rawVal / 1024);
	sensorVolt_5Vnorm = (sensorVolt - MIN_V) * 5.0 / (MAX_V - MIN_V);
	RS_gas = (5.0 - sensorVolt_5Vnorm) / sensorVolt_5Vnorm;

	ratio = RS_gas / R0; //Replace R0 with the value found using the sketch above
	float x = 1538.46 * ratio;
	float ppm = pow(x, -1.709);
	Serial.printf("\nPPM: %f  RAW:%f  sensorVolt=%f  sensorVolt_5Vnorm=%f", ppm, rawVal, sensorVolt, sensorVolt_5Vnorm);
	return ppm;
}

float GasSensorClass::GetlastPPM() {
	return this->lastPPM;
}

float GasSensorClass::TestSensor() {
	float raw=0;
	for (int i = 0; i < 1; i++) {
		//this->ReadSerialJsonOnce();
		this->lastAnalogueReading = raw;
	}
	return raw;
}

/*
float ReadSensorNotUsed() {
	int raw = 0;
	float res = 0;
	int AVG_COUNT = 60;
	float ppm = 0;

	//log	L::LogLine(4, __FUNCTION__, "READING GAS LEVEL FROM analog MUX channel " + String(muxChannel));
	delayNonBlocking(10);  // allow voltage to settle after valve open
	AnalogMux.OpenChannel(muxChannel);
	for (int i = 0; i < AVG_COUNT; i++) {
		delayNonBlocking(1000);
		raw = analogRead(pinNbr);
		res += raw;
		//log 		L::LogLine(4, __FUNCTION__, "Raw Value: " + String(raw) );
	}
	AnalogMux.CloseMUXpwr();
	res /= AVG_COUNT;
	this->lastPPM = ConvertToPPM(res);
	//log L::LogLine(1, __FUNCTION__, "Raw Value: " + String(res) + "  =  " + this->lastPPM + " ppm" );
	return res;
}

*/

GasSensorClass GasSensor;

