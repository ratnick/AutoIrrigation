/*
    Name:       AutoIrrigation.ino
    Created:	17-09-2018 15:08:50
    Author:     Nikolaj Nøhr-Rasmussen
*/


#define HARDWARE_DESCRIPTION "WeMOS D1 r2, 5V/12V valve switch"
#define DEVICE_ID "prototype"

//#define MEASURE_INTERNAL_VCC      // When enabling, we cannot use analogue reading of sensor. They are mutually exclusive. 
#define MEASURE_EXTERNAL_VCC      // When enabling, we use A0 as external voltmeter. This disables sensing water level (by hardware). 
#define SIMULATE_WATERING true      // open the valve in every loop
// See also SIMULATE_SENSORS in SensorHandler.h

#define USE_WIFI

// Hmmm - which cloud to use:
//#define USE_GOOGLE_CLOUD
//#define USE_AZURE
#define USE_FIREBASE

// Program control:
//#define RUN_ONCE
#define FORCE_NEW_VALUES false		// Will overwrite all values in persistent memory
#define USE_DEEP_SLEEP				// When enabling, connect D0 to RST (on Wemos D1 mini)
#define DEEP_SLEEP_DURATION_SECS 120
#define NBR_OF_LOOPS_BEFORE_SLEEP 1
#define LOOP_DELAY 6 //secs

#ifdef USE_WIFI
#include <ArduinoJson.hpp>
#include <ArduinoJson.h>
#include <WiFiServerSecureBearSSL.h>
#include <WiFiServerSecureAxTLS.h>
#include <WiFiServerSecure.h>
#include <ESP8266WiFiType.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiServer.h>
#include <WiFiClient.h>
#endif

#ifdef USE_FIREBASE
#include <ESP8266HTTPClient.h>
#include <FirebaseObject.h>
#include <FirebaseHttpClient.h>
#include <FirebaseError.h>
#include <FirebaseCloudMessaging.h>
#include <FirebaseArduino.h>
#include <Firebase.h>
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

// Sensors and actuators
WaterSensorClass waterSensorA;
WaterValveClass waterValveA;
VoltMeterClass externalVoltMeter;
const int VALVE_OPEN_DURATION = 2;   // time a valve can be open in one run-time cycle, i.e. between deepsleeps
const int VALVE_SOAK_TIME     = 1;   // time between two valve openings

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
//#define FIREBASE_AUTH "AIzaSyBTr7zi9ZTDFMWdCJ61NHd2zn9JpurufyU"
#define FB_DEVICE_PATH "/irrdevices/"
String FB_BasePath;
#endif
const int JSON_BUFFER_LENGTH = 250;

void setup() {
	Serial.begin(115200);
	// init global configuration

	InitDebugLevel(2);

	// read persistent memory
	PersistentMemory.init(false);   // false: read from memory.  true: initialize
//	PersistentMemory.Printps();
//	PersistentMemory.UnitTest();  // just for testing
//	PersistentMemory.Printps();

	// re-enter deep sleep immediately if the counter has not reached maximum. Otherwise reset counters and continue to loop()
	LogLine(1, __FUNCTION__, "\n\nSleep cycle " + String(PersistentMemory.ps.currentSleepCycle) + " of " + String(PersistentMemory.ps.maxSleepCycles) + "\n");
	if (PersistentMemory.ps.currentSleepCycle < PersistentMemory.ps.maxSleepCycles) {
		LogLine(1, __FUNCTION__, "Going to straight to deep sleep (if you are debugging, you can skip this by hard reset)");
		PersistentMemory.ps.currentSleepCycle++;
		PersistentMemory.WritePersistentMemory();
		#ifdef USE_DEEP_SLEEP
			DeepSleepHandler.GoToDeepSleep(PersistentMemory.GetsecondsToSleep());
		#endif
	}
	else {
		LogLine(1, __FUNCTION__, "*** Resetting persistent memory (to reset counters)");
		PersistentMemory.ps.currentSleepCycle=0;
		PersistentMemory.WritePersistentMemory();
	}

	#if defined(MEASURE_INTERNAL_VCC) || defined(MEASURE_EXTERNAL_VCC)
		externalVoltMeter.lastSummarizedReading = PersistentMemory.ps.lastVccSummarizedReading;
		externalVoltMeter.init(A0, "5V voltmeter", SensorHandlerClass::ExternalVoltMeter, PersistentMemory.ps.lastVccSummarizedReading);
	#else
		// set up sensors and actuators
		waterSensorA.init(A0, "humidity sensor A", SensorHandlerClass::SoilHumiditySensor);
	#endif
	waterValveA.init(D5, "water valve A", VALVE_OPEN_DURATION, VALVE_SOAK_TIME);

	#ifdef USE_WIFI 
		initWifi();
		PrintIPAddress();
		delay(200);
		WiFi.macAddress(macAddr);
		PersistentMemory.SetmacAddress(macAddr);

		// repeating the fetching of time. 
		// TODO: move this to Wifi_nnr and generalize it.
		configTime(0, 0, "pool.ntp.org", "time.nist.gov");
		Serial.println("Waiting on time sync...");
		while (time(nullptr) < 1510644967) {
			delay(10);
		}
		Serial.println("Time fetched OK");
		getCurrentTime();
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
		ConnectAndUploadToCloud(UploadState);
	#endif
}


int wifiIndex = 0;

void loop() {

	int cnt = 0;
	int del;
	float vccTmp;
	
	ConnectAndUploadToCloud(GetState);  // maybe the user has changed values via the app

	#if defined(MEASURE_INTERNAL_VCC) || defined(MEASURE_EXTERNAL_VCC)
		vccTmp = externalVoltMeter.ReadVoltage();
		LogLine(0, __FUNCTION__, "Voltage reading" + String(vccTmp));
	#endif

	// enter control loop (sensor reading and actuator control)
	while (cnt++ < 2) {
		if ((waterSensorA.CheckIfWater() == WaterSensor.NO_WATER) || SIMULATE_WATERING) {
			LogLine(1, __FUNCTION__, "Low water detected");
			ConnectAndUploadToCloud(UploadTelemetry);
			waterValveA.OpenValve();
			ConnectAndUploadToCloud(UploadTelemetry);
			waterValveA.KeepOpen();
			ConnectAndUploadToCloud(UploadTelemetry);
			waterValveA.CloseValve();
			ConnectAndUploadToCloud(UploadTelemetry);
			waterValveA.WaitToSoak();
		}
		else {
			if (cnt == 0) {
				ConnectAndUploadToCloud(UploadTelemetry);  // this is to ensure we send a telemetry at least once
			}
		}
	}

	#ifdef USE_DEEP_SLEEP
		// update persistent memory in case something has changed
		PersistentMemory.ps.lastVccSummarizedReading = externalVoltMeter.lastSummarizedReading;
		PersistentMemory.WritePersistentMemory();
		if (loopCount++ >= NBR_OF_LOOPS_BEFORE_SLEEP) {
			// go to deep sleep (counter is reset during wakeup/setup() )
			Serial.print("\nGo to deep sleep.");
			DeepSleepHandler.GoToDeepSleep(PersistentMemory.ps.secondsToSleep);
		}
	#endif

	#ifdef RUN_ONCE
		while (true) {
			delay(10000);
			LogLine(1, __FUNCTION__, "wait forever");
		}
	#else
		LogLine(3, __FUNCTION__, "waiting mainLoopDelay");
		delay(PersistentMemory.GetmainLoopDelay() * 1000);
	#endif
}

void ConnectAndUploadToCloud(UploadType uploadType) {
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
				JsonObject& jsoMetadata = jsoStatic.createNestedObject("metadata");
				JsonObject& jsoState = jsoStatic.createNestedObject("state");
					JsonObject& stateTime = jsoState.createNestedObject("timestamp");
			//JsonObject& jsoStaticTele = jsonBufferRoot.createObject();
				JsonObject& jsoTelemetryCur = jsoStatic.createNestedObject("telemetry_current");
				JsonObject& jsoTelemetry = jsoStatic.createNestedObject("telemetry");
					JsonObject& jsoTeleTimestamp = jsoTelemetry.createNestedObject("timestamp");
				JsonObject& jsoLog = jsoStatic.createNestedObject("log");

			switch (uploadType) {
				case GetState:			GetState_(); break;
				case UploadState:		UploadState_(stateTime, jsoStatic, jsoMetadata, jsoState, jsoTelemetryCur, jsoTelemetry, jsoLog);			break;
				case UploadTelemetry:	UploadTelemetry_(jsoTelemetry, jsoTeleTimestamp);			break;
//				case UploadLog:			UploadLog_(jsoLog, logTxt);			break;
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
							case UploadState:
								LogLine(1, __FUNCTION__, "** UPLOAD STATE");
								requestResult = sendHttpMsg(uploadType, PersistentMemory.GetStateJson());
								break;
							case GetState:
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
		// set up wifi (configure wifi network if necessary.  
		// TODO: recheck wifi requested by user through push button
		wifiIndex = initWifi(PersistentMemory.ps.wifiSSID, PersistentMemory.ps.wifiPwd);
		delay(250);
		if (WiFi.status() != WL_CONNECTED) {
			LogLine(0, __FUNCTION__, "Could not connect to wifi: ");
		}
		else {
			PrintIPAddress();
			strcpy(PersistentMemory.ps.wifiSSID, wifiDevice.currentSSID.c_str());
			strcpy(PersistentMemory.ps.wifiPwd, wifiDevice.pwd.c_str());
		}
	}
#endif
}

boolean IsStateDataUpdatedByUser() {
	boolean res = Firebase.getBool(FB_BasePath + "/state/UserUpdate");
	if (res) {
		LogLine(3, __FUNCTION__, "true");
	}
	else {
		LogLine(3, __FUNCTION__, "false");
	}
	return res;
}

boolean DeviceExistsInFirebase() {
	String fbTestIfExists = Firebase.getString(FB_BasePath + "/state/SSID");
	if (fbTestIfExists.length() == 0) {
		UploadLog_("Device seems not to exist. Double checking.");
		delay(1000);
		fbTestIfExists = Firebase.getString(FB_BasePath + "/metadata/macAddr");
		LogLine(2, __FUNCTION__, "** CREATE BASIC DEVICE:  " + FB_BasePath + " returns: |" + fbTestIfExists + "|");
		if (fbTestIfExists.length() == 0) {
			LogLine(0, __FUNCTION__, "** Still zero. Device does not exist");
			delay(1000);
			return false;
		}
	}
	return true;
}

boolean IsPersistentMemorySet() {
	// Check if values are reasonable. Not bullit proof!
	return (PersistentMemory.GetCloudUsername().equals("initial value") );
}

void RemoveDummiesFromFirebase() {
	String s = FB_BasePath + "/telemetry/x";
	String res = Firebase.getString(s);
	if (res.length()>0) {
		LogLine(2, __FUNCTION__, "true");
		Firebase.remove(s);
	}
	else {
		LogLine(2, __FUNCTION__, "false");
	}
}


void GetState_() {
	LogLine(2, __FUNCTION__, "begin");
	if (IsStateDataUpdatedByUser()) {
		PersistentMemory.SetdeviceLocation(Firebase.getString(FB_BasePath + "/metadata/location"));
		PersistentMemory.SetdeviceID(Firebase.getString(FB_BasePath + "/metadata/deviceID"));
		PersistentMemory.SetsecondsToSleep(Firebase.getInt(FB_BasePath + "/state/secsToSleep"));
		PersistentMemory.SetmaxSleepCycles(Firebase.getInt(FB_BasePath + "/state/maxSlpCycles"));
		PersistentMemory.SetmainLoopDelay(Firebase.getInt(FB_BasePath + "/state/mainLoopDelay"));

		int val = Firebase.getInt(FB_BasePath + "/state/openDur");
		PersistentMemory.SetvalveOpenDuration(val);
		waterValveA.SetvalveOpenDuration(val);

		val = Firebase.getInt(FB_BasePath + "/state/soakTime");
		PersistentMemory.SetvalveSoakTime(val);
		waterValveA.SetvalveSoakTime(val);

		Firebase.setBool(FB_BasePath + "/state/UserUpdate", false);
		PersistentMemory.Printps();
	}
}

void CreateNewDevice(
	JsonObject& jsoStatic, JsonObject& jsoMetadata, JsonObject& jsoState, 
	JsonObject& jsoTelemetryCur, JsonObject& jsoTelemetry, JsonObject& jsoLog) {

	if (!IsPersistentMemorySet() || FORCE_NEW_VALUES) {
		int secondsToSleep = DEEP_SLEEP_DURATION_SECS % MAX_DEEP_SLEEP_SECS;
		int maxSleepCycles = DEEP_SLEEP_DURATION_SECS / MAX_DEEP_SLEEP_SECS;
		PersistentMemory.init(true, secondsToSleep, maxSleepCycles, DEVICE_ID, VALVE_OPEN_DURATION, VALVE_SOAK_TIME, LOOP_DELAY);   // false: read from memory.  true: initialize
		WiFi.macAddress(macAddr);
		PersistentMemory.SetmacAddress(macAddr);
	}

	jsoMetadata["macAddr"] = PersistentMemory.GetmacAddress();
	jsoState["SSID"] = PersistentMemory.GetwifiSSID();
	jsoTelemetryCur["x"] = "-";   
	jsoTelemetry["x"] = "-";	
	jsoLog["x"] = "-";

	jsoStatic.prettyPrintTo(Serial);
	Firebase.set(FB_BasePath, jsoStatic);
	if (Firebase.failed()) { LogLine(0, __FUNCTION__, "** CREATE BASIC DEVICE FAILED:  " + FB_BasePath + " - Firebase error msg: " + Firebase.error()); }
}

void UploadState_(
	JsonObject& stateTime, JsonObject& jsoStatic, JsonObject& jsoMetadata, 
	JsonObject& jsoState, JsonObject& jsoTelemetryCur, JsonObject& jsoTelemetry, 
	JsonObject& jsoLog) {

	LogLine(4, __FUNCTION__, "begin");

	// Check if device has been created. 
	// If not, create it. 
	// If it exists, read current state values from Firebase into persistent memory
	if ( !DeviceExistsInFirebase() ) {
		CreateNewDevice(jsoStatic, jsoMetadata, jsoState, jsoTelemetryCur, jsoTelemetry, jsoLog);
		LogLine(3, __FUNCTION__, FB_BasePath + " created");
		UploadLog_(FB_BasePath + " created");
	}
	else {
		ConnectAndUploadToCloud(GetState);
		RemoveDummiesFromFirebase();
	}

	if (IsStateDataUpdatedByUser()) {
		ConnectAndUploadToCloud(GetState);
	}

	// update values in persistent memory and write them back to Firebase
	stateTime[".sv"] = "timestamp";

	jsoMetadata["macAddr"] = PersistentMemory.GetmacAddress();
	jsoMetadata["location"] = PersistentMemory.GetdeviceLocation();
	jsoMetadata["deviceID"] = PersistentMemory.GetDeviceID();
	jsoMetadata["softwareVersion"] = SOFTWARE_VERSION;
	jsoMetadata["hardware"] = HARDWARE_DESCRIPTION;
	SendToFirebase("set", "metadata", jsoMetadata);

	jsoState["UserUpdate"] = false;
	jsoState["SSID"] = PersistentMemory.GetwifiSSID();
	jsoState["secsToSleep"] = PersistentMemory.GetsecondsToSleep();
	jsoState["maxSlpCycles"] = PersistentMemory.GetmaxSleepCycles();
	jsoState["curSleepCycle"] = PersistentMemory.GetcurrentSleepCycle();
	jsoState["openDur"] = PersistentMemory.GetvalveOpenDuration();
	jsoState["soakTime"] = PersistentMemory.GetvalveSoakTime();
	jsoState["mainLoopDelay"] = PersistentMemory.GetmainLoopDelay();
	#if defined(MEASURE_INTERNAL_VCC) || defined(MEASURE_EXTERNAL_VCC)
		jsoState["mode"] = "Battery Voltage";
	#else
		jsoState["mode"] = "Soil humidity";
	#endif
	#ifdef USE_DEEP_SLEEP
		jsoState["useDeepSleep"] = true;
	#else
		jsoState["useDeepSleep"] = false;
	#endif
	#ifdef RUN_ONCE
		jsoState["runOnce"] = true;
	#else
		jsoState["runOnce"] = false;
	#endif

	SendToFirebase("set", "state", jsoState);
}

void UploadTelemetry_(JsonObject& jsoTelemetry, JsonObject& jsoTeleTimestamp) {
	LogLine(4, __FUNCTION__, "begin");

	jsoTeleTimestamp[".sv"] = "timestamp";
	jsoTelemetry["wifi"] = WiFi.RSSI(); // signal strength
	jsoTelemetry["lastOpenTimestamp"] = waterValveA.lastOpenTimestamp;
	jsoTelemetry["humidity"] = waterSensorA.lastAnalogueReading;
	jsoTelemetry["valveState"] = waterValveA.valveState;

#if defined(MEASURE_INTERNAL_VCC) || defined(MEASURE_EXTERNAL_VCC)
	jsoTelemetry["Vcc"] = externalVoltMeter.ReadVoltage();
	jsoTelemetry["lastAnalogueReading"] = externalVoltMeter.lastAnalogueReading;
#endif
	SendToFirebase("push", "telemetry", jsoTelemetry);
	SendToFirebase("set", "telemetry_current", jsoTelemetry);
}

void UploadLog_(String _txt) {
	DynamicJsonBuffer  jsonBufferRoot(JSON_BUFFER_LENGTH);
	JsonObject& jsoStatic = jsonBufferRoot.createObject();
	JsonObject& jsoLog = jsoStatic.createNestedObject("log");
	JsonObject& jsoLogTimestamp = jsoLog.createNestedObject("timestamp");

	jsoLogTimestamp[".sv"] = "timestamp";
	jsoLog["txt"] = _txt;
	//jsoLog.prettyPrintTo(Serial);
	SendToFirebase("push", "log", jsoLog);
}

void SendToFirebase(String cmd, String subPath, JsonObject& jso) {

	// NOTE: If Firebase makes error apparantly without reason, try to update the fingerprint in FirebaseHttpClient.h. See https://github.com/FirebaseExtended/firebase-arduino/issues/328

	String s = FB_BasePath + "/" + subPath + "/";
	LogLine(3, __FUNCTION__, cmd + " to firebase: " + s);
	//jso.prettyPrintTo(Serial);

	if (jso.measureLength() < JSON_BUFFER_LENGTH) {

		if (cmd.equals("set")) { Firebase.set(s, jso); }
		if (cmd.equals("push")) { Firebase.push(s, jso); }

		if (Firebase.failed()) {
			LogLine(0, __FUNCTION__, "** SET/PUSH FAILED:  " + s + " - Firebase error msg: " + Firebase.error());
		}
	}
	else {
		LogLine(0, __FUNCTION__, "**** ERROR SET/PUSH FAILED: JSON too long");
		Serial.println(jso.measureLength());
		jso.prettyPrintTo(Serial);
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
#if defined(MEASURE_INTERNAL_VCC) || defined(MEASURE_EXTERNAL_VCC)
		root["Vcc"] = externalVoltMeter.ReadVoltage();
		root["lastAnalogueReading"] = externalVoltMeter.lastAnalogueReading;
#endif
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
		case UploadState:
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
		case GetState:
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

			if (uploadType == GetState) {
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

