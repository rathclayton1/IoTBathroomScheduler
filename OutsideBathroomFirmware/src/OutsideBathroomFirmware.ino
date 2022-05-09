/*
 * Project: IotBathroomScheduler- OutsideBathroom Firmware
 * Description: Firmware for argon outside bathroom to display if bathroom is occupied, 
 * as well as sound buzzer when appointment expires.
 * Author: Dylan Schulz, Clayton Rath, Sean Stille, Justin Vang
 * Date: End of semester
 */

#include "DHT22Gen3_RK.h"
#include <math.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


SerialLogHandler logHandler;

SYSTEM_THREAD(ENABLED);
// SYSTEM_MODE(MANUAL);

int blue = D3;
int red = D4;
int iterate = 0;

unsigned long appointmentTime = 0;
unsigned long notificationTime = 0;
int duration;
boolean isOccupied = false;
String nextName;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


void nameHandler(const char *event, const char *data){
    nextName = data;
    updateScreen(nextName);
    nextName += "is up soon!";
    
    if (appointmentTime > notificationTime){
        Particle.publish("Discord", nextName, PUBLIC);
    }
}

/**
 * This should handle the lights
 */
void statusHandler(const char *event, const char *data){
    if (data[0] == 't'){
        isOccupied = false;
    }
    else{
        isOccupied = true;
    }
}

void timeHandler(const char *event, const char *data){
    String stringData = (String)data;
    appointmentTime = stringData.toInt();
    notificationTime = appointmentTime + duration - 5000; //5000 for 5 minute warning.
    
    
}

void durationHandler(const char *event, const char *data){
    String stringData = (String)data;
    duration = stringData.toInt();

    
}


void setup() {
    Particle.subscribe("hook-response/BathroomStatus", statusHandler, MY_DEVICES);
    Particle.subscribe("hook-response/getTime", timeHandler, MY_DEVICES);
    Particle.subscribe("hook-response/getDuration", durationHandler, MY_DEVICES);
    Particle.subscribe("hook-response/getName", nameHandler, MY_DEVICES);
    pinMode(red, OUTPUT);
    pinMode(blue, OUTPUT);
    Serial.begin(9600);

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
        Serial.println(F("SSD1306 allocation failed"));
        for(;;); // Don't proceed, loop forever
    }

    // Show initial display buffer contents on the screen --
    // the library initializes this with an Adafruit splash screen.
    display.display();
    delay(2000); // Pause for 2 seconds

    // Clear the buffer
    display.clearDisplay();
    display.display();
}

void loop() {
String data = String(10);
delay(10000);

Particle.publish("getDuration", data, PRIVATE);

Particle.publish("getTime", data, PUBLIC);


    Particle.publish("BathroomStatus", data, PUBLIC);

    Particle.publish("getName", data, PUBLIC);
   
    if (isOccupied){
        digitalWrite(blue, HIGH);
        digitalWrite(red,LOW);
    }
    else{
        digitalWrite(blue, LOW);
        digitalWrite(red, LOW);
    }
}

void updateScreen(String name){
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(1, 2);
    display.setTextColor(WHITE);
    display.print(name);
    display.print(F(" is up next"));
    display.display();
}
