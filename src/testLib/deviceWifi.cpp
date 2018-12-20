#include "./deviceWifi.h"

// initialize static variable 

DeviceWifi *DeviceWifi::_instance = NULL;

DeviceWifi::DeviceWifi(){
    _setDefaults();
}

DeviceWifi::DeviceWifi(char encriptKeyWord[32]){
    _setDefaults();
	strncpy(_encriptKeyWord, encriptKeyWord, 32);
	_encripted = true;
}

void DeviceWifi::setEncriptKeyword(char encriptKeyWord[32]) {
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
	DeviceWifi::_instance->_apConnected=1;
}

void DeviceWifi::_wiFiApDisconnected() {
	Serial.println("Desconectou");
	DeviceWifi::_instance->_apConnected=0;
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

	//<TO-DO>
	#ifndef ESP32
	WiFi.onEvent(_wiFiEvent,WIFI_EVENT_ANY);
	#else
	WiFi.onEvent(_wiFiEventConnected,SYSTEM_EVENT_AP_STACONNECTED);
	WiFi.onEvent(_wiFiEventDisconnected,SYSTEM_EVENT_AP_STADISCONNECTED);
	#endif

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
		String argSSID = _instance->urlDecode(_instance->_webServer->arg("s" + String(i)));
		String argPSK = _instance->urlDecode(_instance->_webServer->arg("p" + String(i)));

		Serial.println("SSID#" + String(i) + "=" + argSSID);
		Serial.println(" PWD#" + String(i) + "=" + argPSK);

		// NOTE: permit to define null password for wifi networks

		if(argSSID!=""){
		// if(argSSID!="" && argPSK!=""){
			argSSID.toCharArray(_instance->wifiCredentials[i].savedSSID, 32);
			//argPSK.toCharArray(savedPSK, 64);
			_instance->_numWifiCredentials++;
			_instance->_gotCredentials=1;
			//Decrypt Password
			char pass[argPSK.length()+1];
			argPSK.toCharArray(pass, argPSK.length()+1);
			char mySSID[argSSID.length()+1];
			argSSID.toCharArray(mySSID, argPSK.length()+1);
			char iv1[17] = "AnE9cKLPxGwyPPVU";
			char iv2[17] = "sK33DE5TaC9nRUSt";
			uint8_t var3[64];
			uint8_t var4[64];
			uint8_t *convKeyLogin=_instance->_convert_key(_instance->_encriptKeyWord);
			uint8_t *convKeySSID=_instance->_convert_key(mySSID);

			Serial.print("_convert_key(_encriptKeyWord): " + String(_instance->_encriptKeyWord) + " ");
			for (int j = 0; j < 16; j++) {
					Serial.print(convKeyLogin[j]);
			}
			Serial.println("");

			Serial.print("_convert_key(mySSID): "+ String(mySSID)+ " ");
			for (int j = 0; j < 16; j++) {		
					Serial.print(convKeySSID[j]);
			}
			Serial.println("");

			AES deck1(_instance->_convert_key(_instance->_encriptKeyWord), _instance->_convert_iv(iv2), AES::AES_MODE_128, AES::CIPHER_DECRYPT);
			AES deck2(_instance->_convert_key(mySSID), _instance->_convert_iv(iv1), AES::AES_MODE_128, AES::CIPHER_DECRYPT);
			deck1.process(_instance->_convert_hex(pass), var3, _instance->_plength);
			deck2.process(var3, var4, _instance->_plength);
			for (int j = 0; j < _instance->_plength; j++) {
					_instance->wifiCredentials[i].savedPSK[j]=var4[j];
			}
			for (int j = 1; j <= _instance->_plength; j++) {
					if (_instance->wifiCredentials[i].savedPSK[_instance->_plength-j]==' ' || _instance->wifiCredentials[i].savedPSK[_instance->_plength-j] == '\0'){
						_instance->wifiCredentials[i].savedPSK[_instance->_plength-j]='\0';
					}else {
						Serial.printf("char @ position %d = %c", j, _instance->wifiCredentials[i].savedPSK[_instance->_plength-j]);
						Serial.printf("psk%d='%s'", i, _instance->wifiCredentials[i].savedPSK);
						break;
					}
			}

		}

	}

	if(_instance->_gotCredentials){
		// reset wifi credentials from file
		//Removing the Wifi Configuration
		SPIFFS.remove(_instance->_wifiFile);
	}

	
	_instance->_webServer->send(200, "text/html", page);



}

//while connected to ESP wifi make a GET to http://192.168.4.1/wifisave?s=SSID_NAME&p=SSID_PASSWORD
void DeviceWifi::getWifiCredentialsNotEncripted(){
	Serial.println("Handle getWifiCredentials");
	String page = "<http><body><b>getWifiCredentials</b><table><tr>";

	//get up to 3 wifi credentials
	for(int i=0;i<3;i++){
		String argSSID = DeviceWifi::_instance->urlDecode(DeviceWifi::_instance->_webServer->arg("s" + String(i)));
		String argPSK = _instance->urlDecode(DeviceWifi::_instance->_webServer->arg("p" + String(i)));

		Serial.println("SSID#" + String(i) + "=" + argSSID);
		Serial.println(" PWD#" + String(i) + "=" + argPSK);


		// NOTE: accept null passwords for networking
		if(argSSID!=""){
			argSSID.toCharArray(_instance->wifiCredentials[i].savedSSID, 32);
			argPSK.toCharArray(_instance->wifiCredentials[i].savedPSK, 64);
			_instance->_numWifiCredentials++;
			_instance->_gotCredentials=1;
			page += "<td>" + argSSID + "</td>";
		}
	}

	if(_instance->_gotCredentials){
		// reset wifi credentials from file
		//Removing the Wifi Configuration
		SPIFFS.remove(_instance->_wifiFile);
	}

	page = page + "</tr></table></body></http>";

	_instance->_webServer->send(200, "text/html", page);
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