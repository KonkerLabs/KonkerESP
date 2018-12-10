/*
  Morse.h - Library for flashing Morse code.
  Created by David A. Mellis, November 2, 2007.
  Released into the public domain.
*/
#ifndef KonkerSettings_h
#define KonkerSettings_h

#include "Arduino.h"

class KonkerSettings{
  public:
    KonkerSettings();
    KonkerSettings(String httpDomain, String httpPubTopicFormat, String httpSubTopicFormat);
    KonkerSettings(String httpDomain, String httpPubTopicFormat, String httpSubTopicFormat,
                 String mqttAdress, String mqttPubTopicFormat, String mqttSubTopicFormat);

    String getHttpDomain();
    bool setHttpDomain(String httpDomain);

    String getHttpRootDomain();

    int getHttpPort();
    bool setHttpPort(int httpPort);

    String getHttpPubTopicFormat();
    bool setHttpPubTopicFormat(String httpPubTopicFormat);
    String getHttpSubTopicFormat();
    bool setHttpSubTopicFormat(String httpSubTopicFormat);

    String getMqttAdress();
    bool setMqttAdress(String mqttAdress);

    int getMqttPort();
    bool setMqttPort(int mqttPort);

    String getMqttRootDomain();

    String getMqttPubTopicFormat();
    bool setMqttPubTopicFormat(String mqttPubTopicFormat);
    String getMqttSubTopicFormat();
    bool setMqttSubTopicFormat(String mqttSubTopicFormat);


  private:
    String _deviceKeyword = "<device>";
    String _channelKeyword = "<channel>";

    String _httpDomain = "";//"http://data.demo.konkerlabs.net:80";
    String _rootHttpDomain="";//"data.demo.konkerlabs.net";
    int _httpPort=80;
    String _httpPubTopicFormat = "";//"/pub/" + _deviceKeyword + "/" + _channelKeyword;
    String _httpSubTopicFormat = "";//"/sub/" + _deviceKeyword + "/" + _channelKeyword;

    String _mqttAdress = "";//"mqtt://mqtt.demo.konkerlabs.net:1883";
    String _rootMqttDomain ="";//"mqtt.demo.konkerlabs.net";
    int _mqttPort=1883;
    String _mqttPubTopicFormat = "";//"data/" + _deviceKeyword + "/pub/" + _channelKeyword;
    String _mqttSubTopicFormat = "";//"data/" + _deviceKeyword + "/sub/" + _channelKeyword;

    bool validateTopicFormat(String topicFormat, String &topicAdjusted);
};

KonkerSettings::KonkerSettings(){
}

KonkerSettings::KonkerSettings(String httpDomain, String httpPubTopicFormat, String httpSubTopicFormat){
  setHttpDomain(httpDomain) && setHttpPubTopicFormat(httpPubTopicFormat) && setHttpSubTopicFormat(httpSubTopicFormat);
}

KonkerSettings::KonkerSettings(String httpDomain, String httpPubTopicFormat, String httpSubTopicFormat,
                 String mqttAdress, String mqttPubTopicFormat, String mqttSubTopicFormat){

  setHttpDomain(httpDomain) && setHttpPubTopicFormat(httpPubTopicFormat) && setHttpSubTopicFormat(httpSubTopicFormat);
  setMqttAdress(httpDomain) && setMqttPubTopicFormat(httpPubTopicFormat) && setMqttSubTopicFormat(httpSubTopicFormat);

}

bool KonkerSettings::validateTopicFormat(String topicFormat, String &topicAdjusted){
    if(topicFormat.length()<1){
      return 0;
    }

    //add 1st slash if not exists
    if(topicFormat.charAt(0)!='/'){
      topicAdjusted="/" + topicFormat;
    }

    int i = topicFormat.indexOf(_deviceKeyword);
    if(i==-1){//not found _deviceKeyword
      return 0;
    }

    i = topicFormat.indexOf(_channelKeyword);
    if(i==-1){//not found _channelKeyword
      return 0;
    }
   return 1;
}

String KonkerSettings::getHttpDomain(){
   return _httpDomain;
}

bool KonkerSettings::setHttpDomain(String httpDomain){
    //"http://data.demo.konkerlabs.net:80"

    if(httpDomain.length()<4){
      return 0;
    }

    //remove the last slash if exists
    if(httpDomain.charAt(httpDomain.length()-1)=='/'){
      httpDomain=httpDomain.substring(0,httpDomain.length()-1);
    }

    int i = httpDomain.indexOf("://");
    int domainStart=i+3;
    if(i!=4 && i!=5){//if not set, assume http://
      httpDomain= "http://" + httpDomain;
      i=4;
      domainStart=7;
    }
    i =httpDomain.indexOf(":",i+1);//check if port is set
    if(i==-1){
      i =httpDomain.indexOf("https");
      if(i==0){
        setHttpPort(443);
      }else{
        setHttpPort(80);
      }
      _rootHttpDomain=httpDomain.substring(domainStart);
    }else{
      setHttpPort(httpDomain.substring(i+1).toInt());
      _rootHttpDomain=httpDomain.substring(domainStart,i);
    }

   _httpDomain = httpDomain;
   return 1;
}

String KonkerSettings::getHttpRootDomain(){
   return _rootHttpDomain;
}

String KonkerSettings::getHttpPubTopicFormat(){
   return _httpPubTopicFormat;
}

bool KonkerSettings::setHttpPubTopicFormat(String httpPubTopicFormat){
  return validateTopicFormat(httpPubTopicFormat,_httpPubTopicFormat);
}

int KonkerSettings::getHttpPort(){
   return _httpPort;
}

bool KonkerSettings::setHttpPort(int httpPort){
   _httpPort = httpPort;
   return 1;
}

String KonkerSettings::getHttpSubTopicFormat(){
   return _httpSubTopicFormat;
}

bool KonkerSettings::setHttpSubTopicFormat(String httpSubTopicFormat){
  return validateTopicFormat(httpSubTopicFormat,_httpSubTopicFormat);
}

String KonkerSettings::getMqttAdress(){
   return _mqttAdress;
}

bool KonkerSettings::setMqttAdress(String mqttAdress){
    //"mqtt://mqtt.demo.konkerlabs.net:1883"

    if(mqttAdress.length()<4){
      return 0;
    }

    //remove the last slash if exists
    if(mqttAdress.charAt(mqttAdress.length()-1)=='/'){
      mqttAdress=mqttAdress.substring(0,mqttAdress.length()-1);
    }

    int i = mqttAdress.indexOf("://");
    int domainStart=i+3;
    if(i!=4 && i!=5){//if not set, assume mqtt://
      mqttAdress= "mqtt://" + mqttAdress;
      i=4;
      domainStart=7;
    }
    i =mqttAdress.indexOf(":",i+1);//check if port is set
    if(i==-1){
      i =mqttAdress.indexOf("mqtts");
      if(i==0){
        setMqttPort(8883);
      }else{
        setMqttPort(1883);
      }
      _rootMqttDomain=mqttAdress.substring(domainStart);
    }else{
      setMqttPort(mqttAdress.substring(i+1).toInt());
      _rootMqttDomain=mqttAdress.substring(domainStart,i);
    }
   _mqttAdress = mqttAdress;
   return 1;
}

String KonkerSettings::getMqttRootDomain(){
   return _rootMqttDomain;
}

int KonkerSettings::getMqttPort(){
   return _mqttPort;
}

bool KonkerSettings::setMqttPort(int mqttPort){
   _mqttPort = mqttPort;
   return 1;
}

String KonkerSettings::getMqttPubTopicFormat(){
   return _mqttPubTopicFormat;
}

bool KonkerSettings::setMqttPubTopicFormat(String mqttPubTopicFormat){
  return validateTopicFormat(mqttPubTopicFormat,_mqttPubTopicFormat);
}

String KonkerSettings::getMqttSubTopicFormat(){
   return _mqttSubTopicFormat;
}

bool KonkerSettings::setMqttSubTopicFormat(String mqttSubTopicFormat){
  return validateTopicFormat(mqttSubTopicFormat,_mqttSubTopicFormat);
}

#endif