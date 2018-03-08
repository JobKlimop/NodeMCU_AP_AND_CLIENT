#include <EEPROM.h>
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>

const char* APssid = "LightDuino";
const char* APpass = "LightDuinoPass1!";

int eeAddress = 0;
const char* ssid;
const char* pass;

ESP8266WebServer server(80);

void onAccessPoint() {
  delay(1000);
   Serial.println();
   
   WiFi.softAP(APssid, APpass);

   IPAddress apip = WiFi.softAPIP();
   Serial.print("Visit: ");
   Serial.println(apip);

   server.on("/", handleRoot);
   server.on("/data", setNetworkData);
   server.on("/clear", clearEepRom);
    
   server.begin();
   Serial.println("Server has started");
}

void onWifiClient() {
  delay(1000);
  WiFi.begin(ssid, pass);
  Serial.println("");

  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("");
  Serial.println("Connection made to: " + String(ssid));
  Serial.print("IP adress: ");
  Serial.println(WiFi.localIP());
  
  server.on("/", handleRoot);
    
  server.begin();
  Serial.println("HTTPServer started!");

  
}

void handleRoot() {
  server.send(200, "text/html", "<h1>Connection made!</h1>");
}

void setNetworkData() {
  String received = server.arg("plain");
  server.send(200, "text/plain", received);

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(server.arg("plain"));

  ssid = root["ssid"];
  pass = root["pass"];

  Serial.print("Content of body: ");
  Serial.println(received);

  EEPROM.begin(128);
  EEPROM.put(eeAddress, ssid);
  eeAddress += strlen(ssid) + 1;
  EEPROM.put(eeAddress, pass);
  if(EEPROM.commit() == true) {
    Serial.println("EEPROM.commit = TRUE");
  } else {
    Serial.println("EEPROM.commit = FALSE");
  }
  Serial.print("EeAddress: ");
  Serial.println(eeAddress);
  eeAddress = 0;

  getSsid();
  getPass();
  
  delay(1000);
  ESP.restart();
}

void clearEepRom() {
  int a;
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 0);
    Serial.println(i, a);
  }
  Serial.println("EEPROM cleared");
  ESP.restart();
}

void getSsid() {
  ssid = EEPROM.get(eeAddress, ssid);
  Serial.print("GetSSID: ");
  Serial.println(ssid);
}

void getPass() {
  eeAddress += strlen(ssid) + 1;
  pass = EEPROM.get(eeAddress, pass);
  eeAddress = 0;
  Serial.print("GetPASS: ");
  Serial.println(pass);
}

void setup() {
  // put your setup code here, to run once:
  delay(1000);
  Serial.begin(9600);
  Serial.println("");
  
  getSsid();
  
  Serial.print("STARTUP SSID: ");
  Serial.println(ssid);

  if(ssid == '\0'){
    onAccessPoint();
  } else {
    onWifiClient();
  }

}

void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();
}
