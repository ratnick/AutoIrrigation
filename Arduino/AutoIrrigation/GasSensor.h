// GasSensor.h

#ifndef _GASSENSOR_h
#define _GASSENSOR_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "SensorHandler.h"


class GasSensorClass
{
 public:
	char name[15];
	SensorHandlerClass::SensorType sensorType;
	struct GasTelemetry {
		float cur_CO_ppm_measurement;
		float last_CO_ppm_measurement;
		float sens_val;
		int phase;
	};

	GasSensorClass::GasTelemetry gasTm;

	void init(int _pinNbr, char _name[], int _muxChannel, SensorHandlerClass::SensorType _sensorType);
	boolean GetTelemetryJson(FirebaseJson *json);
	float TestSensor();
	float GetlastPPM();
	float ConvertToPPM(float rawVal);

private:
	int muxChannel;
	int pinNbr;
	float lastAnalogueReading; 
	float lastPPM;
	boolean lastDigitalReading;


};

extern GasSensorClass GasSensor;

#endif

