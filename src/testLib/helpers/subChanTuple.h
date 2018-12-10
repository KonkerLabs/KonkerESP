#ifndef subChanTuple
#define subChanTuple


#ifdef ESP8266
#include <functional>
#define CHANNEL_CALLBACK_SIGNATURE std::function<void(uint8_t*, unsigned int)> chan_callback
#else
#define CHANNEL_CALLBACK_SIGNATURE void (*chan_callback)(uint8_t*, unsigned int)
#endif


struct chan_clbk_tuple
{
   char chan[32]={'\0'};
   CHANNEL_CALLBACK_SIGNATURE;
};

typedef struct chan_clbk_tuple ChanTuple;

ChanTuple *sub_channels_calbacks;
int num_channels=0;


void copySubChannelTuple(ChanTuple &destiny, ChanTuple &origin){
  strncpy(destiny.chan,origin.chan,32);
  destiny.chan_callback = origin.chan_callback;
}


void setSubChannelTuple(ChanTuple &destiny, char const chan[], CHANNEL_CALLBACK_SIGNATURE){
  strncpy(destiny.chan,chan,32);
  destiny.chan_callback = chan_callback;
}



void callSubChannelCallback(char chan[32], byte* payload, unsigned int length){
    int size =num_channels;
    for(int c =0;c<size;c++){
        if(strncmp(sub_channels_calbacks[c].chan,chan,32) == 0){
            sub_channels_calbacks[c].chan_callback(payload,length);
        }
    }
}


void addSubChannelTuple(char const chan[],CHANNEL_CALLBACK_SIGNATURE){
    num_channels=num_channels+1;

    ChanTuple *new_sub_channels_calbacks=new ChanTuple[num_channels];

    for(int c =0;c<num_channels-1;c++){
        copySubChannelTuple(new_sub_channels_calbacks[c],sub_channels_calbacks[c]);
    }

    setSubChannelTuple(new_sub_channels_calbacks[num_channels-1],chan,chan_callback);

    sub_channels_calbacks=new ChanTuple[num_channels];
    for(int c =0;c<num_channels;c++){
        copySubChannelTuple(sub_channels_calbacks[c],new_sub_channels_calbacks[c]);
    }

    delete [] new_sub_channels_calbacks;
}
#endif