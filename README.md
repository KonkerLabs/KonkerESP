# KonkerESP

#### https://github.com/KonkerLabs/KonkerESPExamples

Include konker.h or konkerMQTT.h  to work with this library

Initilize the konker lib at setup

konkerConfig(<root server>,<model prefix>,<encriptation flag>);

<root server> : domain / ip of the Konker plataform

Example: (char*)"data.demo.konkerlabs.net:80"

<model prefix>: A user defined prefix to especify the type of the device, for example LIGHT, MOTOR, etc.

Example: (char*)"S0101"

<encriptation flag>: Boolean flag.  If set true, the WiFi password will be expected to be sent encripted to the device.

The encriptation keys is SSID and the Konker password  of the device. To understand better please open the file konker.k and verify the getWifiCredentialsEncripted function

If set false, the password has to be sent in plain text.

Example:

void setup(){
    Serial.begin(115200);
    Serial.println("Setup");

    konkerConfig((char*)"data.demo.konkerlabs.net:80",(char*)"S0101",false);

    Serial.println("Setup finished");
}

After the setup in the main loop put:

konkerLoop();

Example:

void loop(){

    konkerLoop();

}


You could now use  the functions:

pubHTTP(<channel>, <message>)

pubMQTT(<channel>, <message>)  <-- to use MQTT functions you have to include konkerMQTT.h

Example:

StaticJsonBuffer<220> jsonBuffer;
JsonObject& jsonMSG = jsonBuffer.createObject();


delay(10);

jsonMSG["deviceId"] = (String)getChipId();
jsonMSG["p"] = presenceCount;

char bufferJ[1024];
jsonMSG.printTo(bufferJ, sizeof(bufferJ));
char mensagemjson[1024];
strcpy(mensagemjson,bufferJ);

if(!pubMQTT(status_channel, mensagemjson)){
    Serial.println("Message plublished");
}else{
    Serial.println("Failed to publish message");
}

subHTTP(<channel>, <callback function for this channel>)  <-- subscriptions in HTTP are a GET request. To verify if the value in the channel had changed you have to make polling.

subHTTP usually have to be in a loop or schedulled funcion.


subMQTT(<channel>, <callback function for this channel>)  <-- to use MQTT functions you have to include konkerMQTT.h. Since MQTT subscriptions stays listenning to the channel, no polling is needed.

The callback function will be automatically called when some new value arrives at the channel.

The callback function must have this format:

void function_name(byte* payload, unsigned int length){

Device credentials and setup

PROCEDURE
1 - THE DEVICE HAS TO BE CONFIGURED IN FACTORY WITH THE KONKER PLATFORM CREDENTIALS
  THE DEVICE SEARCH FOR wifi: KonkerDevNetwork  password: konkerkonker123
  when connected, the device will search for the platform credentials file in the root folder from the host hotspot Example: \S010113610232
  THE FILE HAS THIS FORMAT EXAMPLE:
    {"srv":"mqtt.demo.konkerlabs.net","prt":"1883","usr":"jnu56qt1bb1i","pwd":"3S7usR9g5K","prx":"data"}

2 -THE DEVICE WILL REBOOT, AND CREATE A HOTSPOT WITH ITS NAME, EXAMPLE: S010113610232
   CONNECT TO THE HOTSPOT AND MAKE A GET REQUEST LIKE THE EXAMPLE BELOW TO SEND THE WIFI Credentials
   You could save up to 3 diferrent wifi credentials for 3 different wifi
   http://192.168.4.1/wifisave?s0=SSID_NAME&p0=ENCRIPTED_SSID_PASSWORD
   More than one credential will be
   http://192.168.4.1/wifisave?s0=SSID_NAME1&p0=ENCRIPTED_SSID_PASSWORD1&s1=SSID_NAME2&p1=ENCRIPTED_SSID_PASSWORD2
      OR IF konkerConfig encription flag is off
   http://192.168.4.1/wifisave?s0=SSID_NAME&p0=SSID_PASSWORD
   
 
   
   
   
   
   
 #Firmware updates
 
 For firmware updates check just call the function: checkForUpdates();
 Tip, dont leave the execution of checkForUpdates(); in the main loop.  Call this function hourly or dayly for example.
