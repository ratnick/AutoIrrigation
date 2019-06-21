// globals.h

#ifndef _GLOBALS_h
#define _GLOBALS_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

const int LED_PIN = LED_BUILTIN;

#define MAX_DEEP_SLEEP 3600000000   // micro seconds
//#define MAX_KEPT_LOGLINES 5
//#define MAX_LINE_LENGTH 100

struct PersistentDataStruct {
	char deviceID[20];
	char macAddress[30];
	char deviceLocation[30];
	char wifiSSID[20];
	char wifiPwd[15];
	char cloudUserName[30];
	char cloudPwd[15];
	char runMode[10];
	char wakeTime[6];
	int totalSecondsToSleep;
	int secondsToSleep;  
	int maxSleepCycles;  
	int currentSleepCycle; // counts which sleep cycle we are at right now.
	float lastVccSummarizedReading;
	int valveOpenDuration;
	int valveSoakTime;
	int humLimit;
	int mainLoopDelay;
	boolean deepSleepEnabled;

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

struct DeviceConfig {
	int WifiIndex = 0;
	unsigned long LastWifiTime = 0;
	int WiFiConnectAttempts = 0;
	int wifiPairs = 1;
	String currentSSID;
	String pwd;
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


/*
Google Cloud Names

Quickstart path: https://console.cloud.google.com/home/dashboard?walkthrough_tutorial_id=iot_core_quickstart&project=autoirrigation

Project: AutoIrrigation
Topic: irrigation_topic
Device registry: my-registry (in region "europe-west1")
IOT Device: irrigation-device

*/

#endif
