// AnalogMux.h (http://www.ti.com/lit/ds/symlink/cd74hc4052.pdf)

#ifndef _ANALOGMUX_h
#define _ANALOGMUX_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

class AnalogMuxClass
{
 private:

	 int S0 = 0;
	 int S1 = 0;
	 int pwrCtrlPin; // if a pin is used to control power relay / optocoupler or similar. Set to zero if unused
	 boolean pwrCtrlPinActive;  

 public:
	void init(int _S0, int _S1, int _pwrCtrlPin, boolean _pwrCtrlPinActive);
	void OpenChannel(int ch);
	void CloseMUXpwr();
};

extern AnalogMuxClass AnalogMux;

#endif

