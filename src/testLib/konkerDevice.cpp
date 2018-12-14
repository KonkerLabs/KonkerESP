
#include "./konkerDevice.h"
KonkerDevice* KonkerDevice::_instance = NULL;

//private
bool KonkerDevice::_interpretHTTPCode(int httpCode){
  
  if (httpCode > 0 && httpCode<300) { //Check the returning code
    return 1;

  }else{
    Serial.println(String(httpCode));
    return 0;
  }
}

void KonkerDevice::_setDefaults(){
	strncpy(_base_name,"S0000",6);
	strncpy(_healthFile,"/health.json",13);
	strncpy(_configFile,"/config.json",13);
	strncpy(_wifiFile,"/wifi.json",13);
	//mqtt
	_client=PubSubClient(_espClient);
}


#ifdef konkerMQTTs
bool KonkerDevice::_checkSecureFingerprint(const char* fingerprint, const char* secureServer){
  if (espClient.verify(fingerprint, secureServer)) {
    Serial.println("Connection secure.");
		return 1;
  } else {
    Serial.println("Connection insecure! Halting execution.");
   	return 0;
  }
}

bool KonkerDevice::_checkSecureFingerprint(const char* fingerprint){
  if (espClient.verify(fingerprint, server)) {
    Serial.println("Connection secure.");
		return 1;
  } else {
    Serial.println("Connection insecure! Halting execution.");
   	return 0;
  }
}
#endif


void KonkerDevice::_callback(char* topic, byte* payload, unsigned int length){

  Serial.print("Message received [");
  Serial.print(topic);
  Serial.print("] ");

  // assert(KonkerDevice::_instance != NULL);

  KonkerDevice::_instance->_subChanTuple.callSubChannelCallback(topic, payload, length);

}


bool KonkerDevice::_connectMQTT(char r_server[], int r_port, char r_device_login[], char r_device_pass[]){
    if(_client.connected()){
      Serial.print("Already connected to MQTT broker ");
      //return 1;
    }else{
      Serial.print("Trying to connect to MQTT broker ");
    }
    


	Serial.print(" URI:" + String(r_server) + " Port:" + String(r_port) + ", ");

	_client.setServer(r_server, r_port);

	//<TO-DO>
	_client.setCallback(_callback);

	Serial.print(" U:" + String(r_device_login) + " P:" + String(r_device_pass));

	int connectCode=_client.connect(r_device_login, r_device_login, r_device_pass);

	Serial.println(", connectCode=" + String(connectCode));


    if (connectCode==1) { //Check the returning code
      Serial.println("sucess");
      Serial.println("");
      return 1;
    }else{
      Serial.println("failed");
      Serial.println("");
      appendToFile(_healthFile,(char*)"1", _mqttFailureAdress);
      delay(3000);
      #ifndef ESP32
      ESP.reset();
      #else
      ESP.restart();
      #endif
      return 0;
		}
}

bool KonkerDevice::_connectMQTT(){
	char *server;
	konker.getMqttRootDomain().toCharArray(server, konker.getMqttRootDomain().length()+1);
	return _connectMQTT(server,konker.getMqttPort(),_device_login,_device_pass);
}



//public

void KonkerDevice::mqttLoop(){
  _client.loop();
}


KonkerDevice::KonkerDevice(String newName){
	_encripted=0;
	char chrNewName[6];
	newName.toCharArray(chrNewName,6);
	strncpy(_base_name, chrNewName,6);
	_setDefaults();
}

KonkerDevice::KonkerDevice(String newName, bool isEncripted){
	char chrNewName[6];
	newName.toCharArray(chrNewName,6);
	strncpy(_base_name, chrNewName,6);
	_encripted=isEncripted;
	_setDefaults();
}


KonkerDevice::KonkerDevice(String newName, String httpDomain, String httpPubTopicFormat, String httpSubTopicFormat,
                 String mqttAdress, String mqttPubTopicFormat, String mqttSubTopicFormat, bool isEncripted){

	konker.setHttpDomain(httpDomain) && konker.setHttpPubTopicFormat(httpPubTopicFormat) && konker.setHttpSubTopicFormat(httpSubTopicFormat);
	konker.setMqttAdress(httpDomain) && konker.setMqttPubTopicFormat(httpPubTopicFormat) && konker.setMqttSubTopicFormat(httpSubTopicFormat);
	char chrNewName[6];
	newName.toCharArray(chrNewName,6);
	strncpy(_base_name, chrNewName,6);
	_encripted=isEncripted;
	_setDefaults();
}


char *KonkerDevice::getChipId(){
  return  _chipId;
}



void KonkerDevice::clearDeviceConfig(){
	WiFi.begin("", "");
	WiFi.disconnect(true);
	for(int i=0;i<3;i++){
		strcpy(deviceWifi.wifiCredentials[i].savedSSID, "");
		strcpy(deviceWifi.wifiCredentials[i].savedPSK, "");
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

bool KonkerDevice::connectToWiFiAndKonker(){
  if(WiFi.status()!=WL_CONNECTED){
    if(!deviceWifi.connectToSavedWifi()){
      Serial.println("Failed to connect to WiFi");
      appendToFile(_healthFile,(char*)"1", _netFailureAdress);
      return 0;
    }
  }


	Serial.println("Cheking server connections..");
	#ifdef pubsubMQTT
	if(!connectMQTT()){
		Serial.println("Failed!");
    	appendToFile(_healthFile,(char*)"1", _mqttFailureAdress);
		return 0;
	}
	#endif
	if(!testHTTPSubscribeConn()){
		Serial.println("Failed!");
	    appendToFile(_healthFile,(char*)"1", _httpFailureAdress);
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
		if(deviceWifi.connect(ssidConfig, ssidPassConfig) == 3 && deviceWifi.checkSignal(powerLimit)==1) {
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

//Expected format->   {"mqtt":"mqtt://mqtt.demo.konkerlabs.net:1883", "http":"http://data.demo.konkerlabs.net:80", "usr":"jsu55qtsbbii","pwd":"3S7onsR9gDqK"}
bool KonkerDevice::setPlataformCredentials(char *configFilePath){
	//Set the parameters for konker object 
	//Example:
	//konker.setHttpDomain("http://data.demo.konkerlabs.net:80");

	//For format use theese keywords <device> and <channel>, they will be automaticaly replaced at PUB or SUB functions
	//konker.setHttpPubTopicFormat("/pub/<device>/<channel>");
	//konker.setHttpSubTopicFormat("/sub/<device>/<channel>");

	//konker.setMqttAdress("mqtt://mqtt.demo.konkerlabs.net:1883")
	//konker.setMqttPubTopicFormat("data/<device>/pub/<channel>");
	//konker.setMqttSubTopicFormat("data/<device>/sub/<channel>")


	String _mqttJsonKeyword="mqtt";
	String _httpJsonKeyword="http";
	String _usrJsonKeyword="usr";
	String _pwdJsonKeyword="pwd";


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
	if (configJson.containsKey(_mqttJsonKeyword) && configJson.containsKey(_httpJsonKeyword) &&
	configJson.containsKey(_usrJsonKeyword) && configJson.containsKey(_pwdJsonKeyword)){

		//For format use theese keywords <device> and <channel>, they will be automaticaly replaced at PUB or SUB functions
		konker.setHttpDomain(configJson[_httpJsonKeyword]);
		konker.setHttpPubTopicFormat("/pub/<device>/<channel>");
		konker.setHttpSubTopicFormat("/sub/<device>/<channel>");

		konker.setMqttAdress(configJson[_mqttJsonKeyword]);
		konker.setMqttPubTopicFormat("data/<device>/pub/<channel>");
		konker.setMqttSubTopicFormat("data/<device>/sub/<channel>");

		strcpy(_device_login, configJson[_usrJsonKeyword]);
		strcpy(_device_pass, configJson[_pwdJsonKeyword]);

	}else{
		Serial.println("Unexpected json format");
		return 0;
	}

	//////////////////////////
	///step4 check for empty values
	if(strcmp(configJson[_httpJsonKeyword], "") == 0){
		Serial.println("Unexpected json format, " + _httpJsonKeyword + " is empty");
		return 0;
	}
	if(configJson[_mqttJsonKeyword]== 0){
		Serial.println("Unexpected json format, " + _mqttJsonKeyword + " is empty");
		return 0;
	}
	if(strcmp(configJson[_usrJsonKeyword], "") == 0){
		Serial.println("Unexpected json format, " + _usrJsonKeyword + " is empty");
		return 0;
	}
	if(strcmp(configJson[_pwdJsonKeyword], "") == 0){
		Serial.println("Unexpected json format, " + _pwdJsonKeyword + " is empty");
		return 0;
	}


	//For format use theese keywords <device> and <channel>, they will be automaticaly replaced at PUB or SUB functions
	konker.setHttpDomain(configJson[_httpJsonKeyword]);
	konker.setHttpPubTopicFormat("/pub/<device>/<channel>");
	konker.setHttpSubTopicFormat("/sub/<device>/<channel>");

	konker.setMqttAdress(configJson[_mqttJsonKeyword]);
	konker.setMqttPubTopicFormat("data/<device>/pub/<channel>");
	konker.setMqttSubTopicFormat("data/<device>/sub/<channel>");

	strcpy(_device_login, configJson[_usrJsonKeyword]);
	strcpy(_device_pass, configJson[_pwdJsonKeyword]);


	//if reached this part then return 1 meaning it is OK
	return 1;
}

bool KonkerDevice::setPlataformCredentials(char *mqtt, char *http, char *user, char *password){
	Serial.println("Set platform credentials...");
	//	"{\"mqtt\":\"mqtt://mqtt.demo.konkerlabs.net:1883\",\"http\":\"http://data.demo.konkerlabs.net:80\",\"usr\":\"vmqb4o2j59d3\",\"pwd\":\"y5E9FOhuwsr5\""}"

	char configuration[1024]={'\0'};
	strcat(configuration,"{\"mqtt\":\"");
	strcat(configuration,mqtt);
	strcat(configuration,"\", \"http\":\"");
	strcat(configuration,http);
	strcat(configuration,"\", \"usr\":\"");
	strcat(configuration,user);
	strcat(configuration,"\", \"pwd\":\"");
	strcat(configuration,password);
	strcat(configuration,"\"}\0");

	Serial.println("configuration=" + String(configuration));
		//cria file system se não existir
	spiffsMount();
	File configFile = SPIFFS.open(_configFile, "w");
	if (!configFile) {
		if (_debug) Serial.println("Could not open the file with write permition!");
		return 0;
	}
	Serial.println("Saving config file " + (String)_configFile);




	configFile.print(configuration);
	configFile.close();

	//Removing the Wifi Configuration
	SPIFFS.remove(_wifiFile);
	//save hinitial ealth state flags
	saveFile(_healthFile,(char*)"000");

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

	http.begin(address); //Specify request destination

	int statusCode = http.GET();

	response = http.getString();

	Serial.println("statusCode=" + String(statusCode));
	Serial.println("response=" + String(response));
	if (statusCode == 200)
	{
		File configFile = SPIFFS.open(_configFile, "w");
		if (!configFile)
		{
			if (_debug)
				Serial.println("Could not open the file with write permition!");
			return 0;
		}
		Serial.println("Saving config file /crd.json");
		configFile.print(response);
		configFile.close();

		//Removing the Wifi Configuration
		SPIFFS.remove(_wifiFile);
		//save hinitial ealth state flags
		saveFile(_healthFile, (char *)"000");

		//wifiManager.resetSettings();

		if (_debug)
		{
			Serial.print("Status code from server :");
			Serial.println(statusCode);
			Serial.print("Response body from server: ");
			Serial.println(response);
		}
		return 1;
	}
	else
	{
		return 0;
	}
}

void KonkerDevice::setup(){
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
  if(deviceWifi.connectToSavedWifi()){
    if(setPlataformCredentials((char*)_configFile)){
			checkConnections();
		}
    return;
  }

	int arquivoWifiPreConfigurado=0;

	if(strcmp(WiFi.SSID().c_str(),(char*)"KonkerDevNetwork")!=0){
		if(WiFi.SSID().c_str()[0]!='\0'){
			Serial.println("Saving wifi memory");

			//ordering
			for(int i=0;i<deviceWifi.getNumWifiCredentials()-1;i++){
				char tempSSID[32]="";
				char tempPSK[64]="";
				strcpy(tempSSID, deviceWifi.wifiCredentials[i].savedSSID);
				strcpy(tempPSK, deviceWifi.wifiCredentials[i].savedPSK);
				if(String(tempSSID).indexOf(WiFi.SSID(), 0)>0){
					if(i==1){
							strcpy(deviceWifi.wifiCredentials[1].savedSSID, deviceWifi.wifiCredentials[0].savedSSID);
							strcpy(deviceWifi.wifiCredentials[1].savedPSK, deviceWifi.wifiCredentials[0].savedPSK);
							
							strcpy(deviceWifi.wifiCredentials[0].savedSSID, tempSSID);
							strcpy(deviceWifi.wifiCredentials[0].savedPSK, tempPSK);
					}
					if(i==2){
							strcpy(deviceWifi.wifiCredentials[2].savedSSID, deviceWifi.wifiCredentials[0].savedSSID);
							strcpy(deviceWifi.wifiCredentials[2].savedPSK, deviceWifi.wifiCredentials[0].savedPSK);
							
							strcpy(deviceWifi.wifiCredentials[0].savedSSID, tempSSID);
							strcpy(deviceWifi.wifiCredentials[0].savedPSK, tempPSK);
					}
				}
			}
			

			for(int i=0;i<deviceWifi.getNumWifiCredentials()-1;i++){
				Serial.printf("saving wifi '%s' password '%s'\n", deviceWifi.wifiCredentials[i].savedSSID, deviceWifi.wifiCredentials[i].savedPSK);
				deviceWifi.saveWifiInFile(_wifiFile, deviceWifi.wifiCredentials[i].savedSSID, deviceWifi.wifiCredentials[i].savedPSK,i);
			}
		}
	}else{
		Serial.println("Wifi memory has KonkerDevNetwork, ignoring..");
	}


	arquivoWifiPreConfigurado=setPlataformCredentials((char*)_configFile);//se for outro nome é só mudar aqui

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
		arquivoWifiPreConfigurado=setPlataformCredentials((char*)_configFile);//se for outro nome é só mudar aqui

	}

	//desliga led indicando que passou pela configuração de fábrica
	digitalWrite(statusLedPin, HIGH);



	//esta parte só chega se já passou pelo modo fábrica acima
	//lembrando
	//global variables: server, port, _device_login, _device_pass
	//variables delcared in main.h from LibKonkerESP8266
	//char server[64];
	//int port;
	//char _device_login[32];
	//char _device_pass[32];

	//Tem arquivo wifi? Se tem configura o wifi
	//se não tem entra em modo AP


	//aqui
	if(deviceWifi.connectToSavedWifi()){
    	return;
  	}

	//MODO AP (nome do device, sem senha)

	//se conectar, cria o HTTP server
	//recebe a configuração WiFi via POST enviado pelo app do celular

	//tenta conctar no wifi passado pelo app
	//se falhar ou passar do timout,reboota
	if(deviceWifi.startApForWifiCredentials(getChipId(),120000)){
		Serial.println("Credentials received, trying to connect to production WiFi");
		if(!deviceWifi.connectToSavedWifi()){
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
		for(int i=0;i<deviceWifi.getNumWifiCredentials();i++){				
			deviceWifi.saveWifiInFile(_wifiFile,deviceWifi.wifiCredentials[i].savedSSID,deviceWifi.wifiCredentials[i].savedPSK, i);
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

///////////////////
///http
bool KonkerDevice::pubHttp(char const channel[], char const msg[], int& ret_code){
  //throtle this call 
  if ((millis()-_last_time_http_request) < _millis_delay_per_http_request){
      delay((millis()-_last_time_http_request));
  }
  _last_time_http_request = millis();

  bool pubCode=0;


  HTTPClient http;  //Declare an object of class HTTPClient
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Accept", "application/json");
  http.setAuthorization(_device_login, _device_pass);

  String pubUrl=konker.getHttpDomain()+ konker.getHttpPubTopic(_device_login,(String)channel);
  
  http.begin(pubUrl);

  int httpCode=http.POST(String(msg));
  //Serial.println("Publishing to " + String(topic) + "; Body: " + String(msg) + "; httpcode: " + String(httpCode));
  //Serial.print(">");
  http.end();   //Close connection

  ret_code = httpCode;

  pubCode=_interpretHTTPCode(httpCode);


  if (!pubCode){
    Serial.println("failed");
    Serial.println("");
    return 0;
  }else{
    Serial.println("sucess");
    Serial.println("");
    return 1;
  }

}

bool KonkerDevice::pubHttp(char const channel[], char const msg[]){
  int ret_code;
  return pubHttp(channel, msg, ret_code);

}


bool KonkerDevice::subHttp(char const channel[],CHANNEL_CALLBACK_SIGNATURE){
  //throtle this call 
  if ((millis()-_last_time_http_request) < _millis_delay_per_http_request){
      delay((millis()-_last_time_http_request));
  }
  _last_time_http_request = millis();

  bool subCode=0;


  HTTPClient http;  //Declare an object of class HTTPClient
  http.addHeader("Content-Type", "application/json");
  http.setAuthorization(_device_login, _device_pass);


  String pubUrl=konker.getHttpDomain()+ konker.getHttpSubTopic(_device_login,(String)channel);
  
  http.begin(pubUrl);


  int httpCode = http.GET();


  Serial.print(">");

  subCode=_interpretHTTPCode(httpCode);
  
  if (!subCode){
    Serial.println("failed SUB request");
    Serial.println("");
  }else{
    Serial.println("sucess SUB request");
    Serial.println("");

    String strPayload = http.getString();
    //Serial.println("strPayload=" + strPayload);
    int playloadSize=http.getSize();
    http.end();   //Close connection
    if (strPayload!="[]"){    
      unsigned char* payload = (unsigned char*) strPayload.c_str(); // cast from string to unsigned char*
      chan_callback(payload,playloadSize);
    }
    return 1;
  }
  http.end();   //Close connection
  return 0;
}


bool KonkerDevice::testHTTPSubscribeConn(){
    //throtle this call 
  if ((millis()-_last_time_http_request) < _millis_delay_per_http_request){
      delay((millis()-_last_time_http_request));
  }
  _last_time_http_request = millis();
  bool subCode=0;

  HTTPClient http;  //Declare an object of class HTTPClient
  http.addHeader("Content-Type", "application/json");
  http.setAuthorization(_device_login, _device_pass);

  String pubUrl=konker.getHttpDomain()+ konker.getHttpSubTopic(_device_login,"test");
  
  http.begin(pubUrl);


  int httpCode = http.GET();

  //Serial.println("Testing HTTP subscribe to: " + url_to_call + "; httpcode:" + String(httpCode));
  //Serial.print(">");

  subCode=_interpretHTTPCode(httpCode);


  if (!subCode){
    Serial.println("test failed");
    Serial.println("");
    http.end();   //Close connection
    return 0;
  }else{
    Serial.println("sucess");
    Serial.println("");

    String strPayload = http.getString();
    Serial.println("strPayload=" + strPayload);
    http.end();   //Close connection
    return 1;
  }
}

////////////////
//mqtt


bool KonkerDevice::pubMQTT(char const channel[], char const msg[]){
	int pubCode=-1;
	char topic[32];


	konker.getMqttPubTopic(_device_login,(String)channel).toCharArray(topic,32);

	Serial.println("Publishing to: " + String(topic) + " msg: " + msg );

	delay(200);
	pubCode=_client.publish(topic, msg);

	if (pubCode!=1){
		Serial.println("failed");
		Serial.println("pubCode:" + (String)pubCode);
		appendToFile(_healthFile,(char*)"1", _mqttFailureAdress);
		delay(3000);
		#ifndef ESP32
		ESP.reset();
		#else
		ESP.restart();
		#endif
		return 0;
	}else{
		Serial.println("sucess");
		Serial.println("pubCode:" + (String)pubCode);
		return 1;
	}

}

bool KonkerDevice::subMQTT(char const channel[],CHANNEL_CALLBACK_SIGNATURE){
	int subCode=-1;
	char topic[32];

	konker.getMqttPubTopic(_device_login,(String)channel).toCharArray(topic,32);

	Serial.println("Subscribing to: " + String(topic));

	delay(200);
	subCode=_client.subscribe(topic);

	if (subCode!=1){
		Serial.println("failed");
		Serial.println("");
		appendToFile(_healthFile,(char*)"1", _mqttFailureAdress);
		delay(3000);
		#ifndef ESP32
		ESP.reset();
		#else
		ESP.restart();
		#endif
		return 0;
	}else{
		_subChanTuple.addSubChannelTuple(topic,chan_callback);
		Serial.println("sucess");
		Serial.println("");
		return 1;
	}

}

