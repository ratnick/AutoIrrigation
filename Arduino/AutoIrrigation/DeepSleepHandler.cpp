#include "DeepSleepHandler.h"
#include "PersistentMemory.h"
#include "LogLib.h"
#include <TimeLib.h>


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


boolean isValidNumber(String str) {
	for (byte i = 0; i < str.length(); i++)
	{
		if (isDigit(str.charAt(i))) return true;
	}
	return false;
}

void DeepSleepHandlerClass::GotoSleepAndWakeUpAtTime(String wakeupTime) {

	int hh = 0;
	int mm = 0;
	int hh_now = 0;
	int mm_now = 0;
	int totalSecondsToWakeup = 0;

	LogLinef(1, __FUNCTION__, "Wake up at: %s", wakeupTime.c_str());

	// check that input is OK and convert to time_t
	if (!isValidNumber(wakeupTime)) {
		LogLinef(1, __FUNCTION__, "NOT SET TO SLEEP AS REQUESTED. WakeupTime is NOT in the format HHMM: %s", wakeupTime.c_str());
	}
	else {
		char str[3];
		wakeupTime.substring(0, 2).toCharArray(str, 3);
		hh = atoi(str);
		wakeupTime.substring(2, 4).toCharArray(str, 3);
		mm = atoi(str);

		if (0 <= hh && hh < 24 && 0 <= mm && mm < 60) {
			mm_now = GetCurrentMinute();
			hh_now = GetCurrentHour();

			int mm_diff = mm - mm_now; 
			int hh_diff = hh - hh_now;
			if (mm_diff < 0) { 
				mm_diff += 60; 
				hh_diff -= 1;
			}
			if (hh_diff < 0) { hh_diff += 24; }
			totalSecondsToWakeup = hh_diff * 60 * 60 + mm_diff * 60;
			LogLinef(3, __FUNCTION__, "HHMM: %d:%d   now=%d:%d   diff=%d:%d", hh, mm, hh_now, mm_now, hh_diff, mm_diff);
			LogLinef(2, __FUNCTION__, "totalSecondsToWakeup = %d", totalSecondsToWakeup);
			SetDeepSleepPeriod(totalSecondsToWakeup);
			GoToDeepSleep();
		}
		else {
			LogLinef(1, __FUNCTION__, "NOT SET TO SLEEP AS REQUESTED. HH or MM is out of bound. Format is HHMM: %d %d", hh, mm);
		}
	}
}

void DeepSleepHandlerClass::SetDeepSleepPeriod(int _secondsToSleep) {

	//Serial.println("max deep sleep: " + uint64ToString(ESP.deepSleepMax()));

	if (PersistentMemory.GetdeepSleepEnabled()) {
		if (_secondsToSleep < MIN_SLEEP_TIME_SECS) {
			_secondsToSleep = MIN_SLEEP_TIME_SECS;
			LogLinef(0, __FUNCTION__, "_secondsToSleep too low: %d", _secondsToSleep);
		}
	}
	int secondsToSleep = _secondsToSleep % MAX_DEEP_SLEEP_SECS;
	int maxSleepCycles = _secondsToSleep / MAX_DEEP_SLEEP_SECS;
	PersistentMemory.SetsecondsToSleep(secondsToSleep);
	PersistentMemory.SetmaxSleepCycles(maxSleepCycles);
	LogLinef(3, __FUNCTION__, "SetDeepSleepPeriod(): secondsToSleep = %d  maxSleepCycles = %d", secondsToSleep, maxSleepCycles);
}

void DeepSleepHandlerClass::GoToDeepSleep() {

	uint64_t deepSleepPeriod = 0;

	LogLinef(3, __FUNCTION__, "currentSleepCycle = %d  GetmaxSleepCycles = %d", PersistentMemory.ps.currentSleepCycle, PersistentMemory.GetmaxSleepCycles());
	if (PersistentMemory.ps.currentSleepCycle < PersistentMemory.GetmaxSleepCycles() ) {
		PersistentMemory.ps.currentSleepCycle++;
		deepSleepPeriod = MAX_DEEP_SLEEP_SECS;
	}
	else {
		LogLine(4, __FUNCTION__, "*** Resetting persistent memory (to reset counters)");
		PersistentMemory.ps.currentSleepCycle = 0;
		deepSleepPeriod = PersistentMemory.GetsecondsToSleep();
	}

	PersistentMemory.WritePersistentMemory();
	uint32_t secs = deepSleepPeriod;
	deepSleepPeriod = deepSleepPeriod * 1000000;
	LogLinef(1, __FUNCTION__, "deepSleepPeriod=%d secs    DeepSleepEnabled:%d", secs, PersistentMemory.GetdeepSleepEnabled());
	if (PersistentMemory.GetdeepSleepEnabled()) {
		LogLine(1, __FUNCTION__, "Go to deep sleep");
		ESP.deepSleep(deepSleepPeriod);
	}
	else {
		LogLine(2, __FUNCTION__, "Skip deep sleep (not enabled)");
	}
}

DeepSleepHandlerClass DeepSleepHandler;




