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
unsigned char quit = 0;


char FND[4], LED[8], TextLED[2][100], Draw_Matrix[10][7];
int dot, Count_jinsu, Count_total, Text_len, Text_mode = TEXT_ALPHA_MODE, i, firstExec, led_mode, mode;
int y, x, curser, firstExec;

unsigned char fpga_number[11][10] = {
	{ 0x3e,0x7f,0x63,0x73,0x73,0x6f,0x67,0x63,0x7f,0x3e }, // 0
{ 0x0c,0x1c,0x1c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x1e }, // 1
{ 0x7e,0x7f,0x03,0x03,0x3f,0x7e,0x60,0x60,0x7f,0x7f }, // 2
{ 0xfe,0x7f,0x03,0x03,0x7f,0x7f,0x03,0x03,0x7f,0x7e }, // 3
{ 0x66,0x66,0x66,0x66,0x66,0x66,0x7f,0x7f,0x06,0x06 }, // 4
{ 0x7f,0x7f,0x60,0x60,0x7e,0x7f,0x03,0x03,0x7f,0x7e }, // 5
{ 0x60,0x60,0x60,0x60,0x7e,0x7f,0x63,0x63,0x7f,0x3e }, // 6
{ 0x7f,0x7f,0x63,0x63,0x03,0x03,0x03,0x03,0x03,0x03 }, // 7
{ 0x3e,0x7f,0x63,0x63,0x7f,0x7f,0x63,0x63,0x7f,0x3e }, // 8
{ 0x3e,0x7f,0x63,0x63,0x7f,0x3f,0x03,0x03,0x03,0x03 }, // 9
{ 0x1c,0x36,0x63,0x63,0x63,0x7f,0x7f,0x63,0x63,0x63 } // A
};


void clock_plus_hour();

void clock_plus_minute();

void Clock_FND_set_to_borad_time();

int POW(int n, int p);

void reset_para();

#endif /* Header_h */
