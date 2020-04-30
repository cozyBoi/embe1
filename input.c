#include "output.h"
#include "main.h"
#include "input.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>
#include <termios.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <time.h>
#include <sys/shm.h>
#include <linux/input.h>


void entry_input(){
    printf("init input\n");
    struct input_event ev[BUFF_SIZE];
    int fd, rd, value, size = sizeof(struct input_event);
    char* device = "/dev/input/event0";
    int dev, buff_size;
    unsigned char push_sw_buff[MAX_BUTTON];
    struct in_packet in_pac;
    memset(&in_pac, 0, sizeof(struct in_packet));
    dev = open("/dev/fpga_push_switch", O_RDWR);
    printf("dev : %d\n", dev);
    if (dev<0) {
        printf("Device Open Error\n");
        close(dev);
        return -1;
    }
    
    while (1) {
        printf("input start\n");
        rd = read(fd, ev, size * BUFF_SIZE);
        printf("input read\n");
        while (ev[0].type == 1 && ev[0].value == KEY_PRESS && ev[0].code == 115) {
            //volume +, mode change
            printf("push\n");
            rd = read(fd, ev, size * BUFF_SIZE);
            if(ev[0].value != KEY_PRESS){
                printf("pop\n");
            }
        }
        
        
        while (ev[0].type == 1 && ev[0].value == KEY_PRESS && ev[0].code == 114) {
            //volume -, mode change
            rd = read(fd, ev, size * BUFF_SIZE);
        }
        
        read(dev, &push_sw_buff, buff_size);
        
        key_t key = ftok("./", 1);
        int shmid = shmget(key, sizeof(struct in_packet), IPC_CREAT|0644);
        struct in_packet* shmaddr = (struct in_packet*)shmat(shmid, NULL, 0);
        
        in_pac.type = ev[0].type;
        in_pac.value = ev[0].value;
        in_pac.code = ev[0].code;
        strcpy(in_pac.push_sw_buff, push_sw_buff);
        
        strcpy(shmaddr, &in_pac);
        usleep(100000);
    }
    close(dev);
}
