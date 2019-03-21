// DeepSleepHandler.h

#ifndef _DEEPSLEEPHANDLER_h
#define _DEEPSLEEPHANDLER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

// hardware based constant ()
// According to the ESP8266 SDK, you can only sleep for 4,294,967,295 µs, which is about ~71 minutes.
// https://www.losant.com/blog/making-the-esp8266-low-powered-with-deep-sleep
#define MAX_DEEP_SLEEP_SECS 3600

class DeepSleepHandlerClass
{
protected:


public:
	/* void init(); */
	void GoToDeepSleep(int secondsToSleep);

private:

};

extern DeepSleepHandlerClass DeepSleepHandler;

#endif

