#ifndef secureCheck
#define secureCheck
//Definindo os objetos de Wifi e pubsubclient

bool checkSecureFingerprint(const char* fingerprint, const char* secureServer){
  if (espClient.verify(fingerprint, secureServer)) {
    Serial.println("Connection secure.");
		return 1;
  } else {
    Serial.println("Connection insecure! Halting execution.");
   	return 0;
  }
}

bool checkSecureFingerprint(const char* fingerprint){
  if (espClient.verify(fingerprint, server)) {
    Serial.println("Connection secure.");
		return 1;
  } else {
    Serial.println("Connection insecure! Halting execution.");
   	return 0;
  }
}

#endif