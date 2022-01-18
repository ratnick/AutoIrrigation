// globals.h

#ifndef _GLOBALS_h
#define _GLOBALS_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include <WifiLib.h>
const int LED_PIN = LED_BUILTIN;

#define MAX_DEEP_SLEEP 3600000000   // micro seconds
//#define MAX_KEPT_LOGLINES 5
//#define MAX_LINE_LENGTH 100
#define MAX_WAKEUPTIMES 4

struct PersistentDataStruct {
	// Metadata
	char macAddress[28];
	char deviceLocation[28];
	char deviceID[20];
	char sensorType[20];
	char hardware[50];
	char softwareVersion[80];

	// state
	char wifiSSID[20];
	char wifiPwd[16];
	boolean runOnce;
	boolean runOTA;
		// timestamp assigned on server side. not used in arduino.
	int currentSleepCycle; // counts which sleep cycle we are at right now.
	int secondsToSleep;
	int maxSleepCycles;
	int deviceStatus;  // 0:OK, see definitions below of DEVICE_STATUS_xx

	// settings
		// userupdate assigned on server side. not used in arduino.
	char runMode[16];
	int valveOpenDuration;
	int valveSoakTime;
	int humLimit;
	int mainLoopDelay;
	int debugLevel;
	boolean deepSleepEnabled;
	char wakeupTime[MAX_WAKEUPTIMES][7];  // 4 sets of 6 chars. Format: HHMMSS
	char pauseWakeTime[7];  // Format: HHMMSS
	double vccAdjustment;
	double vccMinLimit;

	// not sent to Firebase
	int totalSecondsToSleep;
	float lastVccSummarizedReading;

	//TODO	boolean stopInstantly;   /* if the sensor thinks something is wrong, this bit is flipped, and it will never open the valve until reset*/
	//TODO	time_t lastValveopenTimeStamp;
	//TODO	time_t lastValveopenDuration;
	//	char logLines[MAX_KEPT_LOGLINES][MAX_LINE_LENGTH];
};

enum HTTPRequestReturnType {
	notCalledYet,
	HTTPOK,
	Auth401Error,
	Forbidden403Error,
	WifiTimeoutError
};

enum BoardType {
	NodeMCU,
	WeMos,
	SparkfunThing,
	Other
};

struct Device {
	BoardType boardType = Other;            // OperationMode enumeration: NodeMCU, WeMos, SparkfunThing, Other
	unsigned int deepSleepSeconds = 0;      // Number of seconds for the ESP8266 chip to deepsleep for.  GPIO16 needs to be tied to RST to wake from deepSleep http://esp8266.github.io/Arduino/versions/2.0.0/doc/libraries.html
	WifiDevice wifi;
/*	int WifiIndex = 0;
	unsigned long LastWifiTime = 0;
	int WiFiConnectAttempts = 0;
	int wifiPairs = 5;
	String currentSSID;
	String pwd;*/
//	BoardType boardType = Other;            // OperationMode enumeration: NodeMCU, WeMos, SparkfunThing, Other
//	SensorMode sensorMode = None;           // OperationMode enumeration: DemoMode (no sensors, fakes data), Bmp180Mode, Dht11Mode
//	unsigned int deepSleepSeconds = 0;      // Number of seconds for the ESP8266 chip to deepsleep for.  GPIO16 needs to be tied to RST to wake from deepSleep http://esp8266.github.io/Arduino/versions/2.0.0/doc/libraries.html
//	SensorType sensorType = SENSORTYPE;
};

enum UploadType {
	UploadTelemetry,
	UploadStateAndSettings,
	GetSettings,
	UploadLog
};

// all above 10 is treated as a fault
const int DEVICE_STATUS_OK = 0;
const int DEVICE_STATUS_LOW_BATTERY = 1;
const int DEVICE_STATUS_WARNING = 2;
const int DEVICE_STATUS_FAULT = 10;


/*
Google Cloud Names

Quickstart path: https://console.cloud.google.com/home/dashboard?walkthrough_tutorial_id=iot_core_quickstart&project=autoirrigation

Project: AutoIrrigation
Topic: irrigation_topic
Device registry: my-registry (in region "europe-west1")
IOT Device: irrigation-device

*/

#endif
