#define TEST_RECEIVE true
#define MAX_DEVICES 10
#define MAX_SVGS  10
#define MAX_SVG_RELATIONS  20
#define MAX_MODES 10
#define ALARM_OUT_REPEAT 5 * 1000
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
#include "TimeAlarms.h"
struct radioMessage {
  unsigned long code;
  unsigned int size;
};
enum alarm_status {STATUS_ARMED, STATUS_DISARMED, STATUS_PREALARM, STATUS_ALARM  };
struct current_status_struct {
  alarm_status status;
  alarm_status previousStatus;
  unsigned long statusTime;
  int alarmCount;
};
current_status_struct current_status;
parser::mode *modes;
parser::deviceStruct *devices;
bool testRadio = false;
bool testWired = false;
volatile unsigned long receivedData = 0;
int currentDeviceLenght = 0;
int currentModesLenght = 0;
radioMessage stringToRadioMessage(String);
String radioMessageToString(radioMessage);
void reloadSettings();
parser m_parser;
parser::mode currentMode;
AsyncWebServer server(80);
time_t getNtpTime() {
  return time(NULL);
}

void blink();
void setupArduinoOTA();
m_pcf8574 pcf;
RCSwitch mySwitch = RCSwitch();
bool sendData(String address, String type, String value) {
  return false;
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
  reloadSettings();
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
    if(request->hasParam("testWired")) {
      if(request->getParam("testWired")->value().equalsIgnoreCase("1")) {
        testWired = true;
      }
      else {
        testWired = false;
      }
    }
    if(request->hasParam("testRadio")) {
      if(request->getParam("testRadio")->value().equalsIgnoreCase("1")) {
        testRadio = true;
      }
      else {
        testRadio = false;
      }
    }
    if(request->hasParam("address") && request->hasParam("type") && request->hasParam("value")) {
      sendData(request->getParam("address")->value(), request->getParam("type")->value(), request->getParam("value")->value());
    }
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
  server.on("/data/gettime.json", [](AsyncWebServerRequest *request){
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->printf("{\"hour\" :%u,\"minute\" :%u,\"seconds\" :%u}", hour(), minute(), second());
    request->send(response);
  });

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
}//////////////////////////////////CONFIG_END/////////////////////////////////////////

int small_counter = 0;
int big_counter = 0;
///////////////////////////////////LOOP////////////////////////////////////////////////
void loop() {
  unsigned int periodic0 = 0;
  uint16_t pcfval = 0;
  String radioval = "";
  blink();
  pcf.handle();
  if(pcf.valuesChanged()) {
    Serial.println(pcf.read(), BIN);
  }

ArduinoOTA.handle();
//  mySwitch.send(5393, 24);
if(mySwitch.available()) {
  unsigned long value = mySwitch.getReceivedValue();
  for (size_t i = 0; i < currentDeviceLenght; i++) {
    if(devices[i].isSensor && !devices[i].globallyDisabled && (devices[i].type == parser::RADIO_PIR) && devices[i].address.equalsIgnoreCase(String(value))) {
      devices[i].lastActive = millis();
    }
  }
}
if(pcf.valuesChanged()) {
  pcfval = pcf.read();
}
else {
  pcfval = 0;
}
if(mySwitch.available()) {
  radioMessage msg;
  msg.code = mySwitch.getReceivedValue();
  msg.size = mySwitch.getReceivedBitlength();
  radioval = radioMessageToString(msg);
}
else {
  radioval = "0:0";
}
for (size_t i = 0; i < currentDeviceLenght; i++) {
  if(devices[i].isSensor && !devices[i].globallyDisabled) {
    if(devices[i].type == parser::RADIO_PIR && devices[i].address.equalsIgnoreCase(radioval)) {
      devices[i].lastActive = millis();
    }
    else if(devices[i].type == parser::WIRED_PIR && (1 << devices[i].address.toInt() & pcfval)) {
      devices[i].lastActive = millis();
    }
  }
}
for (size_t i = 0; i < currentDeviceLenght; i++) {
  if(millis() - devices[i].lastActive < (30 * 1000)) {
    devices[i].isAlarmed = true;
  }
  else {
    devices[i].isAlarmed = false;
  }
}
bool alarmnow = false;
switch (current_status.status) {
  case STATUS_ARMED:
  for (size_t i = 0; i < currentDeviceLenght; i++) {
    if(devices[i].isAlarmed) {
      alarmnow = true;
      for (size_t ii = 0; ii < currentMode.disabledSensorsSize; ii++) {
        if(currentMode.disabledSensors[ii] == devices[i].id) {
          alarmnow = false;
          break;
        }
      }
      if(alarmnow) {
        current_status.previousStatus = current_status.status;
        current_status.status = STATUS_PREALARM;
        current_status.statusTime = millis();
      }
    }
  }
  break;
  case STATUS_PREALARM:
    if(((millis() - current_status.statusTime) > (currentMode.preAlarmTime * 1000)) && (current_status.alarmCount < currentMode.maxAlarms)) {
      current_status.previousStatus = current_status.status;
      current_status.status = STATUS_ALARM;
      current_status.statusTime = millis();
      current_status.alarmCount = current_status.alarmCount + 1;
    }
  break;
  case STATUS_ALARM:
    if((millis() - current_status.statusTime) > (currentMode.alarmTime * 1000)) {
      current_status.previousStatus = current_status.status;
      current_status.status = STATUS_ARMED;
      current_status.statusTime = millis();
      for (size_t x = 0; x < currentDeviceLenght; x++) {
        if(devices[x].type == parser::WIRED_SIREN) {
          pcf.output(devices[x].address, 0);
        }
      }
    }
    else if((millis() - periodic0) > ALARM_OUT_REPEAT) {
      periodic0 = millis();
      for (size_t i = 0; i < currentDeviceLenght; i++) {
        if(!devices[i].isSensor) {
          if(devices[i].type == parser::RADIO_SIREN) {
            radioMessage r = stringToRadioMessage(devices[i].address);
            mySwitch.send(r.code, r.size);
          }
          else if(devices[i].type == parser::WIRED_SIREN) {
            pcf.output(devices[i].address, 1);
          }
        }
      }
    }
  break;
}

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
    //devices[1].isAlarmed = !devices[1].isAlarmed;
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
time_t stringToTime(String str) {
  int temp;
  String h = String(str);
  String m = h;
  temp = h.indexOf(":");
  m = m.substring(temp + 1);
  h.remove(temp);
  time_t t = h.toInt() * SECS_PER_HOUR + m.toInt() * SECS_PER_MIN;
  return t;
}
void alarmsHandler() {
  int day = weekday();
  day = 7 - day;
  day = day - 1;
  if(day == 0) {
    day = 6;
  }
  day = 1 << day;
  for (size_t i = 0; i < currentModesLenght; i++) {
    if(modes[i].activedays && day == 0) {
      continue;
    }
    if(modes[i].alarmID_start == Alarm.getTriggeredAlarmId()) {
      if(modes[i].armsSystem && (current_status.status == STATUS_DISARMED)) {
        current_status.previousStatus = current_status.status;
        current_status.status = STATUS_ARMED;
        current_status.statusTime = millis();
      }
      currentMode = modes[i];
    }
    else if(modes[i].alarmID_end == Alarm.getTriggeredAlarmId()) {
      if(modes[i].disarmsSystem && (current_status.status == STATUS_ARMED)) {
        current_status.previousStatus = current_status.status;
        current_status.status = STATUS_DISARMED;
        current_status.statusTime = millis();
      }
    }
  }
}
void setupSchedules() {
  for (size_t i = 0; i < currentModesLenght; i++) {
    if(modes[i].autoStart) {
      modes[i].alarmID_start = Alarm.alarmRepeat(stringToTime(modes[i].startTime), alarmsHandler);
    } else if(modes[i].autoEnd) {
      modes[i].alarmID_end = Alarm.alarmRepeat(stringToTime(modes[i].endTime), alarmsHandler);
    }
  }
}
void reloadSettings() {
  current_status.status = STATUS_DISARMED;
  current_status.previousStatus = STATUS_DISARMED;
  current_status.statusTime = 0;
  current_status.alarmCount = 0;
  currentDeviceLenght = m_parser.parseJSONFiletoStructDevices("/www/data/getdevices.json", &devices);
  for (size_t i = 0; i < currentDeviceLenght; i++) {
    Serial.println("reloadSettings");
    Serial.printf("%s %s\n", devices[i].address.c_str(), devices[i].name);
  }
  currentModesLenght = m_parser.parseFiletoStructMode("/www/data/getmode.json", &modes);
  currentMode = modes[0];
  for (size_t i = 0; i < currentModesLenght; i++) {
    Serial.printf("modename:%s disabledSensorsSize:%d\n", modes[i].modename.c_str(), modes[i].disabledSensorsSize);
  }
}
radioMessage stringToRadioMessage(String str) {
  int index = str.indexOf(":");
  radioMessage ret;
  String s = str;
  s.remove(index);
  ret.code = s.toInt();
  ret.size = str.substring(index + 1).toInt();
}
String radioMessageToString(radioMessage msg) {
  String ret;
  ret = String(msg.code) + ":" + String(msg.size);
  return ret;
}
