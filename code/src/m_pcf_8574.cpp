
#include "m_pcf_8574.h"


void m_pcf8574::setup() {
  TwoWire *testWire = new TwoWire;
  pcf8574_0 = new PCF857x(0x38, testWire);
  pcf8574_1 = new PCF857x(0x39, testWire);
  PCFInterruptFlag = false;
  testWire->begin(2,12);
  testWire->setClock(1000L);
  pcf8574_0->begin();
  pcf8574_1->begin();
  pinMode(5, INPUT_PULLUP);
  pcf8574_0->resetInterruptPin();
  pcf8574_1->resetInterruptPin();
  attachInterrupt(digitalPinToInterrupt(5), m_pcf8574::PCFInterrupt, FALLING);
}

void ICACHE_RAM_ATTR m_pcf8574::PCFInterrupt() {
  m_pcf8574::PCFInterruptFlag = true;
}

void m_pcf8574::handle() {
  if(m_pcf8574::PCFInterruptFlag) {
    currentValue = pcf8574_0->read8() << 8;
    currentValue = currentValue | pcf8574_1->read8();
    m_pcf8574::PCFInterruptFlag = false;
    m_valuesChanged = true;
  }
}
uint16_t m_pcf8574::read() {
  m_valuesChanged = false;
  return currentValue;
}

bool m_pcf8574::valuesChanged() {
  return m_valuesChanged;
}
bool m_pcf8574::PCFInterruptFlag = false;
