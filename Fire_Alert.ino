
// #include "DHT.h"
#include <SPI.h>
#include <Wire.h>
#include <EEPROM.h>
// #include <WiFi.h>
// #include <esp_task_wdt.h>
#include <Adafruit_GFX.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_SSD1306.h>
// #include <ArtronShop_LineNotify.h>

#define SW1 16
#define SW2 17
// Line Token
#define Line_Token "Oz8Std7nyz5atmRCkKGtyRTGdenQXAo804gWSRRMSCL"
const char *ssid = "realme C55";
const char *pass = "tatty040347";
// Address Master and component
#define EEPORM_SIZE 9
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

float rl_temp, rl_hum,rl_light;
int time_check = 5000, timeless = 0, compass = 1;
bool break_out = false, key_pass = false, Right_open = true, Left_open = false, Normal_mode = true, Fear_mode = false;

void print(String input, String value)
{
  Serial.print(input);
  Serial.print(" ");
  Serial.println(value);
}
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
void SHOW_WARNING()
{
  display.clearDisplay();
  display.display();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(20, 30);
  display.println(F("FIRE ALERT!!"));
  display.display();
}
bool VALUE_PROVE(float temp, float hum)
{
  if (temp >= 50 && hum <= 25)
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
}
void SAVE_DATA(int addr, float value)
{
  EEPORM.WriteUchar(addr, value);
  EEPORM.commit();
}
void CAL_MEAN(String name)
{
  if (name == "temp")
  {
    float Mean_value = (EEPROM.readUChar(0) + EEPORM.readUChar(2)) / 2;
    SAVE_DATA(1, Mean_value);
  }
  else if (name == "hum")
  {
    float Mean_value = (EEPROM.readUChar(3) + EEPORM.readUChar(5)) / 2;
    SAVE_DATA(4, Mean_value);
  }
  else if (name == "light")
  {
    float Mean_value = (EEPROM.readUChar(6) + EEPORM.readUChar(8)) / 2;
    SAVE_DATA(7, Mean_value);
  }
}
int Get_time(int timecheck)
{
  return timecheck;
}
void setup()
{
  Serial.begin(115200);
  Serial.println("---------------------Setup working------------------");
  pinMode(SW1, INPUT_PULLUP);
  pinMode(SW2, INPUT_PULLUP);
  // LINE.begin(Line_Token);
  // WiFi.begin(ssid,pass);
  //  Wire.begin();
  //   dht.begin();
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_addr))
  {
    Serial.println(F("SSD1306 allocation failed"));
  }
  if (!AHT10.begin())
  {
    Serial.println("AHT10 Not found!!");
  }
  if (!EEPROM.begin(EEPROM_SIZE))
  {
    Serial.println("Failed init EEPROM");
    delay(1000);
    ESP.restart();
  }

  // AHT 10 Start Collecting
  sensors_event_t humidity, temp;
  AHT10.getEvent(&humidity, &temp);
  rl_temp = temp.temperature;
  rl_hum = humidity.relative_humidity;

  print("Temp AHT10 ", rl_temp.str());
  print("Hum AHT10", rl_hum.str());
  SAVE_DATA(0, rl_temp);
  SAVE_DATA(3, rl_hum);
  // AHT 10 End

  // DHT 22 Event Started!!
  print("Temp DHT22", rl_temp.str());
  print("Hum DHT22", rl_hum.str());
  SAVE_DATA(2, rl_temp);
  SAVE_DATA(5, rl_hum);
  // DHT 22 End

  // BH1750_VCC Event Started!!
  print("Temp BH_VCC", rl_light.str());
  SAVE_DATA(6, rl_light);
  // BH1750_VCC

  // BH1750_GND Event Started!!
  print("Temp BH_GND", rl_light.str());
  SAVE_DATA(8, rl_light);
  //  BH1750_GND
  
  // Calculate Mean light,hum,temp value
  CAL_MEAN("temp");
  CAL_MEAN("hum");
  CAL_MEAN("light");
  // End calculate
   Serial.println("---------------------Setup finished!!------------------");

}

void loop()
{
  if (Normal_mode)
  {
    if (millis() - timeless > 5000)
    {
      timeless = millis();
      if (Right_open && !Left_open)
      {
        // AHT 10 Event Started
        sensors_event_t humidity, temp;
        AHT10.getEvent(&humidity, &temp);
        rl_temp = temp.temperature;
        rl_hum = humidity.relative_humidity;
        print("Temp", rl_temp.str());
        print("Hum", rl_hum.str());
        SAVE_DATA(0, rl_temp);
        SAVE_DATA(4, rl_hum);
        // AHT 10 Event Finished!!

        if(VALUE_PROVE())
        Right_open = !Right_open;
        Left_open = !Left_open;
      }
      else if (!Right_open && Left_open)
      {
        // DHT 22 Event Starte
        Right_open = !Right_open;
        Left_open = !Left_open;
      }
      // AHT10_START();
    }
  }
  else if (Fear_mode)
  {
  }
}
/*
    if (Serial.available() > 0)
    {
      String TEXT_INPUT = Serial.readString();
      TEXT_INPUT.trim();
      TEXT_INPUT.toLowerCase();
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
*/