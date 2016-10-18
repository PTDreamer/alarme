#include "indexHandler.h"
#include <ArduinoJson.h>

indexHandler::indexHandler(parser::deviceStruct *mdevices, int *mcurrentDeviceLenght) {
  devices = mdevices;
  currentDeviceLenght = mcurrentDeviceLenght;
}
bool indexHandler::canHandle(AsyncWebServerRequest *request) {
  if(request->url().equalsIgnoreCase("/data/statuschange.json")){
    if(request->method() == HTTP_POST){
      return true;
    }
  }
  else if(request->url().equalsIgnoreCase("/data/alarms.json")) {
    if(request->method() == HTTP_GET){
      return true;
    }
  }
  return false;
}
void indexHandler::handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  if(request->method() == HTTP_POST && request->url() == "/data/statuschange.json") {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject((const char*)data);
    if (!root.success())
    {
      AsyncResponseStream *response = request->beginResponseStream("application/json");
      response->print("{ \"status\": \"failure\"}");
      request->send(response);
      return;
    }
    if(root.containsKey("armed"))
      currentState.isArmed = root["armed"];
    if(root.containsKey("forced"))
      currentState.isForced = root["forced"];
    if(root.containsKey("newMode"))
      currentState.activeMode = root["newMode"];
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->print("{ \"status\": \"success\"}");
    request->send(response);
    return;
  }
}

void indexHandler::handleRequest(AsyncWebServerRequest *request) {
  if(request->url().equalsIgnoreCase("/data/alarms.json")) {
    String json = "{\"alarms\":[";
  for (size_t i = 0; i < *currentDeviceLenght; i++) {
    json += "{\"id\":" + String(devices[i].id) + ",\"state\":" + String(devices[i].isAlarmed) + "}";
    if(i != *currentDeviceLenght -1)
      json += ",";
  }
  json += "]";
  json += ",\"armed\":" + String(currentState.isArmed);
  json += ",\"activeMode\":" + String(currentState.activeMode);
  json += ",\"forcedMode\":" + String(currentState.isForced);
  json += "}";

  request->send(200, "text/json", json);
  json = String();
  }
}
void indexHandler::handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  Serial.println("indexHandler upload handler");
}
