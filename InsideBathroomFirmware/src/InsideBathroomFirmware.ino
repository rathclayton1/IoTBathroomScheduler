/*
 * Project: IotBathroomScheduler- InsideBathroom Firmware
 * Description: Bathroom schedule for firmware one.
 * Author: Dylan Schulz, Clayton Rath, Sean Stille, Justin Vang
 * Date: Sometime
 */
#include "HC_SR04.h"
#include "DHT22Gen3_RK.h"

double voltage;
double brightness;
double humidity;
double tempC;

bool overTime = false;
bool checkLight = false;
bool doorIsClosed = false;
bool bathroomInUse = false;

const int trigPin = D4;
const int echoPin = D5;
const int photoResistor = A1;
const pin_t DHT_DATA_PIN = A3;
const int buzzer = A5;
const int delayTime = 1000;
const int freq = 128;
const int hz = 440;
const double maxDoorDistance = 38.5; //Farthest distance the door can open in relation to distance sensor

HC_SR04 rangefinder = HC_SR04(trigPin, echoPin, 1.0, 500.0);
DHT22Gen3 dht(A2, A4);
void setTempAndHumid(DHTSample sample);

void setup() {
  pinMode(photoResistor, AN_INPUT);
  pinMode(buzzer, OUTPUT); 
  dht.setup();
  Particle.variable("bathroomInUse", bathroomInUse);
  Particle.variable("temperatureC", tempC);
  Particle.variable("humidity", humidity);
  Serial.begin(9600);
}

void loop() {
  checkLightOn(); //maybe check every so often (delay) to avoid error?
  //delay(delayTime); maybe put delay in the checkLightOn function, I think we're supposed to avoid delaying in loop() when possible
  checkDoorClosed();
  if (checkLight && doorIsClosed) bathroomInUse = true;
  else bathroomInUse = false;
  dht.loop();
  dht.getSample(DHT_DATA_PIN, setTempAndHumid);
}

/*
* Check if the light in the bathroom is on (very sensitive).
* Give resistor a couple of seconds to tell if it is on and off.
* Sometimes works well, other times it doesn't. 
* Says on when off sometimes and on when off sometimes, but works sometimes. 
* Is there a missing light variable? or variable? Is sitting in front of monitor. Will test when covered by cone (not really works).
*/
void checkLightOn() { 
  voltage = analogRead(photoResistor) / 4095.0 * 3.3;
  brightness = ((3.3 / voltage) - 1) * 8; //use the equation to get illuminance
  if(voltage > 1.3 || brightness < 9.9) { //change from # to appropriate number. Using voltage for now, change to brightness maybe (works).
    Serial.printlnf("Brightness: %f", brightness);
    Serial.printlnf("Voltage: %f", voltage);
    checkLight = true;
    Serial.println("Bathroom light is on (unless it isn't)");
    analogWrite(buzzer, freq, hz); //used to test photo-resistor 
    delay(delayTime / 2);
    voltage = 0.0; //reset voltage 
  } else {
    analogWrite(buzzer, freq * 0, hz); //used to test photo-resistor 
    checkLight = false;
    Serial.println("light is off (maybe)");
    voltage = 0.0;
  
  }
}

//If HC_SR04 detects the door within maxDoorDistance, it considers it open, otherwise considers it not open
void checkDoorClosed() {
  if (rangefinder.getDistanceInch() <= maxDoorDistance) doorIsClosed = false;
  else doorIsClosed = true;
}

//If the time for the alloted time is up, then the buzzer will go off.
void timeUseIsUp(bool timer) {
  if(overTime) {
    analogWrite(buzzer,freq,hz);
    delay(delayTime);
  } else {
    analogWrite(buzzer, freq * 0, hz);
  }
}

//Update the temperature and humidity cloud variables
void setTempAndHumid(DHTSample sample) {
	if (sample.isSuccess()) {
		tempC = sample.getTempC();
		humidity = sample.getHumidity();
	}
	else {
		Serial.printlnf("DHT sample not valid: sampleResult = %d", (int) sample.getSampleResult());
	}
}
