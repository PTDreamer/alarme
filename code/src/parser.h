#include <Arduino.h>

class parser {
public:
  enum deviceType {REMOTE, RADIO_PIR, WIRED_PIR, WIRED_SIREN, RADIO_SIREN, WIRED_BUZZER, RADIO_BUZZER };
  struct mode {
    int id;
    const char* modename;
  };
  struct deviceStruct {
    int id;
    const char* name;
    deviceType type;
    const char* address;
    bool isSensor;
    bool isDisabled;
    bool isAlarmed;
    const char* mqtt;
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
int parseJSONFiletoStructDevices(String filename, deviceStruct* devices, int & alarmTime, int & preAlarmTime);
int parseStructDevicesToJSONFile(char *filename, deviceStruct* devices, int alarmTime, int preAlarmTime, int lenght);
int parseSVGFiletoStructSVG(char *filename, svg* svgs);
int parseStructSVGToJSONFile(char *filename, svg* svgs, int lenght);
int parseSVGFiletoStructMode(char *filename, mode* modes);
int parseStructModeToJSONFile(char *filename, mode* modes, int lenght);
};
