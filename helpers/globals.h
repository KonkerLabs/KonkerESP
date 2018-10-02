#ifndef globals
#define globals

#include <FS.h>
#ifndef ESP32
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif
#include <DNSServer.h>
#ifndef ESP32
#include <ESP8266WebServer.h>
#else
#include <WebServer.h>
#endif

#include <ArduinoJson.h>


bool shouldSaveConfig = false;
bool failedComm=-1;
char server[64];
int port;
char NAME[6]="S0000";
char ChipId[32];
char device_login[32];
char device_pass[32];
const char sub_dev_modifier[4] = "sub";
const char pub_dev_modifier[4] = "pub";
char prefix[5] = "data";
char _health_channel[] = "_health";
char _rootDomain[64]="data.demo.konkerlabs.net";
int _rootPort=80;

char *getChipId(){
  return  ChipId;
}

bool interpretHTTPCode(int httpCode){
  
  if (httpCode > 0 && httpCode<300) { //Check the returning code
    return 1;

  }else{
    Serial.println(String(httpCode));
    return 0;
  }
}

#include "../helpers/subChanTuple.h"

#endif