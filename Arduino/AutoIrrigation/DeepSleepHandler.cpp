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

boolean isNotValidNumber(String str) {
	//LogLinef(5, __FUNCTION__, "try %s", str.c_str());
	for (byte i = 0; i < str.length(); i++)
	{
		if (!isDigit(str.charAt(i))) return true;
	}
	return false;
}

boolean isValidTimeString(String str) {
	int n;
	char buf[3];

	buf[2] = '\0';

	if (isNotValidNumber(str)) {
		LogLine(1, __FUNCTION__, "invalid number");
		return false;
	}
	else {
		buf[0] = str[0]; buf[1] = str[1];
		n = atoi(buf);
		LogLinef(4, __FUNCTION__, "HH: %s=%d", buf, n);
		if (n > 23) return false;

		buf[0] = str[2]; buf[1] = str[3];
		n = atoi(buf);
		LogLinef(4, __FUNCTION__, "MM: %s=%d", buf, n);
		if (n > 59) return false;

		buf[0] = str[4]; buf[1] = str[5];
		n = atoi(buf);
		LogLinef(4, __FUNCTION__, "SS: %s=%d", buf, n);
		if (n > 59) return false;
	}
	LogLine(1, __FUNCTION__, "TRUE");
	return true;
}

void DeepSleepHandlerClass::GotoSleepAndWakeUpAtTime(String wakeupTime) {

	int hh = 0;
	int mm = 0;
	int hh_now = 0;
	int mm_now = 0;
	int totalSecondsToWakeup = 0;

	LogLinef(2, __FUNCTION__, "Wake up at: %s", wakeupTime.c_str());

	// check that input is OK and convert to time_t
	if (isValidTimeString(wakeupTime)) {
		PersistentMemory.SetWakeTime(WAKEUP_IDX_SIMPLEDELAY, wakeupTime);
		GoToDeepSleep();
	}
	else {
		LogLinef(0, __FUNCTION__, "NOT SET TO SLEEP AS REQUESTED. HH or MM is out of bound. Format is HHMM: %d %d", hh, mm);
	}
}

void DeepSleepHandlerClass::GotoSleepAndWakeAfterDelay(int secondsToSleep) {

	LogLinef(2, __FUNCTION__, "Wake up after %d seconds", secondsToSleep);
	if (secondsToSleep > 0) {
		char wakeupTime[7];
		uint64_t t = GetOnboardTime() + secondsToSleep;
		ConvertToShortTimeStr(t, wakeupTime);
		LogLinef(3, __FUNCTION__, "Time string = %s", wakeupTime);
		PersistentMemory.SetWakeTime(WAKEUP_IDX_SIMPLEDELAY, wakeupTime);
	}
	else {
		PersistentMemory.SetWakeTime(WAKEUP_IDX_SIMPLEDELAY, "HHMMSS\0");
	}
	GoToDeepSleep();
}

uint64_t GetNearestWakeupTime() {
	int hh = 0;
	int mm = 0;
	int ss = 0;
	int hh_now = 0;
	int mm_now = 0;
	int ss_now = 0;
	uint64_t nearestSecondsToWakeup = 24 * 60 * 60;
	uint64_t tmpNearestSecondsToWakeup = 0;

	String wakeupTime;

	LogLinef(3, __FUNCTION__, "begin");

	for (int i = 0; i < MAX_WAKEUPTIMES; i++) {
		wakeupTime = PersistentMemory.GetWakeTime(i);
		if (isValidTimeString(wakeupTime)) {
			LogLinef(4, __FUNCTION__, "Sleep time %d OK: %s", i, wakeupTime.c_str());
	
			char str[3];
			wakeupTime.substring(0, 2).toCharArray(str, 3);
			hh = atoi(str);
			wakeupTime.substring(2, 4).toCharArray(str, 3);
			mm = atoi(str);
			wakeupTime.substring(4, 6).toCharArray(str, 3);
			ss = atoi(str);

			hh_now = GetCurrentHour();
			mm_now = GetCurrentMinute();
			ss_now = GetCurrentSecond();

			int hh_diff = hh - hh_now;
			int mm_diff = mm - mm_now;
			int ss_diff = ss - ss_now;

			if (ss_diff < 0) {
				ss_diff += 60;
				mm_diff -= 1;
			}
			if (mm_diff < 0) {
				mm_diff += 60;
				hh_diff -= 1;
			}
			if (hh_diff < 0) { hh_diff += 24; }

			tmpNearestSecondsToWakeup = hh_diff * 60 * 60 + mm_diff * 60 + ss_diff;
			if (tmpNearestSecondsToWakeup < nearestSecondsToWakeup) {
				nearestSecondsToWakeup = tmpNearestSecondsToWakeup;
			}
			LogLinef(4, __FUNCTION__, "B: nearestSecondsToWakeup = %d, tmpNearestSecondsToWakeup=%d", (long int) nearestSecondsToWakeup, (long int) tmpNearestSecondsToWakeup);
			LogLinef(3, __FUNCTION__, "HHMM: %d:%d:%d   now=%d:%d:%d   diff=%d:%d:%d", hh, mm, ss, hh_now, mm_now, ss_now, hh_diff, mm_diff, ss_diff);
		}
		else {
			LogLinef(3, __FUNCTION__, "Sleep time %d NOT OK: %s", i, wakeupTime.c_str());
		}
	}
	return nearestSecondsToWakeup;
}

void SetDeepSleepPeriod(int _secondsToSleep) {

	//Serial.println("max deep sleep: " + uint64ToString(ESP.deepSleepMax()));

	// Find nearest wakeup time set


	if (PersistentMemory.GetdeepSleepEnabled()) {
		if (_secondsToSleep < MIN_SLEEP_TIME_SECS) {
			LogLinef(0, __FUNCTION__, "_secondsToSleep too low: %d. Set to %d", _secondsToSleep, MIN_SLEEP_TIME_SECS);
			_secondsToSleep = MIN_SLEEP_TIME_SECS;
		}
	}
	int secondsToSleep = _secondsToSleep % MAX_DEEP_SLEEP_SECS;
	int maxSleepCycles = _secondsToSleep / MAX_DEEP_SLEEP_SECS;
	LogLinef(1, __FUNCTION__, "secondsToSleep = %d  maxSleepCycles = %d", secondsToSleep, maxSleepCycles);
	PersistentMemory.SetsecondsToSleep(secondsToSleep);
	PersistentMemory.SetmaxSleepCycles(maxSleepCycles);
	LogLinef(1, __FUNCTION__, "Done");
}

void DeepSleepHandlerClass::GoToDeepSleep() {

	uint64_t deepSleepPeriod = 0;
	LogLinef(1, __FUNCTION__, "currentSleepCycle = %d  GetmaxSleepCycles = %d", PersistentMemory.ps.currentSleepCycle, PersistentMemory.GetmaxSleepCycles());

	// continue sleeping?
	if (PersistentMemory.ps.currentSleepCycle < PersistentMemory.GetmaxSleepCycles() ) {
		PersistentMemory.ps.currentSleepCycle++;
		deepSleepPeriod = MAX_DEEP_SLEEP_SECS;
	}
	else {
		LogLine(4, __FUNCTION__, "*** Resetting persistent memory (to reset counters)");
		PersistentMemory.ps.currentSleepCycle = 0;
		// When to wake up next time?
		//deepSleepPeriod = GetNearestWakeupTime();
		SetDeepSleepPeriod(GetNearestWakeupTime());
		deepSleepPeriod = PersistentMemory.GetsecondsToSleep();  // set in SetDeepSleepPeriod
	}

	PersistentMemory.WritePersistentMemory();
	uint32_t secs = deepSleepPeriod;
	deepSleepPeriod = deepSleepPeriod * 1000000;
	if (PersistentMemory.GetdeepSleepEnabled()) {
		LogLinef(1, __FUNCTION__, "Go to DeepSleep.  deepSleepPeriod=%d secs    DeepSleepEnabled:%d", secs, PersistentMemory.GetdeepSleepEnabled());
		ESP.deepSleep(deepSleepPeriod);
	}
	else {
		LogLine(2, __FUNCTION__, "Skip deep sleep (not enabled)");
	}
}

DeepSleepHandlerClass DeepSleepHandler;

