// GoogleCloudHandler.h

#ifndef _GOOGLECLOUDHANDLER_h
#define _GOOGLECLOUDHANDLER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

class GoogleCloudHandlerClass
{
 protected:

 public:
	 struct MessageData {
		 String deviceID;
		 String deviceTimestamp;
		 String msgId;
		 int    measurement1;
	 };

	 struct Cloud {
		 String			justfortest;
	 };

	void init();
};

extern GoogleCloudHandlerClass GoogleCloudHandler;

#endif

