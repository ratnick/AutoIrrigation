// 
// 
// 

#include "WaterValve.h"
#include "LogLib.h"

void WaterValveClass::init(int _pinNbr, char _name[], int _openSeconds, int _soakSeconds) {
	strcpy(name, _name);
	pinNbr = _pinNbr;
	openSeconds = _openSeconds;
	soakSeconds = _soakSeconds;
	lastOpenTimestamp = "never";
	valveState = 0;

	pinMode(LED_BUILTIN, OUTPUT);
	for (int i = 0; i < 5; i++) {
		digitalWrite(LED_BUILTIN, LOW);
		digitalWrite(LED_BUILTIN, HIGH);
	}
	pinMode(pinNbr, OUTPUT);
	Serial.printf("\nWaterValveClass::init: pin:%d name:%s, open:%d, soak:%d\n", pinNbr, name, openSeconds, soakSeconds);
}


void WaterValveClass::SetvalveOpenDuration(int valveOpenDuration_){
	openSeconds = valveOpenDuration_;
}

void WaterValveClass::SetvalveSoakTime(int valveSoakTime_) {
	soakSeconds = valveSoakTime_;
}

void WaterValveClass::OpenValve() {
	lastOpenTimestamp = TimeString();
	LogLine(2, __FUNCTION__, "");
	digitalWrite(pinNbr, OPEN_VALVE);
	digitalWrite(LED_BUILTIN, LOW);
	valveState = 1;
}

void WaterValveClass::KeepOpen() {
	LogLine(3, __FUNCTION__, "");
	delay(1000 * openSeconds);
}

void WaterValveClass::CloseValve() {
	LogLine(2, __FUNCTION__, "");
	digitalWrite(pinNbr, CLOSE_VALVE);
	digitalWrite(LED_BUILTIN, HIGH);
	valveState = 0;
}

void WaterValveClass::WaitToSoak() {
	LogLine(3, __FUNCTION__, "soakSeconds=" + String(soakSeconds));
	delay(1000 * soakSeconds);
}

void WaterValveClass::OpenValveAndWaitToSoak() {
	OpenValve();
	KeepOpen();
	CloseValve();
	WaitToSoak();
}

WaterValveClass WaterValve;

