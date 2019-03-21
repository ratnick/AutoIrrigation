// 
// 
// 

#include "DeepSleepHandler.h"

/*void DeepSleepHandlerClass::init()
{


}*/

void DeepSleepHandlerClass::GoToDeepSleep(int secondsToSleep) {

	time_t deepSleepPeriod = secondsToSleep;

	if (deepSleepPeriod > 0) {
		Serial.printf("%s %i %s", "GoToDeepSleep(): deepSleepPeriod = ", deepSleepPeriod, " seconds\n");
		ESP.deepSleep(deepSleepPeriod * 1000000);
	}
	else {
		Serial.println("Deep sleep request NOT met. Value is zero");
	}
}


DeepSleepHandlerClass DeepSleepHandler;




