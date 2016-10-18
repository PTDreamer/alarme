#ifndef SETTINGS_HANDLER_H_
#define SETTINGS_HANDLER_H_
#include <ESPAsyncWebServer.h>

class settingsHandler: public AsyncWebHandler {

private:
  public:
    settingsHandler();
    bool canHandle(AsyncWebServerRequest *request);
    void handleRequest(AsyncWebServerRequest *request);
    void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
    void handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
};

#endif
