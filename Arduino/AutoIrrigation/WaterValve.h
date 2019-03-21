// WaterValve.h

#ifndef _WATERVALVE_h
#define _WATERVALVE_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

class WaterValveClass
{
 protected:


 public:
	int pinNbr;
	char name[15];
	int openSeconds;
	int soakSeconds;
	int valveState; // 0=closed, 1=open
	void init(int _pinNbr, char _name[], int _openSeconds, int _soakSeconds);
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

