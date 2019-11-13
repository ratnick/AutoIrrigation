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
		LogLinef(4, __FUNCTION__, "invalid number: %s", str.c_str());
		return false;
	}
	else {
		buf[0] = str[0]; buf[1] = str[1];
		n = atoi(buf);
		LogLinef(5, __FUNCTION__, "HH: %s=%d", buf, n);
		if (n > 23) return false;

		buf[0] = str[2]; buf[1] = str[3];
		n = atoi(buf);
		LogLinef(5, __FUNCTION__, "MM: %s=%d", buf, n);
		if (n > 59) return false;

		buf[0] = str[4]; buf[1] = str[5];
		n = atoi(buf);
		LogLinef(5, __FUNCTION__, "SS: %s=%d", buf, n);
		if (n > 59) return false;
	}
	LogLine(4, __FUNCTION__, "TRUE");
	return true;
}

void DeepSleepHandlerClass::GotoSleepAndWakeUpAtTime(String wakeupTime) {

	LogLinef(2, __FUNCTION__, "Wake up at: %s", wakeupTime.c_str());

	// check that input is OK and convert to time_t
	if (isValidTimeString(wakeupTime)) {
		PersistentMemory.SetWakeTime(WAKEUP_IDX_SIMPLEDELAY, wakeupTime);
		GoToDeepSleep();
	}
	else {
		LogLinef(0, __FUNCTION__, "NOT SET TO SLEEP AS REQUESTED. wakeupTime is out of bound. Format is HHMMSS");
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
	boolean foundValue = false;

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
				foundValue = true;
			}
			LogLinef(4, __FUNCTION__, "B: nearestSecondsToWakeup = %d, tmpNearestSecondsToWakeup=%d", (long int) nearestSecondsToWakeup, (long int) tmpNearestSecondsToWakeup);
			LogLinef(3, __FUNCTION__, "HHMM: %d:%d:%d   now=%d:%d:%d   diff=%d:%d:%d", hh, mm, ss, hh_now, mm_now, ss_now, hh_diff, mm_diff, ss_diff);
		}
		else {
			LogLinef(3, __FUNCTION__, "Sleep time %d NOT OK: %s", i, wakeupTime.c_str());
		}
	}
	if (!foundValue) { nearestSecondsToWakeup = 0; }
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
	float floatSecondsToSleep = _secondsToSleep * DRIFTING_ADJUSTMENT;
	_secondsToSleep = lroundf(floatSecondsToSleep);
	int secondsToSleep = _secondsToSleep % MAX_DEEP_SLEEP_SECS;
	int maxSleepCycles = _secondsToSleep / MAX_DEEP_SLEEP_SECS;
	LogLinef(1, __FUNCTION__, "secondsToSleep = %d  maxSleepCycles = %d", secondsToSleep, maxSleepCycles);
	PersistentMemory.SetsecondsToSleep(secondsToSleep);
	PersistentMemory.SetmaxSleepCycles(maxSleepCycles);
	LogLinef(1, __FUNCTION__, "Done");
}

boolean DeepSleepHandlerClass::ContinueSleeping() {

	if ((PersistentMemory.ps.currentSleepCycle == 0) && (PersistentMemory.GetsecondsToSleep() == MAX_DEEP_SLEEP_SECS)) {
		LogLine(4, __FUNCTION__, "continue sleeping");
		return true;
	}
	else {
		LogLine(4, __FUNCTION__, "stop sleeping");
		return false;
	}
}

void DeepSleepHandlerClass::GoToDeepSleep() {

	boolean lastCycle = false;
	uint64_t deepSleepPeriod = 0;
	LogLinef(1, __FUNCTION__, "Start: currentSleepCycle = %d  GetmaxSleepCycles = %d", PersistentMemory.ps.currentSleepCycle, PersistentMemory.GetmaxSleepCycles());

	// if currentSleepCycle > 0 then we have an ongoing sleep session
	if (PersistentMemory.ps.currentSleepCycle == 1) {            
		// last cycle. Reset counters
		LogLine(4, __FUNCTION__, "Resetting sleep counters");
		deepSleepPeriod = PersistentMemory.GetsecondsToSleep();
		PersistentMemory.ps.currentSleepCycle = 0;
		PersistentMemory.SetsecondsToSleep(0);  
		PersistentMemory.SetmaxSleepCycles(0);
		PersistentMemory.SetWakeTime(WAKEUP_IDX_SIMPLEDELAY, "HHMMSS\0");
		lastCycle = true;
	}
	if (PersistentMemory.ps.currentSleepCycle > 1) {        
		// just continue
		PersistentMemory.ps.currentSleepCycle--;
		deepSleepPeriod = MAX_DEEP_SLEEP_SECS;
	} 


	// if currentSleepCycle = 0, we are about to start a new sleep session. Find out how long that is.
	if ((PersistentMemory.ps.currentSleepCycle <= 0) && (!lastCycle)) {
		LogLine(4, __FUNCTION__, "Starting new sleep cycle");
		// When to wake up next time?
		SetDeepSleepPeriod(GetNearestWakeupTime());
		PersistentMemory.SetWakeTime(WAKEUP_IDX_SIMPLEDELAY, "HHMMSS\0");
		if (PersistentMemory.GetmaxSleepCycles() > 0) {
			deepSleepPeriod = MAX_DEEP_SLEEP_SECS;
			PersistentMemory.ps.currentSleepCycle = PersistentMemory.GetmaxSleepCycles();
		}
		else {
			deepSleepPeriod = PersistentMemory.GetsecondsToSleep();  // set in SetDeepSleepPeriod
			PersistentMemory.ps.currentSleepCycle = 0;
		}
	}

	PersistentMemory.WritePersistentMemory();
	if (deepSleepPeriod < MIN_SLEEP_TIME_SECS) { deepSleepPeriod = MIN_SLEEP_TIME_SECS; }
	uint32_t secs = deepSleepPeriod;
	deepSleepPeriod = deepSleepPeriod * 1000000;
	if (PersistentMemory.GetdeepSleepEnabled()) {
		LogLinef(1, __FUNCTION__, "Finish: currentSleepCycle = %d  GetmaxSleepCycles = %d", PersistentMemory.ps.currentSleepCycle, PersistentMemory.GetmaxSleepCycles());
		LogLinef(1, __FUNCTION__, "Go to DeepSleep.  deepSleepPeriod=%d secs    DeepSleepEnabled:%d", secs, PersistentMemory.GetdeepSleepEnabled());
		ESP.deepSleep(deepSleepPeriod);
	}
	else {
		LogLine(2, __FUNCTION__, "Skip deep sleep (not enabled)");
	}
}

void TestHelper1(int idx, int secondsToSleep) {
	char wakeupTime[7];
	uint64_t t = GetOnboardTime() + secondsToSleep;
	ConvertToShortTimeStr(t, wakeupTime);
	LogLinef(3, __FUNCTION__, "Time string = %s", wakeupTime);
	PersistentMemory.SetWakeTime(idx, wakeupTime);

}


void DeepSleepHandlerClass::TestDeepSleep() {

	// Before running:
	//  - change MAX_DEEP_SLEEP_SECS from 1801 to 30 seconds, 
	//  - First two lines in Setup():
	// 			InitUnitTest();							// remember to remove this
	//			DeepSleepHandler.TestDeepSleep();	// remember to remove this
	
	// Note: We use valveOpenDuration() as step counter in this procedure. Set to 0 using Firebase interface before starting.

	int curStep = PersistentMemory.GetvalveOpenDuration();
	PersistentMemory.SetvalveOpenDuration(curStep + 1);
	LogLine(0, __FUNCTION__, "=============================================");
	LogLinef(0, __FUNCTION__, "*********** STEP %d ", curStep);
	LogLine(0, __FUNCTION__, "=============================================");
	switch (curStep) {

	case 0:
		// reset all 
		for (int i = 0; i < 4; i++) {
			PersistentMemory.SetWakeTime(i, "hhmmss");
		}
		PersistentMemory.SettotalSecondsToSleep(0);

		LogLine(4, __FUNCTION__, "set totSlp to 20s => deep sleep 20s");
		LogLine(0, __FUNCTION__, "=============================================");
		DeepSleepHandler.GotoSleepAndWakeAfterDelay(20);
		break;

	case 1:
		LogLine(4, __FUNCTION__, "sleep => deep sleep minimum (10s) because no value is set");
		LogLine(0, __FUNCTION__, "=============================================");
		DeepSleepHandler.GoToDeepSleep();
		break;

	case 2:
		LogLine(4, __FUNCTION__, "set totSlp to 40s => 1 full + 10 s");
		LogLine(0, __FUNCTION__, "=============================================");
		DeepSleepHandler.GotoSleepAndWakeAfterDelay(40);
		break;

	case 3:
		PersistentMemory.SettotalSecondsToSleep(20);
		LogLine(4, __FUNCTION__, "set totSlp to 20s (but done wrong) => deep sleep 10s (minimum)");
		LogLine(0, __FUNCTION__, "=============================================");
		DeepSleepHandler.GoToDeepSleep();
		break;

	case 4:
		LogLine(4, __FUNCTION__, "sleep => deep sleep 10s (minimum), still not set correctly");
		LogLine(0, __FUNCTION__, "=============================================");
		DeepSleepHandler.GoToDeepSleep();
		break;

	case 5:
		LogLine(4, __FUNCTION__, "set time1 = now + 75s. ");
		TestHelper1(1, 75);
		LogLine(4, __FUNCTION__, "set totSlp to 120s => 2 full + 15s => wake up after 1:10 ");
		LogLine(0, __FUNCTION__, "=============================================");
		DeepSleepHandler.GotoSleepAndWakeAfterDelay(120);
		break;

	case 6:
		LogLine(4, __FUNCTION__, "set totSlp to 20s => deep sleep 20s");
		LogLine(0, __FUNCTION__, "=============================================");
		DeepSleepHandler.GotoSleepAndWakeAfterDelay(20);
		break;

	case 7:
		LogLine(4, __FUNCTION__, "set time2 = now + 180s");
		TestHelper1(1, 180);
		LogLine(4, __FUNCTION__, "sleep => deep sleep 20s");
		LogLine(0, __FUNCTION__, "=============================================");
		DeepSleepHandler.GotoSleepAndWakeAfterDelay(20);
		break;

	case 8:
		LogLine(4, __FUNCTION__, "sleep => deep sleep 20s");
		LogLine(0, __FUNCTION__, "=============================================");
		DeepSleepHandler.GoToDeepSleep();
		break;

	case 9:
		LogLine(4, __FUNCTION__, "set totSlp to 0s  => deep sleep until time2");
		LogLine(0, __FUNCTION__, "=============================================");
		PersistentMemory.SettotalSecondsToSleep(0);
		DeepSleepHandler.GoToDeepSleep();
		break;

	default:
		LogLine(4, __FUNCTION__, "TEST COMPLETED");
		PersistentMemory.SetvalveOpenDuration(0);
	}
}

DeepSleepHandlerClass DeepSleepHandler;

