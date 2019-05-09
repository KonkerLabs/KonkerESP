/** 
 * KONKER LIBRARY USED TO EASILY CREATE NEW DEVICE
 * CONNECTED TO THE KONKER PLATFORM
 */
#ifndef __KONKER_H__
#define __KONKER_H__

// define device state machine

#define KONKER_DEVICE_STATE_FACTORY -1
#define KONKER_DEVICE_STATE_NETWORK 0
#define KONKER_DEVICE_STATE_OPERATIONAL 1

#define IP_DEVICE 0
#define IP_GATEWAY 1
#define IP_SUBNET 2

typedef struct  {
    char ssid[32];
    char password[64];
} Credential;

#define MAX_WIFI_CREDENTIALS 3

class WifiManager { 

    public: 

        WifiManager(char *filename) {
            strncpy(this->filename, 32, filename);
            this->load();
        }
        ~WifiManager() {};

        Credential* getCurrent() {if (current >=0) { return credential[current];} else { return null;}};
        Credential* getNext() {if (current < 3) current++; else current = 0 ; return credential[current]; };
        
        void clearCredentials() { for(int i=0; i < 3; i++) {credentials[i].ssid[0] = '\0'; credentials[i].password[0] = '\0';}};
        void addCredential(char *ssid, char *password) { 
            current++; 
            if (current > 3) 
                current =0; 
            strncpy(credentials[current].ssid, 32, ssid);
            strncpy(credentials[current].password, 64, password);
        };

        void save(); 
        void load() {

        };

        void reset() {
            // remove configuration file 
            // clear cached data 
            for (int i=0; i < MAX_WIFI_CREDENTIALS; i++) {
                credentials[i].ssid[0] = '\0';
                credentials[i].password[0] = '\0';
            }
        };

        void connect() { 
            // try to connect to the next available WIFI network 
            int status = WiFi.status(); 
            int retries = MAX_WIFI_CREDENTIALS*2;
            while (status != WL_CONNECTED && retries >= 0) {
                // reset WIFI
                WiFi.mode(WIFI_OFF);
                delay(100);
                WiFi.mode(WIFI_STA);
                delay(100);
                // reconnect with next connection 
                this->index++;
                if (this->index > MAX_WIFI_CREDENTIALS) this->index = 0;
                // if this specific WIFI credential is defined ...
                if (this->credentials[this->index].ssid[0] != '\0') { 
                    WiFi.begin(this->credentials[this->index].ssid, this->credentials[this->index].password);
                }
                retries++;
           }
        }

        String getConnectMessage(int status_code) {
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
        
        
    private: 

        Credential credentials[3];
        int current=-1;
        int index=-1;

        char filename[32];

};


public class KonkerDevice {

    public:

        class Protocol {
            public: 
                virtual void publish(char *); 
                virtual void subscribe(char *channel); 

                void secure(bool flag=true) { this.secure = flag; };

            protected:
                bool secure = false; 

        }; 

        class MQTT::Protocol {
            public:

            MQTT() {};
            ~MQTT() {};
        };

        class HTTP::Protocol {

            public:

                HTTP() {};
                ~HTTP() {};

        };

        KonkerDevice() {};
        KonkerDevice(char *name, char *passwd) {
                this->name = name;
                this->passwd = passwd;
        };
        ˜KonkerDevice() {};

        void setIP(uint8_t i0, uint8_t i1, uint8_t i2, uint8_t i3) { 
            this->setIP(IP_DEVICE, i0, i1, i2, i3); 
            this->config_ip |= 0x01;
        };

        void setGateway(uint8_t i0, uint8_t i1, uint8_t i2, uint8_t i3) { 
            this->setIP(IP_GATEWAY, i0, i1, i2, i3); 
            this->config_ip |= 0x02;
        };

        void setNetwork(uint8_t i0, uint8_t i1, uint8_t i2, uint8_t i3) { 
            this->setIP(IP_SUBNET, i0, i1, i2, i3); 
            this->config_ip |= 0x04;
        };

        bool isConfigured() {
            return this->config_ip == 0x07; 
        }

        void resetAll() {
            // clear all configuration from device 
            this->wifiConfig.reset();
        }

        void setName(char *name) {
            #ifndef ESP32
            unsigned long chipid = ESP.getChipId();
            #else
            unsigned long chipid = ESP.getEfuseMac();
            #endif
            snprintf(this->name, 32, "%6s%ld", name, chipid);
        }

        char* getName() {
            return this->name;
        }

    private:

        int state = KONKER_DEVICE_STATE_FACTORY;
        char name[32];
        char *password; 

        IPAddress ips[3];
        uint config_ip = 0;

        WifiManager wifi_manager = WifiManager();

        void initializeIPs() {
            for (int i=0; i < size(ips); i++) {
                ips[i][0] = 0;
                ips[i][1] = 0;
                ips[i][2] = 0;
                ips[i][3] = 0;
            }
        }
        
        void setIP(int index, uint8_t i0, uint8_t i1, uint8_t i2, uint8_t i3) { 
            this->ip[index][0] = i0;
            this->ip[index][1] = i1;
            this->ip[index][2] = i2;
            this->ip[index][3] = i3;
        };


    

};


#endif
