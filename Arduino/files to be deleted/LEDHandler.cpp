// 
// 
// 

#include "LEDHandler.h"
#include "OTALib.h"

int LED_PIN = LED_BUILTIN;

void initFlashLED() {
	pinMode(LED_PIN, OUTPUT);
	LED_OFF();
}

void LED_ON()
{
	digitalWrite(LED_PIN, HIGH);
}

void LED_OFF()
{
	digitalWrite(LED_PIN, LOW);
}

void LED_Flashes(int count, int blinkDelayMs)
{
	for (int i = 0; i < count; i++) {
		LED_ON();
		delayNonBlocking(blinkDelayMs);
		LED_OFF();
		delayNonBlocking(blinkDelayMs);
	}
}
