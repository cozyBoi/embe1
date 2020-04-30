#ifndef output_h
#define output_h



typedef struct packet{
    char FND[4];
    char LED[8];
    char TextLED[2][100];
    char Draw_Matrix[10][7];
    int mode;
    int led_mode;
    int Text_len;
    int Text_mode;
    int curser;
    int y;
    int x;
};


void out_to_FND(char data[4]);

void out_to_LED(char data_arr[8]);

void out_to_LCD(char str[100], int len);

void out_to_Matrix_alpha(int mode);

unsigned char arr_to_int(char arr[7]);

void out_to_Matrix(char matrix[10][7]);


#endif /* Header_h */
