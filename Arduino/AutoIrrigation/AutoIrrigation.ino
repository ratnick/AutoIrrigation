/*
    Name:       AutoIrrigation.ino
    Created:	17-09-2018 15:08:50
    Author:     Nikolaj Nøhr-Rasmussen
*/

#include "LEDHandler.h"
#include "AnalogMux.h"
#define HARDWARE_DESCRIPTION "WeMOS D1 r2, 5V/12V valve switch"
#define DEVICE_ID "prototype"

#define USE_WIFI

// Hmmm - which cloud to use:
//#define USE_GOOGLE_CLOUD
//#define USE_AZURE
#define USE_FIREBASE

// Program control:
//#define RUN_ONCE
#define USE_DEEP_SLEEP				// When enabling, connect D0 to RST (on Wemos D1 mini)
#define NBR_OF_LOOPS_BEFORE_SLEEP 1
#define DEEP_SLEEP_SOAK_THRESHOLD 20     // if soaking time exceeds this limit, we will use deep sleep instead of delay()
#define LOOP_DELAY 10 //secs
#define FORCE_NEW_VALUES false // Will overwrite all values in persistent memory. Enable once, disable and recompile
//#define MEASURE_INTERNAL_VCC      // When enabling, we cannot use analogue reading of sensor. 
#define SIMULATE_WATERING false      // open the valve in every loop
// See also SIMULATE_SENSORS in SensorHandler.h

#ifdef USE_WIFI
#include <ArduinoJson.hpp>
#include <ArduinoJson.h>
#include <WiFiUdp.h>
#include <WiFiServer.h>
#include <WiFiClient.h>
#endif

#ifdef USE_FIREBASE
#include <SD.h>
#include <SPI.h>
#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>
#include "FirebaseModel.h"
#endif

#ifdef USE_GOOGLE_CLOUD
#include <jwt.h>
#include <CloudIoTCoreMQTTClient.h>
#include <CloudIoTCoreDevice.h>
#include <CloudIoTCore.h>
#include <PubSubClient.h>
#include <PipedStream.h>
#include <LoopbackStream.h>
#include <rBase64.h>
#include <CertStoreBearSSL.h>
#include <BearSSLHelpers.h>
#include "backoff.h"
#endif

#include "cli.h"
#include <LogLib.h>
#include <EEPROM.h>
#include <SPI.h>
#include <TimeLib.h>        // http://playground.arduino.cc/code/time - installed via library manager
#include <time.h>
#include "globals.h"
#include "ciotc_config.h" // Wifi configuration here
#include "WaterValve.h"
#include "WaterSensor.h"
#include "VoltMeter.h"
#include "DeepSleepHandler.h"
#include "LEDHandler.h"
#include "PersistentMemory.h"
#ifdef USE_AZURE
#include "AzureCloudHandler.h"
#endif

// Wifi and network globals
DeviceConfig wifiDevice;
const char timeServer[] = "0.dk.pool.ntp.org"; // Danish NTP Server 
WiFiClientSecure wifiClient;
byte macAddr[6];
//WiFiClient wifiClient;

//Hardware pin configuration on WeMOS D1
const int MUX_S0 = D8; // S0 = Pin A on schematic
const int MUX_S1 = D7; // S1 = Pin B on schematic
const int HUM_SENSOR_PWR_CTRL = D1;
const int VALVE_CTRL = D5;
const int ANALOG_INPUT = A0;
const int CHANNEL_HUM = 0;
const int CHANNEL_BATT = 1;

// Sensors and actuators
WaterSensorClass waterSensorA;
WaterValveClass waterValveA;
VoltMeterClass externalVoltMeter;
const int VALVE_OPEN_DURATION = 2;   // time a valve can be open in one run-time cycle, i.e. between deepsleeps
const int VALVE_SOAK_TIME     = 30;   // time between two valve openings

// General control
int loopCount = 0;  // used in conjunction with NBR_OF_LOOPS_BEFORE_SLEEP
const char SOFTWARE_VERSION[] = "AutoIrrigation.ino - Compiled: " __DATE__ " " __TIME__;

#ifdef USE_GOOGLE_CLOUD
// Google Cloud globals
CloudIoTCoreDevice *device;
#endif

#ifdef USE_FIREBASE
#define FIREBASE_HOST "irrfire.firebaseio.com" 
#define FIREBASE_AUTH "w3Q5F3zWG3RkudEjGUJZVi2wiVDvZCKY3VkVywkC"
#define FB_DEVICE_PATH "/irrdevices/"
String FB_BasePath;
FirebaseData firebaseData; // FirebaseESP8266 data object
#endif
const int JSON_BUFFER_LENGTH = 250;

void ConnectAndUploadToCloud(UploadType uploadType, boolean firstRun = false);

void setup() {
	Serial.begin(115200);
	InitDebugLevel(2);

	// read persistent memory
	PersistentMemory.init(false);   // false: read from memory.  true: initialize
	//PersistentMemory.Printps();
	//PersistentMemory.UnitTest();  // just for testing
	//PersistentMemory.Printps();

	LogLine(1, __FUNCTION__, "\n\nSleep cycle " + String(PersistentMemory.ps.currentSleepCycle) + " of " + String(PersistentMemory.ps.maxSleepCycles) + "\n");
	if (PersistentMemory.ps.currentSleepCycle != 0) {
		DeepSleepHandler.GoToDeepSleep();
	}

	initFlashLED();
	AnalogMux.init(MUX_S1, MUX_S0, HUM_SENSOR_PWR_CTRL, HIGH);
	externalVoltMeter.lastSummarizedReading = PersistentMemory.ps.lastVccSummarizedReading;
	externalVoltMeter.init(ANALOG_INPUT, "5V voltmeter", CHANNEL_BATT, SensorHandlerClass::ExternalVoltMeter, PersistentMemory.ps.lastVccSummarizedReading);
	waterSensorA.init(ANALOG_INPUT, "humidity sensor A", CHANNEL_HUM, SensorHandlerClass::SoilHumiditySensor, PersistentMemory.ps.humLimit);
	waterValveA.init(VALVE_CTRL, "water valve A", PersistentMemory.ps.valveOpenDuration, PersistentMemory.ps.valveSoakTime);
	#ifdef USE_WIFI 
		ConnectToWifi();
		WiFi.macAddress(macAddr);
		PersistentMemory.SetmacAddress(macAddr);

		// TODO: move time fetching to Wifi_nnr and generalize it.
		configTime(0, 0, "pool.ntp.org", "time.nist.gov");   // note - it may take some HOURS before actual time is correct.
		AdjustTime(2);  // configTime adjustments does not work!! Hence we adjust and include summertime permanently.
		while (time(nullptr) < 1510644967) {
			delay(10);
		}
		getCurrentTime();
		LogLine(1, __FUNCTION__, "Time fetched and adjusted");
	#endif

	#ifdef USE_GOOGLE_CLOUD
		device = new CloudIoTCoreDevice(project_id, location, registry_id, device_id, private_key_str);
		//Serial.println(getJwt());
		//getJwt();
		//LogLine(1, __FUNCTION__, "Renewed JWT");
		//ConnectAndUploadToCloud(GetConfig);
		//ConnectAndUploadToCloud(UploadState);
	#endif
	#ifdef USE_FIREBASE
		FB_BasePath = FB_DEVICE_PATH + PersistentMemory.GetmacAddress();
		Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);  
		//RemoveTelemetryFromFirebase(); UploadLog_("Setup(): ALL TELEMETRY REMOVED");
		ConnectAndUploadToCloud(UploadStateAndSettings);
	#endif
}

boolean WaterIfNeeded() {
	int del;
	float vccTmp;
	boolean valveWasOpened = false;

	vccTmp = externalVoltMeter.ReadVoltage();
	// update persistent memory in case something has changed
	PersistentMemory.ps.lastVccSummarizedReading = externalVoltMeter.lastSummarizedReading;
	PersistentMemory.WritePersistentMemory();
	LogLine(1, __FUNCTION__, "Voltage reading" + String(vccTmp));

	LED_Flashes(1, 300);
	if ((waterSensorA.CheckIfWater() == WaterSensor.DRY) || SIMULATE_WATERING) {
		LogLine(1, __FUNCTION__, "Low water detected. Watering and soaking.");

		waterValveA.OpenValve();
		ConnectAndUploadToCloud(UploadTelemetry);
		waterValveA.KeepOpen();

		waterValveA.CloseValve();
		ConnectAndUploadToCloud(UploadTelemetry);
		valveWasOpened = true;
	}
	if (!valveWasOpened) {
		LogLine(1, __FUNCTION__, "High water detected. Skip watering and soaking.");
		ConnectAndUploadToCloud(UploadTelemetry);  // this is to ensure we send a telemetry at least once
	}
	return valveWasOpened;
}

int wifiIndex = 0;
boolean firstRun = false; // TODO REMOVE  true;
void loop() {
	String rm;

	ConnectAndUploadToCloud(GetSettings, firstRun);  // maybe the user has changed values via the app
	firstRun = false;

	rm = PersistentMemory.GetrunMode();
	LogLine(4, __FUNCTION__, "runmode = " + rm);
	if (rm.equals("normal")) {
		if (WaterIfNeeded()) {
			// use deep sleep if it's enabled and we want to soak for a longer period of time
			if ( (waterValveA.soakSeconds <= DEEP_SLEEP_SOAK_THRESHOLD) || (!PersistentMemory.GetdeepSleepEnabled()) ) {
				waterValveA.WaitToSoak();
			} else {
				DeepSleepHandler.SetDeepSleepPeriod(waterValveA.soakSeconds);
				DeepSleepHandler.GoToDeepSleep();
			}
		}
		if (loopCount++ >= NBR_OF_LOOPS_BEFORE_SLEEP) {
			DeepSleepHandler.SetDeepSleepPeriod(PersistentMemory.GettotalSecondsToSleep());
			DeepSleepHandler.GoToDeepSleep();
		}
	}
	else if (rm.equals("soil")) {
		waterSensorA.TestSensor();
		ConnectAndUploadToCloud(UploadTelemetry);
	}
	else if (rm.equals("batt")) {
		externalVoltMeter.TestSensor();
		ConnectAndUploadToCloud(UploadTelemetry);
	}
	else if (rm.equals("testhw")) {
		testHardware();
	}
	else {
		LogLine(0, __FUNCTION__, "Illegal option chosen: " + rm + " Resetting runMode to normal");
		PersistentMemory.SetrunMode("normal");
		LED_Flashes(200, 100);
	}


	#ifdef RUN_ONCE
		while (true) {
			delay(10000);
			LogLine(1, __FUNCTION__, "wait forever");
		}
	#else
	int d = PersistentMemory.GetmainLoopDelay();
		LogLine(2, __FUNCTION__, "waiting mainLoopDelay: " + String(d));
		delay(d * 1000);
	#endif
}

void ConnectAndUploadToCloud(UploadType uploadType, boolean firstRun) {
	HTTPRequestReturnType requestResult;
	String logTxt = "";

	requestResult = notCalledYet;
#ifdef USE_WIFI
	if (WiFi.status() == WL_CONNECTED) {
		LogLine(4, __FUNCTION__, "wifi connected OK");

		#ifdef USE_FIREBASE
			String s;
			String s_tmp;
			DynamicJsonBuffer  jsonBufferRoot(JSON_BUFFER_LENGTH);
			DynamicJsonBuffer  jsonBufferTele(JSON_BUFFER_LENGTH);

			String jsonStr;
			// Indents correspond to hierachy
			JsonObject& jsoStatic = jsonBufferRoot.createObject();
				JsonObject& jsoMetadata		= jsoStatic.createNestedObject("metadata");
				JsonObject& jsoSettings		= jsoStatic.createNestedObject("settings");
				JsonObject& jsoState		= jsoStatic.createNestedObject("state");
					JsonObject& stateTime			= jsoState.createNestedObject("timestamp");
				JsonObject& jsoTelemetryCur = jsoStatic.createNestedObject("telemetry_current");
				JsonObject& jsoTelemetry	= jsoStatic.createNestedObject("telemetry");
					JsonObject& jsoTeleTimestamp	= jsoTelemetry.createNestedObject("timestamp");
				JsonObject& jsoLog			= jsoStatic.createNestedObject("log");

			switch (uploadType) {
				case GetSettings:				GetSettings_(firstRun); break;
				case UploadStateAndSettings:	UploadStateAndSettings_(stateTime, jsoStatic, jsoMetadata, jsoState, jsoSettings, jsoTelemetryCur, jsoTelemetry, jsoLog);			break;
				case UploadTelemetry:			UploadTelemetry_(jsoTelemetry, jsoTeleTimestamp);			break;
				default:
					LogLine(0, __FUNCTION__, "Error - JSON Command not defined");
					break;
			}
		#endif

		#ifdef USE_GOOGLE_CLOUD
					do {
						if (backoff()) {
							getJwt();

							switch (uploadType) {
							case UploadTelemetry:
								LogLine(1, __FUNCTION__, "** SEND TELEMETRY");
								requestResult = sendHttpMsg(uploadType, CreateTelemetryJson());
								break;
							case UploadStateAndSettings:
								LogLine(1, __FUNCTION__, "** UPLOAD STATE");
								requestResult = sendHttpMsg(uploadType, PersistentMemory.GetStateJson());
								break;
							case GetSettings:
								LogLine(1, __FUNCTION__, "** GET CONFIG");
								requestResult = sendHttpMsg(uploadType, "");
								break;
							default:
								LogLine(0, __FUNCTION__, "Error - variant not defined");
								break;
							}

							switch (requestResult) {
							case HTTPOK:
								LogLine(1, __FUNCTION__, "Successful HTTP transmission");
								break;
							case Auth401Error:
								LogLine(0, __FUNCTION__, "HTTP / 1.1 401 Authorization Error");
								SOFT_RESET_ARDUINO();
								break;
							case Forbidden403Error:
								LogLine(0, __FUNCTION__, "HTTP / 1.1 403 Forbidden Error");
								SOFT_RESET_ARDUINO();
								break;
							case WifiTimeoutError:
								LogLine(0, __FUNCTION__, "Wifi timeout");
								break;
							default:
								LogLine(0, __FUNCTION__, "Unidentified error in HTTP request");
								break;
							}
						}
						else {
							delay(1000);
						}
						LogLine(3, __FUNCTION__, "Requestresult=" + String(requestResult));
					} while (requestResult != HTTPOK);

					delay(10); // too fast
					cliLoop();
		#endif

	}
	else {  // wifi not connected
		// TODO: recheck wifi requested by user through push button
		ConnectToWifi();
	}
#endif
}

boolean IsPersistentMemorySet() {
	// Check if values are reasonable. Not bullit proof!
	return (PersistentMemory.GetCloudUsername().equals("initial value"));
}

void InitPersistentMemoryIfNeeded() {
	if (!IsPersistentMemorySet() || FORCE_NEW_VALUES) {
		int totalSecondsToSleep = 20;
		PersistentMemory.init(true, totalSecondsToSleep, DEVICE_ID, VALVE_OPEN_DURATION, VALVE_SOAK_TIME, LOOP_DELAY);   // false: read from memory.  true: initialize
		DeepSleepHandler.SetDeepSleepPeriod(totalSecondsToSleep);
		WiFi.macAddress(macAddr);
		PersistentMemory.SetmacAddress(macAddr);
		#if defined (USE_DEEP_SLEEP)
			PersistentMemory.SetdeepSleepEnabled(true);
		#else
			PersistentMemory.SetdeepSleepEnabled(false);
		#endif
	}
}

#ifdef USE_FIREBASE

boolean IsSettingsDataUpdatedByUser() {
	String s = FB_BasePath + "/settings/" + fb.UserUpdate;
	if (Firebase.getBool(firebaseData, s)) {
		if (firebaseData.boolData()) {
			LogLine(4, __FUNCTION__, "true - " + s);
			return true;
		}
		else {
			LogLine(4, __FUNCTION__, "false (not set by user) - " + s);
		}
	}
	else {
		LogLine(4, __FUNCTION__, "false (data does not exist in Firebase) - " + s + " " + firebaseData.errorReason());
	}
	return false;
}

boolean DeviceExistsInFirebase() {
	if (!Firebase.getString(firebaseData, FB_BasePath + "/state/" + fb.SSID)) {
		UploadLog_("Device seems not to exist. Double checking.");
		if (!Firebase.getString(firebaseData, FB_BasePath + "/metadata/" + fb.macAddr)) {
			LogLine(0, __FUNCTION__, "** Device does not exist");
			return false;
		}
	}
	return true;
}

void RemoveDummiesFromFirebase(String path) {
	String s = FB_BasePath + path;   // "/telemetry/x";
	String res;
	if (Firebase.getString(firebaseData, s)) {
		LogLine(2, __FUNCTION__, "dummy exists. Will remove it. " + s);
		Firebase.deleteNode(firebaseData, s);
	}
	else {
		LogLine(2, __FUNCTION__, "dummy did not exist");
	}
}

void RemoveTelemetryFromFirebase() {
	String s = FB_BasePath + "/telemetry";
	LogLine(2, __FUNCTION__, "	removing telemetry");
	Firebase.deleteNode(firebaseData, s);
}

void GetSettings_(boolean firstRun) {
	int val;
	boolean b;
	LogLine(4, __FUNCTION__, "begin");
	if (firstRun || IsSettingsDataUpdatedByUser()) {
		if (Firebase.getString(firebaseData, FB_BasePath + "/metadata/" + fb.location)) { 
			PersistentMemory.SetdeviceLocation(firebaseData.stringData()); }
		if (Firebase.getString(firebaseData, FB_BasePath + "/metadata/" + fb.deviceID)) { 
			PersistentMemory.SetdeviceID(firebaseData.stringData()); }
		if (Firebase.getInt(firebaseData, FB_BasePath + "/settings/" + fb.mainLoopDelay)) {
			PersistentMemory.SetmainLoopDelay(firebaseData.intData()); }
		if (Firebase.getBool(firebaseData, FB_BasePath + "/settings/" + fb.deepSleepEnabled)) {
			b = firebaseData.boolData();
			LogLine(3, __FUNCTION__, fb.deepSleepEnabled + "=" + String(b));
			PersistentMemory.SetdeepSleepEnabled(b);	
		}
		if (Firebase.getString(firebaseData, FB_BasePath + "/settings/" + fb.runMode)) {
			PersistentMemory.SetrunMode(firebaseData.stringData()); }
		if (Firebase.getInt(firebaseData, FB_BasePath + "/settings/" + fb.totalSecondsToSleep)) {
			PersistentMemory.SettotalSecondsToSleep(firebaseData.intData());
			DeepSleepHandler.SetDeepSleepPeriod(PersistentMemory.GettotalSecondsToSleep());
		}
		if (Firebase.getInt(firebaseData, FB_BasePath + "/settings/" + fb.openDur)) {
			val = firebaseData.intData();
			PersistentMemory.SetvalveOpenDuration(val);
			waterValveA.SetvalveOpenDuration(val);
		}
		if (Firebase.getInt(firebaseData, FB_BasePath + "/settings/" + fb.humLimit)) {
			val = firebaseData.intData();
			PersistentMemory.SethumLimit(val);
			waterSensorA.SethumLimitPct(val);
		}

		if (Firebase.getInt(firebaseData, FB_BasePath + "/settings/" + fb.soakTime)) {
			val = firebaseData.intData();
			LogLine(3, __FUNCTION__, fb.soakTime + "=" + String(val));
			PersistentMemory.SetvalveSoakTime(val);
			waterValveA.SetvalveSoakTime(val);
		}

		Firebase.setBool(firebaseData, FB_BasePath + "/settings/" + fb.UserUpdate, false);

		PersistentMemory.Printps();
		UploadLog_("New settings read");
	}
}

void CreateNewDevice(
	JsonObject& jsoStatic, JsonObject& jsoMetadata, JsonObject& jsoState, JsonObject& jsoSettings,
	JsonObject& jsoTelemetryCur, JsonObject& jsoTelemetry, JsonObject& jsoLog) {

	InitPersistentMemoryIfNeeded();

	jsoTelemetryCur["x"] = "-";
	jsoTelemetry["x"] = "-";
	jsoLog["x"] = "-";

	jsoMetadata[fb.macAddr] = PersistentMemory.GetmacAddress();
	jsoState[fb.SSID] = PersistentMemory.GetwifiSSID();
	jsoSettings[fb.runMode] = "normal";
	jsoSettings[fb.mainLoopDelay] = PersistentMemory.GetmainLoopDelay();
	jsoSettings[fb.totalSecondsToSleep] = PersistentMemory.GettotalSecondsToSleep();
	jsoSettings[fb.UserUpdate] = false;
	jsoSettings[fb.openDur] = PersistentMemory.GetvalveOpenDuration();
	jsoSettings[fb.soakTime] = PersistentMemory.GetvalveSoakTime();
	jsoSettings[fb.humLimit] = PersistentMemory.GethumLimit();
	if (PersistentMemory.GetdeepSleepEnabled()) {  // It only works if using "true" or "false". ??
		jsoSettings[fb.deepSleepEnabled] = true;
	}
	else {
		jsoSettings[fb.deepSleepEnabled] = false;
	}

	jsoStatic.prettyPrintTo(Serial);
	Firebase.setJSON(firebaseData, FB_BasePath, ConvertJsonToString(jsoStatic));

	SendToFirebase("set", "metadata", jsoMetadata);
	SendToFirebase("set", "state", jsoState);
	SendToFirebase("set", "settings", jsoSettings);
}

void UploadStateAndSettings_(
	JsonObject& stateTime, JsonObject& jsoStatic, JsonObject& jsoMetadata, 
	JsonObject& jsoState, JsonObject& jsoSettings, JsonObject& jsoTelemetryCur, JsonObject& jsoTelemetry,
	JsonObject& jsoLog) {

	boolean settingsUpdatedByUser = false;
	LogLine(4, __FUNCTION__, "begin");

	// Check if device has been created. 
	// If not, create it. 
	// If it exists, read current state values from Firebase into persistent memory
	if ( !DeviceExistsInFirebase() ) {
		CreateNewDevice(jsoStatic, jsoMetadata, jsoState, jsoSettings, jsoTelemetryCur, jsoTelemetry, jsoLog);
		LogLine(3, __FUNCTION__, FB_BasePath + " created");
		UploadLog_(FB_BasePath + " created");
	}
	else {
		ConnectAndUploadToCloud(GetSettings);
		RemoveDummiesFromFirebase("/telemetry/x");
	}

	if (IsSettingsDataUpdatedByUser()) {
		ConnectAndUploadToCloud(GetSettings);
	}

	// update values in persistent memory and write them back to Firebase
	stateTime[".sv"] = "timestamp";

	jsoMetadata[fb.macAddr] = PersistentMemory.GetmacAddress();
	jsoMetadata[fb.location] = PersistentMemory.GetdeviceLocation();
	jsoMetadata[fb.deviceID] = PersistentMemory.GetDeviceID();
	jsoMetadata[fb.softwareVersion] = SOFTWARE_VERSION;
	jsoMetadata[fb.hardware] = HARDWARE_DESCRIPTION;
	SendToFirebase("set", "metadata", jsoMetadata);

	jsoState[fb.SSID] = PersistentMemory.GetwifiSSID();
	jsoState[fb.secsToSleep] = PersistentMemory.GetsecondsToSleep();
	jsoState[fb.maxSlpCycles] = PersistentMemory.GetmaxSleepCycles();
	jsoState[fb.curSleepCycle] = PersistentMemory.GetcurrentSleepCycle();
	#ifdef RUN_ONCE
		jsoState[fb.runOnce] = true;
	#else
		jsoState[fb.runOnce] = false;
	#endif
	SendToFirebase("set", "state", jsoState);

	// Is there a need to update settings at all?
/*	jsoSettings[fb.UserUpdate] = false;
	jsoSettings[fb.totalSecondsToSleep] = PersistentMemory.GettotalSecondsToSleep();

	jsoSettings[fb.openDur] = PersistentMemory.GetvalveOpenDuration();
	jsoSettings[fb.soakTime] = PersistentMemory.GetvalveSoakTime();
	jsoSettings[fb.mainLoopDelay] = PersistentMemory.GetmainLoopDelay();
	#if defined(MEASURE_INTERNAL_VCC) 
		jsoSettings[fb.runMode] = "Internal Voltage";
	#else
		jsoSettings[fb.runMode] = PersistentMemory.GetrunMode();
	#endif
	if (PersistentMemory.GetdeepSleepEnabled()) {  // It only works if using "true" or "false". ??
		jsoSettings[fb.deepSleepEnabled] = true;
	}
	else {
		jsoSettings[fb.deepSleepEnabled] = false;
	}
	SendToFirebase("set", "settings", jsoSettings);
*/
}

void UploadTelemetry_(JsonObject& jsoTelemetry, JsonObject& jsoTeleTimestamp) {
	LogLine(4, __FUNCTION__, "begin");

	jsoTeleTimestamp[".sv"] = "timestamp";
	jsoTelemetry[fb.wifi] = WiFi.RSSI(); // signal strength
	jsoTelemetry[fb.lastOpenTimestamp] = waterValveA.lastOpenTimestamp;
	jsoTelemetry[fb.humidity] = waterSensorA.GetHumidityPct();
	jsoTelemetry[fb.valveState] = waterValveA.valveState;
	jsoTelemetry[fb.Vcc] = externalVoltMeter.GetlastSummarizedReading();
	jsoTelemetry[fb.lastAnalogueReading] = externalVoltMeter.GetlastAnalogueReadingVoltage();

	SendToFirebase("set", "telemetry_current", jsoTelemetry);
	if (PersistentMemory.GetrunMode().equals("normal")) {
		SendToFirebase("push", "telemetry", jsoTelemetry);
	}
}

void UploadLog_(String _txt) {
	DynamicJsonBuffer  jsonBufferRoot(JSON_BUFFER_LENGTH);
	JsonObject& jsoStatic = jsonBufferRoot.createObject();
	JsonObject& jsoLog = jsoStatic.createNestedObject("log");
	JsonObject& jsoLogTimestamp = jsoLog.createNestedObject("timestamp");

	jsoLogTimestamp[".sv"] = "timestamp";
	jsoLog[fb.text] = _txt;
	//jsoLog.prettyPrintTo(Serial);
	SendToFirebase("push", "log", jsoLog);
}

String ConvertJsonToString(JsonObject& jso) {
	String jsonStr;
	jso.printTo(jsonStr);
	return jsonStr;
}

void SendToFirebase(String cmd, String subPath, JsonObject& jso) {

	boolean res = true;
	// NOTE: If Firebase makes error apparantly without reason, try to update the fingerprint in FirebaseHttpClient.h. See https://github.com/FirebaseExtended/firebase-arduino/issues/328

	String s = FB_BasePath + "/" + subPath + "/";
	LogLine(3, __FUNCTION__, cmd + " to firebase: " + s);
	//jso.prettyPrintTo(Serial);

	if (jso.measureLength() < JSON_BUFFER_LENGTH) {

		if (cmd.equals("set"))    { res = Firebase.setJSON(firebaseData, s, ConvertJsonToString(jso)); }
		if (cmd.equals("update")) { res = Firebase.updateNode(firebaseData, s, ConvertJsonToString(jso)); }
		if (cmd.equals("push"))   { res = Firebase.pushJSON(firebaseData, s, ConvertJsonToString(jso)); }

		if (res == false) {
			LogLine(0, __FUNCTION__, "** SET/PUSH FAILED:  " + s + " - Firebase error msg: " + firebaseData.errorReason());
		}
	}
	else {
		UploadLog_("**** ERROR SET/PUSH FAILED: JSON too long");
		LogLine(0, __FUNCTION__, "**** ERROR SET/PUSH FAILED: JSON too long");
		Serial.println(jso.measureLength());
		jso.prettyPrintTo(Serial);
		int i = 0 / 0;  // crash
	}
}

#endif

void ConnectToWifi() {
	int wifiIndex = initWifi(PersistentMemory.ps.wifiSSID, PersistentMemory.ps.wifiPwd);
	delay(250);
	if (wifiIndex == -1 || WiFi.status() != WL_CONNECTED) {
		LogLine(0, __FUNCTION__, "Could not connect to wifi. ");
	}
	else if (wifiIndex == 100 || wifiIndex == 200) {
		// Connected. Do nothing.
}
	else {   // then we have swapped SSID. Update persistent memory so we keep using the new hotspot
		PrintIPAddress();
		PersistentMemory.SetwifiSSID(wifiDevice.currentSSID);
		PersistentMemory.SetwifiPwd(wifiDevice.pwd);
	}
}

#ifdef MEASURE_INTERNAL_VCC
	ADC_MODE(ADC_VCC); //vcc read
	float GetInternalVcc() {
		//at the beginning of sketch
		return (ESP.getVcc() / 1000.0);
	}
#endif

#ifdef USE_GOOGLE_CLOUD

#include "FS.h"
#include <time.h>
#include "backoff.h"
#include "cli.h"

	String CreateTelemetryJson() {
		DynamicJsonBuffer  jsonBuffer(200);
		String jsonStr;
		JsonObject& root = jsonBuffer.createObject();
		root["Wifi"] = WiFi.RSSI(); // signal strength
		root["lastOpenTimestamp"] = waterValveA.lastOpenTimestamp;
		float dur = (1.0 * waterValveA.openSeconds);
		root["openDuration"] = dur;
		root["macAddress"] = WiFi.macAddress();
		root["Vcc"] = externalVoltMeter.ReadVoltage();
		root["lastAnalogueReading"] = externalVoltMeter.lastAnalogueReadingVoltage;
		root.printTo(jsonStr);
		return jsonStr;
	}


	// Clout IoT configuration that you don't need to change
	const char* host = CLOUD_IOT_CORE_HTTP_HOST;
	const int httpsPort = CLOUD_IOT_CORE_HTTP_PORT;

	unsigned int priv_key[8];
	unsigned long iss = 0;
	String jwt;
	boolean wasErr;

	String getJwt() {
		if (iss == 0 || time(nullptr) - iss > 3600) {  // TODO: exp in device
		  // Disable software watchdog as these operations can take a while.
			ESP.wdtDisable();
			iss = time(nullptr);
			LogLine(1, __FUNCTION__, "Refreshing JWT");
			jwt = device->createJWT(iss);
			ESP.wdtEnable(0);
		}
		return jwt;
	}

	HTTPRequestReturnType sendHttpMsg(UploadType uploadType, String data) {
		HTTPRequestReturnType result = notCalledYet;
		LogLine(2, __FUNCTION__, "start");

		if (!wifiClient.connect(host, httpsPort)) {
			LogLine(0, __FUNCTION__, "Wifi connection failed");
			return WifiTimeoutError;
		}

		String header0;
		String authstring = "authorization: Bearer " + String(jwt.c_str());
		String header1 = "host: cloudiotdevice.googleapis.com\n";
		String header2;
		String header3 = "cache-control: no-cache\n";
		String header4;
		String postdata;

		LogLine(2, __FUNCTION__, "data (before encoding)=" + data);
		switch (uploadType) {
		case UploadStateAndSettings:
			header0 = String("POST  ") + device->getSetStatePath() + String(" HTTP/1.1");
			header2 = "method: post\n";

			rbase64.encode(data);
			postdata = String("{\"state\": {\"binary_data\": \"") + rbase64.result() + String("\"}}");
			header4 =
				authstring + "\n" +
				"content-type: application/json\n" +
				"content-length:" + postdata.length() + "\n\n" +
				postdata + "\n\n\n";
			break;
		case UploadTelemetry:
			header0 = String("POST  ") + device->getSendTelemetryPath().c_str() + String(" HTTP/1.1");
			header2 = "method: post\n";

			rbase64.encode(data);
			postdata = String("{\"binary_data\": \"") + rbase64.result() + String("\"}");
			header4 =
				authstring + "\n"
				"content-type: application/json\n"
				"content-length:" + postdata.length() + "\n\n" +
				postdata + "\n\n\n";
			break;
		case GetSettings:
			header0 = String("GET ") + device->getLastConfigPath().c_str() + String(" HTTP/1.1");
			header2 = "method: get\n";
			header4 = authstring + "\n" +
				"\n";
			break;

		}

		wifiClient.println(header0.c_str());
		wifiClient.print(header1.c_str());
		wifiClient.print(header2.c_str());
		wifiClient.print(header3.c_str());
		wifiClient.print(header4.c_str());

		LogLine(3, __FUNCTION__, "header0: " + header0);
		LogLine(3, __FUNCTION__, "header1: " + header1);
		LogLine(3, __FUNCTION__, "header2: " + header2);
		LogLine(3, __FUNCTION__, "header3: " + header3);
		LogLine(3, __FUNCTION__, "header4: " + header4);

		Serial.print("wait for reply.");
		while (!wifiClient.available()) {
			delay(100);
			Serial.print('.');
		}
		LogLine(3, __FUNCTION__, "\n\nReply from server:");


		while (wifiClient.connected()) {
			String line = wifiClient.readStringUntil('\n');
			LogLine(3, __FUNCTION__, line);
			if (line.startsWith("HTTP/1.1 200 OK")) {
				// reset backoff
				result = HTTPOK;
				resetBackoff();
			}
			if (line.startsWith("HTTP/1.1 403 Forbidden")) {
				// reset backoff
				result = Forbidden403Error;
			}
			if (line.startsWith("HTTP/1.1 401")) {
				// reset backoff
				result = Auth401Error;
			}
			if (line == "\r") {
				break;
			}
		}

		while (wifiClient.available()) {
			String line = wifiClient.readStringUntil('\n');

			if (uploadType == GetSettings) {
				if (line.indexOf("binaryData") > 0) {
					String val = line.substring(line.indexOf(": ") + 3, line.indexOf("\","));
					LogLine(2, __FUNCTION__, "Config received:" + val);
					rbase64.decode(val);
					String config = rbase64.result();
					LogLine(1, __FUNCTION__, "Decoded to:" + config);
					Serial.println(config.indexOf("LEDON", 1));
					if (config.indexOf("LEDON") > 0) {
						LogLine(2, __FUNCTION__, "LED ON");
						digitalWrite(LED_BUILTIN, LOW);
					}
					else {
						LogLine(2, __FUNCTION__, "LED OFF");
						digitalWrite(LED_BUILTIN, HIGH);
					}
				}
			}

		}
		LogLine(2, __FUNCTION__, "Complete.");
		wifiClient.stop();

		// Blink to show we're alive
		for (int i = 0; i < 3; i++) {
			digitalWrite(LED_BUILTIN, HIGH);
			delay(200);
			digitalWrite(LED_BUILTIN, LOW);
			delay(200);
		}
		return result;
	}

	void DELETEsendState(String data) {

		LogLine(2, __FUNCTION__, "start");

		delay(50);
		if (!wifiClient.connect(host, httpsPort)) {
			LogLine(0, __FUNCTION__, "connection failed");
			return;
		}

		LogLine(2, __FUNCTION__, "data (before encoding)=" + data);
		rbase64.encode(data);
		String postdata =
			String("{\"state\": {\"binary_data\": \"") + rbase64.result() +
			String("\"}}");

		LogLine(3, __FUNCTION__, "binary postdata=" + postdata);

		String header = String("POST  ") + device->getSetStatePath() + String(" HTTP/1.1");
		String authstring = "authorization: Bearer " + String(jwt.c_str());
		String extraHeaders =
			"host: cloudiotdevice.googleapis.com\n"
			"method: post\n"
			"cache-control: no-cache\n"
			"content-type: application/json\n"
			"Accept: application/json\n";

		LogLine(3, __FUNCTION__, "SENDING HEADER: " + header);
		LogLine(3, __FUNCTION__, "SENDING AUTH STRING: " + authstring);

		LogLine(2, __FUNCTION__, "Sending device state");

		wifiClient.println(header);
		wifiClient.println("host: cloudiotdevice.googleapis.com");
		wifiClient.println("method: post");
		wifiClient.println("cache-control: no-cache");
		wifiClient.println(authstring.c_str());
		wifiClient.println("content-type: application/json");
		wifiClient.print("content-length:");
		wifiClient.println(postdata.length());
		wifiClient.println();
		wifiClient.println(postdata);
		wifiClient.println();
		wifiClient.println();

		int unavailCount = 0;
		while (!wifiClient.available() && unavailCount < 50) {
			delay(100);
			Serial.print('.');
			unavailCount++;
		}

		Serial.println("Receiving data:");
		while (wifiClient.connected()) {
			String line = wifiClient.readStringUntil('\n');
			Serial.println(line);
			if (line.startsWith("HTTP/1.1 200 OK")) {
				// reset backoff
				resetBackoff();
			}
			if (line == "\r") {
				break;
			}
		}
		while (wifiClient.available()) {
			String line = wifiClient.readStringUntil('\n');
		}
		LogLine(2, __FUNCTION__, "Complete.");
		wifiClient.stop();
	}

	boolean DELETEsendTelemetry(String data) {
		boolean result = false;
		LogLine(2, __FUNCTION__, "start");

		if (!wifiClient.connect(host, httpsPort)) {
			LogLine(0, __FUNCTION__, "Wifi connection failed");
			return false;
		}

		LogLine(2, __FUNCTION__, "data (before encoding)=" + data);
		rbase64.encode(data);
		String postdata =
			String("{\"binary_data\": \"") + rbase64.result() + String("\"}");
		LogLine(3, __FUNCTION__, "binary postdata=" + postdata);

		// TODO(class): Move to common helper
		String header = String("POST  ") + device->getSendTelemetryPath().c_str() +
			String(" HTTP/1.1");
		String authstring = "authorization: Bearer " + String(jwt.c_str());

		LogLine(3, __FUNCTION__, "SENDING HEADER: " + header);
		LogLine(3, __FUNCTION__, "SENDING AUTH STRING: " + authstring);

		LogLine(2, __FUNCTION__, "Sending telemetry");

		wifiClient.println(header.c_str());
		wifiClient.println("host: cloudiotdevice.googleapis.com");
		wifiClient.println("method: post");
		wifiClient.println("cache-control: no-cache");
		wifiClient.println(authstring.c_str());
		wifiClient.println("content-type: application/json");
		wifiClient.print("content-length:");
		wifiClient.println(postdata.length());
		wifiClient.println();
		wifiClient.println(postdata);
		wifiClient.println();
		wifiClient.println();

		Serial.println(header);
		Serial.println(authstring);
		Serial.print("content-length:");
		Serial.println(postdata.length());
		Serial.println();
		Serial.println(postdata);
		Serial.println();
		Serial.println();

		while (!wifiClient.available()) {
			delay(100);
			Serial.print('.');
		}

		while (wifiClient.connected()) {
			String line = wifiClient.readStringUntil('\n');
			Serial.println(line);
			if (line.startsWith("HTTP/1.1 200 OK")) {
				// reset backoff
				result = true;
				LogLine(2, __FUNCTION__, "HTTP request OK.");
				resetBackoff();
			}
			if (line == "\r") {
				break;
			}
		}
		while (wifiClient.available()) {
			String line = wifiClient.readStringUntil('\n');
		}
		LogLine(2, __FUNCTION__, "Complete.");
		wifiClient.stop();
		return result;
	}

	void DELETEgetConfig() {

		String header = String("GET ") + device->getLastConfigPath().c_str() + String(" HTTP/1.1");
		String authstring = "authorization: Bearer " + String(jwt.c_str());
		LogLine(3, __FUNCTION__, "SENDING HEADER: " + header);
		LogLine(3, __FUNCTION__, "SENDING AUTH STRING: " + authstring);

		if (!wifiClient.connect(host, httpsPort)) {
			Serial.println("connection failed");
			return;
		}

		// Connect via https.
		wifiClient.println(header.c_str());
		wifiClient.println(authstring.c_str());
		wifiClient.println("host: cloudiotdevice.googleapis.com");
		wifiClient.println("method: get");
		wifiClient.println("cache-control: no-cache");
		wifiClient.println();

		LogLine(2, __FUNCTION__, "Reply from server:");
		while (wifiClient.connected()) {
			String line = wifiClient.readStringUntil('\n');
			LogLine(2, __FUNCTION__, line);
			if (line == "\r") {
				break;
			}
		}
		while (wifiClient.available()) {
			String line = wifiClient.readStringUntil('\n');
			LogLine(2, __FUNCTION__, line);
			if (line.indexOf("binaryData") > 0) {
				String val = line.substring(line.indexOf(": ") + 3, line.indexOf("\","));
				LogLine(2, __FUNCTION__, "Config received:" + val);
				String config = val; // rbase64.decode(val);
				//String config = rbase64.result();
				LogLine(1, __FUNCTION__, "Decoded to:" + config);
				if (val == "AAAA") {
					LogLine(3, __FUNCTION__, "LED ON");
					digitalWrite(LED_BUILTIN, LOW);
				}
				else {
					LogLine(3, __FUNCTION__, "LED OFF");
					digitalWrite(LED_BUILTIN, HIGH);
				}
				resetBackoff();
			}
		}
		wifiClient.stop();
	}


#endif

void TestLoop(String logStr, int LEDflash, int LEDblinkdelay) {
	UploadLog_(logStr);
	LogLine(3, __FUNCTION__, logStr);
	LED_Flashes(LEDflash, LEDblinkdelay);
}

void testHardware() {
	TestLoop("testHardware: begin", 1, 500);

	AnalogMux.OpenChannel(0);
	TestLoop("testHardware: open OpenChannel", 1, 3000);

	AnalogMux.CloseMUXpwr();
	TestLoop("testHardware: CloseMUXpwr", 1, 3000);

	float f = externalVoltMeter.ReadVoltage();
	TestLoop("testHardware: ReadVoltage", 1, 3000);

	boolean b = waterSensorA.CheckIfWater();
	TestLoop("testHardware: CheckIfWater", 1, 3000);

	waterValveA.OpenValve();
	TestLoop("testHardware: OpenValve", 1, 1000);

	waterValveA.CloseValve();
	TestLoop("testHardware: CloseValve", 1, 3000);

	LED_Flashes(500, 100);

}

void SOFT_RESET_ARDUINO() {
	LogLine(0, __FUNCTION__, "\n**************************************************\n**************************************************\n\nRESETTING ARDUINO WHEN BLINKING IS DONE\n\n**************************************************\n**************************************************\n");
	for (int m = 30; m>0; m--) {
		LogLine(0, __FUNCTION__, String(m) + " minutes to reset");
		for (int s = 0; s < 60; s++) {
			digitalWrite(LED_BUILTIN, HIGH);
			delay(500);
			digitalWrite(LED_BUILTIN, LOW);
			delay(500);
		}
	}
	LogLine(0, __FUNCTION__, "GO RESET");
	ESP.restart();
}

