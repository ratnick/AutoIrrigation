// 
// 
// 

#include "WifiHandler.h"

// constants
const int WifiTimeoutMilliseconds = 60000;  // 60 seconds
const int MaxRetriesWithSamePwd = 5;
const char timeServer[] = "0.dk.pool.ntp.org"; // Danish NTP Server 


boolean WifiHandlerClass::init(boolean usePredefinedSSID)
{

	WifiPair wifiPairA = { "TP-LINK NNR", "x" };
	String c = "lkklJ";
	String p = "LKJLJ";

	//	WifiPair wifiPair[2] = {

	String wifiPair[2][2]  = {
		{"TP-LINK NNR", "x"},
		{"Nik Z5", "x"}
	}	;

	// variables for object
	WifiIndex = 0;
	LastWifiTime = 0;
	WiFiConnectAttempts = 0;
	wifiPairs = 1;

	int MaxLoopsThroughAll = 5;
	int retry;

	//WiFi.mode(WIFI_STA);  // Ensure WiFi in Station/Client Mode

	if (WiFi.status() == WL_NO_SHIELD) {
		Serial.println("WiFi shield not present");
		while (true);  // don't continue
	}

	if (WiFi.status() == WL_CONNECTED) {
		return true;
	}
	else {
		while (MaxLoopsThroughAll-- >= 0) {
			WifiIndex = 0;
			Serial.println("Not connected. Trying all known wifi hotspots.");

			while (++WifiIndex <= wifiPairs) {

				if (LastWifiTime > millis()) { delay(500); }

				Serial.printf("\n%s %s\n", "initWifi: trying ", wifiPair[WifiIndex][0].c_str());
				WiFi.begin("lkjlkj", "lkjlkj");
//					wifiPair[WifiIndex][0].c_str()   , wifiPair[WifiIndex][1].c_str() );

				// NNR: We should not proceed before we are connected to a wifi
				delay(500);
				retry = 0;
				while (WiFi.status() != WL_CONNECTED && retry++ < MaxRetriesWithSamePwd) {
					delay(500);
					Serial.print(".");
				}

				if (WiFi.status() == WL_CONNECTED) {
					Serial.println(" => connected OK");
					return true;
				}
				else {
					Serial.println(" could not connect");
					WiFiConnectAttempts++;
					LastWifiTime = millis() + WifiTimeoutMilliseconds;
					//if (wifiDevice.WifiIndex++ > wifiDevice.wifiPairs) { wifiDevice.WifiIndex = 1; }
				}
			}
		}
	}

	Serial.println("*** Could not connect to Wifi at all. Try 1) power cycling. 2) look if your SSID is defined in the list.");
	while (true);  // don't continue


}

/*void initWifiDevice(int wifiSet) {   //wifiset always zero at the moment. But in time, it will be used for retries at different wifi ssid

	wifiDevice.wifiPairs = 3;		 // nbr of combinations below
	switch (wifiSet) {
	case 3:
		wifiDevice.ssid = "TP-LINK NNR";
		wifiDevice.pwd = "x";
		break;
	case 2:
		wifiDevice.ssid = "Nik Z5";
		wifiDevice.pwd = "x";
		break;
	case 1:
		wifiDevice.ssid = "nohrTDC";
		wifiDevice.pwd = "x";
		break;
	case 4:
		wifiDevice.ssid = "NikSession";
		wifiDevice.pwd = "x";
		break;
	case 5:
		wifiDevice.ssid = "TeliaGateway58-98-35-B5-DB-17";
		wifiDevice.pwd = "E5B004E62E";
		break;
	other:
		break;

	}
}*/

int initWifi() {
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

/*void getCurrentTime() {
	int ntpRetryCount = 0;
	while (timeStatus() == timeNotSet && ++ntpRetryCount < 3) { // get NTP time
																//Serial.println(WiFi.localIP());
		setSyncProvider(getNtpTime);
		setSyncInterval(60 * 60);
	}
}*/



WifiHandlerClass WifiHandler;

