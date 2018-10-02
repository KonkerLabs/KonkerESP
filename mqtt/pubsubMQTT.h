#ifndef pubsubMQTT
#define pubsubMQTT

#include "../helpers/globals.h"
#include <PubSubClient.h>
#include "./management/sendHealth.h"
#include "./helpers/fileHelper.h"

#ifdef konkerMQTTs
  WiFiClientSecure espClient;
  #include "secureCheck.h"
#endif
#ifndef konkerMQTTs
	WiFiClient espClient;
#endif


PubSubClient client(espClient);

void MQTTLoop(){
  client.loop();
}

void callback(char* topic, byte* payload, unsigned int length) {
  int i;
  int state=0;

  Serial.print("Message received [");
  Serial.print(topic);
  Serial.print("] ");

  callSubChannelCallback(topic, payload, length);

}


bool connectMQTT(char r_server[], int r_port, char r_device_login[], char r_device_pass[]){
    if(client.connected()){
      Serial.print("Already connected to MQTT broker ");
      //return 1;
    }else{
      Serial.print("Trying to connect to MQTT broker ");
    }
    


	  Serial.print(" URI:" + String(r_server) + " Port:" + String(r_port) + ", ");

	  client.setServer(r_server, r_port);
	  client.setCallback(callback);

	  Serial.print(" U:" + String(r_device_login) + " P:" + String(r_device_pass));
	  
		int connectCode=client.connect(r_device_login, r_device_login, r_device_pass);

		Serial.println(", connectCode=" + String(connectCode));


    if (connectCode==1) { //Check the returning code
      Serial.println("sucess");
      Serial.println("");
      return 1;
    }else{
      Serial.println("failed");
      Serial.println("");
      appendToFile(healthFile,(char*)"1", _mqttFailureAdress);
      delay(3000);
      #ifndef ESP32
      ESP.reset();
      #else
      ESP.restart();
      #endif
      return 0;
		}
}


bool connectMQTT(){
	return connectMQTT(server,port,device_login,device_pass);
}


void buildMQTTSUBTopic(char const channel[], char *topic){
    strcpy (topic,prefix);
    strcat (topic,"/");
    strcat (topic,device_login);
    strcat (topic,"/");
    strcat (topic,sub_dev_modifier);
    strcat(topic,"/");    
    strcat (topic,channel);
}

void buildMQTTPUBTopic(char const channel[], char *topic){
    strcpy (topic,prefix);
    strcat (topic,"/");
    strcat (topic,device_login);
    strcat (topic,"/");
    strcat (topic,pub_dev_modifier);
    strcat(topic,"/");    
    strcat (topic,channel);
}





bool pubMQTT(char const channel[], char const msg[]){
  int pubCode=-1;
  char topic[32];

  buildMQTTPUBTopic(channel, topic);

  //Serial.println("Publishing to: " + String(topic) + " msg: " + msg );
  //Serial.print(">");
  delay(200);
  pubCode=client.publish(topic, msg);

  if (pubCode!=1){
    Serial.println("failed");
    Serial.println("pubCode:" + (String)pubCode);
    failedComm=1;
    appendToFile(healthFile,(char*)"1", _mqttFailureAdress);
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



bool subMQTT(char const channel[],CHANNEL_CALLBACK_SIGNATURE){
  int subCode=-1;
  char topic[32];

  buildMQTTSUBTopic(channel, topic);
  
  //Serial.println("Subscribing to: " + String(topic));
  //Serial.print(">");
  delay(200);
  subCode=client.subscribe(topic);

  if (subCode!=1){
    Serial.println("failed");
    Serial.println("");
    failedComm=1;
    appendToFile(healthFile,(char*)"1", _mqttFailureAdress);
    delay(3000);
    #ifndef ESP32
    ESP.reset();
    #else
    ESP.restart();
    #endif
    return 0;
  }else{
    addSubChannelTuple(topic,chan_callback);
    Serial.println("sucess");
    Serial.println("");
    return 1;
  }

}

#endif
