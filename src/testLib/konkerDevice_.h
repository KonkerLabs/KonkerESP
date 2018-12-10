#ifndef KonkerDevice
#define KonkerDevice
#include "Arduino.h"
#include "konkerSettings.h"
#include "deviceWifi.h"

#include <DNSServer.h>
#ifndef ESP32
#include <ESP8266WebServer.h>
#else
#include <WebServer.h>
#endif

#include <ArduinoJson.h>

#include "./helpers/fileHelper.h"
#include "./helpers/jsonhelper.h"


#include "Arduino.h"

class KonkerDevice{
	
  private:
    bool _encripted = false;
	char _chipId[32];
	char _base_name[6];
	void _setDefaults();

  public:
	//KonkerSettings konker;
	//DeviceWifi deviceWifi;

	#ifndef ESP32
	int resetPin=D5;
	#else
	int resetPin=13;
	#endif


	int statusLedPin=2;
	char name[32];

    KonkerDevice(String httpDomain, String httpPubTopicFormat, String httpSubTopicFormat, bool encripted);
    KonkerDevice(String httpDomain, String httpPubTopicFormat, String httpSubTopicFormat,
                 String mqttAdress, String mqttPubTopicFormat, String mqttSubTopicFormat, bool encripted);

	void setup(char newName[6]);

	void KonkerDevice::clearDeviceConfig();	

	bool KonkerDevice::connectToWiFiAndKonker();

	bool KonkerDevice::checkConnections();

}



KonkerDevice::KonkerDevice(String httpDomain, String httpPubTopicFormat, String httpSubTopicFormat, bool encripted){
	strncpy(_base_name,"S0000",6);
	konker.setHttpDomain(httpDomain) && konker.setHttpPubTopicFormat(httpPubTopicFormat) && konker.setHttpSubTopicFormat(httpSubTopicFormat);
	deviceWifi=DeviceWifi();
	_encripted=encripted;
	_setDefaults();
	
}


KonkerDevice::KonkerDevice(String httpDomain, String httpPubTopicFormat, String httpSubTopicFormat,
                 String mqttAdress, String mqttPubTopicFormat, String mqttSubTopicFormat, bool encripted){
  strncpy(_base_name,"S0000",6);
  konker.setHttpDomain(httpDomain) && konker.setHttpPubTopicFormat(httpPubTopicFormat) && konker.setHttpSubTopicFormat(httpSubTopicFormat);
  success= success && konker.setMqttAdress(httpDomain) && konker.setMqttPubTopicFormat(httpPubTopicFormat) && konker.setMqttSubTopicFormat(httpSubTopicFormat);

  _encripted=encripted;
  _setDefaults();
}

void KonkerDevice::_setDefaults(){
	strncpy(_base_name,"S0000",6);
}


void KonkerDevice::clearDeviceConfig(){
	WiFi.begin("", "");
	WiFi.disconnect(true);
	for(int i=0;i<3;i++){
		strcpy(wifiCredentials[i].savedSSID, "");
		strcpy(wifiCredentials[i].savedPSK, "");
	}

	formatFileSystem();
	Serial.println("Full reset done! FileSystem formated!");
	Serial.println("You must remove this device from Konker plataform if registred, and redo factory configuration.");

	delay(5000);
	#ifndef ESP32
	ESP.reset();
	#else
	ESP.restart();,
	#endif
	delay(1000);
}

void KonkerDevice::setup(char newName[6]){
	pinMode(resetPin, OUTPUT);
	digitalWrite(resetPin, HIGH);

	Serial.print("HttpDomain: ");
	Serial.println(konker.getHttpDomain());
	Serial.print("HttpPort: ");
	Serial.println(konker.getHttpPort());

	Serial.print("MqttAdress: ");
	Serial.println(konker.getMqttAdress());
	Serial.print("MqttPort: ");
	Serial.println(konker.getMqttPort());


	strncpy(_base_name, newName,6);
	#ifndef ESP32
	String stringNewName=String(_base_name) + String(ESP.getChipId());
	#else
	String stringNewName=String(_base_name) + (uint32_t)ESP.getEfuseMac();
	#endif
	strncpy(name, stringNewName.c_str(),32);


	pinMode(statusLedPin, OUTPUT);
	digitalWrite(statusLedPin, LOW);
	//cria file system se não existir
	spiffsMount();

	if (digitalRead(resetPin) == LOW){//reset and format FS all if resetPin is low
		Serial.println("resetPin pin in LOW state. Formating FS.. (WAIT FOR REBOOT)");
		clearDeviceConfig();
	}

  //tenta se conectar ao wifi configurado
  //caso já exista o arquivo de wifi ele vai tentar se conectar
  if(tryConnectClientWifi()){
    if(getPlataformCredentials((char*)"/crd.json")){
			checkConnections();
		}
    return;
  }

	int arquivoWifiPreConfigurado=0;

	if(strcmp(WiFi.SSID().c_str(),(char*)"KonkerDevNetwork")!=0){
		if(WiFi.SSID().c_str()[0]!='\0'){
			Serial.println("Saving wifi memory");

			//ordering
			for(int i=0;i<numWifiCredentials-1;i++){
				char tempSSID[32]="";
				char tempPSK[64]="";
				strcpy(tempSSID, wifiCredentials[i].savedSSID);
				strcpy(tempPSK, wifiCredentials[i].savedPSK);
				if(String(tempSSID).indexOf(WiFi.SSID(), 0)>0){
					if(i==1){
							strcpy(wifiCredentials[1].savedSSID, wifiCredentials[0].savedSSID);
							strcpy(wifiCredentials[1].savedPSK, wifiCredentials[0].savedPSK);
							
							strcpy(wifiCredentials[0].savedSSID, tempSSID);
							strcpy(wifiCredentials[0].savedPSK, tempPSK);
					}
					if(i==2){
							strcpy(wifiCredentials[2].savedSSID, wifiCredentials[0].savedSSID);
							strcpy(wifiCredentials[2].savedPSK, wifiCredentials[0].savedPSK);
							
							strcpy(wifiCredentials[0].savedSSID, tempSSID);
							strcpy(wifiCredentials[0].savedPSK, tempPSK);
					}
				}
			}

			for(int i=0;i<numWifiCredentials-1;i++){
				Serial.printf("saving wifi '%s' password '%s'\n", wifiCredentials[i].savedSSID, wifiCredentials[i].savedPSK);
				saveWifiConnectionInFile(wifiFile, wifiCredentials[i].savedSSID, wifiCredentials[i].savedPSK,i);
			}
		}
	}else{
		Serial.println("Wifi memory has KonkerDevNetwork, ignoring..");
	}


	arquivoWifiPreConfigurado=getPlataformCredentials((char*)"/crd.json");//se for outro nome é só mudar aqui

	while (!arquivoWifiPreConfigurado){
		//only exits this function if connected or reached timout, only connect if specific signal strength is mesured (47dBm)
		checkForFactoryWifi((char*)"KonkerDevNetwork",(char*)"konkerkonker123",47,10000);
		//se conectar no FactoryWifi
		//baixar arquivo pré wifi

		getPlataformCredentialsFromGateway();
		//retorno da função do Luis 1 -> recebeu e guardou o arquivo wifipré. 0->deu algum problema e não tem wifipré
		//se tiver arquivo pré wifi
		//configura pré wifi
		//Formato esperado: {"srv":"mqtt.demo.konkerlabs.net","prt":"1883","usr":"jnu56qt1bb1i","pwd":"3S7usR9g5K","prx":"data"}
		arquivoWifiPreConfigurado=getPlataformCredentials((char*)"/crd.json");//se for outro nome é só mudar aqui

	}

	//desliga led indicando que passou pela configuração de fábrica
	digitalWrite(statusLedPin, HIGH);



	//esta parte só chega se já passou pelo modo fábrica acima
	//lembrando
	//global variables: server, port, device_login, device_pass
	//variables delcared in main.h from LibKonkerESP8266
	//char server[64];
	//int port;
	//char device_login[32];
	//char device_pass[32];

	//Tem arquivo wifi? Se tem configura o wifi
	//se não tem entra em modo AP


	//aqui
	if(tryConnectClientWifi()){
    	return;
  	}

	//MODO AP (nome do device, sem senha)

	//se conectar, cria o HTTP server
	//recebe a configuração WiFi via POST enviado pelo app do celular

	//tenta conctar no wifi passado pelo app
	//se falhar ou passar do timout,reboota
	if(startAPForWifiCredentials(getChipId(),120000)){
		Serial.println("Credentials received, trying to connect to production WiFi");
		if(!tryConnectClientWifi()){
			Serial.println("Failed! Rebooting...");
			delay(3000);
			#ifndef ESP32
			ESP.reset();
			#else
			ESP.restart();
			#endif
		}


		Serial.println("WiFi configuration done!");
		Serial.println("Saving wifi memory");
		for(int i=0;i<numWifiCredentials;i++){
			saveWifiConnectionInFile(wifiFile,wifiCredentials[i].savedSSID,wifiCredentials[i].savedPSK, i);
		}


		delay(1000);
		Serial.println("Rebooting...");
		#ifndef ESP32
		ESP.reset();
		#else
		ESP.restart();
		#endif
	}else{
		Serial.println("Timout! Rebooting...");
		delay(3000);
		   #ifndef ESP32
		ESP.reset();
		#else
		ESP.restart();
		#endif
	}



}

bool KonkerDevice::connectToWiFiAndKonker(){
  if(WiFi.status()!=WL_CONNECTED){
    if(!connectToSavedWifi()){
      Serial.println("Failed to connect to WiFi");
      appendToFile(healthFile,(char*)"1", _netFailureAdress);
      return 0;
    }
  }


	Serial.println("Cheking server connections..");
	#ifdef pubsubMQTT
	if(!connectMQTT()){
		Serial.println("Failed!");
    	appendToFile(healthFile,(char*)"1", _mqttFailureAdress);
		return 0;
	}
	#endif
	if(!testHTTPSubscribeConn()){
		Serial.println("Failed!");
	    appendToFile(healthFile,(char*)"1", _httpFailureAdress);
		return 0;
	}


	return 1;
}

bool KonkerDevice::checkConnections(){
  if(!connectToWiFiAndKonker()){
    Serial.println("Failed to connect");
    //TODO save in FS the failures
    delay(3000);
    #ifndef ESP32
    ESP.reset();
    #else
    ESP.restart();
    #endif
  }

  //Serial.println("Device connected to WiFi: " + (String)WiFi.SSID());
  //Serial.print("IP Address:");
  //Serial.println(WiFi.localIP());
  
  return 1;
}

//only exits this function if connected or reached timout. 1 if connection was made, 0 if not
bool KonkerDevice::checkForFactoryWifi(char *ssidConfig, char *ssidPassConfig, int powerLimit, int connectTimeout){
	Serial.println("Searching for " + (String)ssidConfig + ":" + (String)ssidPassConfig);

	//wifi power is weekened (remember to always set wifipower back to maximum 20.5dBm before atempting to connect to a customer WiFi)
	#ifndef ESP32
	WiFi.setOutputPower(2);
	#endif

	Serial.println(".");

	boolean keepConnecting = true;
	unsigned long start = millis();
	while (keepConnecting) {
		if(connect(ssidConfig, ssidPassConfig) == 3 && checkSignal(powerLimit)==1) {
			keepConnecting = false;
			Serial.println("Connected to "  + (String)ssidConfig);

			Serial.print("IP Address:");
			Serial.println(WiFi.localIP());
			return 1;
		}
		Serial.print(".");
		if (millis() > start + connectTimeout) {
			Serial.println("");
			Serial.println("Factory connection window timed out");
			keepConnecting = false;
			return 0;
		}
		delay(100);
	}
	#ifndef ESP32
	WiFi.setOutputPower(20.5);// return wifi power to maximum
	#endif
	return 0;
}

//Expected format->   {"srv":"mqtt.demo.konkerlabs.net","prt":"1883","usr":"jnu55qtsbbii","pwd":"3S7rnsR9gDqK", ,"prx":"data"}
bool KonkerDevice::getPlataformCredentials(char *configFilePath){
	//////////////////////////
	///step1 open file
	char fileContens[1024];
	if(!readFile(configFilePath,fileContens)){
		return 0;//if fail to open file
	}

	//////////////////////////
	///step2 config file opened
	Serial.println("Parsing: " + (String)fileContens);
	DynamicJsonBuffer jsonBuffer;
	JsonObject& configJson = jsonBuffer.parseObject(fileContens);
	if (configJson.success()) {
		Serial.println("Json file parsed!");
	}else{
		Serial.println("Failed to read Json file");
		return 0;
	}


	//////////////////////////
	///step3 copy values
	Serial.println("Config file exists, reading...");

	char bufferConfigJson[1024]="";
	configJson.printTo(bufferConfigJson, 1024);
	Serial.println("Content: "+ String(bufferConfigJson));
	if (configJson.containsKey("srv") && configJson.containsKey("prt") &&
	configJson.containsKey("usr") && configJson.containsKey("pwd") && configJson.containsKey("prx")){


		//global variables: server, port, device_login, device_pass
		//variables delcared in main.h from LibKonkerESP8266
		//char server[64];
		//int port;
		//char device_login[32];
		//char device_pass[32];
		//char prefix[32];
		strcpy(server, configJson["srv"]);
		port=atoi(configJson["prt"]);
		strcpy(device_login, configJson["usr"]);
		strcpy(device_pass, configJson["pwd"]);
		strcpy(prefix, configJson["prx"]);
	}else{
		Serial.println("Unexpected json format");
		return 0;
	}

	//////////////////////////
	///step4 check for empty values
	if(strcmp(server, "") == 0){
		Serial.println("Unexpected json format, srv is empty");
		return 0;
	}
	if(port== 0){
		Serial.println("Unexpected json format, port is empty");
		return 0;
	}
	if(strcmp(device_login, "") == 0){
		Serial.println("Unexpected json format, usr is empty");
		return 0;
	}
	if(strcmp(device_pass, "") == 0){
		Serial.println("Unexpected json format, pwd is empty");
		return 0;
	}
	if(strcmp(prefix, "") == 0){
		Serial.println("Unexpected json format, prx is empty");
		return 0;
	}

	//if reached this part then return 1 meaning it is OK
	return 1;
}

bool KonkerDevice::setPlatformCredentials(char *server, char *port, char *user, char *password, char *prefix){
  //Creating the variables we will use: "response" to keep the Server response and "address" to keep the address used to access the server

	Serial.println("Get platform credentials...");
	//	"{\"srv\":\"mqtt.demo.konkerlabs.net\",\"prt\":\"1883\",\"usr\":\"vmqb4o2j59d3\",\"pwd\":\"y5E9FOhuwsr5\", \"prx\":\"data\"}"
	char configuration[1024]={'\0'};
	strcat(configuration,"{\"srv\":\"");
	strcat(configuration,server);
	strcat(configuration,"\", \"prt\":\"");
	strcat(configuration,port);
	strcat(configuration,"\", \"usr\":\"");
	strcat(configuration,user);
	strcat(configuration,"\", \"pwd\":\"");
	strcat(configuration,password);
	strcat(configuration,"\", \"prx\":\"");
	strcat(configuration,prefix);
	strcat(configuration,"\"}\0");
	
	Serial.println("configuration=" + String(configuration));
		//cria file system se não existir
	spiffsMount();
    File configFile = SPIFFS.open("/crd.json", "w");
    if (!configFile) {
      if (DEBUG) Serial.println("Could not open the file with write permition!");
      return 0;
    }
	Serial.println("Saving config file /crd.json");







    configFile.print(configuration);
    configFile.close();

    //Removing the Wifi Configuration
    SPIFFS.remove(wifiFile);
    //save hinitial ealth state flags
    saveFile(healthFile,(char*)"000");

    //wifiManager.resetSettings();


    return 1;

}

bool  KonkerDevice::getPlataformCredentialsFromGateway(){
  //Creating the variables we will use: "response" to keep the Server response and "address" to keep the address used to access the server
  String response = "";
  String gateway_IP = WiFi.gatewayIP().toString();
	Serial.println("Get platform credentials...");

  String address = "http://" + gateway_IP + ":8081/" + String(getChipId());
	Serial.println("address=" + address);


  //The IP of our client is the Gateway IP
	HTTPClient http; 

  http.begin(address);     //Specify request destination
  
  int statusCode = http.GET();

	response=http.getString();

	Serial.println("statusCode=" + String(statusCode));
	Serial.println("response=" + String(response));
  if (statusCode == 200){
    File configFile = SPIFFS.open("/crd.json", "w");
    if (!configFile) {
      if (DEBUG) Serial.println("Could not open the file with write permition!");
      return 0;
    }
		Serial.println("Saving config file /crd.json");
    configFile.print(response);
    configFile.close();

    //Removing the Wifi Configuration
    SPIFFS.remove(wifiFile);
    //save hinitial ealth state flags
    saveFile(healthFile,(char*)"000");

    //wifiManager.resetSettings();

    if (DEBUG){
      Serial.print("Status code from server :");
      Serial.println(statusCode);
      Serial.print("Response body from server: ");
      Serial.println(response);
    }
    return 1;
  }
  else {
    return 0;
  }
}
#endif


