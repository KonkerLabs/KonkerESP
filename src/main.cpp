#include "./testLib/konkerDevice.h"


KonkerDevice* device = KonkerDevice::getInstance("name");


void setup(){
    device->setup();
}
void loop(){
    device->mqttLoop();
    String jhgha;
}