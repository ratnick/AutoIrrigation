// 
// 
// 

#include "backoff.h"
#include "NNR_OTAupdate.h"

int backOffCount = 0;
long minBackoff = 5000; // 1000 if you don't mind sending lots of data
long maxBackoff = 60000;
long minJitter = 50;
long maxJitter = 1000;

long currDelay = minBackoff;
long lastRequestTime = millis();

void resetBackoff() {
	backOffCount = 0;
}

// Returns true if the backoff duration has passed
bool backoff() {
	if ((millis() - lastRequestTime) > currDelay) {
		backOffCount++;
		currDelay = (backOffCount * backOffCount * minBackoff) + random(minJitter, maxJitter);
		if (currDelay > maxBackoff) {
			currDelay = maxBackoff;
		}
		Serial.printf("Backing off for : %ld ms\n", currDelay);
		delay(500); // FIXME remove
		lastRequestTime = millis();
		return true;
	}
	return false;
}
