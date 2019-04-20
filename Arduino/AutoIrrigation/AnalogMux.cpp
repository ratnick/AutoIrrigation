// Analog multiplexer 74HC4052 (http://www.ti.com/lit/ds/symlink/cd74hc4052.pdf)
// 
// 
/*
Table 2. CD54HC4052, CD74HC4052, CD54HCT4052, CD74HCT4052 Function Table(1)
E  S1 S0 
L  L  L  A0, B0
L  L  H  A1, B1
L  H  L  A2, B2
L  H  H  A3, B3
H  X  X  None
*/

#include "AnalogMux.h"
#include "LogLib.h"

void AnalogMuxClass::init(int _S1, int _S0, int _pwrCtrlPin, boolean _pwrCtrlPinActive)
{
	this->S1 = _S1; 
	this->S0 = _S0; 

	pinMode(S1, OUTPUT);
	pinMode(S0, OUTPUT);
	digitalWrite(S1, LOW);
	digitalWrite(S0, LOW);

	pwrCtrlPin = _pwrCtrlPin;
	if (pwrCtrlPin > 0) {
		pinMode(pwrCtrlPin, OUTPUT);
		pwrCtrlPinActive = _pwrCtrlPinActive;
		digitalWrite(pwrCtrlPin, !pwrCtrlPinActive);
	}

	LogLine(3, __FUNCTION__, "MUX on S0:" + String(S0) + " and S1:" + String(S1) + "  power ctrl:" + String(pwrCtrlPin));
}

void AnalogMuxClass::OpenChannel(int ch) {

	LogLine(3, __FUNCTION__, "Open channel " + String(ch));
	int S1 = this->S1;
	int S0 = this->S0;

	if (pwrCtrlPin > 0) {
		LogLine(4, __FUNCTION__, "activate MUX power");
		digitalWrite(pwrCtrlPin, pwrCtrlPinActive);
		delay(500);
	}
	else {
		LogLine(0, __FUNCTION__, "ERROR: activate MUX power");
	}

	switch (ch) {
	case 0:
		digitalWrite(S1, LOW);
		digitalWrite(S0, LOW);
		break;
	case 1:
		digitalWrite(S1, LOW);
		digitalWrite(S0, HIGH);
		break;
	case 2:
		digitalWrite(S1, HIGH);
		digitalWrite(S0, LOW);
		break;
	case 3:
		digitalWrite(S1, HIGH);
		digitalWrite(S0, HIGH);
		break;
	otherwise:
		LogLine(0, __FUNCTION__, "ERROR: Undefined channel " + String(ch));
		break;
	}
}

void AnalogMuxClass::CloseMUXpwr() {
	if (pwrCtrlPin > 0) {
		digitalWrite(pwrCtrlPin, !pwrCtrlPinActive);
		LogLine(3, __FUNCTION__, "de-activate MUX power");
	}


}



AnalogMuxClass AnalogMux;

