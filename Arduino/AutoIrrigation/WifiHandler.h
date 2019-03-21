// WifiHandler.h

#ifndef _WIFIHANDLER_h
#define _WIFIHANDLER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

class WifiHandlerClass
{
 protected:

 public:
	boolean init(boolean usePredefinedSSID);

private:
	int WifiIndex = 0;
	unsigned long LastWifiTime = 0;
	int WiFiConnectAttempts = 0;
	int wifiPairs = 1;
	struct WifiPair {
		const String ssid;
		const String pwd;
	};
	
};

extern WifiHandlerClass WifiHandler;

#endif

