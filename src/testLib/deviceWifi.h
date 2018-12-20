#ifndef __DEVICE_WIFI_H__
#define __DEVICE_WIFI_H__

#include "Arduino.h"
#include <WString.h>
#ifndef ESP32
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

#ifndef ESP32
#include <ESP8266WebServer.h>
#else
#include <WebServer.h>
#endif

#include "./helpers/jsonhelper.h"
#include "./helpers/fileHelper.h"

#ifndef ESP32
extern "C" {
  #include "user_interface.h"
}
#endif
#include <Crypto.h>


class ConfigWifi{
	private:
	bool i=0;
	bool g=0;
	bool s=0;
	public:

	ConfigWifi() {
		for (int x = 0; x < 4; x++) this->ip[x] = 0;
		for (int x = 0; x < 4; x++) this->gateway[x] = 0;
		for (int x = 0; x < 4; x++) this->subnet[x] = 0;
	}

	void setIP (uint8_t first_octet, uint8_t second_octet, uint8_t third_octet, uint8_t fourth_octet) {
		this->ip[0] = first_octet;
		this->ip[1] = second_octet;
		this->ip[2] = third_octet;
		this->ip[3] = fourth_octet;
		this->i=1;
	}

	void setGateway (uint8_t first_octet, uint8_t second_octet, uint8_t third_octet, uint8_t fourth_octet) {
		this->gateway[0] = first_octet;
		this->gateway[1] = second_octet;
		this->gateway[2] = third_octet;
		this->gateway[3] = fourth_octet;
		this->g=1;
	}

	void setSubnet (uint8_t first_octet, uint8_t second_octet, uint8_t third_octet, uint8_t fourth_octet) {
		this->subnet[0] = first_octet;
		this->subnet[1] = second_octet;
		this->subnet[2] = third_octet;
		this->subnet[3] = fourth_octet;
		this->s=1;
	}


	bool isConfigured() {
		return this->i && this->g && this->s;
	}

	IPAddress ip;   
	IPAddress gateway;   
	IPAddress subnet;  
};



class DeviceWifi{

    private:
		bool _encripted = false;
		char _chipId[32];
		char _base_name[6];
		struct wifi_credentials{
			char savedSSID[32];
			char savedPSK[64];
		};
		typedef struct wifi_credentials WifiCredentials;
		unsigned int _numWifiCredentials=1;

		bool _gotCredentials=0;

		bool _apConnected=0;

		char _encriptKeyWord[32];

		//Usefull variables to Decrypt Password
		//--------------------------------
		uint8_t _converted_iv[16];
		uint8_t _converted_key[16];
		uint8_t _converted_hex[64];
		int _plength=0;
		//--------------------------------
		//Usefull functions to Decrypt the Password
		//--------------------------------
		uint8_t *_convert_hex(char str[]);
		//--------------------------------
		uint8_t *_convert_key(char str[]);
		//--------------------------------
		uint8_t *_convert_iv(char str[]);

		String _getConnectMessage(int status_code);
		String _macToString(const unsigned char* mac);


        #ifdef ESP32
        static void _wiFiEventConnected(system_event_id_t event);
        static void _wiFiEventDisconnected(system_event_id_t event);
        #else
        static void _wiFiEvent(WiFiEvent_t event);
        #endif


		static void _wiFiApConnected();
		static void _wiFiApDisconnected();

        void _setDefaults();

        uint __wifiTimout=10000;

        String _wifiFile="/wifi.json";
        int _STATUS_LED=2;

		#ifndef ESP32
		ESP8266WebServer* _webServer;
		#else
		WebServer* _webServer;
		#endif

        DeviceWifi(char encriptKeyWord[32]);

		static DeviceWifi *_instance; 

    public:
        DeviceWifi();
		static DeviceWifi* getInstance() {
			if (!DeviceWifi::_instance) {
				DeviceWifi::_instance = new DeviceWifi();
			}
			return DeviceWifi::_instance;
		}
		
		void setEncriptKeyword(char encriptKeyWord[32]);

        ConfigWifi wifiConfig;
        WifiCredentials wifiCredentials[3];
		int getNumWifiCredentials();

		int connectionStatus();
		int connect(char *ssid, char *pass);
		int connect(char *ssid, char *pass, int retryies);
		bool saveWifiInFile(String wifiFile, char *ssid, char *psk);
		bool saveWifiInFile(String wifiFile, char *ssid, char *psk, unsigned int arrayIndex);
		bool getWifiFromFile(String wifiFile, char *ssid, char *psk);
        bool getWifiFromFile(String wifiFile, char *ssid, char *psk, unsigned int arrayIndex);
		bool connectToSavedWifi(unsigned int wifiNum);
		bool connectToSavedWifi();
		bool checkSignal(int powerLimit);
		void setupWiFiAp(char *apName);
		void charUrlDecode(char *dst, const char *src);
		String urlDecode(String source);
		static void getWifiCredentialsEncripted();
		static void getWifiCredentialsNotEncripted();
		void setWifiCredentialsNotEncripted(char SSID1[32],char PSK1[64]);
		void setWifiCredentialsNotEncripted(char SSID1[32],char PSK1[64],char SSID2[32],char PSK2[64]);
		void setWifiCredentialsNotEncripted(
			char SSID1[32],char PSK1[64],
			char SSID2[32],char PSK2[64], 
			char SSID3[32],char PSK3[64]);
		bool startApForWifiCredentials(char *apName, int timoutMilis);

		~DeviceWifi() { delete _webServer; }
};

#endif // __DEVICE_WIFI_H__