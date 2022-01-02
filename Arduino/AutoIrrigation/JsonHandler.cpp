// 
// 
// 

#include "JsonHandler.h"

// See: https://github.com/bblanchon/ArduinoJson/blob/master/examples/JsonGeneratorExample/JsonGeneratorExample.ino


void JsonHandlerClass::init()
{
	//v5: root* = jsonBuffer.createObject();
	//JsonObject& root = jsonBuffer.createObject();
}


void JsonHandlerClass::AddJsonRootObject(String name, float value) {
	jsonDoc[name] = value;
	// v5:  String s = "sensor";
	// v5:  root[s] = "gps";
}

JsonHandlerClass JsonHandler;
