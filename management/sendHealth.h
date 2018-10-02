
#ifndef sendHealth
#define sendHealth

#include "../helpers/globals.h"
#ifndef ESP32
#include <ESP8266HTTPClient.h>
#else
#include <HTTPClient.h>
#endif
#include "../helpers/jsonhelper.h"
#include "./helpers/fileHelper.h"
#include "./rest/pubsubREST.h"

unsigned long _last_time_health_send=0;
String healthFile="/health.json";

int _netFailureAdress=0;
int _mqttFailureAdress=1;
int _httpFailureAdress=2;

//envia mensagem para a plataforma
void healthUpdate(char *health_channel){
  if (_last_time_health_send!=0){
    //throtle this call at maximum 1 per minute
    if ((millis()-_last_time_health_send) < 60000){
        return;
    }
  }

  _last_time_health_send = millis();


	StaticJsonBuffer<256> jsonBuffer;
	JsonObject& jsonMSG = jsonBuffer.createObject();

	delay(10);

  char content[3];
  readFile(healthFile,content,0,3);
  int nf = content[_netFailureAdress] - '0';
  int mf = content[_mqttFailureAdress] - '0';
	int hf = content[_httpFailureAdress] - '0';

	//{"p":0}
  jsonMSG["build"] = (String)PIO_SRC_REV;
	jsonMSG["wifi"]=(String)WiFi.SSID();
	jsonMSG["rssi"]=(String)WiFi.RSSI();

  char buffer [16];
  unsigned char bytes[4];
  bytes[0] = WiFi.localIP() & 0xFF;
  bytes[1] = (WiFi.localIP() >> 8) & 0xFF;
  bytes[2] = (WiFi.localIP() >> 16) & 0xFF;
  bytes[3] = (WiFi.localIP() >> 24) & 0xFF; 
  sprintf(buffer, "%d.%d.%d.%d", bytes[0], bytes[1], bytes[2], bytes[3]);  
	jsonMSG["ip"]=(String)buffer;
  
  jsonMSG["nfail"] = nf;
  jsonMSG["mfail"] = mf;
	jsonMSG["hfail"] = hf;


  jsonMSG.printTo(bufferJ, sizeof(bufferJ));
	char mensagemjson[1024];
  strcpy(mensagemjson,bufferJ);
	Serial.println("Publishing on channel:" + (String)health_channel);
	Serial.println("The message:");
	Serial.println(mensagemjson);

	pubHttp(health_channel, mensagemjson);

  if(nf==0 && mf==0 && hf==0){
    return;
  }
  //clear error flags
  saveFile(healthFile,(char*)"000");

}

#endif