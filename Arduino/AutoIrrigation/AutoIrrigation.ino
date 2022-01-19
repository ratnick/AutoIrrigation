/*
    Name:       AutoIrrigation.ino
    Created:	17-09-2018 15:08:50
    Author:     Nikolaj Nøhr-Rasmussen
*/

// Including the libraries below, allows to turn off Deep Search in libraries => faster compiling
#include "FirebaseModel.h"
#include "FirebaseLib.h"
//#include "Firebase_PersistentMemory.h"
#include <jsmn.h>
#include <FirebaseJson.h>
#include <NNR_Wifi.h>
#include <SysCall.h>
#include <sdios.h>
#include <BlockDriver.h>
#include <SDFSFormatter.h>
#include <SdFatConfig.h>
#include <SdFat.h>
#include <MinimumSerial.h>
#include <FreeStack.h>
#include <SDFS.h>
#include <SD.h>
#include <FirebaseJson.h>    // works with 2.2.8, but NOT 2.3.9. This will give random errors in communication with Firebase. I think it's becaise FirebaseJSON struct or class is changed
// end 

// OTA upload
#include <LEAmDNS_Priv.h>
#include <ArduinoOTA.h>
// end

#include "compile_flags.h"

#ifdef USE_WIFI
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


#include <SoftwareSerial.h>
#include <Wire.h>
#include <DHTesp.h>
#include <EEPROM.h>
#include <TimeLib.h>        // http://playground.arduino.cc/code/time - installed via library manager
#include <time.h>

// own libraries
#include "globals.h"
#include "ciotc_config.h" // Wifi configuration here
#include "NNR_PersistentMemory.h"
#include "DeepSleepHandler.h"
#include "NNR_LEDHandler.h"
#include "AnalogMux.h"
#include "NNR_OTAupdate.h"
#include "cli.h"
#include <NNR_Logging.h>
#include "WaterValve.h"
#include "SoilHumiditySensor.h"
#include "DistanceSensor.h"
#include "VoltMeter.h"
#include "Thermometer.h"
#include "serialPortHandler.h"



#ifdef USE_AZURE
#include "AzureCloudHandler.h"
#endif
#ifdef USE_GAS_SENSOR
#include "GasSensor.h"
GasSensorClass gasSensor;
#endif

// Wifi and network globals
Device device;
const char timeServer[] = "0.dk.pool.ntp.org"; // Danish NTP Server 
byte localmacAddr[6];
int wifiIndex = 0;

/*
//Hardware pin configuration on WeMOS D1
const int MUX_S0 = D8; // S0 = Pin A on schematic
const int MUX_S1 = D7; // S1 = Pin B on schematic
const int HUM_SENSOR_PWR_CTRL = D1;
const int VALVE_CTRL = D5;
const int DHT11_SIGNAL_PIN = D6;
const int ANALOG_INPUT = A0;
// MUX channels X0, X1, X2... on schematic
const int CHANNEL_HUM = 0;
const int CHANNEL_BATT = 1;
const int CHANNEL_WATER = 2;
const int CHANNEL_TEMPERATURE = 2;
*/

// Sensors and actuators
DistanceSensorClass sensorA;
SoilHumiditySensorClass soilHumiditySensorA;
WaterValveClass waterValveA;
VoltMeterClass externalVoltMeter;
ThermometerClass thermometer;
const int VALVE_OPEN_DURATION = 2;   // [2] time a valve can be open in one run-time cycle, i.e. between deepsleeps
const int VALVE_SOAK_TIME     = 3;   // [30] time between two valve openings

// General control
int loopCount = 0;  // used in conjunction with NBR_OF_LOOPS_BEFORE_SLEEP

#ifdef USE_GOOGLE_CLOUD
// Google Cloud globals
CloudIoTCoreDevice *device;
#endif

#ifdef USE_FIREBASE
#define FIREBASE_HOST "irrfire.firebaseio.com" 
#define FIREBASE_AUTH "w3Q5F3zWG3RkudEjGUJZVi2wiVDvZCKY3VkVywkC"
//#define FIREBASE_AUTH "QhxlUMo2PjjqCTFwgVLInGH7lFPEA8oSKhoMG76e"
#define FB_DEVICE_PATH "/irrdevices/"
String FB_BasePath;
FirebaseData firebaseData; // FirebaseESP8266 data object
boolean firstRun = true;
#endif

const int JSON_BUFFER_LENGTH = 250;

void InitUnitTest();

void ConnectToWifi() {
	int wifiIndex = initWifi(PersistentMemory.ps.wifiSSID, PersistentMemory.ps.wifiPwd, &device.wifi);
	delayNonBlocking(250);
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

		waterValveA.OpenValve();
		UploadTelemetry_();
		waterValveA.KeepOpen();

		waterValveA.CloseValve();
		UploadTelemetry_();
		valveWasOpened = true;
	}
	if (!valveWasOpened) {
		LogLine(1, __FUNCTION__, "High water detected. Skip watering and soaking.");
		UploadTelemetry_();
	}
	return valveWasOpened;
}


boolean IsPersistentMemorySet() {
	// Check if values are reasonable. Not bullit proof!
//dbg1	return (PersistentMemory.GetCloudUsername().equals("initial value"));
	return true;
}

void InitPersistentMemoryIfNeeded() {
	if (!IsPersistentMemorySet() || FORCE_NEW_VALUES) {
		PersistentMemory.init(true, TOTAL_SECS_TO_SLEEP, DEVICE_ID, HARDWARE_DESCRIPTION, DefaultRunmode.c_str(), VALVE_OPEN_DURATION, VALVE_SOAK_TIME, LOOP_DELAY, DEBUGLEVEL);   // false: read from memory.  true: initialize
		WiFi.macAddress(localmacAddr);
		PersistentMemory.SetmacAddress(localmacAddr);
		#if defined (USE_DEEP_SLEEP)
			PersistentMemory.SetdeepSleepEnabled(true);
		#else
			PersistentMemory.SetdeepSleepEnabled(false);
		#endif
	}
}

boolean first = true;

#ifdef USE_FIREBASE

boolean IsSettingsDataUpdatedByUser() {
	String s = FB_BasePath + "/settings/" + FB_UserUpdate;
	if (Firebase.getBool(firebaseData, s)) {
		if (firebaseData.boolData()) {
			LogLinef(2, __FUNCTION__, "true - %s  " , s.c_str());
			return true;
		}
		else {
			LogLinef(4, __FUNCTION__, "false (not set by user) - %s   ", s.c_str());
		}
	}
	else {
		LogLinef(5, __FUNCTION__, "false (data does not exist in Firebase) - %s + %s", s.c_str(), firebaseData.errorReason().c_str());
	}
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

void GetSettingsFromFirebase_(boolean &firstRun) {
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

		// update firebase with IP address. We use the diviceID for this to be able to identify the device for OTA updates
		Firebase.setString(firebaseData, FB_BasePath + "/metadata/" + FB_deviceID, PersistentMemory.GetDeviceID());
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

		if (Firebase.getBool(firebaseData, FB_BasePath + "/settings/" + FB_runOTAupdate)) {
			b = firebaseData.boolData();
			PersistentMemory.SetRunOTA(b);
			LogLinef(5, __FUNCTION__, " %s = %b", FB_runOTAupdate, b);
		}

		Firebase.setBool(firebaseData, FB_BasePath + "/settings/" + FB_UserUpdate, false);
		LogLinef(4, __FUNCTION__, " %s = %s", FB_deviceID, PersistentMemory.GetDeviceID().c_str());

		PersistentMemory.Printps();
		LogLine(2, __FUNCTION__, "New settings read");
		firstRun = false;
	}
}

void CreateNewDevice() {
	FirebaseJson jsoMetadata;
	FirebaseJson jsoState;
	FirebaseJson jsoSettings;
	FirebaseJson jsoDummy;
	bool res;

	InitPersistentMemoryIfNeeded();

/* NNR 220118
	PersistentMemory.AddMetadataJson(&jsoMetadata);
	PersistentMemory.AddStateJson(&jsoState);
	PersistentMemory.AddSettingsJson(&jsoSettings);
*/
	res = SendToFirebase(set, "metadata", jsoMetadata, firebaseData);
	res = SendToFirebase(set, "state", jsoState, firebaseData);
	res = SendToFirebase(set, "settings", jsoSettings, firebaseData);

	// add some dummies so we can use "push" consistently for log and telemetry
	jsoDummy.set("x", "-");
	res = SendToFirebase(set, "log", jsoDummy, firebaseData);

}

void UploadStateAndSettings_(boolean &firstRun) {

	boolean settingsUpdatedByUser = false;
	LogLine(4, __FUNCTION__, "begin");

	// Check if device has been created. 
	// If it exists, read current state values from Firebase into persistent memory, otherwise create it. 
	if ( DeviceExistsInFirebase() ) {
		GetSettingsFromFirebase_(firstRun);
		InitFirebaseLogging(firebaseData, FB_BasePath, "log", JSON_BUFFER_LENGTH);
	}
	else {
		CreateNewDevice();
		InitFirebaseLogging(firebaseData, FB_BasePath, "log", JSON_BUFFER_LENGTH);   //nnr +
		LogLinef(4, __FUNCTION__, "%s created", FB_BasePath.c_str());
	}
}

void RemoveTelemetryFromFirebase() {
	String s = FB_BasePath + "/telemetry";
	LogLine(2, __FUNCTION__, "	removing telemetry");
	Firebase.deleteNode(firebaseData, s);
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

#endif

void UploadTelemetry_() {

	FirebaseJson jsoTelemetry;
	FirebaseJson jsoTeleTimestamp;
	boolean newTelemetry = true;  // so far only used for gassensor
	bool res;

	LogLine(4, __FUNCTION__, "begin");
	// Build the telemetry. This is partly dependent on which sensors are present.

	#if defined(USE_GAS_SENSOR)
		newTelemetry = gasSensor.GetTelemetryJson(&jsoTelemetry);
		LogLine(4, __FUNCTION__, "telemetry fetched from gas sensor");
	#elif defined(USE_DHT11_SENSOR)
		thermometer.AddTelemetryJson(&jsoTelemetry);
		LogLine(5, __FUNCTION__, "telemetry fetched from DHT11 sensor");
//211009		externalVoltMeter.AddTelemetryJson(&jsoTelemetry);
//		LogLine(5, __FUNCTION__, "telemetry fetched from Voltmeter");
	#elif defined(USE_DISTANCE_SENSOR)
		sensorA.AddTelemetryJson(&jsoTelemetry);
		externalVoltMeter.AddTelemetryJson(&jsoTelemetry);
		LogLine(5, __FUNCTION__, "telemetry fetched from DISTANCE sensor");
	#else
		soilHumiditySensorA.AddTelemetryJson(&jsoTelemetry);
		waterValveA.AddTelemetryJson(&jsoTelemetry);
		externalVoltMeter.AddTelemetryJson(&jsoTelemetry);
	#endif
		#ifdef USE_FIREBASE
			int32_t wifiStrength = WiFi.RSSI();
			jsoTelemetry.add(FB_wifi, wifiStrength);
			if (newTelemetry) {
				res = SendToFirebase(set, "telemetry_current", jsoTelemetry, firebaseData);
				res = SendToFirebase(timestamp, "telemetry_current/timestamp", jsoTelemetry, firebaseData);
				res = SendToFirebase(push, "telemetry", jsoTelemetry, firebaseData);
				res = SendToFirebase(appendtimestamp, "", jsoTelemetry, firebaseData);
			}
		#endif
		LogLine(5, __FUNCTION__, "done");

}

void setup() {
	delayNonBlocking(500);
	Serial.begin(115200);

	InitDebugLevel(DEBUGLEVEL);
	PersistentMemory.SetdebugLevel(DEBUGLEVEL);

	// read persistent memory
	PersistentMemory.init(FORCE_NEW_VALUES, TOTAL_SECS_TO_SLEEP, DEVICE_ID, HARDWARE_DESCRIPTION, DefaultRunmode.c_str(), VALVE_OPEN_DURATION, VALVE_SOAK_TIME, LOOP_DELAY, DEBUGLEVEL);   // false: read from memory.  true: initialize
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
	LogLine(4, __FUNCTION__, "A");
	AnalogMux.init(MUX_S1, MUX_S0, HUM_SENSOR_PWR_CTRL, HIGH);
	externalVoltMeter.lastSummarizedReading = PersistentMemory.ps.lastVccSummarizedReading;
	externalVoltMeter.init(ANALOG_INPUT, "5V voltmeter", CHANNEL_BATT, SensorHandlerClass::ExternalVoltMeter, PersistentMemory.ps.lastVccSummarizedReading);
	LogLine(4, __FUNCTION__, "B");

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
	LogLinef(4, __FUNCTION__, "runmode=%s", rm.c_str());
	if (rm.equals(RUNMODE_SOIL)) {
		soilHumiditySensorA.init(ANALOG_INPUT, "humidity sensor A", CHANNEL_HUM, SensorHandlerClass::SoilHumiditySensor, PersistentMemory.ps.humLimit);
		thermometer.init(ANALOG_INPUT, "thermometer", CHANNEL_TEMPERATURE, SensorHandlerClass::Thermometer);
	}
	else if (rm.equals(RUNMODE_WATER)) {
		soilHumiditySensorA.init(ANALOG_INPUT, "water sensor A", CHANNEL_WATER, SensorHandlerClass::WaterSensor, PersistentMemory.ps.humLimit);
	}
	else if (rm.equals(RUNMODE_DISTANCE) || rm.equals(RUNMODE_SENSORTEST)) {
		LogLine(4, __FUNCTION__, "before");
		sensorA.init(ANALOG_INPUT, "distance sensor A", CHANNEL_HUM, SensorHandlerClass::DistanceSensor, PersistentMemory.ps.humLimit);
		LogLine(4, __FUNCTION__, "after");
	}

#ifdef USE_GAS_SENSOR
	else if (rm.equals(RUNMODE_GAS)) {
		gasSensor.init(ANALOG_INPUT, "gas senso", CHANNEL_HUM, SensorHandlerClass::GasSensor);
	}
#endif
#ifdef USE_DHT11_SENSOR
	else if (rm.equals(RUNMODE_DHT11)) {
		thermometer.init(DHT11_SIGNAL_PIN, "thermohygrometer", 0, SensorHandlerClass::ThermoHygrometer);
	}
#endif

	waterValveA.init(VALVE_CTRL, "water valve A", PersistentMemory.ps.valveOpenDuration, PersistentMemory.ps.valveSoakTime);

	ConnectToWifi();
	WiFi.macAddress(localmacAddr);
	PersistentMemory.SetmacAddress(localmacAddr);

	// TODO: move time fetching to Wifi_nnr and generalize it.
	configTime(0, 0, "pool.ntp.org", "time.nist.gov");   // note - it may take some HOURS before actual time is correct.
	AdjustTime(2);  // configTime adjustments does not work!! Hence we adjust and include summertime permanently.

	while (time(nullptr) < 1510644967) {
		delayNonBlocking(10);
	}
	if (getCurrentTimeB()) {
		LogLine(2, __FUNCTION__, "Time fetched and adjusted");
	}
	else {
		LogLine(0, __FUNCTION__, "Time NOT fetched and adjusted");
	}

	SetupOTA();
	String ipAddr;
	ipAddr = GetIpAddress();

#ifdef USE_FIREBASE
	FB_BasePath = FB_DEVICE_PATH + PersistentMemory.GetmacAddress();
	InitFirebase(FB_BasePath.c_str());
	//RemoveTelemetryFromFirebase(); UploadLog_("Setup(): ALL TELEMETRY REMOVED");
	PersistentMemory.SetdeviceID(ipAddr);
	UploadStateAndSettings_(firstRun);
#endif

	LogLinef(1, __FUNCTION__, "OTA check. IP address:%s", ipAddr.c_str());
}


void loop() {
	String rm;

	#ifdef USE_FIREBASE
		GetSettingsFromFirebase_(firstRun);
		firstRun = false;
		InitFirebaseLogging(firebaseData, FB_BasePath, "log", JSON_BUFFER_LENGTH);

		while (PersistentMemory.GetRunOTA()) {
			LogLine(0, __FUNCTION__, "Starting OTA update. set OTA flag back to false manually");
			ArduinoOTA.handle();
			LogLine(0, __FUNCTION__, "after OTA handling. Checking firebase.");
			GetSettingsFromFirebase_(firstRun);
		}
	#endif

	rm = PersistentMemory.GetrunMode();
	LogLinef(4, __FUNCTION__, "runmode = %s", rm.c_str());
	if (rm.equals(RUNMODE_SOIL) || rm.equals(RUNMODE_WATER) ) {
		if (WaterIfNeeded()) {
			// use deep sleep if it's enabled and we want to soak for a longer period of time
			if ((waterValveA.soakSeconds <= DEEP_SLEEP_SOAK_THRESHOLD) || (!PersistentMemory.GetdeepSleepEnabled())) {
				waterValveA.WaitToSoak();
			}
			else {
				DeepSleepHandler.GotoSleepAndWakeAfterDelay(waterValveA.soakSeconds);
			}
		}
		if (loopCount++ >= NBR_OF_LOOPS_BEFORE_SLEEP) {
			DeepSleepHandler.GotoSleepAndWakeAfterDelay(PersistentMemory.GettotalSecondsToSleep());
		}
	}
	else if (rm.equals(RUNMODE_DISTANCE)) {
		sensorA.ReadSensor();
		UploadTelemetry_();
	}
	else if (rm.equals(RUNMODE_SENSORTEST)) {
		LogLine(4, __FUNCTION__, "Test sensor input");
//		waterValveA.OpenValve();
		sensorA.TestSensor();
//		sensorA.GetDistanceCentimeter();
//		delayNonBlocking(1000);
//		waterValveA.CloseValve();
		UploadTelemetry_();
	}
	else if (rm.equals(RUNMODE_BATTERYTEST)) {
		externalVoltMeter.TestSensor();
		UploadTelemetry_();
	}
	else if (rm.equals(RUNMODE_HARDWARETEST)) {
		testHardware();
	}
#ifdef USE_GAS_SENSOR
	else if (rm.equals(RUNMODE_GAS)) {
		if (!PersistentMemory.GetdeepSleepEnabled()) {
			UploadTelemetry_();
		}
		else {
			DeepSleepHandler.GotoSleepAndWakeAfterDelay(waterValveA.soakSeconds);
		}
		if (loopCount++ >= NBR_OF_LOOPS_BEFORE_SLEEP) {
			DeepSleepHandler.GotoSleepAndWakeAfterDelay(PersistentMemory.GettotalSecondsToSleep());
		}
	}
#endif
#ifdef USE_DHT11_SENSOR
	else if (rm.equals(RUNMODE_DHT11)) {
		UploadTelemetry_();
		LogLine(5, __FUNCTION__, "Uploaded DHT data");
		if (PersistentMemory.GetdeepSleepEnabled()) {
			//			if (loopCount++ >= NBR_OF_LOOPS_BEFORE_SLEEP) {
			LogLine(5, __FUNCTION__, "Call GotoSleep");
			DeepSleepHandler.GotoSleepAndWakeAfterDelay(PersistentMemory.GettotalSecondsToSleep());
			//			}
		}
	}
#endif
	else {
		LogLinef(0, __FUNCTION__, "Illegal option chosen: %s . Resetting runMode to normal", rm.c_str());
		PersistentMemory.SetrunMode(DefaultRunmode);
		LED_Flashes(200, 100);
	}

#ifdef RUN_ONCE
	while (true) {
		LogLine(1, __FUNCTION__, "wait forever (controlled by RUN_ONCE directive)");
		delayNonBlocking(60000);
	}
#else
	int d = PersistentMemory.GetmainLoopDelay();
	LogLinef(3, __FUNCTION__, "waiting mainLoopDelay: %d", d);
	delayNonBlocking(d * 1000);
#endif
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

void InitUnitTest() {
	delayNonBlocking(500);
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
	WiFi.macAddress(localmacAddr);
	PersistentMemory.SetmacAddress(localmacAddr);
	Serial.println(PersistentMemory.GetmacAddress());

	// TODO: move time fetching to Wifi_nnr and generalize it.
	configTime(0, 0, "pool.ntp.org", "time.nist.gov");   // note - it may take some HOURS before actual time is correct.
	AdjustTime(2);  // configTime adjustments does not work!! Hence we adjust and include summertime permanently.
	while (time(nullptr) < 1510644967) {
		delayNonBlocking(10);
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
	UploadStateAndSettings_();
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
			delayNonBlocking(500);
			digitalWrite(LED_BUILTIN, LOW);
			delayNonBlocking(500);
		}
	}
	LogLine(0, __FUNCTION__, "GO RESET");
	ESP.restart();
}

