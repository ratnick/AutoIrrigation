// This "sensor" is special, because it's located at another device (co_monitor).
// What this file does is establish serial comm to that device.
// 

#include "globals.h"
#include "GasSensor.h"
#include "AnalogMux.h"
#include "LogLib.h"
#include <SoftwareSerial.h>

SoftwareSerial serialPort(D1, D2); // (Rx, Tx)


void GasSensorClass::init(int _pinNbr, char _name[], int _muxChannel, SensorHandlerClass::SensorType _sensorType)
{
	// For serial comm to Arduino:
	//serialPort.begin(9600);

	/*
	strcpy(name, _name);
	pinNbr = _pinNbr;
	muxChannel = _muxChannel;
	sensorType = _sensorType;
	lastAnalogueReading = 0.0;

	pinMode(pinNbr, INPUT);
	//log	LogLine(2, __FUNCTION__, "MUX channel:" + String(muxChannel) + " analog pin:" + String(pinNbr) + " name:" + String(name));
	*/
}

void GasSensorClass::ReadSerialJsonOnce(char jsonStr[]) {
	bool noDataReceived = true;
	int cnt = 0;
	int i = 0;

	while (noDataReceived && cnt++ < 20) {
		while (Serial.read() > 0) {
			jsonStr[i++] = Serial.read();
		}
		jsonStr[i] = '\0';
		if (jsonStr[0] == '\0') {
			serialPort.write('X');
			Serial.print(".");
		}
		else {
			Serial.println("JSON received and parsed");
			noDataReceived = false;
		}
		delay(500);
	}
}

/*
float ReadSensorNotUsed() {
	int raw = 0;
	float res = 0;
	int AVG_COUNT = 60;
	float ppm = 0;

	//log	LogLine(4, __FUNCTION__, "READING GAS LEVEL FROM analog MUX channel " + String(muxChannel));
	delay(10);  // allow voltage to settle after valve open
	AnalogMux.OpenChannel(muxChannel);
	for (int i = 0; i < AVG_COUNT; i++) {
		delay(1000);
		raw = analogRead(pinNbr);
		res += raw;
		//log 		LogLine(4, __FUNCTION__, "Raw Value: " + String(raw) );
	}
	AnalogMux.CloseMUXpwr();
	res /= AVG_COUNT;
	this->lastPPM = ConvertToPPM(res);
	//log LogLine(1, __FUNCTION__, "Raw Value: " + String(res) + "  =  " + this->lastPPM + " ppm" );
	return res;
}

*/
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
	float raw;
	for (int i = 0; i < 1; i++) {
		//this->ReadSerialJsonOnce();
		this->lastAnalogueReading = raw;
	}
	return raw;
}


GasSensorClass GasSensor;

