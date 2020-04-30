#ifndef main_h
#define main_h

#define BUFF_SIZE 64
#define KEY_RELEASE 0
#define KEY_PRESS 1
#define MAX_BUTTON 9
#define TEXT_ALPHA_MODE 0
#define TEXT_NUM_MODE 1

#define FND_DEVICE "/dev/fpga_fnd"
#define LED_DEVICE "/dev/fpga_led"
#define FPGA_TEXT_LCD_DEVICE "/dev/fpga_text_lcd"
#define FPGA_DOT_DEVICE "/dev/fpga_dot"
unsigned char quit;
char FND[4], LED[8], TextLED[2][100], Draw_Matrix[10][7];
int dot, Count_jinsu, Count_total, Text_len, Text_mode, i, firstExec, led_mode, mode;
int y, x, curser, firstExec;

unsigned char fpga_number[11][10];

void clock_plus_hour();

void clock_plus_minute();

void Clock_FND_set_to_borad_time();

int POW(int n, int p);

void reset_para();

int p (int semid);

int v(int semid);

#endif /* Header_h */
