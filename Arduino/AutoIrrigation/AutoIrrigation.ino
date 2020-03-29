/*
    Name:       AutoIrrigation.ino
    Created:	17-09-2018 15:08:50
    Author:     Nikolaj Nøhr-Rasmussen
*/


#include "serialPortHandler.h"
#include <SoftwareSerial.h>
#define HARDWARE_DESCRIPTION "PCB v5.3 WeMOS D1 r2, 12V valve switch"
#define DEVICE_ID "#3"

// Main modes of operation (also used when developing new features)
//#define USE_GAS_SENSOR 
#define USE_WIFI

// Choose cloud:
//#define USE_GOOGLE_CLOUD
//#define USE_AZURE
#define USE_FIREBASE

// Program control:
//#define RUN_ONCE					   // Debug mode: no looping, just execute once
#define USE_DEEP_SLEEP				   // [true] When enabling, connect D0 to RST (on Wemos D1 mini)
#define NBR_OF_LOOPS_BEFORE_SLEEP 1    // [1] How many times will we perform a standard main loop before potentially sleeping
#define TOTAL_SECS_TO_SLEEP 20		   // [20] Default sleep time
#define DEEP_SLEEP_SOAK_THRESHOLD 120  // [120] if soaking time exceeds this limit, we will use deep sleep instead of delay()
#define LOOP_DELAY 10                  // [10] secs
#define SLEEP_WHEN_LOW_VOLTAGE		   // as the first thing, measure battery voltage. If too low, go immediately to sleep without connecting to wifi.
//#define MEASURE_INTERNAL_VCC         // When enabling, we cannot use analogue reading of sensor. 

// Development & debugging
#define FORCE_NEW_VALUES false           // [false] Will overwrite all values in persistent memory. Enable once, disable and recompile
#define SIMULATE_WATERING false        // open the valve in every loop
// See also SIMULATE_SENSORS in SensorHandler.h
#define DEBUGLEVEL 4					// Has dual function: 1) serving as default value for Firebase Logging (which can be modified at runtime). 2) Defining debug level on serial port.

#include <jsmn.h>
#include <FirebaseJson.h>

#ifdef USE_WIFI
#include <SD.h>
#include <FirebaseESP8266.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiServer.h>
#include <WiFiClient.h>
#endif

#ifdef USE_FIREBASE
#include <SPI.h>
#include <FirebaseESP8266HTTPClient.h>
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
#include "backoff.h"
#endif

#include "LEDHandler.h"
#include "AnalogMux.h"
#include "cli.h"
#include <LogLib.h>
#include <EEPROM.h>
#include <SPI.h>
#include <TimeLib.h>        // http://playground.arduino.cc/code/time - installed via library manager
#include <time.h>
#include "globals.h"
#include "ciotc_config.h" // Wifi configuration here
#include "WaterValve.h"
#include "SoilHumiditySensor.h"
#include "VoltMeter.h"
#include "DeepSleepHandler.h"
#include "PersistentMemory.h"
#ifdef USE_AZURE
#include "AzureCloudHandler.h"
#endif
#ifdef USE_GAS_SENSOR
#include "GasSensor.h"
GasSensorClass gasSensor;
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
const int CHANNEL_WATER = 2;

// Sensors and actuators
SoilHumiditySensorClass soilHumiditySensorA;
WaterValveClass waterValveA;
VoltMeterClass externalVoltMeter;
const int VALVE_OPEN_DURATION = 2;   // [2] time a valve can be open in one run-time cycle, i.e. between deepsleeps
const int VALVE_SOAK_TIME     = 3;   // [30] time between two valve openings

// General control
int loopCount = 0;  // used in conjunction with NBR_OF_LOOPS_BEFORE_SLEEP
//const char SOFTWARE_VERSION[] = "AutoIrrigation.ino - Compiled: " __DATE__ " " __TIME__;

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
void InitUnitTest();

void setup() {
	delay(500);
	Serial.begin(115200);

	InitDebugLevel(DEBUGLEVEL);
	PersistentMemory.SetdebugLevel(DEBUGLEVEL);

	// read persistent memory
	PersistentMemory.init(FORCE_NEW_VALUES, TOTAL_SECS_TO_SLEEP, DEVICE_ID, HARDWARE_DESCRIPTION, VALVE_OPEN_DURATION, VALVE_SOAK_TIME, LOOP_DELAY, DEBUGLEVEL);   // false: read from memory.  true: initialize
	//PersistentMemory.Printps();
	//PersistentMemory.UnitTest();  // just for testing
	//PersistentMemory.Printps();
//	PersistentMemory.SetdebugLevel(debuglevel);
	Serial.printf("persisten mem debugLevel=%d (should be %d)\n", PersistentMemory.GetdebugLevel(), DEBUGLEVEL);
	SetFBDebugLevel(PersistentMemory.GetdebugLevel());
	LogLinef(2, __FUNCTION__, "Sleep cycle %d of %d", PersistentMemory.ps.currentSleepCycle, PersistentMemory.ps.maxSleepCycles);
	if (PersistentMemory.ps.currentSleepCycle != 0) {
		DeepSleepHandler.GoToDeepSleep();
	}
	if (PersistentMemory.ps.currentSleepCycle == 0 && DeepSleepHandler.ContinueSleeping()) {
		DeepSleepHandler.GoToDeepSleep();
	}

	initFlashLED();
	AnalogMux.init(MUX_S1, MUX_S0, HUM_SENSOR_PWR_CTRL, HIGH);
	externalVoltMeter.lastSummarizedReading = PersistentMemory.ps.lastVccSummarizedReading;
	externalVoltMeter.init(ANALOG_INPUT, "5V voltmeter", CHANNEL_BATT, SensorHandlerClass::ExternalVoltMeter, PersistentMemory.ps.lastVccSummarizedReading);

	// If voltage is too low, go to sleep immediately. No error checking
	float vccTmp;
	#ifdef SLEEP_WHEN_LOW_VOLTAGE
		vccTmp = externalVoltMeter.ReadSingleVoltage();
		LogLinef(4, __FUNCTION__, "Voltage: %fV. ", vccTmp);
	#else
		vccTmp = PersistentMemory.ps.vccMinLimit;
	#endif

	if (vccTmp < PersistentMemory.ps.vccMinLimit) {
		#ifdef USE_WIFI 
			ConnectToWifi();
		#endif
		LogLinef(0, __FUNCTION__, "Voltage too low: %fV. Go to sleep for 12 hours", vccTmp);
		DeepSleepHandler.GotoSleepAndWakeAfterDelay(12 * 60 * 60);
	}	

	String rm = PersistentMemory.GetrunMode();
	if (rm.equals(RUNMODE_SOIL)) {
		soilHumiditySensorA.init(ANALOG_INPUT, "humidity sensor A", CHANNEL_HUM, SensorHandlerClass::SoilHumiditySensor, PersistentMemory.ps.humLimit);
	}
	else if (rm.equals(RUNMODE_WATER)) {
		soilHumiditySensorA.init(ANALOG_INPUT, "water sensor A", CHANNEL_WATER, SensorHandlerClass::WaterSensor, PersistentMemory.ps.humLimit);
	}

#ifdef USE_GAS_SENSOR
	else if (rm.equals(RUNMODE_GAS)) {
		gasSensor.init(ANALOG_INPUT, "gas senso", CHANNEL_HUM, SensorHandlerClass::GasSensor);
	}
#endif
	
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
		if (getCurrentTimeB()) {
			LogLine(2, __FUNCTION__, "Time fetched and adjusted");
		}
		else {
			LogLine(0, __FUNCTION__, "Time NOT fetched and adjusted");
		}

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
	LogLinef(3, __FUNCTION__, "Voltage reading %f", vccTmp);

	LED_Flashes(1, 300);
	if ((soilHumiditySensorA.CheckIfWater() == SoilHumiditySensor.DRY) || SIMULATE_WATERING) {
		LogLine(1, __FUNCTION__, "Low water detected. Watering and soaking.");

		ConnectAndUploadToCloud(UploadTelemetry);
		waterValveA.OpenValve();
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
boolean firstRun = true;
void loop() {
	String rm;

	ConnectAndUploadToCloud(GetSettings, firstRun);  // maybe the user has changed values via the app
	firstRun = false;

	rm = PersistentMemory.GetrunMode();
	LogLinef(5, __FUNCTION__, "runmode = %s", rm.c_str());
	if (rm.equals(RUNMODE_SOIL) || rm.equals(RUNMODE_WATER)) {
		if (WaterIfNeeded()) {
			// use deep sleep if it's enabled and we want to soak for a longer period of time
			if ( (waterValveA.soakSeconds <= DEEP_SLEEP_SOAK_THRESHOLD) || (!PersistentMemory.GetdeepSleepEnabled()) ) {
				waterValveA.WaitToSoak();
			} else {
				DeepSleepHandler.GotoSleepAndWakeAfterDelay(waterValveA.soakSeconds);
			}
		}
		if (loopCount++ >= NBR_OF_LOOPS_BEFORE_SLEEP) {
			DeepSleepHandler.GotoSleepAndWakeAfterDelay(PersistentMemory.GettotalSecondsToSleep());
		}
	}
	else if (rm.equals(RUNMODE_SENSORTEST)) {
		soilHumiditySensorA.TestSensor();
		ConnectAndUploadToCloud(UploadTelemetry);
	}
	else if (rm.equals(RUNMODE_BATTERYTEST)) {
		externalVoltMeter.TestSensor();
		ConnectAndUploadToCloud(UploadTelemetry);
	}
	else if (rm.equals(RUNMODE_HARDWARETEST)) {
		testHardware();
	}
#ifdef USE_GAS_SENSOR
	else if (rm.equals(RUNMODE_GAS)) {
		if (!PersistentMemory.GetdeepSleepEnabled()) {
			ConnectAndUploadToCloud(UploadTelemetry);
		}
		else {
			DeepSleepHandler.GotoSleepAndWakeAfterDelay(waterValveA.soakSeconds);
		}
		if (loopCount++ >= NBR_OF_LOOPS_BEFORE_SLEEP) {
			DeepSleepHandler.GotoSleepAndWakeAfterDelay(PersistentMemory.GettotalSecondsToSleep());
		}
	}
#endif
	else {
		LogLinef(0, __FUNCTION__, "Illegal option chosen: %s . Resetting runMode to normal", rm.c_str());
		PersistentMemory.SetrunMode(RUNMODE_SOIL);
		LED_Flashes(200, 100);
	}

	#ifdef RUN_ONCE
		while (true) {
			LogLine(1, __FUNCTION__, "wait forever (controlled by RUN_ONCE directive)");
			delay(60000);
		}
	#else
	int d = PersistentMemory.GetmainLoopDelay();
		LogLinef(3, __FUNCTION__, "waiting mainLoopDelay: %d", d);
		delay(d * 1000);
	#endif
}

void ConnectAndUploadToCloud(UploadType uploadType, boolean firstRun) {
	HTTPRequestReturnType requestResult;
	String logTxt = "";

	requestResult = notCalledYet;
#ifdef USE_WIFI
	if ( (WiFi.status() == WL_CONNECTED) && IsWifiStrenghtOK() ) {   
		LogLine(4, __FUNCTION__, "wifi connected OK");
		#ifdef USE_FIREBASE
			switch (uploadType) {
				case GetSettings:
					GetSettingsFromFirebase_(firstRun);
					InitFirebaseLogging(&firebaseData, FB_BasePath, "log", JSON_BUFFER_LENGTH);
					break;
				case UploadStateAndSettings:	UploadStateAndSettings_();
					break;
				case UploadTelemetry:			UploadTelemetry_();
					break;
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
//dbg1	return (PersistentMemory.GetCloudUsername().equals("initial value"));
	return true;
}

void InitPersistentMemoryIfNeeded() {
	if (!IsPersistentMemorySet() || FORCE_NEW_VALUES) {
		PersistentMemory.init(true, TOTAL_SECS_TO_SLEEP, DEVICE_ID, HARDWARE_DESCRIPTION, VALVE_OPEN_DURATION, VALVE_SOAK_TIME, LOOP_DELAY, DEBUGLEVEL);   // false: read from memory.  true: initialize
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
	String s = FB_BasePath + "/settings/" + FB_UserUpdate;
	if (Firebase.getBool(firebaseData, s)) {
		if (firebaseData.boolData()) {
			LogLinef(5, __FUNCTION__, "true - %s  " , s.c_str());
			return true;
		}
		else {
			LogLinef(5, __FUNCTION__, "false (not set by user) - %s   ", s.c_str());
		}
	}
	else {
		LogLinef(5, __FUNCTION__, "false (data does not exist in Firebase) - %s + %s", s.c_str(), firebaseData.errorReason().c_str());
	}
	LogLine(4, __FUNCTION__, "end");
	return false;
}

boolean DeviceExistsInFirebase() {
	if (!Firebase.getString(firebaseData, FB_BasePath + "/state/" + FB_wifiSSID)) {
		//UploadLog_("Device seems not to exist. Double checking.");
		Serial.println("XXXXXXXXXXXXXXXX");
		PersistentMemory.PrintpsRAW();
		Serial.println("XXXXXXXXXXXXXXXX");
		if (!Firebase.getString(firebaseData, FB_BasePath + "/metadata/" + FB_macAddress)) {
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
		LogLinef(4, __FUNCTION__, "dummy exists. Will remove it. path=%s ", s.c_str());
		Firebase.deleteNode(firebaseData, s);
	}
	else {
		LogLine(4, __FUNCTION__, "dummy did not exist");
	}
}

void RemoveTelemetryFromFirebase() {
	String s = FB_BasePath + "/telemetry";
	LogLine(2, __FUNCTION__, "	removing telemetry");
	Firebase.deleteNode(firebaseData, s);
}

void GetSettingsFromFirebase_(boolean firstRun) {
	int val;
	boolean b;
	String str;

	LogLine(4, __FUNCTION__, "begin");
	if (firstRun || IsSettingsDataUpdatedByUser()) {
		LogLinef(5, __FUNCTION__, "2 %s  ", FB_BasePath.c_str());
		str = FB_BasePath + "/metadata/" + FB_deviceLocation;
		if (Firebase.getString(firebaseData, str)) {
			PersistentMemory.SetdeviceLocation(firebaseData.stringData());
			LogLinef(5, __FUNCTION__, " %s = %s", FB_deviceLocation, firebaseData.stringData().c_str());
		}
		if (Firebase.getString(firebaseData, FB_BasePath + "/metadata/" + FB_deviceID)) {
			PersistentMemory.SetdeviceID(firebaseData.stringData()); 
			LogLinef(5, __FUNCTION__, " %s = %s", FB_deviceID, firebaseData.stringData().c_str());
		}
		if (Firebase.getInt(firebaseData, FB_BasePath + "/settings/" + FB_mainLoopDelay)) {
			PersistentMemory.SetmainLoopDelay(firebaseData.intData());
			LogLinef(5, __FUNCTION__, " %s = %s", FB_mainLoopDelay, firebaseData.stringData().c_str());
		}
		if (Firebase.getInt(firebaseData, FB_BasePath + "/settings/" + FB_debugLevel)) {
			int d = firebaseData.intData();
			PersistentMemory.SetdebugLevel(d);
			SetFBDebugLevel(d);
			LogLinef(5, __FUNCTION__, " %s = %d", FB_debugLevel, firebaseData.intData());
		}
		String fullPath;
		for (int i = 0; i < MAX_WAKEUPTIMES; i++) {
			fullPath = FB_BasePath + "/settings/" + FB_wakeupTime + String(i);
			if (Firebase.getString(firebaseData, fullPath)) {
				LogLinef(5, __FUNCTION__, " %s = %s", fullPath.c_str(), firebaseData.stringData().c_str());
				PersistentMemory.SetWakeTime(i, firebaseData.stringData());
			}
		}
		if (Firebase.getString(firebaseData, FB_BasePath + "/settings/" + FB_pauseWakeTime)) {
			LogLinef(5, __FUNCTION__, " %s = %s", FB_pauseWakeTime, firebaseData.stringData().c_str());
			PersistentMemory.SetPauseWakeTime(firebaseData.stringData());
		}
		if (Firebase.getBool(firebaseData, FB_BasePath + "/settings/" + FB_deepSleepEnabled)) {
			b = firebaseData.boolData();
			PersistentMemory.SetdeepSleepEnabled(b);
			LogLinef(5, __FUNCTION__, " %s = %b", FB_deepSleepEnabled, b);
		}
		if (Firebase.getString(firebaseData, FB_BasePath + "/settings/" + FB_runMode)) {
			PersistentMemory.SetrunMode(firebaseData.stringData());
			LogLinef(5, __FUNCTION__, " %s = %s", FB_runMode, firebaseData.stringData().c_str());
		}
		if (Firebase.getInt(firebaseData, FB_BasePath + "/settings/" + FB_totalSecondsToSleep)) {
			PersistentMemory.SettotalSecondsToSleep(firebaseData.intData());
		}
		if (Firebase.getInt(firebaseData, FB_BasePath + "/settings/" + FB_valveOpenDuration)) {
			val = firebaseData.intData();
			PersistentMemory.SetvalveOpenDuration(val);
			waterValveA.SetvalveOpenDuration(val);
		}
		if (Firebase.getInt(firebaseData, FB_BasePath + "/settings/" + FB_humLimit)) {
			val = firebaseData.intData();
			PersistentMemory.SethumLimit(val);
			soilHumiditySensorA.SethumLimitPct(val);
		}
		if (Firebase.getFloat(firebaseData, FB_BasePath + "/settings/" + FB_vccAdjustment)) {
			float f = firebaseData.floatData();
			PersistentMemory.SetvccAdjustment(f);
		}
		if (Firebase.getFloat(firebaseData, FB_BasePath + "/settings/" + FB_vccMinLimit)) {
			float f = firebaseData.floatData();
			PersistentMemory.SetvccMinLimit(f);
		}

		if (Firebase.getInt(firebaseData, FB_BasePath + "/settings/" + FB_valveSoakTime)) {
			val = firebaseData.intData();
			LogLinef(5, __FUNCTION__, " %s = %d", FB_valveSoakTime, val);
			PersistentMemory.SetvalveSoakTime(val);
			waterValveA.SetvalveSoakTime(val);
		}

		Firebase.setBool(firebaseData, FB_BasePath + "/settings/" + FB_UserUpdate, false);

		PersistentMemory.Printps();
		LogLine(2, __FUNCTION__, "New settings read");
	}
}

void CreateNewDevice() {
	FirebaseJson jsoMetadata;
	FirebaseJson jsoState;
	FirebaseJson jsoSettings;
	FirebaseJson jsoDummy;

	InitPersistentMemoryIfNeeded();

	PersistentMemory.AddMetadataJson(&jsoMetadata);
	PersistentMemory.AddStateJson(&jsoState);
	PersistentMemory.AddSettingsJson(&jsoSettings);

	SendToFirebase("set", "metadata", &jsoMetadata);
	SendToFirebase("set", "state", &jsoState);
	SendToFirebase("set", "settings", &jsoSettings);

	// add some dummies so we can use "push" consistently for log and telemetry
	jsoDummy.set("x", "-");
	SendToFirebase("set", "log", &jsoDummy);

}

void UploadStateAndSettings_() {

	boolean settingsUpdatedByUser = false;
	LogLine(4, __FUNCTION__, "begin");

	// Check if device has been created. 
	// If it exists, read current state values from Firebase into persistent memory, otherwise create it. 
	if ( DeviceExistsInFirebase() ) {
		ConnectAndUploadToCloud(GetSettings);
	}
	else {
		CreateNewDevice();
		LogLinef(4, __FUNCTION__, "%s created", FB_BasePath.c_str());
	}
	if (IsSettingsDataUpdatedByUser()) {
		ConnectAndUploadToCloud(GetSettings);
	}
}

void UploadTelemetry_() {

	FirebaseJson jsoTelemetry;
	FirebaseJson jsoTeleTimestamp;
	boolean newTelemetry = true;  // so far only used for gassensor

	LogLine(4, __FUNCTION__, "begin");
	// Build the telemetry. This is partly dependent on which sensors are present.

	#ifdef USE_GAS_SENSOR
		newTelemetry = gasSensor.GetTelemetryJson(&jsoTelemetry);
		LogLine(4, __FUNCTION__, "telemetry fetched from gas sensor");
	#else
		soilHumiditySensorA.AddTelemetryJson(&jsoTelemetry);
		waterValveA.AddTelemetryJson(&jsoTelemetry);
	#endif
	externalVoltMeter.AddTelemetryJson(&jsoTelemetry);

	int32_t wifiStrength = WiFi.RSSI();
	jsoTelemetry.add(FB_wifi, wifiStrength); 
	if (newTelemetry) {
		SendToFirebase("set", "telemetry_current", &jsoTelemetry);
		SendToFirebase("timestamp", "telemetry_current/timestamp", &jsoTelemetry);
		SendToFirebase("push", "telemetry", &jsoTelemetry);
		SendToFirebase("appendtimestamp", "", &jsoTelemetry);
	}
}

String pushPath;
void SendToFirebase(String cmd, String subPath, FirebaseJson* jso) {

	String jsoStr;
	boolean res = true;
	// NOTE: If Firebase makes error apparantly without reason, try to update the fingerprint in FirebaseHttpClient.h. See https://github.com/FirebaseExtended/firebase-arduino/issues/328

	String s = FB_BasePath + "/" + subPath + "/";
	(*jso).toString(jsoStr, true);
	LogLinef(4, __FUNCTION__, " Firebase command:%s    path: %s    ", cmd.c_str(), s.c_str());
	LogLinef(5, __FUNCTION__, " JSON:\n%s", jsoStr.c_str());

	if (cmd.equals("set"))		 { 
		res = Firebase.setJSON(firebaseData, s, *jso);
		if (!res) {
			delay(500);
			res = Firebase.setJSON(firebaseData, s, *jso);
		}
	}
	if (cmd.equals("timestamp")) { res = Firebase.setTimestamp(firebaseData, s); }
	if (cmd.equals("update"))	 { res = Firebase.updateNode(firebaseData, s, *jso); }
	if (cmd.equals("push"))		 { 
		res = Firebase.pushJSON(firebaseData, s, *jso); 
		pushPath = firebaseData.dataPath() + "/" + firebaseData.pushName();
	}
	if (cmd.equals("appendtimestamp")) { 
		res = Firebase.setTimestamp(firebaseData, pushPath + "/timestamp");
	}
	if (!res) {
		LogLinef(0, __FUNCTION__, "Firebase SET command failed %s", s.c_str());
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

void TestLoop(String logStr, int LEDflash, int LEDblinkdelay) {
	LogLine(3, __FUNCTION__, logStr.c_str());
	LED_Flashes(LEDflash, LEDblinkdelay);
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

void InitUnitTest() {
	delay(500);
	Serial.begin(115200);

	InitDebugLevel(4);
	PersistentMemory.SetdebugLevel(4);

	// read persistent memory
	PersistentMemory.init(false, 20, "PCB v5.2 - #1", 0, 0, 10, 4);   // false: read from memory.  true: initialize
	SetFBDebugLevel(PersistentMemory.GetdebugLevel());
	LogLine(1, __FUNCTION__, "BEGIN");
	LogLinef(2, __FUNCTION__, "Sleep cycle %d of %d", PersistentMemory.ps.currentSleepCycle, PersistentMemory.ps.maxSleepCycles);

	// check for crazy values
	if (PersistentMemory.ps.currentSleepCycle >= 12 || PersistentMemory.ps.maxSleepCycles >= 12) {
		LogLine(2, __FUNCTION__, "Resetting counters");
		PersistentMemory.SetmaxSleepCycles(0);
		PersistentMemory.SetvalveOpenDuration(0);
		PersistentMemory.ps.currentSleepCycle = 0;
	}

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
	if (getCurrentTimeB()) {
		LogLine(5, __FUNCTION__, "Time fetched and adjusted");
	}
	else {
		LogLine(0, __FUNCTION__, "Time NOT fetched and adjusted");
	}
#endif

/*
#ifdef USE_FIREBASE
	FB_BasePath = FB_DEVICE_PATH + PersistentMemory.GetmacAddress();
	Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
	//RemoveTelemetryFromFirebase(); UploadLog_("Setup(): ALL TELEMETRY REMOVED");
	ConnectAndUploadToCloud(UploadStateAndSettings);
#endif
*/
	if (PersistentMemory.ps.currentSleepCycle != 0) {
		DeepSleepHandler.GoToDeepSleep();
	}
}

void testHardware() {

//	DeepSleepHandler.GotoSleepAndWakeUpAtTime(PersistentMemory.GetWakeTime());
	
	TestLoop("testHardware: begin", 1, 500);

	AnalogMux.OpenChannel(0);
	TestLoop("testHardware: open OpenChannel", 1, 3000);

	AnalogMux.CloseMUXpwr();
	TestLoop("testHardware: CloseMUXpwr", 1, 3000);

	float f = externalVoltMeter.ReadVoltage();
	TestLoop("testHardware: ReadVoltage", 1, 3000);

	boolean b = soilHumiditySensorA.CheckIfWater();
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
		//log		LogLine(0, __FUNCTION__, String(m) + " minutes to reset");
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

