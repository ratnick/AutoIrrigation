// backoff.h

#ifndef _BACKOFF_h
#define _BACKOFF_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

void resetBackoff();
bool backoff();

#endif

