#ifndef input_h
#define input_h



typedef struct in_packet{
    int type;
    int value;
    int code;
    unsigned char push_sw_buff[9];
};


void entry_input();

#endif /* Header_h */
