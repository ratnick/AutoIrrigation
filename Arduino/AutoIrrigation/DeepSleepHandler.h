// DeepSleepHandler.h

#ifndef _DEEPSLEEPHANDLER_h
#define _DEEPSLEEPHANDLER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif


#define MAX_DEEP_SLEEP_SECS 3600  // micro seconds ~1 hour + 1 second (real limit is higher, https://thingpulse.com/max-deep-sleep-for-esp8266/)
// hardware based constant ()
// According to the ESP8266 SDK, you can only sleep for 4,294,967,295 µs, which is about ~71 minutes.
// https://www.losant.com/blog/making-the-esp8266-low-powered-with-deep-sleep

class DeepSleepHandlerClass
{
protected:


public:
	void SetDeepSleepPeriod(int secondsToSleep);
	void GoToDeepSleep();

private:

};

extern DeepSleepHandlerClass DeepSleepHandler;

#endif

