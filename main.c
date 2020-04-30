#include "input.h"
#include "output.h"
#include "main.h"
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
#include <sys/shm.h>

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
    out_to_LCD(TextLED[0], 0);
    out_to_LED(LED);
    Count_total = dot = 0;
    Count_jinsu = 10;
    Text_len = 1;
    Text_mode = TEXT_ALPHA_MODE;
    y = x = 0;
    curser = 0;
    firstExec = 1;
    led_mode = 0;
}

int main() {
    printf("init1\n");
    struct input_event ev[BUFF_SIZE];
    unsigned char push_sw_buff[MAX_BUTTON];
    
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
    dot = 0;
    Count_jinsu = 10;
    Count_total = 0;
    Text_len = 1;
    Text_mode = TEXT_ALPHA_MODE;
    firstExec = 1;
    led_mode = 0;
    mode = 0;
    curser = 0;
    printf("init2\n");
    usleep(1000000);
    while(1){
        printf("input main\n");
        //forK?
        struct input_event*shmaddr_ev;
        unsigned char *shmaddr_sw;
        
        struct in_packet in_pac;
        
        key_t key0 = ftok("./", 1);
        
        int shmid = shmget(key0, sizeof(struct in_packet), IPC_CREAT|0644);
        struct in_packet*shmaddr = (struct in_packet*)shmat(shmid, NULL, 0);
        strcpy(&in_pac,shmaddr);
        
        ev[0].type = in_pac.type;
        ev[0].value = in_pac.value;
        ev[0].code = in_pac.code;
        strcpy(push_sw_buff, in_pac.push_sw_buff);
        
        if (ev[0].type == 1 && ev[0].value == KEY_RELEASE && ev[0].code == 115) {
            mode = (mode + 1) % 4;
            reset_para();
            printf("mode : %d\n", mode);
        }
        
        if (ev[0].type == 1 && ev[0].value == KEY_RELEASE && ev[0].code == 114) {
            mode = mode ? mode - 1 : 3;
            reset_para();
            printf("mode : %d\n", mode);
        }
        
        if (mode == 0) {
            if(firstExec){
                Clock_FND_set_to_borad_time();
                firstExec = 0;
            }
            if (push_sw_buff[0] == 1) {
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
        }
        else if (mode == 3) {
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
            Count_total %= 10000;
            FND[0] = Count_total / 1000;
            FND[1] = (Count_total / 100) - (Count_total / 1000) * 10;
            FND[2] = (Count_total / 10) - (Count_total / 100) * 10;
            FND[3] = (Count_total)-(Count_total / 10) * 10;
        }
        
        
        struct packet pak;
        int i, j;
        for(i = 0; i < 4; i++) pak.FND[i] = FND[i];
        for(i = 0; i < 8; i++) pak.LED[i] = LED[i];
        for(i = 0; i < 2; i++){
            for(j = 0; j < 100; j++){
                pak.TextLED[i][j] = TextLED[i][j];
            }
        }
        for(i = 0; i < 10; i++){
            for(j = 0; j < 7; j++){
                pak.Draw_Matrix[i][j] = Draw_Matrix[i][j];
            }
        }
        pak.mode =mode;
        pak.led_mode = led_mode;
        pak.Text_len = Text_len;
        pak.Text_mode = Text_mode;
        pak.curser = curser;
        pak.y = y;
        pak.x = x;
        
        key_t key2 = ftok("./", 3);
        int shmid_2 = shmget(key2, sizeof(struct packet), IPC_CREAT|0644);
        struct packet*shmaddr_2 = (struct packet*)shmat(shmid_2, NULL, 0);
        strcpy(shmaddr_2, &pak);
    }
    return 0;
}
