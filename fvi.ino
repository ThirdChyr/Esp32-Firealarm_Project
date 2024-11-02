#include <Arduino.h>
#include <Wire.h>
#include <ArtronShop_BH1750.h>

ArtronShop_BH1750 bh1750_VCC(0x5C, &Wire); // Non Jump ADDR: 0x23, Jump ADDR: 0x5C
ArtronShop_BH1750 bh1750_GND(0x23, &Wire);

void setup() {
  Serial.begin(115200);

  Wire.begin();
  while (!bh1750_VCC.begin()) {
    Serial.println("BH1750 not found !");
    delay(1000);
  }
  while (!bh1750_GND.begin()) {
    Serial.println("BH1750 not found !");
    delay(1000);
  }
}

void loop() {
  Serial.print("Light Right: ");
  Serial.println(bh1750_VCC.light());
  Serial.print("Light Left: ");
  Serial.println(bh1750_GND.light());
  Serial.print(" lx");
  Serial.println();
  delay(1000);
}