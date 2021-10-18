/*
 Name:		TestDistanceSensor.ino
 Created:	6/10/2021 6:59:38 PM
 Author:	Nikolaj (lokal)
*/

/*
    PIN SETUP

    GND  -  GND (black)
    5V   -  VCC (Red)
    A0   -  SIG (Yellow)

*/

//Parameters
const int gp2y0a21Pin = A0;

//Variables
int gp2y0a21Val = 0;
float gp2y0a21Valf = 0.0;

void setup() {
    //Init Serial USB
    Serial.begin(115200);
    Serial.println(F("Initialize System"));
    //Init ditance ir
    pinMode(gp2y0a21Pin, INPUT);
}

void loop() {
    testGP2Y0A21();
    delay(1000);
}

void testGP2Y0A21() { /* function testGP2Y0A21 */
  ////Read distance sensor

    gp2y0a21Valf = analogRead(gp2y0a21Pin);
    gp2y0a21Val = analogRead(gp2y0a21Pin);
    Serial.print(F(" - int: "));            Serial.print(gp2y0a21Val);
    Serial.print(F(" - float: "));          Serial.print(gp2y0a21Valf); 
    Serial.print(F(" - average_value: "));  Serial.print(average_value(30));
    Serial.print(F(" - average_value2: "));  Serial.print(average_value2(30));
    Serial.print(F(" - rawToCm: "));        Serial.print(rawToCm(gp2y0a21Valf));
    Serial.print(F(" - distRawToPhys: "));  Serial.print(distRawToPhys(gp2y0a21Val));
    Serial.print(F(" - get_gp2d12: "));     Serial.println(get_gp2d12(gp2y0a21Val));

/*    if (gp2y0a21Val < 200) {
        Serial.println(F("Obstacle detected"));
    }
    else {
        Serial.println(F("No obstacle"));
    }
*/
}

int distRawToPhys(int raw) { /* function distRawToPhys */
  ////IR Distance sensor conversion rule
    float Vout = float(raw) * 0.0048828125; // Conversion analog to voltage
    int phys = 13 * pow(Vout, -1); // Conversion volt to distance

    return phys;
}

uint16_t get_gp2d12(uint16_t value) //Range operation functions for handling range sensor data sent back;
{
    if (value < 30)
        value = 30;
    return ((67870.0 / (value - 3.0)) - 40.0);
}

int rawToCm(float value) //Range operation functions for handling range sensor data sent back;
{
    if (value < 80)
        value = 80;
    if (value > 500)
        value = 500;
    return 4800/(value - 20); //converts the distance to cm
}

int average_value(int average_count) {
    float adjFactor = 1.8;   // rough measure
    int distance_cm;
    int sum = 0;

    for (int i = 0; i < average_count; i++) {
        int sensor_value = analogRead(gp2y0a21Pin);  //read the sensor value
        distance_cm = adjFactor * pow(3027.4 / sensor_value, 1.2134); //convert readings to distance(cm)
        sum = sum + distance_cm;
    }
    return(sum / average_count);
}

int average_value2(int average_count) {
    float adjFactor = 1.8;   // rough measure
    int distance_cm;
    float sum = 0.0;

    for (int i = 0; i < average_count; i++) {
        int sensor_value = analogRead(gp2y0a21Pin);  //read the sensor value
        sum = sum + sensor_value;
    }
    sum = sum / average_count;
    distance_cm = adjFactor * pow(3027.4 / sum, 1.2134); //convert readings to distance(cm)
    return(distance_cm);
}