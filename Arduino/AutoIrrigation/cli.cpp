// 
// 
// 

#include "cli.h"
#include <ESP8266WiFi.h>
#include "OTALib.h"


// Basic loop for reading commands to stop the device, etc...
boolean stopped = false;
void cliLoop() {
	String msg = "";
	while (Serial.available() || stopped) {
		if (Serial.available()) {
			msg = Serial.readStringUntil('\n');
		}

		if (msg == "stop") {
			Serial.println("STOPPING!!!");
			stopped = true;
		}

		if (msg == "go") {
			Serial.println("Resume");
			stopped = false;
		}

		if (msg == "sensor") {
			String data = "Wifi: " + String(WiFi.RSSI()) + " db";
			Serial.println(data);
		}

		if (stopped) {
			delayNonBlocking(10000);
			Serial.print(".");
		}
	}
}