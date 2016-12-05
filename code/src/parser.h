#include <Arduino.h>

class parser {
public:
  enum deviceType {REMOTE, RADIO_PIR, WIRED_PIR, WIRED_SIREN, RADIO_SIREN, WIRED_BUZZER, RADIO_BUZZER };
  struct mode {
    int id;
    String modename;
    bool autoStart;
    bool autoEnd;
    String startTime;
    String endTime;
    bool armsSystem;
    bool disarmsSystem;
    int preAlarmTime;
    int alarmTime;
    int maxAlarms;
    bool paused;
    bool active;
    int activedays;
    int disabledSensorsSize;
    int * disabledSensors;
    uint8_t alarmID_start;
    uint8_t alarmID_end;
  };
  struct deviceStruct {
    int id;
    const char* name;
    deviceType type;
    String address;
    bool isSensor;
    bool isDisabled;
    bool globallyDisabled;
    bool isAlarmed;
    const char* mqtt;
    unsigned long lastActive;
};
struct svg {
  const char* filename;
  const char* friendlyname;
};
struct svgRelation{
  const char* filename;
  const char* svg_id;
  int sensor_id;
};
int parseJSONFiletoStructDevices(String filename, deviceStruct** devices);
int parseStructDevicesToJSONFile(char *filename, deviceStruct* devices, int lenght);
int parseSVGFiletoStructSVG(char *filename, svg* svgs);
int parseStructSVGToJSONFile(char *filename, svg* svgs, int lenght);
int parseFiletoStructMode(char *filename, mode** modes);
int parseStructModeToJSONFile(char *filename, mode* modes, int lenght);
};
