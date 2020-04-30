#include "main.h"
#include "input.h"
#include "output.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>
#include <termios.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <time.h>

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

unsigned char quit = 0;

void user_signal1(int sig)
{
	quit = 1;
}



void clock_plus_hour() {
	FND[1]++;
	if (FND[1] == 10) {
		FND[1] = 0;
		FND[0]++;
	}
	if (FND[0] == 2 && FND[1] == 4) {
		FND[0] = FND[1] = 0;
	}
}

void clock_plus_minute() {
	FND[3]++;
	if (FND[3] == 10) {
		FND[3] = 0;
		FND[2]++;
	}
	if (FND[2] == 6) {
		FND[2] = 0;
		clock_plus_hour();
	}
}

void Clock_FND_set_to_borad_time(){
    time_t rawtime;
    char buffer[50] ={0};
    time(&rawtime);
    sprintf (buffer, "%s", ctime(&rawtime) );
    printf("curr time : %s\n", buffer);
    int i = 0;
    for(i = 0; 1; i++){
        if(buffer[i] == ':') break;
    }
    FND[0] = buffer[i-2] - 0x30;
    FND[1] = buffer[i-1] - 0x30;
    //skip 2 which is ':'
    FND[2] = buffer[i+1] - 0x30;
    FND[3] = buffer[i+2] - 0x30;
}

int POW(int n, int p){
    int i = 0;
    int ret = 1;
    
    for(i = 0; i < p; i++){
        ret *= n;
    }
    
    return ret;
}

void reset_para() {
    int i = 0,j = 0;
    for (i = 0; i < 4; i++) FND[i] = 0;
    for (i = 0; i < 8; i++) LED[i] = 0;
    for (i = 0; i < 100; i++) TextLED[0][i] = 0;
    for (i = 0; i < 100; i++) TextLED[1][i] = 0;
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 7; j++) {
            Draw_Matrix[i][j] = 0;
        }
    }
    out_to_Matrix(Draw_Matrix);
    out_to_FND(FND);
    out_to_LCD(TextLED[0], 8);
    out_to_LED(LED);
    Count_total = dot = 0;
    Count_jinsu = 10;
    Text_len = 1;
    Text_mode = TEXT_ALPHA_MODE;
    y = x = 0;
    curser = 0;
    firstExec = 1;
    out_to_FND(FND);
    out_to_LCD(TextLED[0], 8);
    out_to_LED(LED);
    out_to_Matrix(Draw_Matrix);
}

int main() {
    dot = 0;
    Count_jinsu = 10;
    Count_total = 0;
    Text_len = 1;
    Text_mode = TEXT_ALPHA_MODE;firstExec = 1;
    curser = 0;
	int mode = 0;
	struct input_event ev[BUFF_SIZE];
	int fd, rd, value, size = sizeof(struct input_event);
	char name[256] = "Unknown";
	char* device = "/dev/input/event0";
	if ((fd = open(device, O_RDONLY | O_NONBLOCK)) == -1) {
		printf("%s is not a vaild device.n", device);
	}
	//event0 open
    int j = 0, led_mode = 0;
	int i, k;
	int dev;
	int buff_size;
    unsigned char push_sw_buff[MAX_BUTTON];
    unsigned char prev_push_sw_buff[MAX_BUTTON];

	dev = open("/dev/fpga_push_switch", O_RDWR);
	if (dev<0) {
		printf("Device Open Error\n");
		close(dev);
		return -1;
	}
	(void)signal(SIGINT, user_signal1);
	buff_size = sizeof(push_sw_buff);
	//dev open
	int pid = (int)fork();
    if(pid != 0){
        int ppid = fork();
        if(ppid != 0){
            entry_output();
        }
        else {
            entry_input();
        }
    }
    int semid, semid0;
    semid = semget ((key_t)12345, 1, 0666 | IPC_CREAT);
    semid0 = semget ((key_t)12346, 1, 0666 | IPC_CREAT);
    key_t key0 = ftok("./", 1);
    struct input_event*shmaddr_ev;
    unsigned char *shmaddr_sw;
    int shmid = shmget(key0, sizeof(struct in_packet), IPC_CREAT|0644);
    struct in_packet*shmaddr = (struct in_packet*)shmat(shmid, NULL, 0);
    memset(shmaddr, 0, sizeof(struct packet));
    
    key_t key2 = ftok("./", 3);
    int shmid_2 = shmget(key2, sizeof(struct packet), IPC_CREAT|0644);
    struct packet*shmaddr_2 = (struct packet*)shmat(shmid_2, NULL, 0);
    memset(shmaddr_2, 0, sizeof(struct packet));
    
	while (!quit) {
		rd = read(fd, ev, size * BUFF_SIZE);
		//event0 read

        while (ev[0].type == 1 && ev[0].value == KEY_PRESS && ev[0].code == 115) {
			//volume +, mode change
            rd = read(fd, ev, size * BUFF_SIZE);
            if (ev[0].type == 1 && ev[0].value == KEY_RELEASE && ev[0].code == 115) {
                mode = (mode + 1) % 4;
                reset_para();
                printf("mode : %d\n", mode);
            }
		}
		while (ev[0].type == 1 && ev[0].value == KEY_PRESS && ev[0].code == 114) {
			//volume -, mode change
            rd = read(fd, ev, size * BUFF_SIZE);
            if (ev[0].type == 1 && ev[0].value == KEY_RELEASE && ev[0].code == 114) {
                mode = mode ? mode - 1 : 3;
                reset_para();
                printf("mode : %d\n", mode);
            }
		}
        //printf("before read\n");
        
        
        memcpy(prev_push_sw_buff, push_sw_buff, buff_size);
		read(dev, &push_sw_buff, buff_size);
        //printf("after read\n");
        
        usleep(100000);
		if (mode == 0) {
			//boradÀÇ ½Ã°£À» °¡Á®¿Í¾ßÇÔ
            if(firstExec){
                Clock_FND_set_to_borad_time();
                firstExec = 0;
            }
			if (push_sw_buff[0] == 1 && prev_push_sw_buff[0] != 1) {
				Text_mode = ~Text_mode;
                push_sw_buff[0] = 0;
			}
			else if (push_sw_buff[1] == 1) {
                Clock_FND_set_to_borad_time();
                push_sw_buff[1] = 0;
			}
			else if (push_sw_buff[2] == 1 && Text_mode) {
                clock_plus_hour();
                printf("plus hour\n");
                push_sw_buff[2] = 0;
			}
			else if (push_sw_buff[3] == 1 && Text_mode) {
				clock_plus_minute();
                printf("plus minute\n");
                push_sw_buff[3] = 0;
			}
            out_to_FND(FND);
		}
		else if (mode == 1) {
			if (push_sw_buff[0] == 1) {
				Count_jinsu = Count_jinsu - 2 ? Count_jinsu - 2 : 10;
                if(Count_jinsu == 6) Count_jinsu -= 2;
			}
			else if (push_sw_buff[1] == 1) {
				Count_total += Count_jinsu * Count_jinsu;
			}
			else if (push_sw_buff[2] == 1) {
				Count_total += Count_jinsu;
			}
			else if (push_sw_buff[3] == 1) {
				Count_total += 1;
			}
			//display
			FND[0] = Count_total / POW(Count_jinsu, 3)
            - (Count_total / POW(Count_jinsu, 4)) * POW(Count_jinsu, 1);
			FND[1] = (Count_total / POW(Count_jinsu, 2))
            - (Count_total / POW(Count_jinsu, 3)) * POW(Count_jinsu, 1);
			FND[2] = (Count_total / POW(Count_jinsu, 1))
            - (Count_total / POW(Count_jinsu, 2)) * POW(Count_jinsu, 1);
			FND[3] = (Count_total)
            - (Count_total / POW(Count_jinsu, 1)) * POW(Count_jinsu, 1);
			out_to_FND(FND);
		}
		else if (mode == 2) {
            
			int curr = Text_len - 1;
			if (push_sw_buff[1] == 1 && push_sw_buff[2] == 1) {
				for (i = 0; i < 10; i++) TextLED[0][i] = 0;
				Text_len = 1;
				Count_total++;
			}
			else if (push_sw_buff[4] == 1 && push_sw_buff[5] == 1) {
                if(Text_mode == 0) Text_mode = 1;
                else Text_mode = 0;
                printf("Text_mode : %d\n", Text_mode);
                Count_total+=2;
			}
			else if (push_sw_buff[7] == 1 && push_sw_buff[8] == 1) {
				TextLED[0][Text_len] = ' ';
				Text_len++;
                Count_total+=2;
			}
			else if (push_sw_buff[0] == 1) {
				if (Text_mode == TEXT_NUM_MODE) {
					TextLED[0][Text_len] = '1';
					Text_len++;
				}
				else if (TextLED[0][curr] == 0) {
					TextLED[0][curr] = '.';
				}
				else if (TextLED[0][curr] == '.') {
					TextLED[0][curr] = 'Q';
				}
				else if (TextLED[0][curr] == 'Q') {
					TextLED[0][curr] = 'Z';
				}
				else if (TextLED[0][curr] == 'Z') {
					TextLED[0][curr] = '.';
				}
				else{
					//curr is not 
					TextLED[0][Text_len] = '.';
					Text_len++;
				}
                Count_total++;
			}
			else if (push_sw_buff[1] == 1) {
				if (Text_mode == TEXT_NUM_MODE) {
					TextLED[0][Text_len] = '2';
					Text_len++;
				}
				else if (TextLED[0][curr] == 0) {
					TextLED[0][curr] = 'A';
				}
				else if (TextLED[0][curr] == 'A') {
					TextLED[0][curr] = 'B';
				}
				else if (TextLED[0][curr] == 'B') {
					TextLED[0][curr] = 'C';
				}
				else if (TextLED[0][curr] == 'C') {
					TextLED[0][curr] = 'A';
				}
				else {
					//curr is not us
					TextLED[0][Text_len] = 'A';
					Text_len++;
				}
                Count_total++;
			}
			else if (push_sw_buff[2] == 1) {
				if (Text_mode == TEXT_NUM_MODE) {
					TextLED[0][Text_len] = '3';
					Text_len++;
				}
				else if (TextLED[0][curr] == 0) {
					TextLED[0][curr] = 'D';
				}
				else if (TextLED[0][curr] == 'D') {
					TextLED[0][curr] = 'E';
				}
				else if (TextLED[0][curr] == 'E') {
					TextLED[0][curr] = 'F';
				}
				else if (TextLED[0][curr] == 'F') {
					TextLED[0][curr] = 'D';
				}
				else {
					//curr is not 
					TextLED[0][Text_len] = 'D';
					Text_len++;
				}
                Count_total++;
			}
			else if (push_sw_buff[3] == 1) {
				if (Text_mode == TEXT_NUM_MODE) {
					TextLED[0][Text_len] = '4';
					Text_len++;
				}
				else if (TextLED[0][curr] == 0) {
					TextLED[0][curr] = 'G';
				}
				else if (TextLED[0][curr] == 'G') {
					TextLED[0][curr] = 'H';
				}
				else if (TextLED[0][curr] == 'H') {
					TextLED[0][curr] = 'I';
				}
				else if (TextLED[0][curr] == 'I') {
					TextLED[0][curr] = 'G';
				}
				else {
					//curr is not 
					TextLED[0][Text_len] = 'G';
					Text_len++;
				}
                Count_total++;
			}
			else if (push_sw_buff[4] == 1) {
				if (Text_mode == TEXT_NUM_MODE) {
					TextLED[0][Text_len] = '5';
					Text_len++;
				}
				else if (TextLED[0][curr] == 0) {
					TextLED[0][curr] = 'J';
				}
				else if (TextLED[0][curr] == 'J') {
					TextLED[0][curr] = 'K';
				}
				else if (TextLED[0][curr] == 'K') {
					TextLED[0][curr] = 'L';
				}
				else if (TextLED[0][curr] == 'L') {
					TextLED[0][curr] = 'J';
				}
				else {
					//curr is not 
					TextLED[0][Text_len] = 'J';
					Text_len++;
				}
                Count_total++;
			}
			else if (push_sw_buff[5] == 1) {
				if (Text_mode == TEXT_NUM_MODE) {
					TextLED[0][Text_len] = '6';
					Text_len++;
				}
				else if (TextLED[0][curr] == 0) {
					TextLED[0][curr] = 'M';
				}
				else if (TextLED[0][curr] == 'M') {
					TextLED[0][curr] = 'N';
				}
				else if (TextLED[0][curr] == 'N') {
					TextLED[0][curr] = 'O';
				}
				else if (TextLED[0][curr] == 'O') {
					TextLED[0][curr] = 'M';
				}
				else {
					//curr is not 
					TextLED[0][Text_len] = 'M';
					Text_len++;
				}
                Count_total++;
			}
			else if (push_sw_buff[6] == 1) {
				if (Text_mode == TEXT_NUM_MODE) {
					TextLED[0][Text_len] = '7';
					Text_len++;
				}
				else if (TextLED[0][curr] == 0) {
					TextLED[0][curr] = 'P';
				}
				else if (TextLED[0][curr] == 'P') {
					TextLED[0][curr] = 'R';
				}
				else if (TextLED[0][curr] == 'R') {
					TextLED[0][curr] = 'S';
				}
				else if (TextLED[0][curr] == 'S') {
					TextLED[0][curr] = 'P';
				}
				else {
					//curr is not 
					TextLED[0][Text_len] = 'P';
					Text_len++;
				}
                Count_total++;
			}
			else if (push_sw_buff[7] == 1) {
				if (Text_mode == TEXT_NUM_MODE) {
					TextLED[0][Text_len] = '8';
					Text_len++;
				}
				else if (TextLED[0][curr] == 0) {
					TextLED[0][curr] = 'T';
				}
				else if (TextLED[0][curr] == 'T') {
					TextLED[0][curr] = 'U';
				}
				else if (TextLED[0][curr] == 'U') {
					TextLED[0][curr] = 'V';
				}
				else if (TextLED[0][curr] == 'V') {
					TextLED[0][curr] = 'T';
				}
				else {
					//curr is not 
					TextLED[0][Text_len] = 'T';
					Text_len++;
				}
                Count_total++;
			}
			else if (push_sw_buff[8] == 1) {
				if (Text_mode == TEXT_NUM_MODE) {
					TextLED[0][Text_len] = '9';
					Text_len++;
				}
				else if (TextLED[0][curr] == 0) {
					TextLED[0][curr] = 'W';
				}
				else if (TextLED[0][curr] == 'W') {
					TextLED[0][curr] = 'X';
				}
				else if (TextLED[0][curr] == 'X') {
					TextLED[0][curr] = 'Y';
				}
				else if (TextLED[0][curr] == 'Y') {
					TextLED[0][curr] = 'X';
				}
				else {
					//curr is not 
					TextLED[0][Text_len] = 'X';
					Text_len++;
				}
                Count_total++;
			}

			Count_total %= 10000;
			FND[0] = Count_total / 1000;
			FND[1] = (Count_total / 100) - (Count_total / 1000) * 10;
			FND[2] = (Count_total / 10) - (Count_total / 100) * 10;
			FND[3] = (Count_total)-(Count_total / 10) * 10;
 
			out_to_FND(FND);

			out_to_LCD(TextLED[0], Text_len);

			out_to_Matrix_alpha(Text_mode);
		}
		else if (mode == 3) {
            //여기서 팅김
//            printf("where1\n");
			if (push_sw_buff[0] == 1) {
				int i = 0, j = 0;
				for (i = 0; i < 4; i++) FND[i] = 0;
				for (i = 0; i < 10; i++) {
					for (j = 0; j < 7; j++) {
						Draw_Matrix[i][j] = 0;
					}
				}
                x = y = 0;
                Count_total++;
			}
			else if (push_sw_buff[1] == 1) {
				if (y > 0) y -= 1;
                Count_total++;
			}
			else if (push_sw_buff[2] == 1) {
                if(curser == 0) curser = 1;
                else curser = 0;
                Count_total++;
			}
			else if (push_sw_buff[3] == 1) {
				if (x > 0) x -= 1;
                Count_total++;
			}
			else if (push_sw_buff[4] == 1) {
                if(Draw_Matrix[y][x] == 1){
                   Draw_Matrix[y][x] = 0;
                }
                else{
                   Draw_Matrix[y][x] = 1;
                }
                   
                Count_total++;
			}
			else if (push_sw_buff[5] == 1) {
				if (x < 7) x += 1;
                Count_total++;
			}
			else if (push_sw_buff[6] == 1) {
                int i, j;
                for(i = 0; i < 10; i++){
                    for(j = 0; j < 7; j++){
                        Draw_Matrix[i][j] = 0;
                    }
                }
                Count_total++;
			}
			else if (push_sw_buff[7] == 1) {
				if (y < 7) y += 1;
                Count_total++;
			}
			else if (push_sw_buff[8] == 1) {
				int i = 0, j = 0;
				for (i = 0; i < 10; i++) {
					for (j = 0; j < 7; j++) {
                        if(Draw_Matrix[i][j] == 0){
                           Draw_Matrix[i][j] = 1;
                        }
                        else {
                           Draw_Matrix[i][j] = 0;
                        }
					}
				}
                Count_total++;
			}
            char tmp_Draw_Matrix[10][7];
            if(curser == 1){
                int ii = 0, jj = 0;
                for(ii = 0 ; ii < 10; ii++){
                    for(jj = 0; jj < 7; jj++){
                        tmp_Draw_Matrix[ii][jj] = Draw_Matrix[ii][jj];
                    }
                }
                
                if(k <= 10){
                    tmp_Draw_Matrix[y][x] = 1;
                }
                else{
                    tmp_Draw_Matrix[y][x] = 0;
                    if(k > 20) k = 0;
                }
                out_to_Matrix(tmp_Draw_Matrix);
                k++;
            }
            else{
                out_to_Matrix(Draw_Matrix);
            }
//            printf("where2\n");
            Count_total %= 10000;
            FND[0] = Count_total / 1000;
            FND[1] = (Count_total / 100) - (Count_total / 1000) * 10;
            FND[2] = (Count_total / 10) - (Count_total / 100) * 10;
            FND[3] = (Count_total)-(Count_total / 10) * 10;
//            printf("where3\n");
            out_to_FND(FND);
//            printf("where4\n");
//            printf("where5\n");
		}
        
        char led1[8] = { 1, 0 ,0, 0, 0, 0 ,0 ,0 };
        char led3[8] = { 0, 0 ,1, 0, 0, 0 ,0 ,0 };
        char led4[8] = { 0, 0 ,0, 1, 0, 0 ,0 ,0 };
        if(mode == 0 && Text_mode == 0){
            out_to_LED(led1);
        }
        else if(mode == 0 && Text_mode != 0){
            
            if(led_mode == 1){
                out_to_LED(led3);
                if(j == 10){
                    j = 0;
                    led_mode = 0;
                }
            }
            else{
                out_to_LED(led4);
                if(j == 10){
                    j = 0;
                    led_mode = 1;
                }
            }
            
            j++;
        }
	}

	return 0;
}
