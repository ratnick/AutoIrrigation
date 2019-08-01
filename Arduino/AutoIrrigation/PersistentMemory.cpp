#include <ArduinoJson.hpp>
#include <ArduinoJson.h>
#include "PersistentMemory.h"
#include "LogLib.h"



void PersistentMemoryClass::init(
	bool resetMemory, 
	int _totalSecondsToSleep,
	const char* _deviceID, 
	int _valveOpenDuration, 
	int _valveSoakTime,
	int _mainLoopDelay,
	int _debugLevel) {

	EEPROM.begin(usedSize);
	LogLinef(2, __FUNCTION__, "EEPROM length: %d", usedSize);

	if (resetMemory) {
		strcpy(ps.deviceID, _deviceID);
		strcpy(ps.macAddress, "ABCDEF\0");
		strcpy(ps.deviceLocation, "not set\0");
		strcpy(ps.wifiSSID, "nohrTDC\0");
		strcpy(ps.wifiPwd, "RASMUSSEN\0");
		strcpy(ps.cloudUserName, "initial value\0");
		strcpy(ps.cloudPwd, "not used\0");
		strcpy(ps.runMode, RUNMODE_SOIL.c_str());
		strcpy(ps.wakeTime, "na\0");
		ps.secondsToSleep = _totalSecondsToSleep;
		ps.currentSleepCycle = 0; // counts which sleep cycle we are at right now.
		ps.valveOpenDuration = _valveOpenDuration;
		ps.valveSoakTime = _valveSoakTime;
		ps.humLimit = 50; //pct
		ps.mainLoopDelay = _mainLoopDelay;
		ps.debugLevel = _debugLevel;
		ps.deepSleepEnabled = true;
		WritePersistentMemory();
		LogLine(3, __FUNCTION__, "TOTAL RESET");
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
	DynamicJsonBuffer  jsonBuffer(200);
	String jsonStr;
	JsonObject& root = jsonBuffer.createObject();
	root["deviceID"] = ps.deviceID;
	root["macAddr"] = ps.macAddress;
	root["deviceLocation"] = ps.deviceLocation;
	root["wifiSSID"] = ps.wifiSSID;
	root["secsToSleep"] = ps.secondsToSleep;
	root["maxSlpCycles"] = ps.maxSleepCycles;
	root["currentSleepCycle"] = ps.currentSleepCycle;

	root.printTo(jsonStr);
	return jsonStr;
}

String PersistentMemoryClass::GetDeviceID()			 { return String(ps.deviceID);   }
String PersistentMemoryClass::GetdeviceLocation()	 { return String(ps.deviceLocation); }
String PersistentMemoryClass::GetmacAddress()		 { return String(ps.macAddress); }
String PersistentMemoryClass::GetwifiSSID()			 { return String(ps.wifiSSID); }
String PersistentMemoryClass::GetCloudUsername()	 { return String(ps.cloudUserName); }
String PersistentMemoryClass::GetrunMode()			 { return String(ps.runMode); }
String PersistentMemoryClass::GetWakeTime()			 { return String(ps.wakeTime); }
int PersistentMemoryClass::GettotalSecondsToSleep()  { return ps.totalSecondsToSleep; }
int PersistentMemoryClass::GetsecondsToSleep()		 { return ps.secondsToSleep; }
int PersistentMemoryClass::GetmaxSleepCycles()		 { return ps.maxSleepCycles;}
int PersistentMemoryClass::GetcurrentSleepCycle()	 { return ps.currentSleepCycle;}
int PersistentMemoryClass::GetvalveOpenDuration()	 { return ps.valveOpenDuration; }
int PersistentMemoryClass::GetvalveSoakTime()		 { return ps.valveSoakTime; }
int PersistentMemoryClass::GethumLimit()			 { return ps.humLimit; }
int PersistentMemoryClass::GetmainLoopDelay()        { return ps.mainLoopDelay; }
int PersistentMemoryClass::GetdebugLevel()           { return ps.debugLevel; }
boolean PersistentMemoryClass::GetdeepSleepEnabled() { return ps.deepSleepEnabled;  }

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
	LogLinef(3, __FUNCTION__, "macAddress       %s", s.c_str());
	strcpy(ps.macAddress, s.c_str());
	WritePersistentMemory();
}
void PersistentMemoryClass::SetdeviceID(String deviceID_) {
	strcpy(ps.deviceID, deviceID_.c_str());
	WritePersistentMemory();
}
void PersistentMemoryClass::SetwifiSSID(String wifiSSID_) {
	strcpy(ps.wifiSSID, wifiSSID_.c_str());
	WritePersistentMemory();
}
void PersistentMemoryClass::SetwifiPwd(String wifiPwd_) {
	strcpy(ps.wifiPwd, wifiPwd_.c_str());
	WritePersistentMemory();
}
void PersistentMemoryClass::SetdeviceLocation(String deviceLocation_) {
	strcpy(ps.deviceLocation, deviceLocation_.c_str());
	WritePersistentMemory();
}
void PersistentMemoryClass::SetrunMode(String runMode_) {
	strcpy(ps.runMode, runMode_.c_str());
	WritePersistentMemory();
}
void PersistentMemoryClass::SetWakeTime(String wakeTime_) {
	strcpy(ps.wakeTime, wakeTime_.c_str());
	WritePersistentMemory();
}
void PersistentMemoryClass::SettotalSecondsToSleep(int totalSecondsToSleep_) {
	ps.totalSecondsToSleep = totalSecondsToSleep_;
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
void PersistentMemoryClass::SethumLimit(int humLimit_) {
	ps.humLimit = humLimit_;
	WritePersistentMemory();
}
void PersistentMemoryClass::SetmainLoopDelay(int mainLoopDelay_) {
	ps.mainLoopDelay = mainLoopDelay_;
	WritePersistentMemory();
}
void PersistentMemoryClass::SetdebugLevel(int debugLevel_) {
	ps.debugLevel = debugLevel_;
	WritePersistentMemory();
}
void PersistentMemoryClass::SetdeepSleepEnabled(boolean deepSleepEnabled_) {
	ps.deepSleepEnabled = deepSleepEnabled_;
	WritePersistentMemory();
}

void PersistentMemoryClass::Printps() {

	LogLinef(3, __FUNCTION__, "deviceID          %s", ps.deviceID);
	LogLinef(3, __FUNCTION__, "macAddress        %s", ps.macAddress);
	LogLinef(3, __FUNCTION__, "deviceLocation    %s", ps.deviceLocation);
	LogLinef(3, __FUNCTION__, "wifiSSID          %s", ps.wifiSSID);
	LogLinef(3, __FUNCTION__, "wifiPwd           %s", ps.wifiPwd);
	LogLinef(3, __FUNCTION__, "cloudUserName     %s", ps.cloudUserName);
	LogLinef(3, __FUNCTION__, "cloudPwd          %s", ps.cloudPwd);
	LogLinef(3, __FUNCTION__, "runMode           %s", ps.runMode);
	LogLinef(3, __FUNCTION__, "wakeTime          %s", ps.wakeTime);
	LogLinef(3, __FUNCTION__, "totalSecondsToSleep %d", ps.totalSecondsToSleep);
	LogLinef(3, __FUNCTION__, "secondsToSleep    %d", ps.secondsToSleep);
	LogLinef(3, __FUNCTION__, "maxSleepCycles    %d", ps.maxSleepCycles);
	LogLinef(3, __FUNCTION__, "currentSleepCycle %d", ps.currentSleepCycle);
	LogLinef(3, __FUNCTION__, "lastVccSummarizedReading %f", ps.lastVccSummarizedReading);
	LogLinef(3, __FUNCTION__, "valveOpenDuration %d", ps.valveOpenDuration);
	LogLinef(3, __FUNCTION__, "valveSoakTime %d", ps.valveSoakTime);
	LogLinef(3, __FUNCTION__, "humLimit %d", ps.humLimit);
	LogLinef(3, __FUNCTION__, "mainLoopDelay %d", ps.mainLoopDelay);
	LogLinef(3, __FUNCTION__, "debugLevel %d", ps.debugLevel);
	LogLinef(3, __FUNCTION__, "deepSleepEnabled %d", (int) ps.deepSleepEnabled);
	
}

void PersistentMemoryClass::PrintpsRAW() {
	Serial.println("deviceID          " + String(ps.deviceID));
	Serial.println("macAddress        " + String(ps.macAddress));
	Serial.println("deviceLocation    " + String(ps.deviceLocation));
	Serial.println("wifiSSID          " + String(ps.wifiSSID));
	Serial.println("wifiPwd           " + String(ps.wifiPwd));
	Serial.println("cloudUserName     " + String(ps.cloudUserName));
	Serial.println("cloudPwd          " + String(ps.cloudPwd));
	Serial.println("runMode           " + String(ps.runMode));
	Serial.println("wakeTime          " + String(ps.wakeTime));
	Serial.println("totalSecondsToSleep " + String(ps.totalSecondsToSleep));
	Serial.println("secondsToSleep    " + String(ps.secondsToSleep));
	Serial.println("maxSleepCycles    " + String(ps.maxSleepCycles));
	Serial.println("currentSleepCycle " + String(ps.currentSleepCycle));
	Serial.println("lastVccSummarizedReading " + String(ps.lastVccSummarizedReading));
	Serial.println("valveOpenDuration " + String(ps.valveOpenDuration));
	Serial.println("valveSoakTime " + String(ps.valveSoakTime));
	Serial.println("humLimit " + String(ps.humLimit));
	Serial.println("mainLoopDelay " + String(ps.mainLoopDelay));
	Serial.println("debugLevel " + String(ps.debugLevel));
	Serial.println("deepSleepEnabled " + String(ps.deepSleepEnabled));
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

