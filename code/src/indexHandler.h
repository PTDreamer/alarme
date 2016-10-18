#ifndef INDEX_HANDLER_H_
#define INDEX_HANDLER_H_
#include <ESPAsyncWebServer.h>
#include "parser.h"

class indexHandler: public AsyncWebHandler {
  struct state {
    bool isArmed;
    bool isForced;
    int activeMode;
  };
private:
  parser::deviceStruct *devices;
  int *currentDeviceLenght;
  state currentState;
  public:
    void setState(state newState) {currentState = newState; }
    state getState() { return currentState;}
    indexHandler(parser::deviceStruct *mdevices, int *mcurrentDeviceLenght);
    bool canHandle(AsyncWebServerRequest *request);
    void handleRequest(AsyncWebServerRequest *request);
    void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
    void handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
};

#endif
