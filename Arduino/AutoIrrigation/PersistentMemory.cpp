//#include <ArduinoJson.hpp>
//#include <ArduinoJson.h>
#include "PersistentMemory.h"
#include "LogLib.h"

void PersistentMemoryClass::init(
	bool resetMemory, 
	int _totalSecondsToSleep,
	const char* _deviceID,
	const char* _hardware,
	const char* _runmode,
	int _valveOpenDuration,
	int _valveSoakTime,
	int _mainLoopDelay,
	int _debugLevel) {

	EEPROM.begin(usedSize);
	LogLinef(3, __FUNCTION__, "EEPROM length: %d", usedSize);

	if (resetMemory) {
		// Metadata
		strcpy(ps.deviceID, _deviceID );
		strcpy(ps.macAddress, "ABCDEF\0");
		strcpy(ps.deviceLocation, "not set\0");
		strcpy(ps.sensorType, _runmode);
		strcpy(ps.hardware, _hardware);
		strcpy(ps.softwareVersion, "AutoIrrigation.ino - Compiled: " __DATE__ " " __TIME__ "\0");

		// State
		strcpy(ps.wifiSSID, "nohrTDC\0");
		strcpy(ps.wifiPwd, "RASMUSSEN\0");
		ps.runOnce = false;
		ps.currentSleepCycle = 0; // counts which sleep cycle we are at right now.
		ps.secondsToSleep = 3;
		ps.maxSleepCycles = 0;
		ps.deviceStatus = DEVICE_STATUS_OK;

		// Settings
		strcpy(ps.runMode, _runmode);
		ps.valveOpenDuration = _valveOpenDuration;
		ps.valveSoakTime = _valveSoakTime;
		ps.humLimit = 50; //pct
		ps.mainLoopDelay = _mainLoopDelay;
		ps.debugLevel = _debugLevel;
		ps.deepSleepEnabled = false;
		for (int i = 0; i < MAX_WAKEUPTIMES; i++) {
			strcpy(ps.wakeupTime[i], "HHMMSS\0");
		}
		strcpy(ps.pauseWakeTime, "HHMMSS\0");
		ps.vccAdjustment = 4.01; // Volt
		ps.vccMinLimit = 0.00; // Volt

		// Misc, not sent to Firebase
		ps.totalSecondsToSleep = _totalSecondsToSleep;
		ps.lastVccSummarizedReading = 0;

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

void PersistentMemoryClass::AddMetadataJson(FirebaseJson* json) {
	json->add(FB_macAddress, String(ps.macAddress));
	json->add(FB_deviceLocation, String(ps.deviceLocation));
	json->add(FB_deviceID, String(ps.deviceID));
	json->add(FB_sensorType, String(ps.sensorType));
	json->add(FB_hardware, String(ps.hardware));
	json->add(FB_softwareVersion, String(ps.softwareVersion));
};

void PersistentMemoryClass::AddStateJson(FirebaseJson* json) {
	json->add(FB_wifiSSID, String(ps.wifiSSID));
	json->add(FB_secondsToSleep, ps.secondsToSleep);
	json->add(FB_maxSleepCycles, ps.maxSleepCycles);
	json->add(FB_currentSleepCycle, ps.currentSleepCycle);
	json->add(FB_runOnce, ps.runOnce);
	json->add(FB_deviceStatus, ps.deviceStatus);
};

void PersistentMemoryClass::AddSettingsJson(FirebaseJson* json) {
	json->add(FB_runMode, String(ps.runMode));
	json->add(FB_valveOpenDuration, ps.valveOpenDuration);
	json->add(FB_valveSoakTime, ps.valveSoakTime);
	json->add(FB_humLimit, ps.humLimit);
	json->add(FB_mainLoopDelay, ps.mainLoopDelay);
	json->add(FB_debugLevel, ps.debugLevel);
	json->add(FB_deepSleepEnabled, ps.deepSleepEnabled);
	json->add(FB_totalSecondsToSleep, ps.totalSecondsToSleep);
	json->add(FB_wakeupTime "0", String(ps.wakeupTime[0]));
	json->add(FB_wakeupTime "1", String(ps.wakeupTime[1]));
	json->add(FB_wakeupTime "2", String(ps.wakeupTime[2]));
	json->add(FB_wakeupTime "3", String(ps.wakeupTime[3]));
	json->add(FB_pauseWakeTime, String(ps.pauseWakeTime));
	json->add(FB_vccAdjustment, ps.vccAdjustment);
	json->add(FB_vccMinLimit, ps.vccMinLimit);
};


/*
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
}*/

String PersistentMemoryClass::GetDeviceID()			 { return String(ps.deviceID);   }
String PersistentMemoryClass::GetdeviceLocation()	 { return String(ps.deviceLocation); }
String PersistentMemoryClass::GetmacAddress()		 { return String(ps.macAddress); }
String PersistentMemoryClass::GetwifiSSID()			 { return String(ps.wifiSSID); }
String PersistentMemoryClass::GetrunMode()			 { return String(ps.runMode); }
String PersistentMemoryClass::GetWakeTime(int i)     { return String(ps.wakeupTime[i]); }
String PersistentMemoryClass::GetPauseWakeTime()     { return String(ps.pauseWakeTime); }
int PersistentMemoryClass::GetdeviceStatus()		 { return ps.deviceStatus; }
int PersistentMemoryClass::GettotalSecondsToSleep()  { return ps.totalSecondsToSleep; }
int PersistentMemoryClass::GetsecondsToSleep()		 { return ps.secondsToSleep; }
int PersistentMemoryClass::GetmaxSleepCycles()		 { return ps.maxSleepCycles;}
int PersistentMemoryClass::GetcurrentSleepCycle()	 { return ps.currentSleepCycle;}
int PersistentMemoryClass::GetvalveOpenDuration()	 { return ps.valveOpenDuration; }
int PersistentMemoryClass::GetvalveSoakTime()		 { return ps.valveSoakTime; }
int PersistentMemoryClass::GethumLimit()			 { return ps.humLimit; }
int PersistentMemoryClass::GetmainLoopDelay()        { return ps.mainLoopDelay; }
int PersistentMemoryClass::GetdebugLevel()           { return ps.debugLevel; }
float PersistentMemoryClass::GetvccAdjustment()		 { return ps.vccAdjustment; }
float PersistentMemoryClass::GetvccMinLimit()		 { return ps.vccMinLimit; }
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
	LogLinef(4, __FUNCTION__, "macAddress       %s", s.c_str());
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
	LogLinef(4, __FUNCTION__, "ps.runMode=%s, runMode_.c_str()=%s", ps.runMode, runMode_.c_str());
	WritePersistentMemory();
}
void PersistentMemoryClass::SetWakeTime(int i, String wakeTime_) {
	strcpy(ps.wakeupTime[i], wakeTime_.c_str());
	WritePersistentMemory();
}
void PersistentMemoryClass::SetPauseWakeTime(String wakeTime_) {
	strcpy(ps.pauseWakeTime, wakeTime_.c_str());
	WritePersistentMemory();
}
void PersistentMemoryClass::SetDeviceStatus(int deviceStatus_) {
	ps.deviceStatus, deviceStatus_;
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
void PersistentMemoryClass::SetvccAdjustment(float vccAdj_) {
	ps.vccAdjustment = vccAdj_;
	WritePersistentMemory();
}
void PersistentMemoryClass::SetvccMinLimit(float vccMinLimit_) {
	ps.vccMinLimit = vccMinLimit_;
	WritePersistentMemory();
}
void PersistentMemoryClass::SetdeepSleepEnabled(boolean deepSleepEnabled_) {
	ps.deepSleepEnabled = deepSleepEnabled_;
	WritePersistentMemory();
}

void PersistentMemoryClass::Printps() {

	LogLinef(5, __FUNCTION__, "deviceID          %s", ps.deviceID);
	LogLinef(5, __FUNCTION__, "macAddress        %s", ps.macAddress);
	LogLinef(5, __FUNCTION__, "deviceLocation    %s", ps.deviceLocation);
	LogLinef(5, __FUNCTION__, "wifiSSID          %s", ps.wifiSSID);
	LogLinef(5, __FUNCTION__, "wifiPwd           %s", ps.wifiPwd);
	LogLinef(5, __FUNCTION__, "hardware          %s", ps.hardware);
	LogLinef(5, __FUNCTION__, "sw version        %s", ps.softwareVersion);
	LogLinef(5, __FUNCTION__, "runMode           %s", ps.runMode);
	LogLinef(5, __FUNCTION__, "wakeTime          %s", ps.wakeupTime[0]);
	LogLinef(5, __FUNCTION__, "pauseWakeTime     %s", ps.pauseWakeTime);
	LogLinef(5, __FUNCTION__, "deviceStatus      %d", ps.deviceStatus);
	LogLinef(5, __FUNCTION__, "totalSecondsToSleep %d", ps.totalSecondsToSleep);
	LogLinef(5, __FUNCTION__, "secondsToSleep    %d", ps.secondsToSleep);
	LogLinef(5, __FUNCTION__, "maxSleepCycles    %d", ps.maxSleepCycles);
	LogLinef(5, __FUNCTION__, "currentSleepCycle %d", ps.currentSleepCycle);
	LogLinef(5, __FUNCTION__, "lastVccSummarizedReading %f", ps.lastVccSummarizedReading);
	LogLinef(5, __FUNCTION__, "valveOpenDuration %d", ps.valveOpenDuration);
	LogLinef(5, __FUNCTION__, "valveSoakTime %d", ps.valveSoakTime);
	LogLinef(5, __FUNCTION__, "humLimit %d", ps.humLimit);
	LogLinef(5, __FUNCTION__, "vccAdjustment %f", ps.vccAdjustment);
	LogLinef(5, __FUNCTION__, "vccMinLimit %f", ps.vccMinLimit);
	LogLinef(5, __FUNCTION__, "mainLoopDelay %d", ps.mainLoopDelay);
	LogLinef(5, __FUNCTION__, "debugLevel %d", ps.debugLevel);
	LogLinef(5, __FUNCTION__, "deepSleepEnabled %d", (int) ps.deepSleepEnabled);
	
}

void PersistentMemoryClass::PrintpsRAW() {
	Serial.println("deviceID          " + String(ps.deviceID));
	Serial.println("macAddress        " + String(ps.macAddress));
	Serial.println("deviceLocation    " + String(ps.deviceLocation));
	Serial.println("wifiSSID          " + String(ps.wifiSSID));
	Serial.println("wifiPwd           " + String(ps.wifiPwd));
	Serial.println("runMode           " + String(ps.runMode));
	Serial.println("wakeTime 0       " + String(ps.wakeupTime[0]));
	Serial.println("wakeTime 1       " + String(ps.wakeupTime[1]));
	Serial.println("wakeTime 2       " + String(ps.wakeupTime[2]));
	Serial.println("wakeTime 3       " + String(ps.wakeupTime[3]));
	Serial.println("pauseWakeTime     " + String(ps.pauseWakeTime));
	Serial.println("deviceStatus      " + String(ps.deviceStatus));
	Serial.println("totalSecondsToSleep " + String(ps.totalSecondsToSleep));
	Serial.println("secondsToSleep    " + String(ps.secondsToSleep));
	Serial.println("maxSleepCycles    " + String(ps.maxSleepCycles));
	Serial.println("currentSleepCycle " + String(ps.currentSleepCycle));
	Serial.println("lastVccSummarizedReading " + String(ps.lastVccSummarizedReading));
	Serial.println("valveOpenDuration " + String(ps.valveOpenDuration));
	Serial.println("valveSoakTime " + String(ps.valveSoakTime));
	Serial.println("humLimit " + String(ps.humLimit));
	Serial.println("vccAdjustment " + String(ps.vccAdjustment));
	Serial.println("vccMinLimit " + String(ps.vccMinLimit));
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

