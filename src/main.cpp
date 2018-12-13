#include "./testLib/konkerDevice.h"


KonkerDevice device("name");


void setup(){
    device.setup();
}
void loop(){
    device.mqttLoop();
    String jhgha;
}