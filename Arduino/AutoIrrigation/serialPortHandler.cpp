// Receives a Json via serial port
// Test the functionality using TestSerialComm.ino
// 

#include "serialPortHandler.h"
#include <SoftwareSerial.h>
#include "LogLib.h"
#include "OTALib.h"

SoftwareSerial serialPort(D1, D2); // (Rx, Tx)
boolean newData = false;

void SetupSerialPort() {
	serialPort.begin(9600);
	delayNonBlocking(500);
	//EmptySerialReadBuffer();

}

void EmptySerialReadBuffer() {
	char c;
	while (serialPort.available() > 0) {
		c = serialPort.read();
	}
}

boolean DataAvailableOnSerialPort() {
	if (serialPort.available() > 0) {
		return true;
	}
	else {
		delayNonBlocking(50);
		if (serialPort.available() > 0) {
			return true;
		}
	}
	return false;
}

boolean receiveUntilMarker(char marker) {
	int i = 0;
	char rc = '\0';
	LogLine(4, __FUNCTION__, "wait for start marker");
	while (DataAvailableOnSerialPort()) {
		rc = serialPort.read();
		//Serial.print(rc);
		if (rc == marker) {
			LogLine(4, __FUNCTION__, "start receiving data");
			return true;
		}
	}
	return false;
}

boolean ReadSerialJsonOnce(char* receivedChars, int jsonMaxSize) {
	boolean recvInProgress = false;
	boolean endMarkerReceived = false;
	boolean timeOut = false;
	int timeOutcnt = 100;
	byte ndx = 0;
	char startMarker = '<';
	char endMarker = '>';
	char rc;

	serialPort.write('X');
	serialPort.flush();
//	Serial.println("X sent ...  now wait");
	delayNonBlocking(100);

	recvInProgress = receiveUntilMarker(startMarker);

	newData = false;
	if (!recvInProgress) {
		LogLine(1, __FUNCTION__, "not receiving from serial port");
		return false;
	}

	while (recvInProgress && !endMarkerReceived && !timeOut) {

		while (DataAvailableOnSerialPort() && newData == false) {
			rc = serialPort.read();
			//Serial.print(rc);

			if (rc != endMarker) {
				receivedChars[ndx] = rc;
				ndx++;
				if (ndx >= jsonMaxSize) {
					ndx = jsonMaxSize - 1;
				}
			}
			else {
				receivedChars[ndx] = '\0'; // terminate the string
				recvInProgress = false;
				ndx = 0;
				newData = true;
				endMarkerReceived = true;
				//Serial.print(endMarker);
				return true;
			}

		}
		Serial.print(".");
		delayNonBlocking(50);
		if (timeOutcnt-- <= 0) { 
			timeOut = true; 
		}
	}
}