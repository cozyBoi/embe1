#ifndef input_h
#define input_h



typedef struct in_packet{
    int type;
    int value;
    int code;
    unsigned char push_sw_buff[9];
};

int pp (int semid);

int vv(int semid);

void entry_input();

#endif /* Header_h */
