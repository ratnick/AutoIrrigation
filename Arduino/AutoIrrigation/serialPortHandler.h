// serialPortHandler.h

#ifndef _SERIALPORTHANDLER_h
#define _SERIALPORTHANDLER_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

void SetupSerialPort();
void EmptySerialReadBuffer();
boolean DataAvailableOnSerialPort();
boolean receiveUntilMarker(char marker);
boolean ReadSerialJsonOnce(char* receivedChars, int jsonMaxSize);

#endif

