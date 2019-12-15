#include <WiFiUdp.h>

WiFiUDP NtpUdp;
unsigned int localNtpPort = 8888;  // local port to listen for UDP packets
const int timeZone = 0; 
char isoTime[30];

char* GetISODateTime() {
  sprintf(isoTime, "%4d-%02d-%02dT%02d:%02d:%02d", year(), month(), day(), hour(), minute(), second());
  return isoTime;
}

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte ntpPacketBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets


time_t getNtpTime()
{
  NtpUdp.begin(localNtpPort);
  
  while (NtpUdp.parsePacket() > 0) ; // discard any previously received packets
  Serial.print("Transmit NTP Request...");
  sendNTPpacket(timeServer);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = NtpUdp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println(".. got NTP Response");
      NtpUdp.read(ntpPacketBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)ntpPacketBuffer[40] << 24;
      secsSince1900 |= (unsigned long)ntpPacketBuffer[41] << 16;
      secsSince1900 |= (unsigned long)ntpPacketBuffer[42] << 8;
      secsSince1900 |= (unsigned long)ntpPacketBuffer[43];

      NtpUdp.stop();
      
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  NtpUdp.stop();
  
  Serial.println("..No NTP Response.");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
//void sendNTPpacket(IPAddress &address)
void sendNTPpacket(const char *host)
{
  // set all bytes in the buffer to 0
  memset(ntpPacketBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  ntpPacketBuffer[0] = 0b11100011;   // LI, Version, Mode
  ntpPacketBuffer[1] = 0;     // Stratum, or type of clock
  ntpPacketBuffer[2] = 6;     // Polling Interval
  ntpPacketBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  ntpPacketBuffer[12]  = 49;
  ntpPacketBuffer[13]  = 0x4E;
  ntpPacketBuffer[14]  = 49;
  ntpPacketBuffer[15]  = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:                 
  NtpUdp.beginPacket(host, 123); //NTP requests are to port 123
  NtpUdp.write(ntpPacketBuffer, NTP_PACKET_SIZE);
  NtpUdp.endPacket();
}
