#include "modesHandler.h"
#include <ArduinoJson.h>

modesHandler::modesHandler() {
}
bool modesHandler::canHandle(AsyncWebServerRequest *request) {
  if(request->url().equalsIgnoreCase("/data/pausemode.php")){
    if(request->method() == HTTP_POST){
      return true;
    }
  }
  else if(request->url().equalsIgnoreCase("/data/savemode.php")){
    if(request->method() == HTTP_POST){
      return true;
    }
  }
  else if(request->url().equalsIgnoreCase("/data/delete.php")){
    if(request->method() == HTTP_POST){
      return true;
    }
  }
}
void modesHandler::handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  Serial.printf("On modesHandler handleBody for url:%s\n", request->url().c_str());
  if(request->method() == HTTP_POST && request->url() == "/data/savemode.php") {
    DynamicJsonBuffer jsonBuffer;
    DynamicJsonBuffer jsonBufferModes;
    File modesFile = SPIFFS.open("/www/data/getmode.json", "r");
    if(!modesFile) {
      Serial.println("Could not open modes.json for reading");
      AsyncResponseStream *response = request->beginResponseStream("application/json");
      response->print("{ \"status\": \"failure\"}");
      request->send(response);
      return;
    }
    size_t size = modesFile.size();
    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);
    modesFile.readBytes(buf.get(), size);
    JsonArray& rootModes = jsonBufferModes.parseArray(buf.get());
    modesFile.close();
    JsonObject& root = jsonBuffer.parseObject((const char*)data);
    if (!root.success() || !rootModes.success())
    {
      Serial.printf("XXXX -- - %d %d", root.success(), rootModes.success());
      AsyncResponseStream *response = request->beginResponseStream("application/json");
      response->print("{ \"status\": \"failure\"}");
      request->send(response);
      return;
    }
    if(strcmp(root["id"],"new") == 0) {
      Serial.println("Saving new mode. Looking for available ID");
      int newID = -1;
      bool present;
      for (size_t i = 0; i < rootModes.size(); i++) {
        present = false;
        for (size_t ii = 0; ii < rootModes.size(); ii++) {
          if(rootModes[ii]["id"] == i) {
            present = true;
            break;
          }
        }
        if(present == false) {
          newID = i;
          break;
        }
      }
      if(newID == -1)
        newID = rootModes.size();
      Serial.printf("New ID will be %d", newID);
      root["id"] = newID;
      rootModes.add(root);
      modesFile = SPIFFS.open("/www/data/getmode.json", "w");
      rootModes.printTo(modesFile);
      modesFile.close();
      AsyncResponseStream *response = request->beginResponseStream("application/json");
      response->print("{ \"status\": \"success\", \"newID\" :" + String(newID) + "}");
      request->send(response);
      return;
    }
    else {
      bool found = false;
      for (size_t i = 0; i < rootModes.size(); i++) {
          if(rootModes[i]["id"].as<int>() == root["id"].as<int>()) {
            rootModes.removeAt(i);
            rootModes.add(root);
            modesFile = SPIFFS.open("/www/data/getmode.json", "w");
            rootModes.printTo(modesFile);
            modesFile.close();
            Serial.printf("OverWriting mode %d", i);
            found = true;
            AsyncResponseStream *response = request->beginResponseStream("application/json");
            response->print("{ \"status\": \"success\"}");
            request->send(response);
            return;
            break;
          }
      }
      if(!found) {
        AsyncResponseStream *response = request->beginResponseStream("application/json");
        response->print("{ \"status\": \"failure\"}");
        request->send(response);
        return;
      }
    }
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->print("{ \"status\": \"failure\"}");
    request->send(response);
    return;
  }
  else {
    DynamicJsonBuffer jsonBuffer;
    DynamicJsonBuffer jsonBufferModes;
    File modesFile = SPIFFS.open("/www/data/getmode.json", "r");
    if(!modesFile) {
      Serial.println("Could not open modes.json for reading");
      AsyncResponseStream *response = request->beginResponseStream("application/json");
      response->print("{ \"status\": \"failure\"}");
      request->send(response);
      return;
    }
    size_t size = modesFile.size();
    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);
    modesFile.readBytes(buf.get(), size);
    JsonArray& rootModes = jsonBufferModes.parseArray(buf.get());
    modesFile.close();
    JsonObject& root = jsonBuffer.parseObject((const char*)data);
    if (!root.success() || !rootModes.success())
    {
      Serial.printf("XXXX -- - %d %d", root.success(), rootModes.success());
      AsyncResponseStream *response = request->beginResponseStream("application/json");
      response->print("{ \"status\": \"failure\"}");
      request->send(response);
      return;
    }
    if(request->url() == "/data/delete.php") {
      bool found = false;
      for (size_t i = 0; i < rootModes.size(); i++) {
          if(rootModes[i]["id"].as<int>() == root["id"].as<int>()) {
            rootModes.removeAt(i);
            modesFile = SPIFFS.open("/www/data/getmode.json", "w");
            rootModes.printTo(modesFile);
            modesFile.close();
            Serial.printf("Deleting mode %d", i);
            found = true;
            AsyncResponseStream *response = request->beginResponseStream("application/json");
            response->print("{ \"status\": \"success\"}");
            request->send(response);
            return;
            break;
          }
      }
    }
    else if(request->url() == "/data/pausemode.php") {
      bool found = false;
      for (size_t i = 0; i < rootModes.size(); i++) {
          if(rootModes[i]["id"].as<int>() == root["id"].as<int>()) {
            bool temp = root["pause"].as<bool>();
            rootModes[i].asObject()["paused"] = temp;
            modesFile = SPIFFS.open("/www/data/getmode.json", "w");
            rootModes.printTo(modesFile);
            modesFile.close();
            Serial.printf("Pausing mode %d", i);
            found = true;
            AsyncResponseStream *response = request->beginResponseStream("application/json");
            response->print("{ \"status\": \"success\"}");
            request->send(response);
            return;
            break;
          }
      }
    }
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->print("{ \"status\": \"failure\"}");
    request->send(response);
    return;
  }
}
void modesHandler::handleRequest(AsyncWebServerRequest *request) {
}
void modesHandler::handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
}
