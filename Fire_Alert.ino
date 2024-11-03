#include "DHT.h"
#include <SPI.h>
#include <WiFi.h>
#include <Wire.h>
#include <EEPROM.h>
#include <BH1750.h>
#include <PubSubClient.h>
#include <esp_task_wdt.h>
#include <Adafruit_GFX.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_SSD1306.h>
#include <ArtronShop_BH1750.h>
#include <WiFiClientSecure.h>
#include <ArtronShop_LineNotify.h>

#define SW1 16
#define SW2 17
#define DHT_PIN 32
#define DHT_TYPE DHT22
#define WDT_TIMEOUT 20
// Wifi communication
#define Line_Token "Myw41tE2e4s3wjJ6hyIeZoP4dbCSeBCwL3EcCRMhcqT"
const char *ssid = "realme C55";
const char *pass = "tatty040347";
#define MQTT_SERVER "104.214.189.245"
#define MQTT_PORT 1883
#define MQTT_USERNAME ""
#define MQTT_PASSWORD ""
#define MQTT_NAME "ESP32_1"
// Address Master and component
#define EEPROM_SIZE 36
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_addr 0x3C
#define MCP23017 0x20
// Address Slave

ArtronShop_BH1750 bh1750_VCC(0x5C, &Wire);
ArtronShop_BH1750 bh1750_GND(0x23, &Wire);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);
DHT dhtSensor(DHT_PIN, DHT_TYPE);
Adafruit_AHTX0 AHT10;

// Communication
WiFiClient client;
PubSubClient mqtt(client);
// ending

float rl_temp, rl_hum, mean_temp, mean_hum, pr_value, rl_light, lux_left, lux_right, temp_left, temp_right, hum_left, hum_right, light_left, light_right, light_mean;
int time_check = 5000, timeless = 0, mode_compass = 1, time_count = 0, second = 0, count_down = 20, alert = 10;
bool break_out = false, key_pass = false, Right_open = true, Left_open = false, Normal_mode = true, Fear_mode = false, Alert_mode = false;
String command, key, collect, value,temp_alert,temp_fear;
bool sta1 = false, sett = false, test = false, info = false, feed_dog = true;
void reset_bol()
{
  info = false;
  sta1 = false;
  sett = false;
  test = false;
}
void callback(char *topic, byte *payload, unsigned int length)
{
  for (int i = 0; i < 4; i++)
  {
    collect += (char)payload[i];
  }

  command = collect;
  collect = "";

  for (int i = 5; i < length; i++)
  {
    collect += (char)payload[i];
  }

  key = collect;
  collect = "";
  if (command == "info" || command == "Info")
  {
    info = true;
    sta1 = false;
    sett = false;
    test = false;
  }
  else if (command == "stat" || command == "Stat")
  {
    info = false;
    sta1 = true;
    sett = false;
    test = false;
  }
  else if (command == "sett" || command == "Sett")
  {
    info = false;
    sta1 = false;
    sett = true;
    test = false;
  }
  else if (command == "test" || command == "Test")
  {
    info = false;
    sta1 = false;
    sett = false;
    test = true;
  }
  else
  {
    mqtt.publish("/ESP32/RESPONSE", "Syntax error or have problem");
    info = false;
    sta1 = false;
    sett = false;
    test = false;
  }
}
void print(String input, String value)
{
  Serial.print(input);
  Serial.print(" ");
  Serial.println(value);
}
void SHOW_STATUS(int second, int mode, float hum, float tem)
{
  display.clearDisplay();
  display.setTextSize(0.5);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Fire-Alert Status ");
  display.setCursor(115, 0);
  display.println(second);
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
bool LIGHT_PROVE(float light_left, float light_right)
{
  EEPROM.get(CREATE_ADDR("light_mean"), light_mean);
  if (light_mean + 30 <= light_left || light_mean + 30 <= light_right)
  {
    return true;
  }
  return false;
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
  delay(2000);
  pinMode(SW1, INPUT_PULLUP);
  pinMode(SW2, INPUT_PULLUP);
  Serial.println("Hello, ESP32!");
  Serial.print("---------------------Setup working------------------");

  Serial.println("Configuring WDT...");
  esp_task_wdt_deinit();
  esp_task_wdt_config_t wdt_config = {
      .timeout_ms = WDT_TIMEOUT * 1000, // Convert seconds to milliseconds
      .trigger_panic = true             // Trigger panic on timeout
  };
  esp_task_wdt_init(&wdt_config);
  esp_task_wdt_add(NULL);
  // LINE.begin(Line_Token);
  // WiFi.begin(ssid,pass);
  dhtSensor.begin();
  Wire.begin();

  Wire.beginTransmission(MCP23017);
  Wire.write(0x00); // Address GPA
  Wire.write(0x00); // output
  Wire.endTransmission();

  Wire.beginTransmission(MCP23017);
  Wire.write(0x12);
  Wire.write(0x80);
  Wire.endTransmission();

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
  Serial.println();
  Serial.print("Connecting to ");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  LINE.begin(Line_Token);
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setCallback(callback);

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
void buzzer_on()
{
  Wire.beginTransmission(MCP23017);
  Wire.write(0x12);
  Wire.write(0x00);
  Wire.endTransmission();
}
void buzzer_off()
{
  Wire.beginTransmission(MCP23017);
  Wire.write(0x12);
  Wire.write(0x80);
  Wire.endTransmission();
}
void bh1750_working(String name)
{
  float lux;
  if (name == "vcc")
  {
    lux = bh1750_VCC.light();
    // print("Temp BH_VCC", String(lux));
    SAVE_DATA("light_right", lux);
  }
  else if (name == "gnd")
  {
    lux = bh1750_GND.light();
    // print("Temp BH_GND", String(lux));
    SAVE_DATA("light_left", lux);
  }
}
void dht22_working()
{
  rl_temp = dhtSensor.readTemperature();
  rl_hum = dhtSensor.readHumidity();
  // print("Temp DHT22", String(rl_temp));
  // print("Hum DHT22", String(rl_hum));
  SAVE_DATA("temp_left", rl_temp);
  SAVE_DATA("hum_left", rl_hum);
}
void aht10_working()
{
  sensors_event_t humidity, temp;
  AHT10.getEvent(&humidity, &temp);
  rl_temp = temp.temperature;
  rl_hum = humidity.relative_humidity;
  SAVE_DATA("hum_right", rl_hum);
  SAVE_DATA("temp_right", rl_temp);
  // print("Temp AHT10 ", String(rl_temp));
  // print("Hum AHT10", String(rl_hum));
}
void update_mean()
{
  CAL_MEAN("temp");
  CAL_MEAN("hum");
  CAL_MEAN("light");
  Serial.print("Update successfuly! \n");
}
void update_light()
{
  CAL_MEAN("light");
  Serial.print("Light update");
}
void reset_postition()
{
  Right_open = true;
  Left_open = false;
}
void reset_time()
{
  second = 0;
  if (temp_alert == "alert20")
  {
    alert = 20;
  }
  else if (temp_alert == "alert30")
  {
    alert = 30;
  }
  else if (temp_alert == "alert10")
  {
    alert = 10;
  }
  else
  {
    alert = 10;
  }

  if (temp_fear == "fear30")
  {
    count_down = 30;
  }
  else if (temp_fear == "fear40")
  {
    count_down = 40;
  }
  else if (temp_fear == "fear20")
  {
    count_down = 20;
  }
  else
  {
    count_down = 20;
  }
}
void updatein_nodered(float temp,float hum)
{
  mqtt.publish("/NODE_RED/TEMP", String(temp).c_str()); 
  mqtt.publish("/NODE_RED/HUM", String(hum).c_str());   
}
void update_mode()
{
  if(Normal_mode)
  {
    mqtt.publish("/NODE_RED/MODE","Normal mode");
  }
  else if (Fear_mode)
  {
    mqtt.publish("/NODE_RED/MODE","Fear mode");
  }
  else
  {
    mqtt.publish("/NODE_RED/MODE","Alert mode");
  }
}
void loop()
{
  if (!mqtt.connected())
  {
    Serial.print("MQTT connection... ");
    if (mqtt.connect(MQTT_NAME, MQTT_USERNAME, MQTT_PASSWORD))
    {
      Serial.println("connected");
      mqtt.subscribe("/LINE/MESSAGE");
      mqtt.subscribe("/LINE/RESPONSE");
      mqtt.subscribe("/NODE_RED/TEMP");
      mqtt.subscribe("/NODE_RED/HUM");
      mqtt.subscribe("/NODE_RED/MODE");
      mqtt.subscribe("/NODE_RED/RESET");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      delay(5000);
    }
  }
  else
  {
    mqtt.loop();
  }
  if (Normal_mode)
  {

    if (millis() - timeless > 1000)
    {
      if (digitalRead(SW1) == LOW || digitalRead(SW2) == LOW)
      {
        Serial.println("no feed");
        feed_dog = false;
      }
      else 
      {
        Serial.println("feed");
      }
      if (feed_dog)
      {
        esp_task_wdt_reset();
      }
      mode_compass = 1;
      timeless = millis();
      print("second is", String(second));
      if (Right_open && (second % 30 == 0) && second != 0)
      {
        if (feed_dog)
        {
          esp_task_wdt_reset();
        }
        Serial.print("\nRIGHT_OPEN\n");
        aht10_working();
        // bh1750_working("vcc");

        EEPROM.get(CREATE_ADDR("temp_mean"), mean_temp);
        EEPROM.get(CREATE_ADDR("hum_mean"), mean_hum);
        EEPROM.get(CREATE_ADDR("temp_right"), temp_right);
        EEPROM.get(CREATE_ADDR("hum_right"), hum_right);
        EEPROM.get(CREATE_ADDR("temp_left"), temp_left);
        EEPROM.get(CREATE_ADDR("hum_left"), hum_left);
        print("team_mean", String(mean_temp));
        print("hum_mean", String(mean_hum));
        print("temp_right", String(temp_right));
        print("hum_right", String(hum_right));
        print("temp_left", String(temp_left));
        print("hum_left", String(hum_left));

        if ((VALUE_PROVE(temp_right, hum_right) && (temp_right <= mean_temp + 10)) && VALUE_PROVE(temp_left, hum_left))
        {
          updatein_nodered(temp_right,hum_right);
          update_mode();
          Serial.println(" Right Verify");
          update_mean();
          Right_open = !Right_open;
          Left_open = !Left_open;
          delay(1000);
        }
        else
        {
          Right_open = !Right_open;
          Left_open = !Left_open;
          Normal_mode = false;
          Fear_mode = true;
        }
      }
      else if (Left_open && (second % 60 == 0) && second != 0)
      {
        if (feed_dog)
        {
          esp_task_wdt_reset();
        }
        Serial.print("LEFT_OPEN \n");
        // bh1750_working("gnd");
        dht22_working();

        EEPROM.get(CREATE_ADDR("temp_mean"), mean_temp);
        EEPROM.get(CREATE_ADDR("hum_mean"), mean_hum);
        EEPROM.get(CREATE_ADDR("temp_right"), temp_right);
        EEPROM.get(CREATE_ADDR("hum_right"), hum_right);
        EEPROM.get(CREATE_ADDR("temp_left"), temp_left);
        EEPROM.get(CREATE_ADDR("hum_left"), hum_left);
        print("team_mean", String(mean_temp));
        print("hum_mean", String(mean_hum));
        print("temp_left", String(temp_left));
        print("hum_left", String(hum_left));
        print("temp_right", String(temp_right));
        print("hum_right", String(hum_right));
        if (isnan(temp_left) || isnan(hum_left))
        {
          Fear_mode = false;
          Normal_mode = false;
          Alert_mode = true;
          reset_time();
          reset_postition();
        }
        if ((VALUE_PROVE(temp_left, hum_left) && (temp_left <= mean_temp + 10)) && VALUE_PROVE(temp_right, hum_right))
        {
          updatein_nodered(temp_right,hum_right);
          update_mode();
          if (feed_dog)
          {
            esp_task_wdt_reset();
          }
          Serial.println(" Left Verify");
          update_mean();
          Right_open = !Right_open;
          Left_open = !Left_open;
        }
        else
        {
          if (feed_dog)
          {
            esp_task_wdt_reset();
          }
          Right_open = !Right_open;
          Left_open = !Left_open;
          Normal_mode = false;
          Fear_mode = true;
        }
      }
      if (second == 120)
      {
        Serial.println("Got light value");
        bh1750_working("vcc");
        bh1750_working("gnd");
        update_light();
        second = -1;
      }
      second++;
    }
    if (Serial.available() > 0)
    {
      String TEXT_INPUT = Serial.readString();
      TEXT_INPUT.trim();
      TEXT_INPUT.toLowerCase();
      if (TEXT_INPUT == "test watchdog")
      {
        feed_dog = false;
        Fear_mode = !Fear_mode;
        Normal_mode = !Normal_mode;
        delay(2000);
      }
      else if (TEXT_INPUT == "reset")
      {
        feed_dog = true;
        Fear_mode = !Fear_mode;
        Normal_mode = !Normal_mode;
        delay(2000);
      }
    }
  }
  else if (Fear_mode)
  {
    mode_compass = 2;
    if (millis() - timeless >= 1000)
    {
      update_mode();
      if (feed_dog)
      {
        esp_task_wdt_reset();
      }
      Serial.print("---------------------Fear Mode Start!!------------------\n");
      print("time out in ", String(count_down));
      timeless = millis();
      bh1750_working("vcc");
      bh1750_working("gnd");
      aht10_working();
      dht22_working();
      EEPROM.get(CREATE_ADDR("temp_right"), temp_right);
      EEPROM.get(CREATE_ADDR("hum_right"), hum_right);
      EEPROM.get(CREATE_ADDR("temp_left"), temp_left);
      EEPROM.get(CREATE_ADDR("hum_left"), hum_left);
      EEPROM.get(CREATE_ADDR("light_left"), light_left);
      EEPROM.get(CREATE_ADDR("light_right"), light_right);
      if ((VALUE_PROVE(temp_right, hum_right) || VALUE_PROVE(temp_left, hum_left)) && LIGHT_PROVE(light_left, light_right))
      {
        Serial.println("Fire Alert");
        Normal_mode = false;
        Fear_mode = false;
        Alert_mode = true;
        reset_time();
        reset_postition();
        // SHOW_WARNING();
      }

      if (count_down == 0)
      {
        buzzer_off();
        Normal_mode = true;
        Fear_mode = false;
        Alert_mode = false;
        reset_postition();
        reset_time();
      }
      else
      {
        count_down--;
      }
    }
  }
  if (Alert_mode)
  {
    update_mode();
    if (millis() - timeless >= 1000)
    {
      if (feed_dog)
      {
        esp_task_wdt_reset();
      }
      timeless = millis();
      Serial.print("---------------------Alert Mode Start!!------------------\n");
      print("Alert in ", String(alert));
      buzzer_on();
      Serial.println("Sending messageing");
      if (alert % 2 == 0)
      {
        LINE.send("Fire Alert");
        if (feed_dog)
        {
          esp_task_wdt_reset();
        }
        // delay(1000);
        // buzzer_on();
      }
      else
      {
        // buzzer_off();
      }
      alert--;

      if (alert == 0)
      {
        if (feed_dog)
        {
          esp_task_wdt_reset();
        }
        Normal_mode = false;
        Fear_mode = true;
        Alert_mode = false;
        reset_postition();
        buzzer_off();
        reset_time();
      }
    }
  }
  if (Normal_mode)
  {
    if (feed_dog)
    {
      esp_task_wdt_reset();
    }
    EEPROM.get(CREATE_ADDR("temp_mean"), mean_temp);
    EEPROM.get(CREATE_ADDR("hum_mean"), mean_hum);
    SHOW_STATUS(second, mode_compass, mean_hum, mean_temp);
  }
  else if (Fear_mode)
  {
    if (feed_dog)
    {
      esp_task_wdt_reset();
    }
    EEPROM.get(CREATE_ADDR("temp_mean"), mean_temp);
    EEPROM.get(CREATE_ADDR("hum_mean"), mean_hum);
    SHOW_STATUS(count_down, mode_compass, mean_hum, mean_temp);
  }
  else if (Alert_mode)
  {
    if (feed_dog)
    {
      esp_task_wdt_reset();
    }
    SHOW_WARNING();
  }

  if (info)
  {
    String responseMessage = "All Command include in info,stat,sett,test";
    mqtt.publish("/ESP32/RESPONSE", responseMessage.c_str());
    reset_bol();
  }
  else if (sta1)
  {
    if (key == "temp")
    {
      String responseMessage = "Mean Temp = " + String(mean_temp);
      mqtt.publish("/ESP32/RESPONSE", responseMessage.c_str());
      reset_bol();
    }
    else if (key == "hum")
    {
      String responseMessage = "Mean Humidity = " + String(mean_temp);
      mqtt.publish("/ESP32/RESPONSE", responseMessage.c_str());
      reset_bol();
    }
    else if (key == "mode")
    {
      if (Normal_mode)
      {
        String responseMessage = "Mode = " + String("Normal");
        mqtt.publish("/ESP32/RESPONSE", responseMessage.c_str());
        reset_bol();
      }
      else if (Fear_mode)
      {
        String responseMessage = "Mode = " + String("Fear");
        mqtt.publish("/ESP32/RESPONSE", responseMessage.c_str());
        reset_bol();
      }
    }
  }
  else if (test)
  {
    if (key == "watchdog")
    {
      feed_dog = false;
      mqtt.publish("/ESP32/RESPONSE", "No feed has begun waiting 20 s");
        reset_bol();
    }
    else if (key == "reset")
    {
      feed_dog = true;
      mqtt.publish("/ESP32/RESPONSE", "Back to Feed");
        reset_bol();
    }
  }
  else if (sett)
  {
    if (key == "alert20")
    {
      mqtt.publish("/ESP32/RESPONSE", "Set Alert time to 20 second");
      alert = 20;
      temp_alert = alert;
      reset_bol();
    }
    else if (key == "alert30")
    {
      mqtt.publish("/ESP32/RESPONSE", "Set Alert time to 30 second");
      alert = 30;
      temp_alert = alert;
      reset_bol();
    }
    else if (key == "alert10")
    {
      mqtt.publish("/ESP32/RESPONSE", "Set Alert time to 10 second");
      alert = 10;
      temp_alert = alert;
      reset_bol();
    }
    else if (key == "fear30")
    {
      mqtt.publish("/ESP32/RESPONSE", "Set Fear more time to 30 second");
      count_down = 30;
      temp_fear = count_down;
      reset_bol();
    }
    else if (key == "fear40")
    {
      mqtt.publish("/ESP32/RESPONSE", "Set Fear more time to 40 second");
      count_down = 40;
      temp_fear = count_down;
      reset_bol();
    }
    else if (key == "fear20")
    {
      mqtt.publish("/ESP32/RESPONSE", "Set Fear more time to 20 second");
      count_down = 20;
      temp_fear = count_down;
      reset_bol();
    }
  }
}
