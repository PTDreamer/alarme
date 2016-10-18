#define TEST_RECEIVE true
#define MAX_DEVICES 10
#define MAX_SVGS  10
#define MAX_SVG_RELATIONS  20
#define MAX_MODES 10
#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiSetup.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <RCSwitch.h>
#include "m_pcf_8574.h"
#include <ESPAsyncWebServer.h>
#include <Hash.h>
#include "Time.h"
#include <ArduinoJson.h>
#include "indexHandler.h"
#include "modesHandler.h"
#include "settingsHandler.h"

parser::mode modes[MAX_MODES];
parser::deviceStruct devices[MAX_DEVICES];
bool testRadio = false;
bool testWired = false;
volatile unsigned long receivedData = 0;
int currentDeviceLenght = 0;
int currentModesLenght = 0;
parser m_parser;
AsyncWebServer server(80);
time_t getNtpTime() {
  return time(NULL);
}

void blink();
void setupArduinoOTA();
m_pcf8574 pcf;
RCSwitch mySwitch = RCSwitch();

void onFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){

}
void devicesHandler(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){

}

void saveSVGHandler(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){

}

void setup() {
  pinMode(0, OUTPUT); //ALIVE PIN

  Serial.begin(115200);
  if(!WiFiSetup::begin("AlarmeConfig")) {
    Serial.println("failed to connect and hit timeout");
    ESP.reset();
    delay(1000);
  }
  uint8_t mdnssuccess = MDNS.begin("alarme");
  configTime(3600, 0, "us.pool.ntp.org");
  setSyncProvider(getNtpTime);
  setSyncInterval(1 * 60);
  if (SPIFFS.begin()) {
      Serial.println("mounted file system");
    } else {
      Serial.println("failed to mount FS");
    }
  if(!SPIFFS.exists("www/data/getdevices.json")) {
    File f = SPIFFS.open("/data/getdevices.json", "w");
    f.println("{\"devices\": [],\"preAlarmTime\":0,\"alarmTime\":0}");
    f.close();
  }
  if(!SPIFFS.exists("www/data/getsvgs.json")) {
    File f = SPIFFS.open("/data/getsvgs.json", "w");
    f.println("[]");
    f.close();
  }
  if(!SPIFFS.exists("www/data/getrelations.json")) {
    File f = SPIFFS.open("/data/getrelations.json", "w");
    f.println("[]");
    f.close();
  }
  if(!SPIFFS.exists("www/data/alarmmodes.json")) {
    File f = SPIFFS.open("/data/alarmmodes.json", "w");
    f.println("[]");
    f.close();
  }
  mySwitch.enableReceive(4); // Receiver on interrupt 0 => that is pin #2
  mySwitch.enableTransmit(16);
  mySwitch.setProtocol(1);

  Serial.println("Start!");
  Serial.print("Connected to access point! IP Address=");
  Serial.print(WiFi.localIP());

  MDNS.addService("http","tcp",80);
  setupArduinoOTA();
  ArduinoOTA.begin();
  pcf.setup();

  server.on("/data/getsystemstatus.json",[](AsyncWebServerRequest *request){
  FSInfo fs_info;
  SPIFFS.info(fs_info);
  String json = "{";
  json += "\"heap\":"+String(ESP.getFreeHeap());
  json += ", \"analog\":"+String(analogRead(A0));
  json += ", \"gpio\":"+String((uint32_t)(((GPI | GPO) & 0xFFFF) | ((GP16I & 0x01) << 16)));
  json += ", \"fstotalbytes\":"+String(fs_info.totalBytes);
  json += ", \"fsusedbytes\":"+String(fs_info.usedBytes);
  json += ", \"fsblocksize\":"+String(fs_info.blockSize);
  json += ", \"fspagesize\":"+String(fs_info.pageSize);
  json += "}";
  request->send(200, "text/json", json);
  json = String();
  });
  server.on("/data/getteststatus.json",[](AsyncWebServerRequest *request){
  String json = "{";
  json += "\"testRadio\":"+String(testRadio);
  json += ", \"testWired\":"+String(testWired);
  json += ", \"receivedData\":"+String(receivedData);
  json += "}";
  request->send(200, "text/json", json);
  json = String();
  });

  server.on("/data/getdisabled.json",[](AsyncWebServerRequest *request){
  String json = "[";
  for (size_t i = 0; i < currentDeviceLenght; i++) {
    json += "{\"id\":" + String(devices[i].id) + ",\"isDisabled\":" + String(devices[i].isDisabled) + "}";
    if(i != currentDeviceLenght -1)
      json += ",";
  }
  json += "]";
  request->send(200, "text/json", json);
  json = String();
  });

  //server.on("/data/statuschange.json", HTTP_POST, [](AsyncWebServerRequest *request){
  //  Serial.print("WTF");
  //}, NULL, statusChangeBodyHandler);
  server.addHandler(new indexHandler(devices, &currentDeviceLenght));
  server.addHandler(new modesHandler());
  server.addHandler(new settingsHandler());

  server.serveStatic("/", SPIFFS, "/www/").setDefaultFile("index.html");

server.onNotFound([](AsyncWebServerRequest *request){
  Serial.printf("NOT_FOUND: ");
  if(request->method() == HTTP_GET)
    Serial.printf("GET");
  else if(request->method() == HTTP_POST)
    Serial.printf("POST");
  else if(request->method() == HTTP_DELETE)
    Serial.printf("DELETE");
  else if(request->method() == HTTP_PUT)
    Serial.printf("PUT");
  else if(request->method() == HTTP_PATCH)
    Serial.printf("PATCH");
  else if(request->method() == HTTP_HEAD)
    Serial.printf("HEAD");
  else if(request->method() == HTTP_OPTIONS)
    Serial.printf("OPTIONS");
  else
    Serial.printf("UNKNOWN");
  Serial.printf(" http://%s%s\n", request->host().c_str(), request->url().c_str());

  if(request->contentLength()){
    Serial.printf("_CONTENT_TYPE: %s\n", request->contentType().c_str());
    Serial.printf("_CONTENT_LENGTH: %u\n", request->contentLength());
  }

  int headers = request->headers();
  int i;
  for(i=0;i<headers;i++){
    AsyncWebHeader* h = request->getHeader(i);
    Serial.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
  }

  int params = request->params();
  for(i=0;i<params;i++){
    AsyncWebParameter* p = request->getParam(i);
    if(p->isFile()){
      Serial.printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
    } else if(p->isPost()){
      Serial.printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
    } else {
      Serial.printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
    }
  }

  request->send(404);
});

  server.begin();
  int at, pat;
  currentDeviceLenght = m_parser.parseJSONFiletoStructDevices("/www/data/getdevices.json",devices , at, pat);
  for (size_t i = 0; i < currentDeviceLenght; i++) {
    Serial.printf("%s %s\n", devices[i].address, devices[i].name);
  }
}//////////////////////////////////CONFIG_END/////////////////////////////////////////

int small_counter = 0;
int big_counter = 0;
///////////////////////////////////LOOP////////////////////////////////////////////////
void loop() {
  blink();
  pcf.handle();
  if(pcf.valuesChanged()) {
    Serial.println(pcf.read(), BIN);
  }

  ArduinoOTA.handle();
//  mySwitch.send(5393, 24);
  #ifdef TEST_RECEIVE
  if (mySwitch.available()) {
    int value = mySwitch.getReceivedValue();
    if (value == 0) {
      Serial.print("Unknown encoding");
    } else {
      Serial.print("Received ");
      Serial.print( mySwitch.getReceivedValue() );
      Serial.print(" / ");
      Serial.print( mySwitch.getReceivedBitlength() );
      Serial.print("bit ");
      Serial.print("Protocol: ");
      Serial.println( mySwitch.getReceivedProtocol() );
    }
    mySwitch.resetAvailable();
}
  #endif
}

void setupArduinoOTA() {
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
    type = "sketch";
    else // U_SPIFFS
    type = "filesystem";
    SPIFFS.end();
    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
}

void blink() {
  static int small_counter = 0;
  static int big_counter = 0;
  if(big_counter == 10) {
    digitalWrite(0, !digitalRead(0));
    devices[1].isAlarmed = !devices[1].isAlarmed;
    small_counter = 0;
    big_counter = 0;
  }
  else if(small_counter == 1000) {
    ++big_counter;
    small_counter = 0;
  }
  else {
    ++small_counter;
  }
}
