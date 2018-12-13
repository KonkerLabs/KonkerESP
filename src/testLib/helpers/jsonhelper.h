#ifndef jsonhelper
#define jsonhelper

#include <ArduinoJson.h>
#include "fileHelper.h"
long long char2LL(char *str);

bool parse_JSON_item(JsonObject& jsonMSG, char *itemName, char *returnVal);

bool parse_JSONArr_item(JsonObject& jsonMSG, char *itemName, char *returnVal);


//----------------- Decodificacao da mensagem Json In -----------------------------
bool parse_JSON_item(String json, char *itemName, char *returnVal);

bool parse_JSON_dataItem_from_array(String json, char *itemName, char *returnVal);

bool parseJSON_data(String json, char *itemName, char *returnVal);

void  parse_JSON_timestamp(String json, char *chrTS, int chrTSsize);

void updateJSON(JsonObject& jsonToUpdate,  String keyNameToSave,  String itemValue);

void updateJSON(JsonObject& jsonToUpdate, JsonObject& jsonNewValues);

bool updateJsonFile(String filePath, JsonObject& jsonNewValues, unsigned int arrayIndex);

bool updateJsonFile(String filePath, String jsonString, unsigned int arrayIndex);

bool updateJsonFile(String filePath, JsonObject& jsonNewValues);

bool updateJsonFile(String filePath, String jsonString);

bool getJsonItemFromFile(String filePath, char *itemName, char *returnVal);

bool getJsonArrayItemFromFile(String filePath, unsigned int arrayIndex, char *itemName, char *returnVal);

#endif
