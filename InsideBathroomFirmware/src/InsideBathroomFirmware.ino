/*
 * Project: IotBathroomScheduler- InsideBathroom Firmware
 * Description: Firmware for argon inside bathroom to monitor if bathroom is occupied, 
 *   as well as sound buzzer when appointment expires.
 * Author: Dylan Schulz, Clayton Rath, Sean Stille, Justin Vang
 * Date: 5/9/22
 * Reflection: It was difficult to get the buzzer and the sensors to work properly.
 */

#include "HC_SR04.h"
#include "DHT22Gen3_RK.h"

double voltage;
double brightness;
double humidity;
double tempC;

int webhookCounter;
int countReset = 4500; //counter to regulate webhook triggers
int appointmentStartTime = 1800000000;
int appointmentLength = 0;

bool lightIsOn = false;
bool doorIsClosed = false;
bool bathroomInUse = false;
bool excessiveHotWaterUsage = false;

const int trigPin = D4;
const int echoPin = D5;
const int photoResistor = A1;
const pin_t DHT_DATA_PIN = A3;
const int buzzer = A5;

const int delayTime = 800;
const int delayPause = 300;
const int freq = 128;
const int hz = 440;
const int defaultStartTime = 1800000000;
const int defaultMinutes = 0;
const int baseDistance = 0;
const double maxDoorDistance = 13.0; //Farthest distance the door can open in relation to distance sensor
const double highHumidity = 91.5;

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
  Particle.variable("excessiveHotWaterUsage", excessiveHotWaterUsage);
  Particle.subscribe("hook-response/currentMinutes", setAppointmentLength, MY_DEVICES);
  Particle.subscribe("hook-response/currentStartTime", setAppointmentStartTime, MY_DEVICES);
  Serial.begin(9600);
}

void loop() {
  
  //Monitor for light and door status
  checkLightOn(); 
  checkDoorClosed(rangefinder.getDistanceInch());
  (lightIsOn && doorIsClosed) ? bathroomInUse = true : bathroomInUse = false;

  //Get DHT sample
  dht.loop();
  dht.getSample(DHT_DATA_PIN, setTempAndHumid);

  //Fetch new current appointment data roughly every 20 seconds
  if (webhookCounter == 0) {
    Particle.publish("currentMinutes");
    Particle.publish("currentStartTime");
    webhookCounter = countReset;
    Serial.println("Webhooks triggered");
  }
  webhookCounter--;
  isTimeUp(); //check if appointment expired

  //Check if too much hot water is being used, sound buzzer if so
  overHotWaterUsage();
  excessiveHotWaterUsage ? analogWrite(buzzer, freq, hz) : analogWrite(buzzer, freq * 0, hz);
}

/*
* Check if the light in the bathroom is on (very sensitive).
* Give resistor a couple of seconds to tell if it is on and off.
*/
void checkLightOn() { 
  voltage = analogRead(photoResistor) / 4095.0 * 3.3;
  brightness = ((3.3 / voltage) - 1) * 8; //use the equation to get illuminance
  if(voltage > 1.3 || brightness < 9.9) { //change from # to appropriate number. Using voltage for now, change to brightness maybe (works).
    //Serial.printlnf("Brightness: %f", brightness);
    //Serial.printlnf("Voltage: %f", voltage);
    lightIsOn = true;
    //delay(delayTime / 2);
    voltage = 0.0; //reset voltage 
  } else { 
    lightIsOn = false;
    voltage = 0.0;
  }
}

//If HC_SR04 detects the door within maxDoorDistance, it considers it open, otherwise considers it not open
void checkDoorClosed(double curDistance) {
  if(curDistance >= maxDoorDistance) { 
    doorIsClosed = true; 
  }
  else if (curDistance <= maxDoorDistance || curDistance <= baseDistance) {
    doorIsClosed = false;
  }
}

//Set appointment length variable
void setAppointmentLength(String event, String data) {
  appointmentLength = data.toInt();
}

//Set appointment start date/time variable
void setAppointmentStartTime(String event, String data) {
  appointmentStartTime = data.substring(0,10).toInt();
}

//If the time for the alloted appointment is up, then the buzzer will go off.
void isTimeUp() {
  if ((int)Time.now() >= (appointmentStartTime + (appointmentLength * 60))) {
    Serial.println("Time's up");
    for (int i=0; i<5; i++) {
      analogWrite(buzzer, freq, hz);
      delay(delayTime);
      analogWrite(buzzer, freq * 0, hz);
      delay(delayPause);
    }
    appointmentStartTime = defaultStartTime;
    appointmentLength = defaultMinutes;
  }
}

//Check if humidity is high, aka someone is using too much hot water.
void overHotWaterUsage() {
  humidity > highHumidity ? excessiveHotWaterUsage = true : excessiveHotWaterUsage = false;
}

//Update the temperature and humidity cloud variables
void setTempAndHumid(DHTSample sample) {
	if (sample.isSuccess()) {
		tempC = sample.getTempC();
		humidity = sample.getHumidity();
	}
  else {
		sample.getSampleResult();
	}
}