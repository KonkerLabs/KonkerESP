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
        void _wiFiEventConnected(system_event_id_t event);
        void _wiFiEventDisconnected(system_event_id_t event);
        #else
        void _wiFiEvent(WiFiEvent_t event);
        #endif


		void _wiFiApConnected();
		void _wiFiApDisconnected();

        void _setDefaults();

        uint __wifiTimout=10000;

        String _wifiFile="/wifi.json";
        int _STATUS_LED=2;

		#ifndef ESP32
		ESP8266WebServer* _webServer;
		#else
		WebServer* _webServer;
		#endif

    public:
        DeviceWifi();
        DeviceWifi(char encriptKeyWord[32]);
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
		void getWifiCredentialsEncripted();
		void getWifiCredentialsNotEncripted();
		void setWifiCredentialsNotEncripted(char SSID1[32],char PSK1[64]);
		void setWifiCredentialsNotEncripted(char SSID1[32],char PSK1[64],char SSID2[32],char PSK2[64]);
		void setWifiCredentialsNotEncripted(
			char SSID1[32],char PSK1[64],
			char SSID2[32],char PSK2[64], 
			char SSID3[32],char PSK3[64]);
		bool startApForWifiCredentials(char *apName, int timoutMilis);

		~DeviceWifi() { delete _webServer; }
};

DeviceWifi::DeviceWifi(){
    _setDefaults();
}

DeviceWifi::DeviceWifi(char encriptKeyWord[32]){
    _setDefaults();
	strncpy(_encriptKeyWord, encriptKeyWord, 32);
	_encripted = true;
}


///PRIVATE
void DeviceWifi::_setDefaults(){
    strncpy(_base_name,{"S0000"},6);
	#ifndef ESP32
	_webServer = new ESP8266WebServer(80);
	#else
	_webServer = new WebServer(int port=80);
	#endif
}


String DeviceWifi::_getConnectMessage(int status_code){
	switch(status_code) {
		case 255: return "WL_NO_SHIELD"; break;
		case 0: return "WL_IDLE_STATUS"; break; 
		case 1: return "WL_NO_SSID_AVAIL"; break; 
		case 2: return "WL_SCAN_COMPLETED"; break; 
		case 3: return "WL_CONNECTED"; break; 
		case 4: return "WL_CONNECT_FAILED"; break; 
		case 5: return "WL_CONNECTION_LOST"; break;
		case 6: return "WL_DISCONNECTED"; break;
	}
	return "UNDEFINED";
}

String DeviceWifi::_macToString(const unsigned char* mac) {
  char buf[20];
  snprintf(buf, sizeof(buf), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(buf);
}

uint8_t *DeviceWifi::_convert_hex(char str[]){
	char substr[5]="0x00";
	int j = 0;
	int psize = (strlen(str));
	_plength = int(psize/2);
	for (byte i = 0; i < psize; i=i+2) {
		substr[2] = str[i];
		substr[3] = str[i+1];
		_converted_hex[j] = strtol(substr, NULL, _plength);
		j++;
	}
	return _converted_hex;
}
//--------------------------------
uint8_t *DeviceWifi::_convert_key(char str[]){
	int j=0;
	for (int i = 0; i < 16; i++ ) {
		if (str[j]=='\0') j=0;
		_converted_key[i]=int(str[j]);
		j++;
	}
	return _converted_key;
}
//--------------------------------
uint8_t *DeviceWifi::_convert_iv(char str[]){
	for (int i = 0; i < 16; i++ ) {
		_converted_iv[i]=int(str[i]);
	}
	return _converted_iv;
}


void DeviceWifi::_wiFiApConnected() {
	Serial.println("Conectado");
	_apConnected=1;
}

void DeviceWifi::_wiFiApDisconnected() {
	Serial.println("Desconectou");
	_apConnected=0;
}


#ifdef ESP32
void DeviceWifi::_wiFiEventConnected(system_event_id_t event) {
	// SYSTEM_EVENT_AP_STACONNECTED:
	_wiFiApConnected();
}
void DeviceWifi::_wiFiEventDisconnected(system_event_id_t event) {
	// SYSTEM_EVENT_AP_STADISCONNECTED:
	_wiFiApDisconnected();
}
#else
void DeviceWifi::_wiFiEvent(WiFiEvent_t event) {	
	switch(event){
	case WIFI_EVENT_SOFTAPMODE_STACONNECTED:
		_wiFiApConnected();
	break;
	case WIFI_EVENT_SOFTAPMODE_STADISCONNECTED:
		_wiFiApDisconnected();
	default:
	break;
	}
}
#endif




///PUBLIC

int DeviceWifi::getNumWifiCredentials(){
	return _numWifiCredentials;
}

int DeviceWifi::connectionStatus(){
	uint wifiStartTime=millis();
	while(WiFi.status() != WL_CONNECTED && (millis()-wifiStartTime)<__wifiTimout) {
			delay(500);
			Serial.print("."); 
	}

	int connRes = WiFi.status();
	Serial.println("\nconnRes=" + (String)connRes + " " +  _getConnectMessage(connRes));
	return connRes;
}

//garantee a disconection before trying to connect
int DeviceWifi::connect(char *ssid, char *pass) {
	WiFi.mode(WIFI_OFF);
	delay(100);
	WiFi.mode(WIFI_STA);
	delay(100);
	//check if we have ssid and pass and force those, if not, try with last saved values
	Serial.println("WiFi.begin("+(String)ssid+", "+(String)pass+")");
	WiFi.begin(ssid, pass);

	if (wifiConfig.isConfigured() && strcmp(WiFi.SSID().c_str(),(char*)"KonkerDevNetwork")!=0) {
		Serial.print("Pre configured IP :");
		Serial.println(wifiConfig.ip);
		Serial.print("Pre configured GATEWWAY: ");
		Serial.println(wifiConfig.gateway);
		Serial.print("Pre configured SUBNET: ");
		Serial.println(wifiConfig.subnet);
		
		WiFi.config(wifiConfig.ip,wifiConfig.gateway, wifiConfig.subnet);
	}

	return connectionStatus();
}

int DeviceWifi::connect(char *ssid, char *pass, int retryies) {
	int connRes=connect(ssid,pass);
	while (connRes!=WL_CONNECTED && retryies>1){
		delay(1000);
		connRes=connect(ssid,pass);
		retryies=retryies-1;
	}
	return connRes;
}

bool DeviceWifi::saveWifiInFile(String wifiFile, char *ssid, char *psk){

  String json="{\"s\":\"" + (String)ssid + "\",\"p\":\"" + (String)psk + "\"}";
	Serial.print("saving json = ");
	Serial.println(json);
  char charJson[112];
  json.toCharArray(charJson, 112);
  return updateJsonFile(wifiFile,charJson);
}

bool DeviceWifi::saveWifiInFile(String wifiFile, char *ssid, char *psk, unsigned int arrayIndex){

	String json="{\"s\":\"" + (String)ssid + "\",\"p\":\"" + (String)psk + "\"}";

	Serial.print("saving json = ");
	Serial.print(json);
	Serial.print(" @ pos = ");
	Serial.println(arrayIndex);

	char charJson[112];
	json.toCharArray(charJson, 112);
	return updateJsonFile(wifiFile, charJson, arrayIndex);
}

bool DeviceWifi::getWifiFromFile(String wifiFile, char *ssid, char *psk){
   if(getJsonItemFromFile(wifiFile,(char*)"s",ssid) &&
   getJsonItemFromFile(wifiFile,(char*)"p",psk)){
		 if (ssid[0] == '\0'){
			 return 0;
		 }else{
			 return 1;
		 }
	 }else{
		 return 0;
	 }
}

bool DeviceWifi::getWifiFromFile(String wifiFile, char *ssid, char *psk, unsigned int arrayIndex){
   if(getJsonArrayItemFromFile(wifiFile,arrayIndex,(char*)"s",ssid) &&
   getJsonArrayItemFromFile(wifiFile,arrayIndex,(char*)"p",psk)){
		 if (ssid[0] == '\0'){
			 Serial.println("ssid is empty!");
			 return 0;
		 }else{
			 return 1;
		 }
	 }else{
		 return 0;
	 }
}

bool DeviceWifi::connectToSavedWifi(unsigned int wifiNum){
	char fileSavedSSID[32]={'\0'};
	char fileSavedPSK[64]={'\0'};


	if(!getWifiFromFile(_wifiFile,fileSavedSSID,fileSavedPSK, wifiNum) && wifiCredentials[wifiNum].savedSSID[0]=='\0'){
		return 0;
	}

	if(fileSavedSSID[0]=='\0' && wifiCredentials[wifiNum].savedSSID[0]=='\0'){
		Serial.println("No credentials for wifiNum = " + String(wifiNum));
		return 0;
	}


	Serial.println("fileSavedSSID=" + (String)fileSavedSSID);
	Serial.println("fileSavePSK=" + (String)fileSavedPSK);


  //check if we have ssid and pass and force those, if not, try with last saved values
	int connRes;
	if(strcmp(fileSavedSSID,"")!=0){
		if (fileSavedSSID[0]!='\0'){
			Serial.println("Trying to connect to saved WiFi  (will try 2 times):" +  (String)fileSavedSSID);
			connRes=connect(fileSavedSSID,fileSavedPSK,2);
		}else{
			Serial.println("Trying to connect to WiFi (will try 2 times):" +  (String)wifiCredentials[wifiNum].savedSSID);
			connRes=connect(wifiCredentials[wifiNum].savedSSID,wifiCredentials[wifiNum].savedPSK,2);
		}

	}else{
		if(wifiCredentials[wifiNum].savedSSID[0]!='\0'){
			Serial.println("Trying to connect to WiFi (will try 2 times):" +  (String)wifiCredentials[wifiNum].savedSSID);
			connRes=connect(wifiCredentials[wifiNum].savedSSID,wifiCredentials[wifiNum].savedPSK,2);
		}else{
			Serial.println("No WiFi saved, ignoring...");
			return 0;
		}
	}

	if(connRes==3){
		Serial.println("Sucess!!");
        digitalWrite(_STATUS_LED, HIGH);
		return 1;
	}else{
		Serial.println("Failed!");
		return 0;
	}
}

bool DeviceWifi::connectToSavedWifi(){
	for(int i=0;i<3;i++){
		if(connectToSavedWifi(i)){
			return 1;
		}
	}
	return 0;
}

//Return 1 if specific signal strength is mesured, 0 if else
bool DeviceWifi::checkSignal(int powerLimit){
	int32_t rssi =WiFi.RSSI();
	Serial.print("Signal strength: ");
	Serial.print(rssi);
	Serial.println("dBm");
	if(rssi<-powerLimit){
		Serial.println("Config device not near enought!");
		Serial.println("Disconnecting!");
		WiFi.mode(WIFI_OFF);
		delay(100);
		Serial.println("checkSignal=0");
		return 0;
	}else{
		delay(100);
		Serial.println("checkSignal=1");
		return 1;
	}
}



void DeviceWifi::setupWiFiAp(char *apName){

	WiFi.mode(WIFI_AP);

	// Append the last two bytes of the MAC (HEX'd) to string to make unique
	uint8_t mac[6];
	WiFi.softAPmacAddress(mac);

	//TO-DO
	//#ifndef ESP32
	//WiFi.onEvent(_wiFiEvent,WIFI_EVENT_ANY);
	//#else
	//WiFi.onEvent(_wiFiEventConnected,SYSTEM_EVENT_AP_STACONNECTED);
	//WiFi.onEvent(_wiFiEventDisconnected,SYSTEM_EVENT_AP_STADISCONNECTED);
	//#endif



	const IPAddress gateway_IP(192, 168, 4, 1); // gateway IP
	const IPAddress subnet_IP( 255, 255, 255, 0); // standard subnet
	WiFi.config(gateway_IP, gateway_IP, subnet_IP); // we use a static IP address
	WiFi.softAP(apName);
}


void DeviceWifi::charUrlDecode(char *dst, const char *src){  
  char a, b;
  while (*src) {
          if ((*src == '%') &&
              ((a = src[1]) && (b = src[2])) &&
              (isxdigit(a) && isxdigit(b))) {
                  if (a >= 'a')
                          a -= 'a'-'A';
                  if (a >= 'A')
                          a -= ('A' - 10);
                  else
                          a -= '0';
                  if (b >= 'a')
                          b -= 'a'-'A';
                  if (b >= 'A')
                          b -= ('A' - 10);
                  else
                          b -= '0';
                  *dst++ = 16*a+b;
                  src+=3;
          } else if (*src == '+') {
                  *dst++ = ' ';
                  src++;
          } else {
                  *dst++ = *src++;
          }
  }
  *dst++ = '\0';
}

String DeviceWifi::urlDecode(String source){
  char chrSource[1024];
  char decodedChrs[1024];
  source.toCharArray(chrSource, 1024);

  charUrlDecode(decodedChrs,chrSource);
  return (String)decodedChrs;
}



//while connected to ESP wifi make a GET to http://192.168.4.1/wifisave?s=SSID_NAME&p=ENCRIPTED_SSID_PASSWORD
void DeviceWifi::getWifiCredentialsEncripted(){
	Serial.println("Handle getWifiCredentials");
	String page = "<http><body><b>getWifiCredentials</b></body></http>";


	//get up to 3 wifi credentials
	for(int i=0;i<3;i++){
		String argSSID = urlDecode(_webServer->arg("s" + String(i)));
		String argPSK = urlDecode(_webServer->arg("p" + String(i)));

		Serial.println("argSSID" + String(i) + "=" + argSSID);
		Serial.println("argPSK" + String(i) + "=" + argPSK);

		if(argSSID!="" && argPSK!=""){
			argSSID.toCharArray(wifiCredentials[i].savedSSID, 32);
			//argPSK.toCharArray(savedPSK, 64);
			_numWifiCredentials++;
			_gotCredentials=1;
			//Decrypt Password
			char pass[argPSK.length()+1];
			argPSK.toCharArray(pass, argPSK.length()+1);
			char mySSID[argSSID.length()+1];
			argSSID.toCharArray(mySSID, argPSK.length()+1);
			char iv1[17] = "AnE9cKLPxGwyPPVU";
			char iv2[17] = "sK33DE5TaC9nRUSt";
			uint8_t var3[64];
			uint8_t var4[64];
			uint8_t *convKeyLogin=_convert_key(_encriptKeyWord);
			uint8_t *convKeySSID=_convert_key(mySSID);

			Serial.print("_convert_key(_encriptKeyWord): " + String(_encriptKeyWord) + " ");
			for (int j = 0; j < 16; j++) {
					Serial.print(convKeyLogin[j]);
			}
			Serial.println("");

			Serial.print("_convert_key(mySSID): "+ String(mySSID)+ " ");
			for (int j = 0; j < 16; j++) {
					Serial.print(convKeySSID[j]);
			}
			Serial.println("");

			AES deck1(_convert_key(_encriptKeyWord), _convert_iv(iv2), AES::AES_MODE_128, AES::CIPHER_DECRYPT);
			AES deck2(_convert_key(mySSID), _convert_iv(iv1), AES::AES_MODE_128, AES::CIPHER_DECRYPT);
			deck1.process(_convert_hex(pass), var3, _plength);
			deck2.process(var3, var4, _plength);
			for (int j = 0; j < _plength; j++) {
					wifiCredentials[i].savedPSK[j]=var4[j];
			}
			for (int j = 1; j <= _plength; j++) {
					if (wifiCredentials[i].savedPSK[_plength-j]==' ' || wifiCredentials[i].savedPSK[_plength-j] == '\0'){
						wifiCredentials[i].savedPSK[_plength-j]='\0';
					}else {
						Serial.printf("char @ position %d = %c", j, wifiCredentials[i].savedPSK[_plength-j]);
						Serial.printf("psk%d='%s'", i, wifiCredentials[i].savedPSK);
						break;
					}
			}

		}

	}

	if(_gotCredentials){
		// reset wifi credentials from file
		//Removing the Wifi Configuration
		SPIFFS.remove(_wifiFile);
	}

	
	_webServer->send(200, "text/html", page);



}

//while connected to ESP wifi make a GET to http://192.168.4.1/wifisave?s=SSID_NAME&p=SSID_PASSWORD
void DeviceWifi::getWifiCredentialsNotEncripted(){
	Serial.println("Handle getWifiCredentials");
	String page = "<http><body><b>getWifiCredentials</b></body></http>";

	//get up to 3 wifi credentials
	for(int i=0;i<3;i++){
		String argSSID = urlDecode(_webServer->arg("s" + String(i)));
		String argPSK = urlDecode(_webServer->arg("p" + String(i)));

		Serial.println("argSSID" + String(i) + "=" + argSSID);
		Serial.println("argPSK" + String(i) + "=" + argPSK);


		if(argSSID!="" && argPSK!=""){
			argSSID.toCharArray(wifiCredentials[i].savedSSID, 32);
			argPSK.toCharArray(wifiCredentials[i].savedPSK, 64);
			_numWifiCredentials++;
			_gotCredentials=1;
		}
	}

	if(_gotCredentials){
		// reset wifi credentials from file
		//Removing the Wifi Configuration
		SPIFFS.remove(_wifiFile);
	}

	_webServer->send(200, "text/html", page);
}


void DeviceWifi::setWifiCredentialsNotEncripted(char SSID1[32],char PSK1[64]){
	// reset wifi credentials from file
	//Removing the Wifi Configuration
	SPIFFS.remove(_wifiFile);
		//cria file system se não existir
	spiffsMount();
	Serial.println("Handle getWifiCredentials");
	String page = "<http><body><b>getWifiCredentials</b></body></http>";


	//get up to 3 wifi credentials
	if(SSID1[0]!='\0') { //} && PSK1[0]!='\0'){
		Serial.println("Wifi 1");
		strncpy(wifiCredentials[0].savedSSID,SSID1,32);
		strncpy(wifiCredentials[0].savedPSK,PSK1,64);

		_numWifiCredentials++;
		_gotCredentials=1;

		saveWifiInFile(_wifiFile, wifiCredentials[0].savedSSID, wifiCredentials[0].savedPSK,0);
	}

	Serial.println("wifi saved");
}


void DeviceWifi::setWifiCredentialsNotEncripted(char SSID1[32],char PSK1[64],char SSID2[32],char PSK2[64]){
	// reset wifi credentials from file
	//Removing the Wifi Configuration
	SPIFFS.remove(_wifiFile);
		//cria file system se não existir
	spiffsMount();
	Serial.println("Handle getWifiCredentials");
	String page = "<http><body><b>getWifiCredentials</b></body></http>";


	//get up to 3 wifi credentials
	if(SSID1[0]!='\0') { //} && PSK1[0]!='\0'){
		Serial.println("Wifi 1");
		strncpy(wifiCredentials[0].savedSSID,SSID1,32);
		strncpy(wifiCredentials[0].savedPSK,PSK1,64);

		_numWifiCredentials++;
		_gotCredentials=1;

		saveWifiInFile(_wifiFile, wifiCredentials[0].savedSSID, wifiCredentials[0].savedPSK,0);
	}
	Serial.println("..");
	if(SSID2[0]!='\0') { //} && PSK2[0]!='\0'){
		Serial.println("Wifi 2");
		strncpy(wifiCredentials[1].savedSSID,SSID2,32);
		strncpy(wifiCredentials[1].savedPSK,PSK2,64);

		_numWifiCredentials++;
		_gotCredentials=1;

		saveWifiInFile(_wifiFile, wifiCredentials[1].savedSSID, wifiCredentials[1].savedPSK,1);
	}

	Serial.println("wifi saved");
}


void DeviceWifi::setWifiCredentialsNotEncripted(
	char SSID1[32],char PSK1[64],
	char SSID2[32],char PSK2[64], 
	char SSID3[32],char PSK3[64]){

	// reset wifi credentials from file
	//Removing the Wifi Configuration
	SPIFFS.remove(_wifiFile);
		//cria file system se não existir
	spiffsMount();
	Serial.println("Handle getWifiCredentials");
	String page = "<http><body><b>getWifiCredentials</b></body></http>";


	Serial.print(".. #1 = ");
	Serial.println(SSID1);
	//get up to 3 wifi credentials
	if(SSID1[0]!='\0') { // } && PSK1[0]!='\0'){
		Serial.println("Wifi 1");
		strncpy(wifiCredentials[0].savedSSID,SSID1,32);
		strncpy(wifiCredentials[0].savedPSK,PSK1,64);

		_numWifiCredentials++;
		_gotCredentials=1;

		saveWifiInFile(_wifiFile, wifiCredentials[0].savedSSID, wifiCredentials[0].savedPSK,0);
	}
	Serial.print(".. #2 = ");
	Serial.println(SSID2);
	if(SSID2[0]!='\0') { // } && PSK2[0]!='\0'){
		Serial.println("Wifi 2");
		strncpy(wifiCredentials[1].savedSSID,SSID2,32);
		strncpy(wifiCredentials[1].savedPSK,PSK2,64);

		_numWifiCredentials++;
		_gotCredentials=1;

		saveWifiInFile(_wifiFile, wifiCredentials[1].savedSSID, wifiCredentials[1].savedPSK,1);
	}
	Serial.print("..#3 = ");
	Serial.println(SSID3);
	if(SSID3[0]!='\0') { //} && PSK3[0]!='\0'){
		Serial.println("Wifi 3");
		strncpy(wifiCredentials[2].savedSSID,SSID3,32);
		strncpy(wifiCredentials[2].savedPSK,PSK3,64);

		_numWifiCredentials++;
		_gotCredentials=1;

		saveWifiInFile(_wifiFile, wifiCredentials[2].savedSSID, wifiCredentials[2].savedPSK,2);
	}


	Serial.println("wifi saved");
}


bool DeviceWifi::startApForWifiCredentials(char *apName, int timoutMilis){
	Serial.println("Starting AP " + (String)apName);
	setupWiFiAp(apName);
	delay(200);


	//Wait for connection with timout
	int counter=0;
	while (!_apConnected && (counter*500)<timoutMilis) {
		delay(500);
		counter=counter+1;
	}
	if((counter*500)>=timoutMilis){
		return 0;
	}


  if (_encripted==true){
    	_webServer->on("/wifisave", this->getWifiCredentialsEncripted);
  }else{
    	_webServer->on("/wifisave", this->getWifiCredentialsNotEncripted);
  }

	_webServer->begin();

	Serial.println("Client connected");


	Serial.println("local ip");
	Serial.println(WiFi.localIP());

	while(!_gotCredentials){
		delay(500);
		_webServer->handleClient();
	}
	_gotCredentials=0;

	_webServer->stop();

	WiFi.disconnect(true);
	delay(1000);
	#ifndef ESP32
	(void)wifi_station_dhcpc_start();
	#else
	WiFi.config({ 0,0,0,0 }, { 0,0,0,0 }, { 0,0,0,0 });
	//(void)tcpip_adapter_dhcpc_start(TCPIP_ADAPTER_IF_STA);
	#endif

	return 1;
}