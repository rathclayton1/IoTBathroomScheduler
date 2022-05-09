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


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void sampleCallback(DHTSample sample);
String tester [] = {"Bob", "Joe", "Clarence", "John"};
void setup() {

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
String name = getName(iterate++);
name += " is currently up";
delay(5000);
updateScreen(name);
digitalWrite(blue,LOW);
digitalWrite(red, HIGH);
delay(5000);
digitalWrite(blue,HIGH);
digitalWrite(red, LOW);
updateScreen(name);
Particle.publish("Discord", name, PUBLIC);

}


String getName(int arraySpot){
    return tester[arraySpot % 4];
}

void updateScreen(String name){
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(1, 2);
    display.setTextColor(WHITE);
    display.print(name);
    display.print(F(" is up in 5 minutes!"));
    display.display();
}
