#ifndef subChanTuple
#define subChanTuple
#include "Arduino.h"
#ifdef ESP8266
#include <functional>
#endif
#ifdef ESP8266
#define CHANNEL_CALLBACK_SIGNATURE std::function<void(uint8_t*, unsigned int)> chan_callback
#else
#define CHANNEL_CALLBACK_SIGNATURE void (*chan_callback)(uint8_t*, unsigned int)
#endif

class SubChanTuple{
  private:
    int _num_channels=0;


    struct chan_clbk_tuple{
        char chan[32]={'\0'};
        CHANNEL_CALLBACK_SIGNATURE;
    };
    typedef struct chan_clbk_tuple ChanTuple;
    ChanTuple *_sub_channels_calbacks;

  public:
    SubChanTuple();
    void copySubChannelTuple(ChanTuple &destiny, ChanTuple &origin);
    void setSubChannelTuple(ChanTuple &destiny, char const chan[], CHANNEL_CALLBACK_SIGNATURE);
    void callSubChannelCallback(char chan[32], byte* payload, unsigned int length);
    void addSubChannelTuple(char const chan[],CHANNEL_CALLBACK_SIGNATURE);
};



#endif
