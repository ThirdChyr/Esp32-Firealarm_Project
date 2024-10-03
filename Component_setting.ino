#include <SPI.h>
#include <Wire.h>
#include <EEPROM.h>
#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArtronShop_LineNotify.h>

#define SW1 16
#define SW2 17
//Line Token 
#define Line_Token "Oz8Std7nyz5atmRCkKGtyRTGdenQXAo804gWSRRMSCL"
const char* ssid = "realme C55";
const char* pass = "tatty040347";
//Address Master and component
#define SCREEN_WIDTH 32
#define SCREEN_HEIGHT 32
#define OLED_addr 0x3C 
#define MPC23017  0x27
//Address Slave
#define ATH10 0x38
#define BH1750_VCC
#define BH1750_GND 
#define DHT22


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Hello, ESP32!");
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(10); // this speeds up the simulation
}
