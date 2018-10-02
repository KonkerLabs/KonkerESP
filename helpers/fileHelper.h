#ifndef fileHelper
#define fileHelper
bool spiffsMounted=0;
#ifdef ESP32
#include <SPIFFS.h>

bool spiffsMount(){
	if (spiffsMounted==0){
		//Serial.println("Mounting FS");
	  if (SPIFFS.begin(true)) {
	  	Serial.println("File system mounted");
			spiffsMounted=1;
			return 1;
		}else{
			Serial.println("Failed to mount file system!");
			return 0;
		}
	}else{
  	//Serial.println("File system already mounted");
		return 1;
	}
}
#else
bool spiffsMount(){
	if (spiffsMounted==0){
		//Serial.println("Mounting FS");
	  if (SPIFFS.begin()) {
	  	Serial.println("File system mounted");
			spiffsMounted=1;
			return 1;
		}else{
			Serial.println("Failed to mount file system!");
			return 0;
		}
	}else{
  	//Serial.println("File system already mounted");
		return 1;
	}
}
#endif




void formatFileSystem(){
	if(spiffsMount()){
		SPIFFS.format();
	}
}

//use me instead of openFile
bool readFile(String filePath, char *output, int initialPosition, int bytes){
	if(spiffsMount()){
		//Serial.println("File exists?: " + filePath);
		if (SPIFFS.exists(filePath)) {
			//Serial.println("File found: " + filePath);
			File foundFile = SPIFFS.open(filePath, "r");
			if (foundFile) {
				foundFile.seek(initialPosition, SeekSet);
				//Serial.println("File opened: " + filePath);
				//String contents = foundFile.readString();
				//Serial.println("Contents: " + contents);
				//contents.toCharArray(output, contents.length()+1);
				foundFile.readBytes(output, bytes);
				//Serial.println("Contents: "  + (String)output);
				foundFile.close();
				return 1;
			}else{
				//Serial.println("Error writing file: "  + filePath);
				return -1;
			}
		}else{
			//Serial.println("File not found: " + filePath);
			return 0;
		}
	}
	return -1;
}

bool readFile(String filePath, char *output){
	return readFile(filePath,output,0,1024);
}

//DEPRECATED, use readFile instead
//keeping it for retrocompatibility
bool openFile(String filePath, char *output){
	return  readFile(filePath,output);
};
bool openFile(String filePath, char *output)   __attribute__ ((deprecated("openFile is deprecated. Use readFile instead!")));



bool saveFile(String filePath, char *dataToSave){
	if(spiffsMount()){
		File myFile = SPIFFS.open(filePath, "w");
		if (!myFile) {
			Serial.println("Failed to open file to save");
			return 0;
		}else{
			myFile.seek(0, SeekSet);

			Serial.println("Saving content: "+ (String)dataToSave);

			myFile.print(dataToSave);
			myFile.close();
			Serial.println("File saved!");
			return 1;
		}
	}else{
		Serial.println("Failed to mount file system");
		return 0;
	}
}


bool appendToFile(String filePath, char *dataToSave, int position){
	if(spiffsMount()){
		File myFile = SPIFFS.open(filePath, "r+");
		if (!myFile) {
			Serial.println("Failed to open file to write");
			return 0;
		}else{
			myFile.seek(position, SeekSet);
			Serial.println("Saving content: "+ (String)dataToSave);

			myFile.print(dataToSave);
			myFile.close();
			Serial.println("File saved!");
			return 1;
		}
	}else{
		Serial.println("Failed to mount file system");
		return 0;
	}
}


//Same as appendToFile, keeping writeFile name for retrocompatibility
//DEPRECATED, use appendToFile instead
bool writeFile(String filePath, char *dataToSave, int position){
	return appendToFile(filePath, dataToSave, position);
}
bool writeFile(String filePath, char *dataToSave, int position)   __attribute__ ((deprecated("writeFile is deprecated. Use appendToFile instead!")));


bool replaceFile(String filePath, String dataToSave){
	if(spiffsMount()){
		if (SPIFFS.exists(filePath)) {
			SPIFFS.remove(filePath);
		}
		Serial.println("Creating new file: " + filePath);
		File myFile = SPIFFS.open(filePath, "w");
		if (!myFile) {
			Serial.println("Failed to open file to write");
			return 0;
		}else{

			Serial.println("Saving content: "+ dataToSave);

			myFile.print(dataToSave);
			myFile.close();
			Serial.println("File created!");
			return 1;
		}
	}
	Serial.println("Failed to open file to write");
	return 0;
}


#endif
