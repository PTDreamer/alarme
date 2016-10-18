#ifndef MODES_HANDLER_H_
#define MODES_HANDLER_H_
#include <ESPAsyncWebServer.h>

class modesHandler: public AsyncWebHandler {

private:
  public:
    modesHandler();
    bool canHandle(AsyncWebServerRequest *request);
    void handleRequest(AsyncWebServerRequest *request);
    void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
    void handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
};

#endif
