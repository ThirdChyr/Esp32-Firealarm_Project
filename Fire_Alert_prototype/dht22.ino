#include <DHT.h>

#define DHT_PIN 32
#define DHT_TYPE DHT22

DHT dhtSensor(DHT_PIN, DHT_TYPE);

void setup() {
  Serial.begin(115200);
  dhtSensor.begin();
}

void loop() {
  float temperature = dhtSensor.readTemperature();
  float humidity = dhtSensor.readHumidity();

  if (isnan(temperature)) {
    Serial.println("Error: Temperature reading is NaN");
  } else {
    Serial.print("Temperature: ");
    Serial.println(temperature);
  }

  if (isnan(humidity)) {
    Serial.println("Error: Humidity reading is NaN");
  } else {
    Serial.print("Humidity: ");
    Serial.println(humidity);
  }

  delay(2000);
}
