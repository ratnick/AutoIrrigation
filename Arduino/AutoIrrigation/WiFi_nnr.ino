

typedef struct
{
	String ssid;
	String pwd;
} hotspot_cred;
const int MAX_HOTSPOTS = 7;

const bool INIT_HOTSPOT = true;  // false: start trying what's already stored in EEPROM from last successful connection

#include "wifipasswords.h"
/* The file wifipasswords.h is private and has the following format:

hotspot_cred SSID_pairs[MAX_HOTSPOTS] = {
	{ "nohrx-yy", "*******" },
	{ "nohrx", ""*******" },
	{ "nohrx_2Gxxx", ""*******" },
	{ "nohrx_xxx", ""*******" },
	{ "CAR HOTSPOT", ""*******" },
	{ "ROOF HOTSPOT", ""*******" },
	{ "Nikxxx", ""*******" }
};

*/

static void initWifiDevice(int wifiSet) {   //wifiset always zero at the moment. But in time, it will be used for retries at different wifi ssid
	wifiDevice.currentSSID = SSID_pairs[wifiSet].ssid;
	wifiDevice.pwd = SSID_pairs[wifiSet].pwd;
}


int initWifi() {
	// return values:
	//  200: success. Is already connected.
	//	100: success connecting to SSID + pwd given in parameters
	//  0-N: success in connecting to (SSID, pwd) tuple number n.
	//   -1: no success in connecting.
	return initWifi("", "");
}

int initWifi(char _wifiSSID[], char _wifiPwd[]) {
	const int WifiTimeoutMilliseconds = 60000;  // 60 seconds
	const int MaxRetriesWithSamePwd = 20;
	int MaxLoopsThroughAll = 5;
	int retry;

	WiFi.mode(WIFI_STA);  // Ensure WiFi in Station/Client Mode
	
	if (WiFi.status() == WL_NO_SHIELD) {
		Serial.println("WiFi shield not present. TERMINATING");
		while (true);  // don't continue
	}

	if ( (WiFi.status() == WL_CONNECTED) && IsWifiStrenghtOK() && !INIT_HOTSPOT ) {   
		return 200;
	}
	else {

		//listNetworks();
		while (MaxLoopsThroughAll-- >= 0) {
			wifiDevice.WifiIndex = -1;
			Serial.println("Not connected. Trying all known wifi hotspots.");

			if (_wifiSSID[0] != 0 && !INIT_HOTSPOT) {
				Serial.print("initWifi: disconnect before retrying - ");
				WiFi.disconnect();
				Serial.print("initWifi: trying the SSID and pwd stored in EEPROM: "); // +String(_wifiSSID));
				WiFi.begin(_wifiSSID, _wifiPwd);

				// NNR: We should not proceed before we are connected to a wifi
				delay(500);
				retry = 0;
				while (WiFi.status() != WL_CONNECTED && retry++ < MaxRetriesWithSamePwd) {
					delay(500);
					Serial.print(".");
				}

				if (WiFi.status() == WL_CONNECTED && IsWifiStrenghtOK() ) {
					Serial.println(" => connected OK with already stored ssid/pwd");
					return 100;
				}
			}

			while (++wifiDevice.WifiIndex <= wifiDevice.wifiPairs) {

				if (wifiDevice.LastWifiTime > millis()) { delay(500); }

				initWifiDevice(wifiDevice.WifiIndex);
				Serial.print("initWifi: trying " + wifiDevice.currentSSID);
				WiFi.begin(wifiDevice.currentSSID.c_str(), wifiDevice.pwd.c_str());
				Serial.print(" .. begin:");

				// NNR: We should not proceed before we are connected to a wifi
				delay(500);
				retry = 0;
				while (WiFi.status() != WL_CONNECTED && retry++ < MaxRetriesWithSamePwd) {
					delay(500);
					Serial.print(".");
				}

				if (WiFi.status() == WL_CONNECTED && IsWifiStrenghtOK() ) {
					Serial.println(" => connected OK");
					return wifiDevice.WifiIndex;  
				}
				else {
					Serial.println(" could not connect");
					wifiDevice.WiFiConnectAttempts++;
					wifiDevice.LastWifiTime = millis() + WifiTimeoutMilliseconds;
					//if (wifiDevice.WifiIndex++ > wifiDevice.wifiPairs) { wifiDevice.WifiIndex = 1; }
				}
			}
		}
	}

	Serial.println("*** Could not connect to Wifi at all. Try 1) power cycling. 2) look if your SSID is defined in the list.");
	delay(10000);
	return -1;
}

const int WIFI_STRENGTH_LIMIT = -75; //db
boolean IsWifiStrenghtOK() {
	int strength = WiFi.RSSI();
	//Serial.printf("\n Wifi strength = %i\n", strength);
	LogLinef(3, __FUNCTION__, "Wifi strength = %ddB", strength);
	return (WiFi.RSSI() > WIFI_STRENGTH_LIMIT);
}

void PrintIPAddress() {

	int ipAddress;
	byte ipQuads[4];

	ipAddress = WiFi.localIP();
	ipQuads[0] = (byte)(ipAddress & 0xFF);;
	ipQuads[1] = (byte)((ipAddress >> 8) & 0xFF);
	ipQuads[2] = (byte)((ipAddress >> 16) & 0xFF);
	ipQuads[3] = (byte)((ipAddress >> 24) & 0xFF);

	//print the local IP address
	Serial.println("Connected with ip address: " + String(ipQuads[0]) + "." + String(ipQuads[1]) + "." + String(ipQuads[2]) + "." + String(ipQuads[3]));
}

void getCurrentTime() {
	boolean res = getCurrentTimeB();
	//LogLine(4, __FUNCTION__, "NTP time fetched: " + String(res));
}

boolean getCurrentTimeB() {
	int ntpRetryCount = 0;
	int max_retries = 3;

	while (timeStatus() == timeNotSet && ntpRetryCount < max_retries) { // get NTP time
		Serial.println(WiFi.localIP());
		setSyncProvider(getNtpTime);
		setSyncInterval(60 * 60);
		ntpRetryCount++;
	}
	LogLinef(4, __FUNCTION__, "NTP time fetched in %d tries", ntpRetryCount);
	return (ntpRetryCount < max_retries);
}

void listNetworks() {
	// scan for nearby networks:
	Serial.println("** Scan Networks **");
	int numSsid = WiFi.scanNetworks();
	if (numSsid == -1) {
		Serial.println("Couldn't get a wifi connection");
		while (true);
	}

	// print the list of networks seen:
	Serial.print("number of available networks:");
	Serial.println(numSsid);

	// print the network number and name for each network found:
	for (int thisNet = 0; thisNet < numSsid; thisNet++) {
		Serial.print(thisNet);
		Serial.print(") ");
		Serial.print(WiFi.SSID(thisNet));
		Serial.print("\tSignal: ");
		Serial.print(WiFi.RSSI(thisNet));
		Serial.print(" dBm");
	//	Serial.print("\tEncryption: ");
	//	printEncryptionType(WiFi.encryptionType(thisNet));
	}
}
