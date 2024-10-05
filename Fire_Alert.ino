#include "DHT.h"
#include <SPI.h>
#include <Wire.h>
#include <EEPROM.h>
#include <WiFi.h>
#include <esp_task_wdt.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArtronShop_LineNotify.h>

#define SW1 16
#define SW2 17
// Line Token
#define Line_Token "Oz8Std7nyz5atmRCkKGtyRTGdenQXAo804gWSRRMSCL"
const char *ssid = "realme C55";
const char *pass = "tatty040347";
// Address Master and component
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_addr 0x3C
#define MPC23017 0x27
// Address Slave
#define ATH10 0x38
#define BH1750_VCC 0x5C
#define BH1750_GND 0x23
#define DHT22 13

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire); 

int timeout = 1000,timeless;

void SHOW_OLED(int value, int mode)
{
  display.clearDisplay();
  display.display();
  display.setTextSize(0.5);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Mode : ", mode);
  display.setCursor(60, 0);
  display.println(F("ADC : "));
  display.setCursor(80, 0);
  display.println(value);
  display.setCursor(0, 15);
  display.println(F("Address : "));
  display.setCursor(60, 15);
  display.println(add);
  display.setCursor(0, 25);
  display.println(F("Data : "));
  display.setCursor(40, 25);
  display.println(cov);
  display.display();
}
void ATH10_Transaction()
{
}
void BH1750_VCC_Transaction()
{
}
void BH1750_GND_Transaction()
{
}
void LINE_SEND_MESSAGE(string value)
{
  LINE.send(value);
}
int CHANGE_MODE()
{
  return 0;
}
int SAVE_DATA(int addr,int value)
{
  EEPORM.WriteUchar(addr,value);
  EEPORM.commit();
}
bool DISITION_GUIDE()
{

}
int Get_time()
{
  return 1000;
}
void setup()
{
  Serial.begin(115200);
  LINE.begin(Line_Token);
  WiFi.begin(ssid,pass);
  Wire.begin();
  dht.begin();
  pinMode(SW1,INPUT_PULLUP);
  pinMode(SW2,INPUT_PULLUP);
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_addr)) 
  {
    Serial.println(F("SSD1306 allocation failed"));
  }
}

void loop()
{
  if(millis() - timeless >= Get_time())
  {
    timeless = millis();
  }
}
