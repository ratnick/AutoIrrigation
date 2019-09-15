// DeepSleepHandler.h

#ifndef _DEEPSLEEPHANDLER_h
#define _DEEPSLEEPHANDLER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif


#define MAX_DEEP_SLEEP_SECS 1801  // seconds (real hardware limit is higher, https://thingpulse.com/max-deep-sleep-for-esp8266/)
#define MIN_SLEEP_TIME_SECS 10

// hardware based constant ()
// According to the ESP8266 SDK, you can only sleep for 4,294,967,295 µs, which is about ~71 minutes.
// https://www.losant.com/blog/making-the-esp8266-low-powered-with-deep-sleep

class DeepSleepHandlerClass
{
protected:


public:
	void GotoSleepAndWakeUpAtTime(String wakeupTime);
	void GotoSleepAndWakeAfterDelay(int secondsToSleep);
	//void SetDeepSleepPeriod(int _secondsToSleep);
	boolean ContinueSleeping();
	void GoToDeepSleep();
	void TestDeepSleep();

private:

};

extern DeepSleepHandlerClass DeepSleepHandler;

#endif

