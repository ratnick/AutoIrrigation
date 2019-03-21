// test.h

#ifndef _TEST_h
#define _TEST_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

class TestClass
{
 protected:


 public:
	void init();
};

extern TestClass Test;

#endif

