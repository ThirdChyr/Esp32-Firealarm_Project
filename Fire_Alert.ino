#include <Adafruit_AHTX0.h>

// #include "DHT.h"
#include <SPI.h>
#include <Wire.h>
// #include <EEPROM.h>
// #include <WiFi.h>
// #include <esp_task_wdt.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
// #include <ArtronShop_LineNotify.h>

#define SW1 16
#define SW2 17
// Line Token
#define Line_Token "Oz8Std7nyz5atmRCkKGtyRTGdenQXAo804gWSRRMSCL"
const char *ssid = "realme C55";
const char *pass = "tatty040347";
// Address Master and component
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_addr 0x3C
#define MPC23017 0x27
// Address Slave
// #define ATH10 0x38
#define BH1750_VCC 0x5C
#define BH1750_GND 0x23
#define DHT22 13

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);
Adafruit_AHTX0 AHT10;

float rl_temp ,rl_hum ;
int time_check = 5000, timeless = 0, compass = 1;
bool break_out = false,key_pass = false;

void SHOW_STATUS(int mode, float hum, float tem)
{
  display.clearDisplay();
  display.setTextSize(0.5);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Fire-Alert Status ");
  display.setCursor(0, 20);
  display.println(F("Mode : "));
  display.setCursor(40, 20);
  if (mode == 1)
  {
    display.println("Stand up Mode!!");
  }
  else if (mode == 2)
  {
    display.println("Fear Mode!!");
  }
  display.setCursor(0, 30);
  display.println(F("Temperature : "));
  display.setCursor(80, 30);
  display.println(String(tem) + " C");

  display.setCursor(0, 40);
  display.println(F("Humidity : "));
  display.setCursor(65, 40);
  display.println(String(hum) + " %");
  display.display();
}
void SHOW_TRANSITION(String chmod)
{

  display.clearDisplay();
  display.display();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(20, 30);
  display.println(F("Chmod complete"));
  display.display();
}
void SHOW_WARNING( )
{
  display.clearDisplay();
  display.display();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(20, 30);
  display.println(F("FIRE ALERT!!"));
  display.display();
}
bool VALUE_PROVE(float temp,float hum)
{
  if(temp >=50 && hum <=25)
  {
    return true;
  }
  else
  {
    return false;
  }
}
int CHANGE_MODE(int compass)
{
  if (compass == 2)
  {
    return 2;
  }
  else if (compass == 1)
  {
    return 1;
  }
  return 1;
}/*
void AHT10_START()
{
   sensors_event_t humidity, temp;
   AHT10.getEvent(&humidity, &temp);
}
/*
int SAVE_DATA(int addr,int value)
{
  //EEPORM.WriteUchar(addr,value);
  //EEPORM.commit();
}
bool DISITION_GUIDE()
{

}*/
int Get_time(int timecheck)
{
  return timecheck;
}
void setup()
{
  Serial.begin(115200);
  if (AHT10.begin())
  {
    Serial.println("Not found AHT10");
  }
  //LINE.begin(Line_Token);
  //WiFi.begin(ssid,pass);
  // Wire.begin();
  //  dht.begin();
  pinMode(SW1, INPUT_PULLUP);
  pinMode(SW2, INPUT_PULLUP);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_addr))
  {
    Serial.println(F("SSD1306 allocation failed"));
  }
  if (!AHT10.begin()) {
    Serial.println("AHT10 Not found!!");
    while (1);
  }
  Serial.println("Start");
  // SHOW_STATUS(compass,20,50);
}

void loop()
{
  if (millis() - timeless > Get_time(time_check))
  {
    timeless = millis();
    //AHT10_START();
    sensors_event_t humidity, temp;
    AHT10.getEvent(&humidity, &temp);
     rl_temp = temp.temperature;
     rl_hum = humidity.relative_humidity;
     Serial.println(rl_temp);
     Serial.println(rl_hum);

    if (Serial.available() > 0)
    {
      String TEXT_INPUT = Serial.readString();
      TEXT_INPUT.trim();
      TEXT_INPUT.toLowerCase();
      // int TEXT_INT = TEXT_INPUT.toInt();
      if (TEXT_INPUT == "mode1")
      {
        compass = CHANGE_MODE(1);
        SHOW_TRANSITION("chmod");
        delay(2000);
      }
      else if (TEXT_INPUT == "mode2")
      {
        compass = CHANGE_MODE(2);
        SHOW_TRANSITION("chmod");
        delay(2000);
      }
      else if(TEXT_INPUT == "trig")
      {
        break_out = true;
      }
      else if(TEXT_INPUT == "notrig")
      {
        break_out = false;
        key_pass = false;
      }
      else if(TEXT_INPUT == "chmod")
      {
        key_pass = true;
      }
    }
    if(rl_temp >= 50 || rl_hum <= 20 ||key_pass)
    {
      if(!VALUE_PROVE(rl_temp,rl_hum))
      {
        time_check = 2000;
        compass = CHANGE_MODE(2);
      }
    }
  }
    if(break_out)
    {
      SHOW_WARNING();
      delay(2000);
    }
    else
    {
    SHOW_STATUS(compass, rl_hum,rl_temp);
    }
  
}
