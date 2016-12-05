#include "parser.h"
#include <ArduinoJson.h>
#include <FS.h>                   //this needs to be first, or it all crashes and burns...

int parser::parseJSONFiletoStructDevices(String filename, deviceStruct** devices) {
  int ret;
  if(!SPIFFS.exists(filename)) {
    Serial.print("FILE NOT FOUND");
    return 0;
  }
  File f = SPIFFS.open(filename, "r");
  char *buffer = new char[f.available()];
  f.readBytes(buffer, f.available());
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject((const char*)buffer);
  if (!root.success()) {
    delete buffer;
    return 0;
  }
  if(!(root.containsKey("devices"))) {
    delete buffer;
    return 0;
  }
  JsonArray& array = root["devices"].asArray();
  Serial.println("creating array");
  *devices = new parser::deviceStruct[array.size()];
  Serial.printf("size:%d\n", array.size());
  if (array.success())
  {
    ret = array.size();
    for(int i=0;i<ret; ++i) {
      Serial.printf("%d\n", i);
      (*devices)[i].id = array[i]["id"];
      (*devices)[i].name = array[i]["name"];
      (*devices)[i].isSensor = array[i].as<JsonObject>().get<bool>("isSensor");
      const char* temp = array[i]["type"];
      if(strcmp(temp, "remote"))
        (*devices)[i].type = REMOTE;
      else if(strcmp(temp, "wiredBuzzer"))
        (*devices)[i].type = WIRED_BUZZER;
      else if(strcmp(temp, "radioBuzzer"))
        (*devices)[i].type = RADIO_BUZZER;
      else if(strcmp(temp, "radioSiren"))
        (*devices)[i].type = RADIO_SIREN;
      else if(strcmp(temp, "wiredSiren"))
        (*devices)[i].type = WIRED_SIREN;
      else if(strcmp(temp, "wiredPIR"))
        (*devices)[i].type = WIRED_PIR;
      else if(strcmp(temp, "radioPIR"))
        (*devices)[i].type = RADIO_PIR;
      (*devices)[i].address = array[i]["address"].as<String>();
      (*devices)[i].isDisabled = array[i].as<JsonObject>().get<bool>("isDisabled");
      (*devices)[i].globallyDisabled = array[i].as<JsonObject>().get<bool>("globallyDisabled");
      (*devices)[i].mqtt = array[i]["mqtt"];
      (*devices)[i].lastActive = 0;
    }
    delete buffer;
    Serial.printf("returning %d\n",ret );
    return ret;
  }
  delete buffer;
  return 0;
}

int parser::parseStructDevicesToJSONFile(char *filename, deviceStruct* devices, int lenght) {
  File f = SPIFFS.open(filename, "w");
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  JsonArray& devs = root.createNestedArray("devices");
    for(int i=0;i<lenght; ++i) {
      JsonObject& device = jsonBuffer.createObject();
      device["id"] = devices[i].id;
      device["name"] = devices[i].name;
      device["isSensor"] = devices[i].isSensor;
      const char* temp;
      switch (devices[i].type){
        case REMOTE:
        temp = "remote";
        break;
        case WIRED_BUZZER:
        temp = "wiredBuzzer";
        break;
        case RADIO_BUZZER:
        temp = "radioBuzzer";
        break;
        case RADIO_SIREN:
        temp = "radioSiren";
        break;
        case WIRED_SIREN:
        temp = "wiredSiren";
        break;
        case WIRED_PIR:
        temp = "wiredPIR";
        break;
        case RADIO_PIR:
        temp = "radioPIR";
        break;
      }
      device["type"] = temp;
      device["address"] = devices[i].address;
      device["isDisabled"] = devices[i].isDisabled;
      device["mqtt"] =devices[i].mqtt;
      devs.add(device);
    }
    root.printTo(f);
    return 0;
}
int parser::parseSVGFiletoStructSVG(char *filename, svg* svgs) {
  int ret;
  if(!SPIFFS.exists(filename)) {
    return 0;
  }
  File f = SPIFFS.open(filename, "r");
  char *buffer = new char[f.available()];
  f.readBytes(buffer, f.available());
  DynamicJsonBuffer jsonBuffer;
  JsonArray& array = jsonBuffer.parseArray((const char*)buffer);
  if (!array.success()) {
    delete buffer;
    return 0;
  }
  ret = array.size();
  for(int i=0;i<ret; ++i) {
    svgs[i].filename = array[i]["name"];
    svgs[i].friendlyname = array[i]["friendly"];
  }
  delete buffer;
  return ret;
}

int parser::parseStructSVGToJSONFile(char *filename, svg* svgs, int lenght) {
  File f = SPIFFS.open(filename, "w");
  DynamicJsonBuffer jsonBuffer;
  JsonArray& array = jsonBuffer.createArray();
  for(int i=0;i<lenght; ++i) {
    JsonObject& svg = jsonBuffer.createObject();
    svg["name"] = svgs[i].filename;
    svg["friendly"] = svgs[i].friendlyname;
    array.add(svg);
  }
    array.printTo(f);
    return 0;
}

int parser::parseFiletoStructMode(char *filename, mode** modes) {
  int ret;
  if(!SPIFFS.exists(filename)) {
    return 0;
  }
  File f = SPIFFS.open(filename, "r");
  char *buffer = new char[f.available()];
  f.readBytes(buffer, f.available());
  DynamicJsonBuffer jsonBuffer;
  JsonArray& array = jsonBuffer.parseArray((const char*)buffer);
  if (!array.success()) {
    delete buffer;
    return 0;
  }
  ret = array.size();
  *modes = new parser::mode[array.size()];
  for(int i=0;i<ret; ++i) {
    (*modes)[i].id = array[i]["id"];
    (*modes)[i].modename = array[i]["name"].as<String>();
    (*modes)[i].autoStart = array[i]["autoStart"].as<bool>();
    (*modes)[i].autoEnd = array[i]["autoEnd"].as<bool>();
    (*modes)[i].startTime = array[i]["startTime"].as<String>();
    (*modes)[i].endTime = array[i]["endTime"].as<String>();
    (*modes)[i].armsSystem = array[i]["armsSystem"].as<bool>();
    (*modes)[i].disarmsSystem = array[i]["disarmsSystem"].as<bool>();
    (*modes)[i].preAlarmTime = array[i]["preAlarmTime"].as<int>();
    (*modes)[i].alarmTime = array[i]["alarmTime"].as<int>();
    (*modes)[i].maxAlarms = array[i]["maxAlarms"].as<int>();
    (*modes)[i].paused = array[i]["paused"].as<bool>();
    (*modes)[i].active = array[i]["active"].as<bool>();
    (*modes)[i].activedays = array[i]["activedays"].as<int>();
    JsonArray &ds = array[i]["disabledSensors"].as<JsonArray>();
    (*modes)[i].disabledSensors = new int[ds.size()];
    for (size_t ii = 0; ii < ds.size(); ii++) {
      (*modes)[i].disabledSensors[ii] = ds[ii];
    }
    (*modes)[i].disabledSensorsSize = ds.size();
  }
  delete buffer;
  return ret;
}

int parser::parseStructModeToJSONFile(char *filename, mode* modes, int lenght) {
  File f = SPIFFS.open(filename, "w");
  DynamicJsonBuffer jsonBuffer;
  JsonArray& array = jsonBuffer.createArray();
  for(int i=0;i<lenght; ++i) {
    JsonObject& m = jsonBuffer.createObject();
    m["id"] = modes[i].id;
    m["name"] = modes[i].modename;
    array.add(m);
  }
    array.printTo(f);
    return 0;
}
