// WaterValve.h

#ifndef _WATERVALVE_h
#define _WATERVALVE_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

//#include <jsmn.h>
#include <FirebaseJson.h>

class WaterValveClass
{
 protected:


 public:
	struct ValveTelemetry {
		char lastOpenTimestamp[40]; 
		int valveState; // 0=closed, 1=open
	};
	WaterValveClass::ValveTelemetry valveTm;
	 
	int pinNbr;
	char name[15];
	int openSeconds;
	int soakSeconds;
	int valveState; // 0=closed, 1=open
	void init(int _pinNbr, char _name[], int _openSeconds, int _soakSeconds);
	void AddTelemetryJson(FirebaseJson* json);
	void OpenValve();
	void WaitToSoak();
	void CloseValve();
	void KeepOpen();
	void OpenValveAndWaitToSoak();
	void SetvalveOpenDuration(int valveOpenDuration_);
	void SetvalveSoakTime(int valveSoakTime_);

	String lastOpenTimestamp;

	const boolean OPEN_VALVE = true;
	const boolean CLOSE_VALVE = false;
};

extern WaterValveClass WaterValve;

#endif

