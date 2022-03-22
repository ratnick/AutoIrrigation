/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the Uno and
  Leonardo, it is attached to digital pin 13. If you're unsure what
  pin the on-board LED is connected to on your Arduino model, check
  the documentation at http://arduino.cc

  This example code is in the public domain.

  modified 8 May 2014
  by Scott Fitzgerald
 */


#include "../Arduino/AutoIrrigation/globals.h"

// the setup function runs once when you press reset or power the board
// OTA upload
#include "NNR_OTAupdate.h"
#include <LEAmDNS_Priv.h>
#include <LEAmDNS_lwIPdefs.h>
#include <LEAmDNS.h>
#include <ESP8266mDNS_Legacy.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>

#include "NNR_Logging.h"
#include "TimeLib.h"

// Wifi
#include <NNR_Wifi.h>
#include <Wire.h>
Device device;
const char timeServer[] = "0.dk.pool.ntp.org"; // Danish NTP Server 
//x WiFiClientSecure wifiClient;
byte localmacAddr[6];


// Firebase
#include <LiquidCrystal.h>
#include <jsmn.h>
#include <FirebaseJson.h>


#include "../Arduino/AutoIrrigation/FirebaseModel.h"
#include <FirebaseLib.h>
#include <FirebaseESP8266.h>
#include <FirebaseESP8266HTTPClient.h>
#include <SPI.h>
#include <SD.h>
#include <SDFSFormatter.h>
#include <SDFS.h>
#include <SdFat.h>
#include <WiFiClientSecure.h>

FirebaseJson jsoTelemetry;
FirebaseJson jsoTeleTimestamp;
boolean newTelemetry = true;  // so far only used for gassensor

void setup() {
	delay(400);
	Serial.begin(115200);
	Serial.println("setup 1");
	InitDebugLevel(5);
	ConnectToWifi();

	// FB_BasePath = FB_DEVICE_PATH + PersistentMemory.GetmacAddress();
	char basePath[] = "/irrdevices/AA:BB:CC:DD:EE:FF";

	InitFirebase(basePath);

}

FirebaseData firebaseData; // FirebaseESP8266 data object
void loop() {

	String testStr = "20210203 22:12:32:435 *1* - IsSettingsDataUpdatedByUser - false (not set by user) - \\irrdevices\\84:CC:A8:A5:91:10\\settings\\Updated";
	LogLine(1, __FUNCTION__, testStr.c_str());
	bool res = SendToFirebase(set, "telemetry_current", jsoTelemetry, firebaseData);

	while (true) {
		delayNonBlocking(1000);
		//Serial.print(".");
	}
	jsoTelemetry.add("abcd", 42);
    res = SendToFirebase(set, "telemetry_current", jsoTelemetry, firebaseData);
}

void ConnectToWifi() {
	//int wifiIndex = initWifi(PersistentMemory.ps.wifiSSID, PersistentMemory.ps.wifiPwd, &device.wifi);
	int wifiIndex = initWifi("nohrTDC", "RASMUSSEN", &device.wifi);
	delayNonBlocking(250);
	if (wifiIndex == -1 || WiFi.status() != WL_CONNECTED) {
		LogLine(0, __FUNCTION__, "Could not connect to wifi. ", false);
	}
	else if (wifiIndex == 100 || wifiIndex == 200) {
		// Connected. Do nothing.
	}
	else {   // then we have swapped SSID. Update persistent memory so we keep using the new hotspot
		PrintIPAddress();
	}
}
