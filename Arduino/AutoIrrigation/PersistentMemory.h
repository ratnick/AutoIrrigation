// PersistentMemory.h

#ifndef _PERSISTENTMEMORY_h
#define _PERSISTENTMEMORY_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <EEPROM.h>
#include <jsmn.h>
#include <FirebaseJson.h>
#include "globals.h"
//#include "FirebaseModel.h"

class PersistentMemoryClass
{
 protected:


 public:
	 PersistentDataStruct ps;

	 void init(
		 bool resetMemory, 
		 int _totalSecondsToSleep = 0,
		 const char* _deviceID="n/a", 
		 const char* _sw_vers="n/a",
		 int _valveOpenDuration = 1,
		 int _valveSoakTime = 1, 
		 int _mainLoopDelay = 600,
		 int _debugLevel = 0);
	 void ReadPersistentMemory();
	 void WritePersistentMemory();

	 void AddMetadataJson(FirebaseJson* json);
	 void AddStateJson(FirebaseJson* json);
	 void AddSettingsJson(FirebaseJson* json);

	 //String GetStateJson();
	 String GetDeviceID();
	 String GetdeviceLocation();
	 String GetmacAddress();
	 String GetwifiSSID();
	 String GetrunMode();
	 String GetWakeTime(int i);
	 String GetPauseWakeTime();
	 int GettotalSecondsToSleep();
	 int GetsecondsToSleep();
	 int GetmaxSleepCycles();
	 int GetcurrentSleepCycle();
	 int GetvalveOpenDuration();
	 int GetvalveSoakTime();
	 int GethumLimit();
	 int GetmainLoopDelay();
	 int GetdebugLevel();
	 float GetvccAdjustment();
	 float GetvccMinLimit();
	 boolean GetdeepSleepEnabled();

	 void SetdeviceID(String id_);
	 void SetwifiSSID(String wifiSSID_);
	 void SetwifiPwd(String wifiPwd_);
	 void SetdeviceLocation(String location_);
	 void SetrunMode(String runMode_);
	 void SetWakeTime(int i, String wakeTime_);
	 void SetPauseWakeTime(String wakeTime_);
	 void SetmacAddress(byte* mac);
	 void SettotalSecondsToSleep(int totalSecondsToSleep_);
	 void SetsecondsToSleep(int secondsToSleep_);
	 void SetmaxSleepCycles(int maxSleepCycles_);
	 void SetvalveOpenDuration(int valveOpenDuration_);
	 void SetvalveSoakTime(int valveSoakTime_);
	 void SethumLimit(int humLimit_);
	 void SetmainLoopDelay(int mainLoopDelay_);
	 void SetdebugLevel(int debugLevel_);
	 void SetvccAdjustment(float vccAdj_);
	 void SetvccMinLimit(float vccMinLimit_);
	 void SetdeepSleepEnabled(boolean deepSleepEnabled_);

	 void Printps();
	 void PrintpsRAW();
	 void UnitTest();

private:
	const byte * b = (const byte*)&ps;
	int startAddr;
	const int usedSize = sizeof(PersistentDataStruct);

};

extern PersistentMemoryClass PersistentMemory;

#endif
