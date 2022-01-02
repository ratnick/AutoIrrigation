// JsonHandler.h

#ifndef _JSONHANDLER_h
#define _JSONHANDLER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
	#include <ArduinoJson.h>
#else
	#include "WProgram.h"
#endif

// See: https://github.com/bblanchon/ArduinoJson/blob/master/examples/JsonGeneratorExample/JsonGeneratorExample.ino

class JsonHandlerClass
{
 protected:
	// Memory pool for JSON object tree.
	//
	// Inside the brackets, 200 is the size of the pool in bytes.
	// Don't forget to change this value to match your JSON document.
	// Use arduinojson.org/assistant to compute the capacity.
	// StaticJsonBuffer allocates memory on the stack, it can be
	// replaced by DynamicJsonBuffer which allocates in the heap.
	//
	//v5: StaticJsonBuffer<200> jsonBuffer;

	 
	// Using ArduinoJsopn v6 library 
	// Allocate the JSON document
	//
	// Inside the brackets, 200 is the RAM allocated to this document.
    // Don't forget to change this value to match your requirement.
    // Use https://arduinojson.org/v6/assistant to compute the capacity.
	StaticJsonDocument<200> jsonDoc; 
	 	
	JsonObject *root;

 public:
	void init();
	void AddJsonRootObject(String name, float value);
};

extern JsonHandlerClass JsonHandler;

#endif

