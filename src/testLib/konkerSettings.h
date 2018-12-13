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

    String getHttpPubTopic(String deviceUser, String channel);
    bool setHttpPubTopicFormat(String httpPubTopicFormat);
    String getHttpSubTopic(String deviceUser, String channel);
    bool setHttpSubTopicFormat(String httpSubTopicFormat);

    String getMqttAdress();
    bool setMqttAdress(String mqttAdress);

    int getMqttPort();
    bool setMqttPort(int mqttPort);

    String getMqttRootDomain();

    String getMqttPubTopic(String deviceUser, String channel);
    bool setMqttPubTopicFormat(String mqttPubTopicFormat);
    String getMqttSubTopic(String deviceUser, String channel);
    bool setMqttSubTopicFormat(String mqttSubTopicFormat);

    const String deviceKeyword = "<device>";
    const String channelKeyword = "<channel>";

  private:

    String _httpDomain = "";//"http://data.demo.konkerlabs.net:80";
    String _rootHttpDomain="";//"data.demo.konkerlabs.net";
    int _httpPort=80;
    String _httpPubTopicFormat = "";//"/pub/" + deviceKeyword + "/" + channelKeyword;
    String _httpSubTopicFormat = "";//"/sub/" + deviceKeyword + "/" + channelKeyword;

    String _mqttAdress = "";//"mqtt://mqtt.demo.konkerlabs.net:1883";
    String _rootMqttDomain ="";//"mqtt.demo.konkerlabs.net";
    int _mqttPort=1883;
    String _mqttPubTopicFormat = "";//"data/" + deviceKeyword + "/pub/" + channelKeyword;
    String _mqttSubTopicFormat = "";//"data/" + deviceKeyword + "/sub/" + channelKeyword;

    bool validateTopicFormat(String topicFormat, String &topicAdjusted);
};

#endif