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


SubChanTuple::SubChanTuple(){}

void SubChanTuple::copySubChannelTuple(ChanTuple &destiny, ChanTuple &origin){
  strncpy(destiny.chan,origin.chan,32);
  destiny.chan_callback = origin.chan_callback;
}

void SubChanTuple::setSubChannelTuple(ChanTuple &destiny, char const chan[], CHANNEL_CALLBACK_SIGNATURE){
  strncpy(destiny.chan,chan,32);
  destiny.chan_callback = chan_callback;
}

void SubChanTuple::callSubChannelCallback(char chan[32], byte* payload, unsigned int length){
    int size =_num_channels;
    for(int c =0;c<size;c++){
        if(strncmp(_sub_channels_calbacks[c].chan,chan,32) == 0){
            _sub_channels_calbacks[c].chan_callback(payload,length);
        }
    }
}

void SubChanTuple::addSubChannelTuple(char const chan[],CHANNEL_CALLBACK_SIGNATURE){
    _num_channels=_num_channels+1;

    ChanTuple *new_sub_channels_calbacks=new ChanTuple[_num_channels];

    for(int c =0;c<_num_channels-1;c++){
        copySubChannelTuple(new_sub_channels_calbacks[c],_sub_channels_calbacks[c]);
    }

    setSubChannelTuple(new_sub_channels_calbacks[_num_channels-1],chan,chan_callback);

    _sub_channels_calbacks=new ChanTuple[_num_channels];
    for(int c =0;c<_num_channels;c++){
        copySubChannelTuple(_sub_channels_calbacks[c],new_sub_channels_calbacks[c]);
    }

    delete [] new_sub_channels_calbacks;
}

#endif
