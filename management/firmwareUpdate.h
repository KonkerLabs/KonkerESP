
#ifndef firmwareUpdate
#define firmwareUpdate

#include "../helpers/globals.h"
#ifndef ESP32
#include <ESP8266HTTPClient.h>
#else
#include <HTTPClient.h>
#endif

#ifndef ESP32
#include <ESP8266httpUpdate.h>
#else
#include <ESP32httpUpdate.h>
#endif



#include "../helpers/jsonhelper.h"


#if !defined(ESP8266) || defined(ESP32)
#include <functional>
#define UPDATE_SUCCESS_CALLBACK_SIGNATURE std::function<void(char[16])> succes_update_callback
#else
#define UPDATE_SUCCESS_CALLBACK_SIGNATURE void (*succes_update_callback)(char[16])
#endif

unsigned long _last_time_update_check=0;





#ifndef ESP32
class ESPHTTPKonkerUpdate: public ESP8266HTTPUpdate{
  public:
  t_httpUpdate_return update(const String& host, uint16_t port, const String& uri = "/",
                               const String& currentVersion = ""){
    HTTPClient http;
    http.begin(host, port, uri);

    Serial.println("Authorizing.."); 
    http.setAuthorization(device_login, device_pass);
    return ESP8266HTTPUpdate::handleUpdate(http, currentVersion, false);
  }
};
#else
class ESPHTTPKonkerUpdate: public ESP32HTTPUpdate{
  public:
  t_httpUpdate_return update(const String& host, uint16_t port, const String& uri = "/",
                               const String& currentVersion = ""){
    HTTPClient http;
    http.begin(host, port, uri);

    Serial.println("Authorizing.."); 
    http.setAuthorization(device_login, device_pass);
    return ESP32HTTPUpdate::handleUpdate(http, currentVersion, false);
  }
};
#endif


void getVersion(String strPayload, char *version){


  if(parse_JSON_item(strPayload,"version",version)){
    Serial.println("Got version =" + String(version));
  }else{
    strcpy(version,"");
    Serial.println("Failed to parse version"); 
  }
}

void updateSucessCallBack(char *version){
  Serial.println("[update] Update ok, sending confirmation."); 
  bool subCode=0;

  String fwUpdateURL= "http://" + String(_rootDomain) + String (":") + String(_rootPort) + String("/firmware/") + String(device_login); 
  HTTPClient http;  //Declare an object of class HTTPClient
  http.begin(fwUpdateURL);  //Specify request destination
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Accept", "application/json");
  http.setAuthorization(device_login, device_pass);

  String smsg=String("{\"version\": \"" + String(version) + "\",\"status\":\"UPDATED\"}");
  int httpCode = http.sendRequest("PUT", String(smsg));


  Serial.println("Confirmantion send: " + fwUpdateURL+  + "; Body: " + smsg + "; httpcode: " + String(httpCode));
  Serial.print(">");

  http.end();   //Close connection


  Serial.print(">");

  subCode=interpretHTTPCode(httpCode);

  if (!subCode){
    Serial.println("failed");
    return;
  }else{
    Serial.println("sucess");
    return;
  }
  Serial.println("failed");
  Serial.println("");
}

bool hasUpdate(char *rootDomain,int rootPort, char *version){
  bool subCode=0;
  Serial.println("Checking for updates...");
  char buffer[100];
  char bffPort[6];
  String sPort=(String)rootPort;
  sPort.toCharArray(bffPort, 6);

  if (String(rootDomain).indexOf("http://", 0)>0){
    strcpy (buffer,rootDomain);
    strcat (buffer,":");
    strcat (buffer,bffPort);
    strcat (buffer,"/firmware/");
    strcat (buffer,device_login);

  }else{
    strcpy (buffer,"http://");
    strcat (buffer,rootDomain);
    strcat (buffer,":");
    strcat (buffer,bffPort);
    strcat (buffer,"/firmware/");
    strcat (buffer,device_login);
  }



  HTTPClient http;  //Declare an object of class HTTPClient
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(2000);
  http.setAuthorization(device_login, device_pass);
  http.begin((String)buffer);  //Specify request destination
  int httpCode = http.GET();

  Serial.println("Checking update: " + String(buffer) + "; httpcode:" + String(httpCode));
  Serial.print(">");

  subCode=interpretHTTPCode(httpCode);

  if (!subCode){
    Serial.println("failed");
    Serial.println("");
  }else{
    Serial.println("sucess");
    Serial.println("");

    String strPayload = http.getString();
    Serial.println("strPayload=" + strPayload);
    int playloadSize=http.getSize();
    http.end();   //Close connection
    if (strPayload!="[]"){
      getVersion(strPayload,version);
      return 1;
    }
  }
  strcpy(version,"");
  return 0;
}

void checkForUpdates(char *rootDomain,int rootPort, char *expectedVersion, UPDATE_SUCCESS_CALLBACK_SIGNATURE){
    if (_last_time_update_check!=0){
      //throtle this call at maximum 1 per minute
      if ((millis()-_last_time_update_check) < 60000){
          //Serial.println("checkForUpdates maximum calls is 1/minute. Please wait more to call again"); 
          return;
      }
    }


    _last_time_update_check = millis();


    char version[16];    
    if (hasUpdate(rootDomain, rootPort, version)){
      if(String(version).indexOf(String(expectedVersion))>=0 || String(version)==""){
        Serial.println("UPDATING...."); 
        ESPHTTPKonkerUpdate ESPhttpKonkerUpdate;
        ESPhttpKonkerUpdate.rebootOnUpdate(false);
        t_httpUpdate_return ret = ESPhttpKonkerUpdate.update(String(rootDomain), rootPort, String("/firmware/") + String(device_login) +String("/binary"));    
        switch(ret) {
        case HTTP_UPDATE_FAILED:
            Serial.println("[update] Update failed.");
            break;
        case HTTP_UPDATE_NO_UPDATES:
            Serial.println("[update] Update no Update.");
            break;
        case HTTP_UPDATE_OK:          
            updateSucessCallBack(version); 
            ESP.restart();
            break;
        }
      }
    }
}

void checkForUpdates(){
  char expectedVersion[16]="";

  checkForUpdates(_rootDomain, _rootPort, expectedVersion, updateSucessCallBack);
}


#endif