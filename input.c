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
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>

int pp (int semid) {
    // p 연산
    struct sembuf p_buf;

    p_buf.sem_num = 0;
    p_buf.sem_op = -1;
    p_buf.sem_flg = SEM_UNDO;

    if (semop (semid, &p_buf, 1) == -1) exit (1);
    return (0);
}
int vv(int semid) {
    // v 연산
    struct sembuf v_buf;

    v_buf.sem_num = 0;
    v_buf.sem_op = 1;
    v_buf.sem_flg = SEM_UNDO;

    if (semop (semid, &v_buf, 1) == -1) exit (1);
    return (0);
}


void entry_input(){
    printf("init input\n");
    struct input_event ev[BUFF_SIZE];
    int fd, rd, value, size = sizeof(struct input_event);
    char* device = "/dev/input/event0";
    int dev, buff_size;
    unsigned char push_sw_buff[MAX_BUTTON];
    struct in_packet in_pac;
    
    if ((fd = open(device, O_RDONLY | O_NONBLOCK)) == -1) {
        printf("%s is not a vaild device.n", device);
    }

    
    memset(&in_pac, 0, sizeof(struct in_packet));
    dev = open("/dev/fpga_push_switch", O_RDWR);
    printf("dev : %d\n", dev);
    if (dev<0) {
        printf("Device Open Error\n");
        close(dev);
        return -1;
    }
    
    int semid;
    semid = semget ((key_t)12345, 1, 0666 | IPC_CREAT);
    
    while (1) {
        pp(semid);
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
        vv(semid);
    }
    close(dev);
}
