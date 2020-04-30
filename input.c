#include "output.h"
#include "main.h"
#include "input.h"
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
unsigned char quit = 0;


char FND[4], LED[8], TextLED[2][100], Draw_Matrix[10][7];
int dot = 0, Count_jinsu = 10, Count_total = 0, Text_len = 1, Text_mode = TEXT_ALPHA_MODE, i, firstExec = 1, led_mode = 0, mode = 0;
int y, x, curser = 0, firstExec;

void entry_input(){
    struct input_event ev[BUFF_SIZE];
    int fd, rd, value, size = sizeof(struct input_event);
    char* device = "/dev/input/event0";
    int dev, buff_size;
    unsigned char push_sw_buff[MAX_BUTTON];

    dev = open("/dev/fpga_push_switch", O_RDWR);
    if (dev<0) {
        printf("Device Open Error\n");
        close(dev);
        return -1;
    }
    
    while (1) {
        rd = read(fd, ev, size * BUFF_SIZE);
        
        while (ev[0].type == 1 && ev[0].value == KEY_PRESS && ev[0].code == 115) {
            //volume +, mode change
            rd = read(fd, ev, size * BUFF_SIZE);
        }
        
        while (ev[0].type == 1 && ev[0].value == KEY_PRESS && ev[0].code == 114) {
            //volume -, mode change
            rd = read(fd, ev, size * BUFF_SIZE);
        }
        
        read(dev, &push_sw_buff, buff_size);
        
        usleep(100000);
        
        key_t key = ftok("./", 1);
        int shmid_ev = shmget(key, 1024, IPC_CREAT|0644);
        struct input_event*shmaddr_ev;
        if(shmid_ev == -1) {
            perror("shmget");
            exit(1);
        }
        
        shmaddr_ev = (struct input_event* *)shmat(shmid_ev, NULL, 0);
        strcpy(shmaddr_ev, ev);
        
        key = ftok("./", 2);
        int shmid_sw = shmget(key, 1024, IPC_CREAT|0644);
        unsigned char *shmaddr_sw;
        if(shmid_sw == -1) {
            perror("shmget");
            exit(1);
        }
        
        shmaddr_sw = (unsigned char *)shmat(shmid_sw, NULL, 0);
        strcpy(shmaddr_sw, push_sw_buff);
        usleep(100000);
    }
}
