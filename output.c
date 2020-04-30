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
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>


char led1[8] = { 1, 0 ,0, 0, 0, 0 ,0 ,0 };
char led3[8] = { 0, 0 ,1, 0, 0, 0 ,0 ,0 };
char led4[8] = { 0, 0 ,0, 1, 0, 0 ,0 ,0 };

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

int ppp (int semid) {
    // p 연산
    struct sembuf p_buf;

    p_buf.sem_num = 0;
    p_buf.sem_op = -1;
    p_buf.sem_flg = SEM_UNDO;

    if (semop (semid, &p_buf, 1) == -1) exit (1);
    return (0);
}
int vvv(int semid) {

    // v 연산
    struct sembuf v_buf;

    v_buf.sem_num = 0;
    v_buf.sem_op = 1;
    v_buf.sem_flg = SEM_UNDO;

    if (semop (semid, &v_buf, 1) == -1) exit (1);
    return (0);
}


void out_to_FND(char data[4]) {
    int dev;
    int i;
    int str_size;
    unsigned char D[4];
    for (i = 0;i<4;i++)
    {
        if ((data[i]>=0x30) && (data[i]<=0x39)) {
            D[i] = data[i] - 0x30;
        }
        else{
            D[i] = data[i];
        }
    }
    
    dev = open(FND_DEVICE, O_RDWR);
    if (dev<0) {
        printf("Device open error : %s\n", FND_DEVICE);
        exit(1);
    }
    
    write(dev, &D, 4);
    
    close(dev);
}

void out_to_LED(char data_arr[8]) {
    int dev;
    
    dev = open(LED_DEVICE, O_RDWR);
    if (dev<0) {
        printf("Device open error : %s\n", LED_DEVICE);
        exit(1);
    }
    
    int data = 0;
    
    for (i = 0; i < 8; i++) {
        data += (1 << (7 - i)) * data_arr[i];
    }
    
    write(dev, &data, 1);
    
    close(dev);
}

void out_to_LCD(char str[100], int len) {
    int dev;
    
    dev = open(FPGA_TEXT_LCD_DEVICE, O_WRONLY);
    if (dev<0) {
        printf("Device open error : %s\n", FPGA_TEXT_LCD_DEVICE);
        exit(1);
    }
    if (len <= 8) {
        write(dev, str, 8);
    }
    else {
        write(dev, &str[len-8], 8);
    }
    
    close(dev);
}

void out_to_Matrix_alpha(int mode) {
    int dev;
    int set_num;
    
    dev = open(FPGA_DOT_DEVICE, O_WRONLY);
    if (dev<0) {
        printf("Device open error : %s\n", FPGA_DOT_DEVICE);
        exit(1);
    }
    set_num = 1;
    if (mode == TEXT_ALPHA_MODE) {
        set_num = 10;
    }
    write(dev, fpga_number[set_num], 10);
    
    close(dev);
}

unsigned char arr_to_int(char arr[7]) {
    unsigned char ret = 0;
    for (i = 0; i < 7; i++) {
        if(arr[i] == 1) ret += (1 << (6 - i));
    }
    return ret;
}

void out_to_Matrix(char matrix[10][7]) {
    int dev;
    int set_num;
    
    dev = open(FPGA_DOT_DEVICE, O_WRONLY);
    //    printf("where 6\n");
    if (dev < 0) {
        printf("Device open error : %s\n", FPGA_DOT_DEVICE);
        exit(1);
    }
    
    unsigned char fpga_data[10];
    //    printf("where 7\n");
    int i, j;
    
    for (i = 0; i < 10; i++) {
        fpga_data[i] = arr_to_int(matrix[i]);
        //printf("%d ", fpga_data[i]);
    }
    printf("\n");
    //
    write(dev, fpga_data, 10);
    
    close(dev);
}

void entry_output(){
    printf("init output\n");
    int k = 0;
    key_t key2 = ftok("./", 3);
    int shmid_2 = shmget(key2, sizeof(struct packet), IPC_CREAT|0644);
    struct packet*shmaddr_2 = (struct packet*)shmat(shmid_2, NULL, 0);
    memset(shmaddr_2, 0, sizeof(struct packet));
    while(1){
        int j = 0;
        
        if(shmaddr_2->mode == 0){
            out_to_FND(shmaddr_2->FND);
            if(shmaddr_2->Text_mode == 0){
                out_to_LED(led1);
            }
            else{
                if(shmaddr_2->led_mode == 1){
                    out_to_LED(led3);
                    if(j == 10){
                        j = 0;
                        shmaddr_2->led_mode = 0;
                    }
                }
                else{
                    out_to_LED(led4);
                    if(j == 10){
                        j = 0;
                        shmaddr_2->led_mode = 1;
                    }
                }
                j++;
            }
        }
        else if (shmaddr_2->mode == 1){
            out_to_FND(shmaddr_2->FND);
            out_to_LCD(shmaddr_2->TextLED[0], shmaddr_2->Text_len);
            out_to_Matrix_alpha(shmaddr_2->Text_mode);
        }
        else if (shmaddr_2->mode == 2){
            out_to_LCD(shmaddr_2->TextLED[0], shmaddr_2->Text_len);
            out_to_Matrix_alpha(shmaddr_2->Text_mode);
            out_to_FND(shmaddr_2->FND);
        }
        else if (shmaddr_2->mode == 3){
            char tmp_Draw_Matrix[10][7];
            if(shmaddr_2->curser == 1){
                int ii = 0, jj = 0;
                for(ii = 0 ; ii < 10; ii++){
                    for(jj = 0; jj < 7; jj++){
                        tmp_Draw_Matrix[ii][jj] = pak.Draw_Matrix[ii][jj];
                    }
                }
                
                if(k <= 10){
                    tmp_Draw_Matrix[pak.y][pak.x] = 1;
                }
                else{
                    tmp_Draw_Matrix[pak.y][pak.x] = 0;
                    if(k > 20) k = 0;
                }
                out_to_Matrix(tmp_Draw_Matrix);
                k++;
            }
            else{
                out_to_Matrix(shmaddr_2->Draw_Matrix);
            }
            out_to_FND(shmaddr_2->FND);
        }
    }
}
