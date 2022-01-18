// SensorHandler.h

#ifndef _SENSORHANDLER_h
#define _SENSORHANDLER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

// Used by subclasses
#include <jsmn.h>
#include <FirebaseJson.h>
#include <NNR_Logging.h>
#include <NNR_OTAupdate.h>

#define SIMULATE_SENSORS false

class SensorHandlerClass
{
 protected:

 public:
	enum SensorType {
		SoilHumiditySensor,
		DistanceSensor,
		WaterSensor,
		GasSensor,
		ExternalVoltMeter,
		Thermometer,
		ThermoHygrometer
	};

	void init(int _pinNbr, SensorType _sensorType, char _metricUnit[], boolean _isAnalogue, float _minConvertedValue, float _maxConvertedValue);
	float ReadSensor();
	float TestSensor();
	float ReadAnalogueValue();
	boolean ReadDigitalValue();

	struct Telemetry {
		;
	};

	int pinNbr;
	char name[15];
	SensorType sensorType;
	char metricUnit[10];  
	boolean isAnalogue;
	float minConvertedValue;
	float maxConvertedValue;

 private:
};

extern SensorHandlerClass SensorHandler;

#endif

