#include <ArtronShop_LineNotify.h>
#include <WiFi.h>

#define sw1 5
#define sw2 18
#define Line_Token "Oz8Std7nyz5atmRCkKGtyRTGdenQXAo804gWSRRMSCL"
const char* ssid = "realme C55";
const char* pass = "tatty040347";

int realtime = 0;
bool state = true;
bool onetime = true;
bool change = false;
bool onehit = 0;
int counting = 0;
int modes = 1;
void ARDUINO_ISR_ATTR swmode();
void setup()
{
  Serial.begin(115200);
  Serial.println("connect to WIFI realme c55");
  WiFi.begin(ssid,pass);

  LINE.begin(Line_Token); 
  pinMode(sw1,INPUT_PULLUP);
  pinMode(sw2,INPUT_PULLUP);


  while (WiFi.status() != WL_CONNECTED) { // วนลูปหากยังเชื่อมต่อ WiFi ไม่สำเร็จ
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());


  attachInterrupt(sw2,swmode,CHANGE);
} 
void stop_onehit()
{
  if(onehit)
  {
    LINE.send("Stop respond");
    onehit = !onehit;
  }
}
void ARDUINO_ISR_ATTR swmode()
{
  if(modes == 1 )
  {
    modes = 2;
  }
  else
  {
    modes = 1;
  }
  change = true;
  Serial.println("Changeing");
}

void loop() 
{
 if(change)
 {
  if(modes == 1)
  {
    LINE.send("Fear mode");
  }
  else if(modes == 2)
  {
    LINE.send("Steady mode");
  }
  change = false;
 }


 if(digitalRead(sw1))
 {
  if(digitalRead(sw2))
  {
    if(millis() - realtime >= 2000)
    {
    realtime = millis();
    LINE.send("respond 2 s");
    }
  }
  else if(digitalRead(sw2) == LOW)
  {
    if(millis() - realtime >= 5000)
    {
    realtime = millis();
    LINE.send("respond 5 s");
    }
    
  }
 }
 }
  
