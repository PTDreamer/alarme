#include <pcf8574_esp.h>
#include <Wire.h>

class m_pcf8574 {
public:
  void setup();
  uint16_t read();
  bool valuesChanged();
  static bool PCFInterruptFlag;
  static void ICACHE_RAM_ATTR PCFInterrupt();
  void handle();
  PCF857x *pcf8574_0;
  PCF857x *pcf8574_1;
  bool m_valuesChanged;
  uint16_t currentValue;
};
