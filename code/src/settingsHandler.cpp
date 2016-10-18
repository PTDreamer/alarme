#include "settingsHandler.h"
#include <ArduinoJson.h>

settingsHandler::settingsHandler() {
}
bool settingsHandler::canHandle(AsyncWebServerRequest *request) {
  if(request->url().equalsIgnoreCase("/data/svg.json")){
    if(request->method() == HTTP_POST){
      return true;
    }
  }
  else if(request->url().equalsIgnoreCase("/data/devices.json")){
    if(request->method() == HTTP_POST){
      return true;
    }
  }
  else if(request->url().equalsIgnoreCase("/data/upload.php")){
    if(request->method() == HTTP_POST){
      return true;
    }
  }
}
void settingsHandler::handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  if(request->url().equalsIgnoreCase("/data/svg.json")) {
    DynamicJsonBuffer jsonBuffer;
    DynamicJsonBuffer jsonBufferSVGs;
    File svgFile = SPIFFS.open("/www/data/getsvgs.json", "r");
    if(!svgFile) {
      Serial.println("Could not open getsvgs.json for reading");
      AsyncResponseStream *response = request->beginResponseStream("application/json");
      response->print("{ \"status\": \"failure\"}");
      request->send(response);
      return;
    }
    size_t size = svgFile.size();
    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);
    svgFile.readBytes(buf.get(), size);
    JsonArray& rootSVG = jsonBufferSVGs.parseArray(buf.get());
    svgFile.close();
    JsonObject& root = jsonBuffer.parseObject((const char*)data);
    if (!root.success() || !rootSVG.success())
    {
      Serial.printf("XXXX -- - %d %d", root.success(), rootSVG.success());
      AsyncResponseStream *response = request->beginResponseStream("application/json");
      response->print("{ \"status\": \"failure\"}");
      request->send(response);
      return;
    }
    bool found = false;
    bool error = false;
    JsonArray& labels = root["labels"].asArray();
    for(size_t ii = 0; ii < labels.size(); ii++) {
      found = false;
      for (size_t i = 0; i < rootSVG.size(); i++) {
        if(rootSVG[i]["name"].as<String>() == labels[ii]["name"].as<String>()) {
          rootSVG[i].asObject().set("friendly", labels[ii]["label"]);
          Serial.printf("LABELS was looking for %s match. Found and label set to %s\n", labels[ii]["name"].as<const char*>(), rootSVG[i]["friendly"].as<const char*>());
          found = true;
        }
      }
      if(!found) {
        error = true;
        Serial.printf("LABELS was looking for %s match and didnt find any\n", labels[ii]["name"]);
      }
    }
    JsonArray& toDelete = root["toDelete"].asArray();
    for(size_t ii = 0; ii < toDelete.size(); ii++) {
      found = false;
      for (size_t i = 0; i < rootSVG.size(); i++) {
        if(rootSVG[i]["name"].as<String>() == toDelete[ii].as<String>()) {
          rootSVG.removeAt(i);
          found = true;
        }
      }
      if(!found)
        error = true;
    }
    if(!error) {
        svgFile = SPIFFS.open("/www/data/getsvgs.json", "w");
        rootSVG.printTo(svgFile);
        svgFile.close();
        AsyncResponseStream *response = request->beginResponseStream("application/json");
        response->print("{ \"status\": \"success\"}");
        request->send(response);
        return;
    }
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->print("{ \"status\": \"failure\"}");
    request->send(response);
    return;
  }
  else if(request->url().equalsIgnoreCase("/data/devices.json")) {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject((const char*)data);
    JsonArray& jdevices = root["devices"].asArray();
    for (size_t i = 0; i < jdevices.size(); i++) {
      JsonObject& jdevice = jdevices[i].asObject();
      jdevice["id"] = i;
    }
    File f = SPIFFS.open("/www/data/getdevices.json", "w");
    root.printTo(f);
    f.close();
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->print("{ \"status\": \"success\"}");
    request->send(response);
  }
}
void settingsHandler::handleRequest(AsyncWebServerRequest *request) {
}
void settingsHandler::handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  static String finalFile;
  if(!index) {
    filename = "/www/images/" + filename;
    int tooMuch = filename.length() - 30;
    if(tooMuch > 0) {
      int dot = filename.lastIndexOf('.');
      String ext = filename.substring(dot);
      filename.remove(tooMuch + ext.length());
      filename = filename + ext;
      Serial.printf("WARNING FILENAME CHANGED TO:%s\n", filename.c_str());
    }
    finalFile = filename.substring(12);
    Serial.printf("filename:%s finalFile:%s", filename.c_str(), finalFile.c_str());
    Serial.printf("UploadStart: %s\n", filename.c_str());
    request->_tempFile = SPIFFS.open(filename, "w");
  }
  if(len) {
    request->_tempFile.write(data, len);
  }
//  Serial.printf("%s", (const char*)data);
  if(final) {
    request->_tempFile.close();
    Serial.printf("UploadEnd: %s (%u)\n", filename.c_str(), index+len);
    request->send(200);
    Serial.printf("finalFile:%s", finalFile.c_str());
    DynamicJsonBuffer jsonBufferSVGs;
    File svgFile = SPIFFS.open("/www/data/getsvgs.json", "r");
    if(!svgFile) {
      Serial.println("Could not open getsvgs.json for reading");
      return;
    }
    size_t size = svgFile.size();
    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);
    svgFile.readBytes(buf.get(), size);
    svgFile.close();
    JsonArray& rootSVG = jsonBufferSVGs.parseArray(buf.get());
    rootSVG.prettyPrintTo(Serial);
    bool found = false;
    for (size_t i = 0; i < rootSVG.size(); i++) {
      if(rootSVG[i]["name"].as<String>() == finalFile) {
        Serial.printf("SVG file:%s already present not adding to getsvgs.json\n", finalFile.c_str());
        found = true;
        break;
      }
    }
    if(!found ) {
      JsonObject& x = rootSVG.createNestedObject();
      x["name"].set(finalFile.c_str());
      x["friendly"] = "";
      Serial.printf("SVG file:%s not present adding it to getsvgs.json\n", finalFile.c_str());
    //  rootSVG.add(x);
      svgFile = SPIFFS.open("/www/data/getsvgs.json", "w");
      rootSVG.printTo(svgFile);
      svgFile.close();
      rootSVG.prettyPrintTo(Serial);
    }
    svgFile.close();
  }
}
