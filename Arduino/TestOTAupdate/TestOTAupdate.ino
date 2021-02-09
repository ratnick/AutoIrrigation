#include <Wire.h>
#include <SPI.h>
#include <LogLib.h>
#include <WifiLib.h>
#include <SysCall.h>
#include <sdios.h>
#include <SdFatConfig.h>
#include <SdFat.h>
#include <MinimumSerial.h>
#include <FreeStack.h>
#include <BlockDriver.h>
#include <SDFSFormatter.h>
#include <SDFS.h>
#include <SD.h>
#include <EEPROM.h>
#include <TimeLib.h>        // http://playground.arduino.cc/code/time - installed via library manager

// OTA upload
#include <LEAmDNS_Priv.h>
#include <LEAmDNS_lwIPdefs.h>
#include <LEAmDNS.h>
#include <ESP8266mDNS_Legacy.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
// end

#include <FirebaseESP8266.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiServer.h>
#include <WiFiClient.h>

#include "FirebaseModel.h"
#include "PersistentMemory.h"
#include "globals.h"
#include "compile_flags.h"

// Wifi and network globals
Device device;
const char timeServer[] = "0.dk.pool.ntp.org"; // Danish NTP Server 
//x WiFiClientSecure wifiClient;
byte localmacAddr[6];

// Wifi debug communication

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin 13 as an output.
	Serial.begin(115200);
	Serial.println("SETUP");
    pinMode(BUILTIN_LED, OUTPUT);

	InitDebugLevel(4);

	PersistentMemory.SetdebugLevel(DEBUGLEVEL);
	PersistentMemory.init(FORCE_NEW_VALUES, 0, DEVICE_ID, HARDWARE_DESCRIPTION, DefaultRunmode.c_str(), 0, 0, LOOP_DELAY, DEBUGLEVEL);   // false: read from memory.  true: initialize

	SetFBDebugLevel(PersistentMemory.GetdebugLevel());

	ConnectToWifi();
	WiFi.macAddress(localmacAddr);
	PersistentMemory.SetmacAddress(localmacAddr);

	// TODO: move time fetching to Wifi_nnr and generalize it.
	configTime(0, 0, "pool.ntp.org", "time.nist.gov");   // note - it may take some HOURS before actual time is correct.
	AdjustTime(2);  // configTime adjustments does not work!! Hence we adjust and include summertime permanently.
	while (time(nullptr) < 1510644967) {
		delay(10);
	}
	if (getCurrentTimeB()) {
		LogLine(2, __FUNCTION__, "Time fetched and adjusted");
	}
	else {
		LogLine(0, __FUNCTION__, "Time NOT fetched and adjusted");
	}

	SetupOTA();
}

// the loop function runs over and over again forever
void loop() {

	LogLine(4, __FUNCTION__, "before OTA handling");
	ArduinoOTA.handle();
	LogLine(4, __FUNCTION__, "after OTA handling");

	Serial.print(".");
	digitalWrite(BUILTIN_LED, HIGH);   // turn the LED on (HIGH is the voltage level)
	delay(100);
	digitalWrite(BUILTIN_LED, LOW);    // turn the LED off by making the voltage LOW
	delay(100);
}

void ConnectToWifi() {
	int wifiIndex = initWifi(PersistentMemory.ps.wifiSSID, PersistentMemory.ps.wifiPwd, &device.wifi);
	delay(250);
	if (wifiIndex == -1 || WiFi.status() != WL_CONNECTED) {
		LogLine(0, __FUNCTION__, "Could not connect to wifi. ", false);
	}
	else if (wifiIndex == 100 || wifiIndex == 200) {
		// Connected. Do nothing.
	}
	else {   // then we have swapped SSID. Update persistent memory so we keep using the new hotspot
		PrintIPAddress();
		PersistentMemory.SetwifiSSID(device.wifi.currentSSID);
		PersistentMemory.SetwifiPwd(device.wifi.pwd);
	}
}


void SetupOTA() {

	// Port defaults to 8266
	ArduinoOTA.setPort(8266);

	// Hostname defaults to esp8266-[ChipID]
	ArduinoOTA.setHostname("myesp8266");

	// No authentication by default
	// ArduinoOTA.setPassword("admin");

	// Password can be set with it's md5 value as well
	// MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
	// ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

	ArduinoOTA.onStart([]() {
		String type;
		if (ArduinoOTA.getCommand() == U_FLASH) {
			type = "sketch";
		}
		else { // U_FS
			type = "filesystem";
		}

		// NOTE: if updating FS this would be the place to unmount FS using FS.end()
		Serial.println("Start updating " + type);
	});
	ArduinoOTA.onEnd([]() {
		Serial.println("\nEnd");
	});
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
		Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
	});
	ArduinoOTA.onError([](ota_error_t error) {
		Serial.printf("Error[%u]: ", error);
		if (error == OTA_AUTH_ERROR) {
			Serial.println("Auth Failed");
		}
		else if (error == OTA_BEGIN_ERROR) {
			Serial.println("Begin Failed");
		}
		else if (error == OTA_CONNECT_ERROR) {
			Serial.println("Connect Failed");
		}
		else if (error == OTA_RECEIVE_ERROR) {
			Serial.println("Receive Failed");
		}
		else if (error == OTA_END_ERROR) {
			Serial.println("End Failed");
		}
	});
	ArduinoOTA.begin();
}
