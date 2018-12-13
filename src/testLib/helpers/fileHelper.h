#ifndef fileHelper
#define fileHelper
#include <FS.h>
#ifdef ESP32
#include <SPIFFS.h>
#endif
bool spiffsMount();




void formatFileSystem();


//use me instead of openFile
bool readFile(String filePath, char *output, int initialPosition, int bytes);

bool readFile(String filePath, char *output);

//DEPRECATED, use readFile instead
//keeping it for retrocompatibility
bool openFile(String filePath, char *output);



bool saveFile(String filePath, char *dataToSave);


bool appendToFile(String filePath, char *dataToSave, int position);


//Same as appendToFile, keeping writeFile name for retrocompatibility
//DEPRECATED, use appendToFile instead
bool writeFile(String filePath, char *dataToSave, int position);


bool replaceFile(String filePath, String dataToSave);


#endif
