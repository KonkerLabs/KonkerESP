#ifndef pubsubREST
#define pubsubREST

#include "../helpers/globals.h"
#ifndef ESP32
#include <ESP8266HTTPClient.h>
#else
#include <HTTPClient.h>
#endif

unsigned long _last_time_http_request=0;
unsigned long _millis_delay_per_http_request=1000;
void buildHTTPSUBTopic(char const channel[], char *topic){
  char bffPort[6];
  itoa (_rootPort,bffPort,10);
  if (String(_rootDomain).indexOf("http://", 0)>0){
    strcpy (topic,_rootDomain);
    strcat (topic,":");
    strcat (topic,bffPort);
    strcat (topic,"/");
    strcat (topic,sub_dev_modifier);
    strcat(topic,"/");   
    strcat (topic,device_login);
    strcat(topic,"/");
    strcat (topic,channel);
  }else{
    strcpy (topic,"http://");
    strcat (topic,_rootDomain);
    strcat (topic,":");
    strcat (topic,bffPort);
    strcat (topic,"/");
    strcat (topic,sub_dev_modifier);
    strcat(topic,"/");    
    strcat (topic,device_login);
    strcat(topic,"/");
    strcat (topic,channel);
  }
}

void buildHTTPPUBTopic(char const channel[], char *topic){
  char bffPort[6];
  itoa (_rootPort,bffPort,10);
  if (String(_rootDomain).indexOf("http://", 0)>0){
    strcpy (topic,_rootDomain);
    strcat (topic,":");
    strcat (topic,bffPort);
    strcat (topic,"/");
    strcat (topic,pub_dev_modifier);
    strcat(topic,"/");   
    strcat (topic,device_login);
    strcat(topic,"/");
    strcat (topic,channel);
  }else{
    strcpy (topic,"http://");
    strcat (topic,_rootDomain);
    strcat (topic,":");
    strcat (topic,bffPort);
    strcat (topic,"/");
    strcat (topic,pub_dev_modifier);
    strcat(topic,"/");    
    strcat (topic,device_login);
    strcat(topic,"/");
    strcat (topic,channel);
  }
}



bool testHTTPSubscribeConn(){
    //throtle this call 
  if ((millis()-_last_time_http_request) < _millis_delay_per_http_request){
      delay((millis()-_last_time_http_request));
  }
  _last_time_http_request = millis();
  bool subCode=0;

  HTTPClient http;  //Declare an object of class HTTPClient
  http.addHeader("Content-Type", "application/json");
  http.setAuthorization(device_login, device_pass);

  char buffer[100];

  buildHTTPSUBTopic("test",buffer);

  http.begin((String)buffer);

  int httpCode = http.GET();

  //Serial.println("Testing HTTP subscribe to: " + url_to_call + "; httpcode:" + String(httpCode));
  //Serial.print(">");

  subCode=interpretHTTPCode(httpCode);


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



bool pubHttp(char const channel[], char const msg[]){
  //throtle this call 
  if ((millis()-_last_time_http_request) < _millis_delay_per_http_request){
      delay((millis()-_last_time_http_request));
  }
  _last_time_http_request = millis();

  bool pubCode=0;


  HTTPClient http;  //Declare an object of class HTTPClient
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Accept", "application/json");
  http.setAuthorization(device_login, device_pass);

  char buffer[100];

  buildHTTPPUBTopic(channel,buffer);

  http.begin((String)buffer);

  int httpCode=http.POST(String(msg));
  //Serial.println("Publishing to " + String(topic) + "; Body: " + String(msg) + "; httpcode: " + String(httpCode));
  //Serial.print(">");
  http.end();   //Close connection

  pubCode=interpretHTTPCode(httpCode);


  if (!pubCode){
    Serial.println("failed");
    Serial.println("");
    failedComm=1;
    return 0;
  }else{
    Serial.println("sucess");
    Serial.println("");
    return 1;
  }

}



bool subHttp(char const channel[],CHANNEL_CALLBACK_SIGNATURE){
  //throtle this call 
  if ((millis()-_last_time_http_request) < _millis_delay_per_http_request){
      delay((millis()-_last_time_http_request));
  }
  _last_time_http_request = millis();

  bool subCode=0;


  HTTPClient http;  //Declare an object of class HTTPClient
  http.addHeader("Content-Type", "application/json");
  http.setAuthorization(device_login, device_pass);

  char buffer[100];

  buildHTTPSUBTopic(channel,buffer);

  http.begin((String)buffer);

  int httpCode = http.GET();



  Serial.print(">");

  subCode=interpretHTTPCode(httpCode);
  
  if (!subCode){
    Serial.println("failed");
    Serial.println("");
    failedComm=1;
  }else{
    Serial.println("sucess");
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


#endif