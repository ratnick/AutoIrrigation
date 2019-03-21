// 
// 
// 

#include "JsonHandler.h"

// See: https://github.com/bblanchon/ArduinoJson/blob/master/examples/JsonGeneratorExample/JsonGeneratorExample.ino


void JsonHandlerClass::init()
{
	root* = jsonBuffer.createObject();
	//JsonObject& root = jsonBuffer.createObject();
}


void JsonHandlerClass::AddJsonRootObject(String name, float value) {
	String s = "sensor";
	root[s] = "gps";
}


JsonHandlerClass JsonHandler;
