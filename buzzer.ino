#include <Wire.h>
#define MCP23017;
void setup()
{
    Wire.begin();
    Wire.beginTransmission(MCP23017); 
  Wire.write(0x00); // Address GPA
  Wire.write(0x00); // output
  Wire.endTransmission();

  // เปิด Port B และกำหนดให้เป็น Output
  Wire.beginTransmission(MCP23017); 
  Wire.write(0x01); // Address GPB
  Wire.write(0x00); // output
  Wire.endTransmission();

}
void testBuzzer() {
  Serial.println("Testing buzzer...");

  // เปิด buzzer
  buzzer_on();
  Serial.println("Buzzer ON");
  delay(1000);  // รอ 1 วินาที

  // ปิด buzzer
  buzzer_off();
  Serial.println("Buzzer OFF");
}

void loop() {
  testBuzzer(); // เรียกใช้งานฟังก์ชันทดสอบ
  delay(5000);  // รอ 5 วินาทีก่อนทำซ้ำ
}