#include "DHT.h"
#include <SPI.h>
#include <Wire.h>
#include <EEPROM.h>
#include <BH1750.h>
// #include <WiFi.h>
// #include <esp_task_wdt.h>
#include <Adafruit_GFX.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_SSD1306.h>
#include <ArtronShop_BH1750.h>
// #include <ArtronShop_LineNotify.h>

#define SW1 16
#define SW2 17
#define DHT_PIN 32
#define DHT_TYPE DHT22

// Line Token
#define Line_Token ""
const char *ssid = " ";
const char *pass = "";
// Address Master and component
#define EEPROM_SIZE 36
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_addr 0x3C
#define MPC23017 0x20
// Address Slave
// #define ATH10 0x38

ArtronShop_BH1750 bh1750_VCC(0x5C, &Wire);
ArtronShop_BH1750 bh1750_GND(0x23, &Wire);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);
DHT dhtSensor(DHT_PIN, DHT_TYPE);
Adafruit_AHTX0 AHT10;

float rl_temp, rl_hum, mean_temp, mean_hum, pr_value, rl_light, lux_left, lux_right;
int time_check = 5000, timeless = 0, mode_compass = 1, time_count = 0;
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
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(20, 30);
  display.println(F("Chmod complete"));
  display.display();
}
void SHOW_WARNING()
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(20, 30);
  display.println(F("FIRE ALERT!!"));
  display.display();
}
bool VALUE_PROVE(float temp, float hum)
{
  if (temp <= 50 && hum >= 25)
  {
    return true;
  }
  else
  {
    return false;
  }
}
int CREATE_ADDR(String name)
{
  if (name == "temp_left")
  {
    return 0;
  }
  else if (name == "temp_mean")
  {
    return 4;
  }
  else if (name == "temp_right")
  {
    return 8;
  }
  else if (name == "hum_left")
  {
    return 12;
  }
  else if (name == "hum_mean")
  {
    return 16;
  }
  else if (name == "hum_right")
  {
    return 20;
  }
  else if (name == "light_left")
  {
    return 24;
  }
  else if (name == "light_mean")
  {
    return 28;
  }
  else if (name == "light_right")
  {
    return 32;
  }
}
void SAVE_DATA(String name, float value)
{
  int address = CREATE_ADDR(name);
  EEPROM.put(address, value);
  EEPROM.commit();
}
void CAL_MEAN(String name)
{
  float temp_left, temp_right, hum_left, hum_right, light_left, light_right, Mean_value;
  if (name == "temp")
  {
    EEPROM.get(CREATE_ADDR("temp_left"), temp_left);
    EEPROM.get(CREATE_ADDR("temp_right"), temp_right);
    Mean_value = (temp_left + temp_right) / 2;
    SAVE_DATA("temp_mean", Mean_value);
  }
  else if (name == "hum")
  {
    EEPROM.get(CREATE_ADDR("hum_left"), hum_left);
    EEPROM.get(CREATE_ADDR("hum_right"), hum_right);
    Mean_value = (hum_left + hum_right) / 2;
    SAVE_DATA("hum_mean", Mean_value);
  }
  else if (name == "light")
  {
    EEPROM.get(CREATE_ADDR("light_left"), light_left);
    EEPROM.get(CREATE_ADDR("light_right"), light_right);
    Mean_value = (light_left + light_right) / 2;
    SAVE_DATA("light_mean", Mean_value);
  }
}

void setup()
{
  Serial.begin(115200);
  // Serial.begin(115200);
  delay(2000);
  Serial.println("Hello, ESP32!");
  Serial.print("---------------------Setup working------------------");
  pinMode(SW1, INPUT_PULLUP);
  pinMode(SW2, INPUT_PULLUP);
  // LINE.begin(Line_Token);
  // WiFi.begin(ssid,pass);
  dhtSensor.begin();
  Wire.begin();

  while (!bh1750_VCC.begin())
  {
    Serial.println("BH1750 not found !");
    delay(1000);
  }
  while (!bh1750_GND.begin())
  {
    Serial.println("BH1750 not found !");
    delay(1000);
  }
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

  print("Temp AHT10 ", String(rl_temp));
  print("Hum AHT10", String(rl_hum));
  SAVE_DATA("temp_left", rl_temp);
  SAVE_DATA("hum_left", rl_hum);
  // AHT 10 End

  // DHT 22 Event Started!!
  rl_temp = dhtSensor.readTemperature();
  rl_hum = dhtSensor.readHumidity();
  print("Temp DHT22", String(rl_temp));
  print("Hum DHT22", String(rl_hum));
  SAVE_DATA("temp_right", rl_temp);
  SAVE_DATA("hum_right", rl_hum);
  // DHT 22 End

  // BH1750_VCC Event Started!!
  lux_left = bh1750_VCC.light();
  print("Temp BH_VCC", String(lux_left));
  SAVE_DATA("light_left", lux_left);
  // BH1750_VCC

  // BH1750_GND Event Started!!
  lux_right = bh1750_GND.light();
  print("Temp BH_GND", String(lux_right));
  SAVE_DATA("light_right", lux_right);
  //  BH1750_GND

  // Calculate Mean light,hum,temp value
  CAL_MEAN("temp");
  CAL_MEAN("hum");
  CAL_MEAN("light");
  // End calculate*/
  Serial.print("---------------------Setup finished!!------------------");
}
void bh1750_working(String name)
{
  float lux;
  if (name == "vcc")
  {
    lux = bh1750_VCC.light();
    print("Temp BH_VCC", String(lux));
    SAVE_DATA("light_right", lux);
  }
  else if (name == "gnd")
  {
    lux = bh1750_GND.light();
    print("Temp BH_GND", String(lux));
    SAVE_DATA("light_left", lux);
  }
}
void dht22_working()
{
  rl_temp = dhtSensor.readTemperature();
  rl_hum = dhtSensor.readHumidity();
  print("Temp DHT22", String(rl_temp));
  print("Hum DHT22", String(rl_hum));
  SAVE_DATA("temp_left", rl_temp);
  SAVE_DATA("hum_left", rl_hum);
}
void aht10_working()
{
  sensors_event_t humidity, temp;
  AHT10.getEvent(&humidity, &temp);
  rl_temp = temp.temperature;
  rl_hum = humidity.relative_humidity;
  SAVE_DATA("light_right", lux_right);
  SAVE_DATA("temp_right", rl_temp);
  print("Temp AHT10 ", String(rl_temp));
  print("Hum AHT10", String(rl_hum));
}
void update_mean()
{
  CAL_MEAN("temp");
  CAL_MEAN("hum");
  CAL_MEAN("light");
  Serial.print("Update successfuly!");
}
void loop()
{
  if (Normal_mode)
  {
    mode_compass = 1;
    if (millis() - timeless > 10000)
    {
      timeless = millis();
      if (Right_open)
      {

        Serial.print("\nRIGHT_OPEN \n");

        aht10_working();
        //bh1750_working("vcc");

        update_mean();
        EEPROM.get(CREATE_ADDR("temp_mean"), mean_temp);
        EEPROM.get(CREATE_ADDR("hum_mean"), mean_hum);
        EEPROM.get(CREATE_ADDR("temp_right"), rl_temp);
        EEPROM.get(CREATE_ADDR("hum_right"), rl_hum);
        if (VALUE_PROVE(rl_temp, rl_hum) && ((rl_temp <= mean_temp + 10 ) && (rl_hum)))
        {
          Right_open = !Right_open;
          Left_open = !Left_open;
          delay(1000);
        }
        else
        {
          Normal_mode = false;
          Fear_mode = true;
        }
      }
      else if (Left_open)
      {

        Serial.print("LEFT_OPEN \n");
        //bh1750_working("gnd");
        dht22_working();

        update_mean();
        EEPROM.get(CREATE_ADDR("temp_mean"), mean_temp);
        EEPROM.get(CREATE_ADDR("hum_mean"), mean_hum);
        EEPROM.get(CREATE_ADDR("temp_right"), rl_temp);
        EEPROM.get(CREATE_ADDR("hum_right"), rl_hum);
        if (VALUE_PROVE(rl_temp, rl_hum))
        {
          Right_open = !Right_open;
          Left_open = !Left_open;
        }
        else
        {
          Normal_mode = false;
          Fear_mode = true;
        }
      }
    }
    if (Serial.available() > 0)
    {
      String TEXT_INPUT = Serial.readString();
      TEXT_INPUT.trim();
      TEXT_INPUT.toLowerCase();
      if (TEXT_INPUT == "crack")
      {
        Fear_mode = !Fear_mode;
        Normal_mode = !Normal_mode;
        delay(2000);
      }
    }
  }
  else if (Fear_mode)
  {
    Serial.print("---------------------Fear Mode Start!!------------------");
    if (millis() - timeless >= 2000)
    {
      
      bh1750_working("vcc");
      bh1750_working("gnd");
      update_mean();
      if()
      timeless = millis();
      mode_compass = 2;
      Serial.println("Clock taking");
      time_count++;
      if (time_count == 5)
      {
        time_count = 0;
        Fear_mode = !Fear_mode;
        Normal_mode = !Normal_mode;
      }
    }
  }

  EEPROM.get(CREATE_ADDR("temp_mean"), mean_temp);
  EEPROM.get(CREATE_ADDR("hum_mean"), mean_hum);
  SHOW_STATUS(mode_compass, mean_hum, mean_temp);
}
