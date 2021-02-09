// Thermometer.h

#ifndef _THERMOMETER_h
#define _THERMOMETER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <jsmn.h>
#include <FirebaseJson.h>
#include "SensorHandler.h"
#include <DHTesp.h>

class ThermometerClass
{
  public:
	SensorHandlerClass::SensorType sensorType;
	struct TemperatureTelemetry {
		double temperature;
		double humidity;
		int lastAnalogueReadingTemp;
		int lastAnalogueReadingHumidity;
	};
	ThermometerClass::TemperatureTelemetry tempTm;
	DHTesp dht;

	void init(int _pinNbr, char _name[], int _muxChannel, SensorHandlerClass::SensorType _sensorType);
	void AddTelemetryJson(FirebaseJson* json);
	float ReadTemperature();
	float ReadHumidity();
	float GetlastAnalogueReadingTemperature();
	float GetlastAnalogueReadingHumidity();
	float TestSensor();

  private:
	int muxChannel;
	int pinNbr;
	char name[15];
	int currentCntTemp = 0;
	int currentCntHum = 0;
	float lastAnalogueReadingTemp;
	float lastAnalogueReadingHumidity;

};

extern ThermometerClass Thermometer;

#endif

