#ifndef __KONKER_DEVICE_H__
#define __KONKER_DEVICE_H__

#include "Arduino.h"

#include "./konkerSettings.h"
#include "./deviceWifi.h"
#include "./helpers/subChanTuple.h"
#ifndef ESP32
#include <ESP8266HTTPClient.h>
#else
#include <HTTPClient.h>
#endif

//mqtt
#include <PubSubClient.h>

#define BAUDRATE 115200

class KonkerDevice{
  private:
    bool _encripted = false;
	char _chipId[32];
	char _base_name[6];
	void _setDefaults();
    char _healthFile[13];//="/health.json";
	char _configFile[13];//="/config.json";
	char _wifiFile[1];//="/wifi.json";
    int _netFailureAdress=0;
    int _mqttFailureAdress=1;
    int _httpFailureAdress=2;
	char _device_login[32];
	char _device_pass[32];
	#ifdef DEBUG
	bool _debug=1;
	#else
	bool _debug=0;
	#endif
	unsigned long _last_time_http_request=0;
	unsigned long _millis_delay_per_http_request=1000;
	bool _interpretHTTPCode(int httpCode);

	static KonkerDevice* _instance;

	//mqtt
	#ifdef konkerMQTTs
	WiFiClientSecure _espClient;
	const char* _fingerprint = "";
	bool _checkSecureFingerprint(const char* fingerprint, const char* secureServer);
	bool _checkSecureFingerprint(const char* fingerprint);
	#else
	WiFiClient _espClient;
	#endif
	PubSubClient _client;

	
	static void _callback(char* topic, byte* payload, unsigned int length);

	SubChanTuple _subChanTuple;
	bool _connectMQTT(char r_server[], int r_port, char r_device_login[], char r_device_pass[]);
	bool _connectMQTT();

	KonkerDevice(String newName);
	KonkerDevice(String newName, bool isEncripted);
    KonkerDevice(String newName, String httpDomain, String httpPubTopicFormat, String httpSubTopicFormat,
                 String mqttAdress, String mqttPubTopicFormat, String mqttSubTopicFormat, bool isEncripted);

	void removeChar(char *str, char garbage);

  public:

	static KonkerDevice* getInstance(String newName) { 
		if (!KonkerDevice::_instance) {
			KonkerDevice::_instance = new KonkerDevice(newName);
		} 
		return KonkerDevice::_instance;
	}
	
	static KonkerDevice* getInstance(String newName, bool isEncripted) {
		if (!KonkerDevice::_instance) {
			KonkerDevice::_instance = new KonkerDevice(newName, isEncripted);
		}
		return KonkerDevice::_instance;
	}

    static KonkerDevice* getInstance(String newName, String httpDomain, String httpPubTopicFormat, String httpSubTopicFormat,
                 String mqttAdress, String mqttPubTopicFormat, String mqttSubTopicFormat, bool isEncripted) {
		if (!KonkerDevice::_instance) {
			KonkerDevice::_instance = new KonkerDevice(newName, httpDomain, httpPubTopicFormat, httpSubTopicFormat,
				mqttAdress, mqttPubTopicFormat, mqttSubTopicFormat, isEncripted
			);
		}
		return KonkerDevice::_instance;
	}


	KonkerSettings konker;
	DeviceWifi& deviceWifi = *DeviceWifi::getInstance();

	#ifndef ESP32
	int resetPin=D5;
	#else
	int resetPin=13;
	#endif
	int statusLedPin=2;
	char name[32];

	void setup();
	void mqttLoop();

	char *getChipId();

	void clearDeviceConfig();	

	bool connectToWiFiAndKonker();

	bool checkConnections();

    bool pubHttp(char const channel[], char const msg[], int& ret_code);
    bool pubHttp(char const channel[], char const msg[]);
    bool subHttp(char const channel[],CHANNEL_CALLBACK_SIGNATURE);
    bool testHTTPSubscribeConn();
	bool pubMQTT(char const channel[], char const msg[]);
	bool subMQTT(char const channel[],CHANNEL_CALLBACK_SIGNATURE);
    bool checkForFactoryWifi(char *ssidConfig, char *ssidPassConfig, int powerLimit, int connectTimeout);
    bool setPlataformCredentials(char *configFilePath);
    bool getPlataformCredentialsFromGateway();
	bool setPlataformCredentials(char *mqtt, char *http, char *user, char *password);

};


#endif // __KONKER_DEVICE_H__