#include <ArduinoJson.hpp>
#include <ArduinoJson.h>
#include "PersistentMemory.h"
#include "LogLib.h"



void PersistentMemoryClass::init(
	bool resetMemory, 
	int _secondsToSleep, 
	int _maxSleepCycles, 
	const char* _deviceID, 
	int _valveOpenDuration, 
	int _valveSoakTime,
	int _mainLoopDelay)
{
	EEPROM.begin(usedSize);
	LogLine(2, __FUNCTION__, "EEPROM length: " + String(usedSize));

	if (resetMemory) {
		strcpy(ps.deviceID, _deviceID);
//		strcpy(ps.deviceID, "test");
		strcpy(ps.macAddress, "ABCDEF");
		strcpy(ps.deviceLocation, "not set");
		strcpy(ps.wifiSSID, "nohrTDC");
		strcpy(ps.wifiPwd, "RASMUSSEN");
		strcpy(ps.cloudUserName, "initial value");
		strcpy(ps.cloudPwd, "not used");
		ps.secondsToSleep = _secondsToSleep;    // hardware limitation of max MAX_DEEP_SLEEP
		ps.maxSleepCycles = _maxSleepCycles;  // The total sleep time before the loop() is executed is secondsToSleep * maxSleepCycles
		ps.currentSleepCycle = 0; // counts which sleep cycle we are at right now.
		ps.valveOpenDuration = _valveOpenDuration;
		ps.valveSoakTime = _valveSoakTime;
		ps.mainLoopDelay = _mainLoopDelay;

//		for (int i = 0; i < MAX_KEPT_LOGLINES; i++) {
//			strcpy(ps.logLines[i], "not used");
//		}
		WritePersistentMemory();
	}
	else {
		ReadPersistentMemory();
	}
}

void PersistentMemoryClass::ReadPersistentMemory() {
	byte* b = (byte*)&ps;
	int addr;
	for (addr = 0; addr < usedSize; addr++) {
		*b++ = EEPROM.read(addr);
	}
};

void PersistentMemoryClass::WritePersistentMemory() {
	const byte * b = (const byte*)&ps;
	int addr = 0;
	for (addr = 0; addr < usedSize; addr++) {
		//Serial.print(*b);
		EEPROM.write(addr, *b++);
	}
	EEPROM.commit();
	delay(200);
};

String PersistentMemoryClass::GetStateJson() {
	//StaticJsonBuffer<200> jsonBuffer;
	DynamicJsonBuffer  jsonBuffer(200);
	String jsonStr;
	JsonObject& root = jsonBuffer.createObject();
	root["deviceID"] = ps.deviceID;
	root["macAddr"] = ps.macAddress;
	root["deviceLocation"] = ps.deviceLocation;
	root["wifiSSID"] = ps.wifiSSID;
//	root["wifiPwd"] = ps.wifiPwd;
//	root["cloudUserName"] = ps.cloudUserName;
//	root["cloudPwd"] = ps.cloudPwd;
	root["secsToSleep"] = ps.secondsToSleep;
	root["maxSlpCycles"] = ps.maxSleepCycles;
	root["currentSleepCycle"] = ps.currentSleepCycle;

	root.printTo(jsonStr);
	return jsonStr;
}

String PersistentMemoryClass::GetDeviceID()			{ return String(ps.deviceID);   }
String PersistentMemoryClass::GetdeviceLocation()	{ return String(ps.deviceLocation); }
String PersistentMemoryClass::GetmacAddress()		{ return String(ps.macAddress); }
String PersistentMemoryClass::GetwifiSSID()			{ return String(ps.wifiSSID); }
String PersistentMemoryClass::GetCloudUsername()	{ return String(ps.cloudUserName); }
int PersistentMemoryClass::GetsecondsToSleep()		{ return ps.secondsToSleep;}
int PersistentMemoryClass::GetmaxSleepCycles()		{ return ps.maxSleepCycles;}
int PersistentMemoryClass::GetcurrentSleepCycle()	{ return ps.currentSleepCycle;}
int PersistentMemoryClass::GetvalveOpenDuration()	{ return ps.valveOpenDuration; }
int PersistentMemoryClass::GetvalveSoakTime() { return ps.valveSoakTime; }
int PersistentMemoryClass::GetmainLoopDelay() { return ps.mainLoopDelay; }

String mac2String(byte ar[]) {
	String s;
	for (byte i = 0; i < 6; ++i)
	{
		char buf[3];
		sprintf(buf, "%2X", ar[i]);
		s += buf;
		if (i < 5) s += ':';
	}
	return s;
}

void PersistentMemoryClass::SetmacAddress(byte mac[]) { 
	String s;
	s = mac2String(mac);
	LogLine(3, __FUNCTION__, "macAddress          " + s);
	strcpy(ps.macAddress, s.c_str());
	WritePersistentMemory();
}

void PersistentMemoryClass::SetdeviceID(String deviceID_) {
	strcpy(ps.deviceID, deviceID_.c_str());
	WritePersistentMemory();
}

void PersistentMemoryClass::SetdeviceLocation(String deviceLocation_) {
	strcpy(ps.deviceLocation, deviceLocation_.c_str());
	WritePersistentMemory();
}

void PersistentMemoryClass::SetsecondsToSleep(int secondsToSleep_) {
	ps.secondsToSleep = secondsToSleep_;
	WritePersistentMemory();
}

void PersistentMemoryClass::SetmaxSleepCycles(int maxSleepCycles_) {
	ps.maxSleepCycles = maxSleepCycles_;
	WritePersistentMemory();
}

void PersistentMemoryClass::SetvalveOpenDuration(int valveOpenDuration_) {
	ps.valveOpenDuration = valveOpenDuration_;
	WritePersistentMemory();
}
void PersistentMemoryClass::SetvalveSoakTime(int valveSoakTime_) {
	ps.valveSoakTime = valveSoakTime_;
	WritePersistentMemory();
}
void PersistentMemoryClass::SetmainLoopDelay(int mainLoopDelay_) {
	ps.mainLoopDelay = mainLoopDelay_;
	WritePersistentMemory();
}

void PersistentMemoryClass::Printps() {
	LogLine(2, __FUNCTION__, "deviceID          " + String(ps.deviceID));
	LogLine(2, __FUNCTION__, "macAddress        " + String(ps.macAddress));
	LogLine(2, __FUNCTION__, "deviceLocation    " + String(ps.deviceLocation));
	LogLine(2, __FUNCTION__, "wifiSSID          " + String(ps.wifiSSID));
	LogLine(2, __FUNCTION__, "wifiPwd           " + String(ps.wifiPwd));
	LogLine(2, __FUNCTION__, "cloudUserName     " + String(ps.cloudUserName));
	LogLine(2, __FUNCTION__, "cloudPwd          " + String(ps.cloudPwd));
	LogLine(2, __FUNCTION__, "secondsToSleep    " + String(ps.secondsToSleep));
	LogLine(2, __FUNCTION__, "maxSleepCycles    " + String(ps.maxSleepCycles));
	LogLine(2, __FUNCTION__, "currentSleepCycle " + String(ps.currentSleepCycle));
	LogLine(2, __FUNCTION__, "lastVccSummarizedReading " + String(ps.lastVccSummarizedReading));
	LogLine(2, __FUNCTION__, "valveOpenDuration " + String(ps.valveOpenDuration));
	LogLine(2, __FUNCTION__, "valveSoakTime " + String(ps.valveSoakTime));
	LogLine(2, __FUNCTION__, "mainLoopDelay " + String(ps.mainLoopDelay));
	//	for (int i = 0; i < MAX_KEPT_LOGLINES; i++) {
//		LogLine(2, __FUNCTION__, "Logline " + String(i) + ":" + String(ps.logLines[i]));
//	}
}

void PersistentMemoryClass::UnitTest() {
	InitDebugLevel(3);

	LogLine(0, __FUNCTION__, "Note: init(false) has already been called in Setup()");
	Printps();

	LogLine(0, __FUNCTION__, "Re-init to set values");
	init(true);

	Printps();
	LogLine(0, __FUNCTION__, "You may reset now during next 7 seconds to re-init values");
	delay(7000);

	LogLine(0, __FUNCTION__, "reset values");
	strcpy(ps.deviceID, "aaaaaa");
	strcpy(ps.deviceLocation, "bbbbbb");
	strcpy(ps.wifiSSID, "xxxxxx");
	strcpy(ps.wifiSSID, "ccccccc");
	strcpy(ps.wifiPwd, "ddddddddd");
	strcpy(ps.cloudUserName, "aaaa");
	strcpy(ps.cloudPwd, "bbbbb");
	ps.secondsToSleep = 10;
	ps.maxSleepCycles = 2;  // The total sleep time before the loop() is executed is secondsToSleep * maxSleepCycles
	ps.currentSleepCycle = 3; // counts which sleep cycle we are at right now.

	Printps();

	LogLine(0, __FUNCTION__, "re-read values => back to original values");
	ReadPersistentMemory();
	Printps();

	// enter deepsleep
	LogLine(0, __FUNCTION__, "reset values again");
	strcpy(ps.deviceID, "aaaaaa");
	strcpy(ps.deviceLocation, "bbbbbb");
	strcpy(ps.wifiSSID, "xxxxxx");
	strcpy(ps.wifiSSID, "ccccccc");
	strcpy(ps.wifiPwd, "ddddddddd");
	strcpy(ps.cloudUserName, "aaaa");
	strcpy(ps.cloudPwd, "bbbbb");
	ps.secondsToSleep = 10;
	ps.maxSleepCycles = 1;  // The total sleep time before the loop() is executed is secondsToSleep * maxSleepCycles
	ps.currentSleepCycle = 0; // counts which sleep cycle we are at right now.
	Printps();
	WritePersistentMemory();

	// 
	LogLine(0, __FUNCTION__, "Power cycle the device. Should come back with these latest values");
	delay(20000);

}

PersistentMemoryClass PersistentMemory;
