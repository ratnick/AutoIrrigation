// DistanceSensor.h
#ifndef _DISTANCESENSOR_h
#define _DISTANCESENSOR_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "SensorHandler.h"

class DistanceSensorClass : public SensorHandlerClass {
// class DistanceSensorClass {
	public:

	//SensorHandlerClass::SensorType sensorType;
	struct DistanceTelemetry {
		double distCm;
	};
	DistanceSensorClass::DistanceTelemetry soilTm;

	void init(int _pinNbr, char _name[], int _muxChannel, SensorHandlerClass::SensorType _sensorType, int _distLimit);
	void AddTelemetryJson(FirebaseJson* json);
	boolean CheckIfWater();
	float GetlastAnalogueReadingDist();
	float ReadSensor();
	float TestSensor();
	float GetDistanceCentimeter();
	void SetdistLimitPct(int _distLimitPct);

	const boolean DRY = false;
	const boolean WET = true;

private:
	const float DRY_VALUE_DISTANCE = 930.0; // Not used yet, but could be used for hysteresis. Above this value its very low water (dry)
	const float WET_VALUE_DISTANCE = 490.0; // Not used yet, but could be used for hysteresis. Below this value it's wet, and we need to drain
	float distLimitCentimeter;
	int muxChannel;
//	int pinNbr;  // defined in base class
	float lastAnalogueReadingDist;
	boolean lastDigitalReading;

};

extern DistanceSensorClass DistanceSensor;

#endif


