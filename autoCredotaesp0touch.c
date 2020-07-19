/*
  OTAUpdate.ino, Example for the AutoConnect library.
  Copyright (c) 2020, Hieromon Ikasamo
  https://github.com/Hieromon/AutoConnect
  This example is an implementation of a lightweight update feature
  that updates the ESP8266's firmware from your web browser.
  You need a compiled sketch binary file to the actual update and can
  retrieve it using Arduino-IDE menu: [Sketck] -> [Export compiled binary].
  Then you will find the .bin file in your sketch folder. Select the.bin
  file on the update UI page to update the firmware.

  Notes:
  If you receive a following error, you need reset the module before sketch running.
  Update error: ERROR[11]: Invalid bootstrapping state, reset ESP8266 before updating.
  Refer to https://hieromon.github.io/AutoConnect/faq.html#hang-up-after-reset for details.

  This software is released under the MIT License.
  https://opensource.org/licenses/MIT
*/
#include "pin.h"
#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
typedef ESP8266WebServer  WiFiWebServer;
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <WebServer.h>
typedef WebServer WiFiWebServer;
#endif
#include <AutoConnect.h>

WiFiWebServer server;
AutoConnect portal(server);
AutoConnectConfig config;

Ticker blinker;

const int irSensor = 2;

// Timer: Auxiliary variables
long lastTrigger1 = 0;
int swipe=0;
long sec1=0;
long timenow=0;
bool isSwipe = false; 

void ISR_timer1()
{ 
  if(isSwipe){
  if(sec1==8 && swipe ==1)
  {digitalWrite(5,!digitalRead(5)); isSwipe=false; swipe=0; Serial.println("ON/off");}
  if(sec1==8 && swipe ==2)
  {digitalWrite(4,!digitalRead(4)); isSwipe=false; swipe=0; Serial.println("2ON/off");}
  if(sec1==8 && swipe ==3)
  {digitalWrite(16,!digitalRead(16)); isSwipe=false; swipe=0; Serial.println("3ON/off");}
  if(sec1==8)
  { sec1=0; swipe=0; isSwipe=false; Serial.println("finish");}
  sec1++;
  }
  else sec1=0;

  timenow++;

}

ICACHE_RAM_ATTR void detectsSwipe() {
  blinker.detach();
  isSwipe=true;
 if((timenow-lastTrigger1) > 1){
  swipe++;
  Serial.println(swipe);
  }
  timenow=0;
  lastTrigger1=timenow;
  blinker.attach(0.1,ISR_timer1);
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println();
  Serial.begin(115200);
  pinMode(16,OUTPUT);
  pinMode(4,OUTPUT);
  pinMode(5,OUTPUT);
  pinMode(3,OUTPUT);

  pinMode(irSensor,INPUT);

  digitalWrite(16,LOW);
  digitalWrite(5,LOW);
  digitalWrite(4,LOW);
  digitalWrite(3,LOW);

  attachInterrupt(digitalPinToInterrupt(irSensor), detectsSwipe, HIGH);
  blinker.attach(0.1,ISR_timer1);

  server.on("/pin", handlePin);

  server.on("/inline", [](){
    server.send(200, "text/plain", "this works as well");
  });

  // Responder of root page and apply page handled directly from WebServer class.
  server.on("/", []() {
    String content = R"(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8" name="viewport" content="width=device-width, initial-scale=1">
</head>
<body>
Place the root page with the sketch application.&ensp;
__AC_LINK__
         <p><a href="http://192.168.0.101/pin?out=4" class="button"> Fan </a></p>
         <p><a href="http://192.168.0.101/pin?out=5" class="button"> Light </a></p>
         <p><a href="http://192.168.0.101/pin?out=16" class="button"> Dim </a></p>
</body>
</html>
    )";
    content.replace("__AC_LINK__", String(AUTOCONNECT_LINK(COG_16)));
    server.send(200, "text/html", content);
  });

   AutoConnectConfig config("AbhroIoT3.0","passpass"); 
  config.title = "ABIoT3.0 Menu";
  config.ota = AC_OTA_BUILTIN;
  portal.config(config);
  if (portal.begin()) {
    Serial.println("WiFi connected: " + WiFi.localIP().toString());
}
}
void handlePin() {
  if(server.args()==0){
  server.send(200, "text/html", index_html);
  }
  else{
  for (uint8_t i = 0; i < server.args(); i++) {
    if (server.argName(i) == "out") {
      uint32_t out = strtoul(server.arg(i).c_str(), NULL, 10);
      pincode(out);
      }
    }
  }
}

void pincode(int out)
{
 digitalWrite(out,!digitalRead(out));
//Server.send(200,"text/plain",String(digitalRead(out)));
if(digitalRead(out)==HIGH)
  {
    server.send(200,"text/plain","ON"); 
  }
  else
  server.send(200,"text/plain","OFF");
}

void loop() {
  portal.handleClient();
}
