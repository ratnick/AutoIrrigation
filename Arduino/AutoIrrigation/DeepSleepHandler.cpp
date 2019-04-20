#include "DeepSleepHandler.h"
#include "PersistentMemory.h"
#include "LogLib.h"

// source: https://github.com/markszabo/IRremoteESP8266/blob/master/src/IRutils.cpp#L48
String uint64ToString(uint64_t input) {
	String result = "";
	uint8_t base = 10;

	do {
		char c = input % base;
		input /= base;

		if (c < 10)
			c += '0';
		else
			c += 'A' - 10;
		result = c + result;
	} while (input);
	return result;
}

void DeepSleepHandlerClass::SetDeepSleepPeriod(int _secondsToSleep) {

	Serial.println("max deep sleep: " + uint64ToString(ESP.deepSleepMax()));

	if (PersistentMemory.GetdeepSleepEnabled()) {
		if (_secondsToSleep < MIN_SLEEP_TIME_SECS) {
			_secondsToSleep = MIN_SLEEP_TIME_SECS;
			LogLine(0, __FUNCTION__, "_secondsToSleep too low: " + String(_secondsToSleep) );
		}
	} 
	int secondsToSleep = _secondsToSleep % MAX_DEEP_SLEEP_SECS;
	int maxSleepCycles = _secondsToSleep / MAX_DEEP_SLEEP_SECS;
	PersistentMemory.SetsecondsToSleep(secondsToSleep);
	PersistentMemory.SetmaxSleepCycles(maxSleepCycles);
	LogLine(2, __FUNCTION__, "SetDeepSleepPeriod(): secondsToSleep = " + String(secondsToSleep) + "  maxSleepCycles =" + String(maxSleepCycles));
}

void DeepSleepHandlerClass::GoToDeepSleep() {

	uint64_t deepSleepPeriod = 0;

	LogLine(2, __FUNCTION__, "currentSleepCycle = " + String(PersistentMemory.ps.currentSleepCycle ) + "  GetmaxSleepCycles = " + String(PersistentMemory.GetmaxSleepCycles()));
	if (PersistentMemory.ps.currentSleepCycle < PersistentMemory.GetmaxSleepCycles() ) {
		PersistentMemory.ps.currentSleepCycle++;
		deepSleepPeriod = MAX_DEEP_SLEEP_SECS;
	}
	else {
		LogLine(2, __FUNCTION__, "*** Resetting persistent memory (to reset counters)");
		PersistentMemory.ps.currentSleepCycle = 0;
		deepSleepPeriod = PersistentMemory.GetsecondsToSleep();
	}

	PersistentMemory.WritePersistentMemory();
	uint32_t secs = deepSleepPeriod;
	deepSleepPeriod = deepSleepPeriod * 1000000;
	LogLine(1, __FUNCTION__, "deepSleepPeriod=" + String(secs) + " secs    DeepSleepEnabled:" + String(PersistentMemory.GetdeepSleepEnabled()));
	if (PersistentMemory.GetdeepSleepEnabled()) {
		LogLine(1, __FUNCTION__, "Go to deep sleep");
		ESP.deepSleep(deepSleepPeriod);
	}
	else {
		LogLine(1, __FUNCTION__, "Skip deep sleep (not enabled)");
	}
}

DeepSleepHandlerClass DeepSleepHandler;




