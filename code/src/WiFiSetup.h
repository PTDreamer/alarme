#ifndef WiFiSetup_H
#define WiFiSetup_H

class WiFiSetup
{
public:
	static bool begin(void){WiFiSetup::begin("SistemaRega");}
	static bool begin(char const *ssid,const char *passwd=NULL);
};

#endif
