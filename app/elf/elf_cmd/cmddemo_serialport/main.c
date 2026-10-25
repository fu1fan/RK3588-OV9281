#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h> /*PPSIX 终端控制定义*/
#include "serial.h"

char ver[20] = {"ver01.001"};

int fd;
struct_tty_param tty_param = {9600, 8, 1, 'N', 0};

unsigned char send_buff[100], receive_buff[100];
unsigned int send_num = 0, receive_num = 0;
char dev[20];

//数据收发应用
char active_send_buff[500] = "test serial port";
int active_send_mode = 0, active_send_num = 16, active_send_time = 1000, active_send_time_count = 0;
int real_send_num = 0;
int loopback_send_mode = 0;

/*
 * @description : 自定义打印函数
 * @param - buff: 打印数据缓冲区
 * @param - lens: 打印数据长度
 * @param - mode: 打印格式
 * @return		: 无
 */
void func_my_print(unsigned char *buff, unsigned int lens, unsigned char mode)
{
    int i = 0;

    switch (mode)
    {
    case 'H': //按照16进制打印数据
        for (i = 0; i < lens; i++)
        {
            printf("0X%02X  ", buff[i]);
        }
        break;
    case 'h': //按照16进制打印数据
        for (i = 0; i < lens; i++)
        {
            printf("0x%02x  ", buff[i]);
        }
        break;
    case 'd': //按照10进制打印数据
        for (i = 0; i < lens; i++)
        {
            printf("%02d  ", buff[i]);
        }
        break;
    case 'c': //按照字符打印数据
        for (i = 0; i < lens; i++)
        {
            printf("%c", buff[i]);
        }
        break;
    default:
        break;
    }
    printf("\n");
}

/*
 * @description  : 打印参数设置格式
 * @param - pname: 函数名
 * @return		 : 无
 */
static void print_usage(const char *pname)
{
    printf("Usage: %s device [-l databit] [-s] [-o] [-e] [-n] [-m] [-s] [-hw]  [-b baudrate] [-t data time] [-L] [-v]"
           "\n\t'-l' for 5/6/7/8 data bit"
           "\n\t'-s' for 2 stop bit"
           "\n\t'-o' for PARODD "
           "\n\t'-e' for PARENB"
           "\n\t'-n' for check disable"
           "\n\t'-m' for MARK check "
           "\n\t'-s' for SPACE check"
           "\n\t'-hw' for HW flow control enable"
           "\n\t'-b baudrate' for different baudrate"
           "\n\t'-t data time' for interval set time actively sends the data,unit is s"
           "\n\t'-L' data loopback,send the received data"
           "\n\t'-v' show version"
           "\n\texample :  8n1 9600    --> ./cmddemo_serialport ttymxc6 "
           "\n\texample :  8o1 115200  --> ./cmddemo_serialport -o -b 115200"
           "\n\texample :  7o2 2400 hardware flow control and data loopback  --> cmddemo_serialport ttymxc6 -l 7 -s -o -b 2400 -L -hw\n ",
           pname);
}

/*
 * @description      : 解析函数带入参数
 * @param - numb     : 参数个数
 * @param - *param   : 带入参数数组指针
 * @param - *tty     : 串口应用参数
 * @return		     : 无
 */
void get_param(int numb, char *param[], struct_tty_param *tty)
{
    int i = 0, len = 0;
    unsigned char data_bit = 0;

    if (numb <= 2)
        return;

    for (i = 2; i < numb; i++)
    {
        if (!strcmp(param[i], "-l"))
        {
            i++;
            data_bit = atoi(param[i]);
            switch (data_bit)
            {
            case 5:
                tty->data_bit = 5;
                break;
            case 6:
                tty->data_bit = 6;
                break;
            case 7:
                tty->data_bit = 7;
                break;
            case 8:
                tty->data_bit = 8;
                break;
            default:
                tty->data_bit = 8;
                break;
            }
            continue;
        }
        if (!strcmp(param[i], "-s"))
        {
            tty->stop_bit = 2;
            continue;
        }
        if (!strcmp(param[i], "-o"))
        {
            tty->check = 'O';
            continue;
        }
        if (!strcmp(param[i], "-e"))
        {
            tty->check = 'E';
            continue;
        }
        if (!strcmp(param[i], "-m"))
        {
            tty->check = 'M';
            continue;
        }
        if (!strcmp(param[i], "-s"))
        {
            tty->check = 'S';
            continue;
        }
        if (!strcmp(param[i], "-n"))
        {
            tty->check = 'N';
            continue;
        }
        if (!strcmp(param[i], "-hw"))
        {
            tty->hardware = 1;
            continue;
        }
        if (!strcmp(param[i], "-b"))
        {
            i++;
            tty->baudrate = atoi(param[i]);
            continue;
        }
        if (!strcmp(param[i], "-t"))
        {
            active_send_mode = 1;

            i++;
            len = strlen(param[i]);
            if (len > 0)
            {
                active_send_num = len;
                memcpy(active_send_buff, param[i], len); //要发送的字符串

                i++;
                len = atoi(param[i]); //发送间隔
                if (len > 0)
                {
                    active_send_time = len * 1000; //转换为ms单位
                    active_send_time_count = active_send_time;
                }
            }
            continue;
        }
        if (!strcmp(param[i], "-L"))
        {
            loopback_send_mode = 1;
            continue;
        }
        if (!strcmp(param[i], "-v"))
        {
            printf("tty_demo ver:  %s\n", ver);
            continue;
        }
    }
}

/*
 * @description  : 主函数
 * @param - argc : 参数个数
 * @param - *argv: 带入参数数组指针
 * @return		 : 执行结果
 */
int main(int argc, char *argv[])
{
    int result = 0;

    //检测是否有参数
    if (argc < 2 || strncmp(argv[1], "tty", 3))
    {
        print_usage(argv[0]);
        exit(1);
    }

    //检测是否有--h或--help
    if ((!strcmp(argv[1], "--h")) || (!strcmp(argv[1], "--help")))
    {
        print_usage(argv[0]);
        exit(1);
    }

    strcpy(dev, "/dev/");
    strcat(dev, argv[1]);

    //从main函数带来的参数解析为串口参数
    get_param(argc, argv, &tty_param);

    //当知道设备名称时可以直接赋值dev，例strcpy(dev, "/dev/ttymxc1");
    //打开串口 设置可读写，不被输入影响，不等待外设响应
    fd = open(dev, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0)
    {
        perror(dev);
        printf("Can't Open Serial Port %s \n", dev);
        exit(0);
    }
    else
    {
        printf("baudrate=%ld,data_bit=%d,stop_bit=%d,check='%c'\n", tty_param.baudrate, tty_param.data_bit, tty_param.stop_bit, tty_param.check);
        //设置串口参数
        if ((result = func_set_opt(fd, tty_param.baudrate, tty_param.data_bit, tty_param.stop_bit, tty_param.check, tty_param.hardware)) < 0)
        {
            perror("set_opt error");
            exit(0);
        }
        //设置串口为阻塞方式
        /*if(fcntl(fd, F_SETFL, 0)<0)
        {
            printf("fcntl failed!\n");
        }
        else
        {
            printf("fcntl=%d\n",fcntl(fd, F_SETFL,0));
        }*/
        //设置串口为非阻塞方式
        /*if(fcntl(fd, F_SETFL, FNDELAY)<0)
        {
            printf("fcntl failed!\n");
        }
        else
        {
            printf("fcntl=%d\n",fcntl(fd, F_SETFL,FNDELAY));
        }*/
    }

    while (1)
    {
        receive_num = func_receive_frame(fd, receive_buff, sizeof(receive_buff)); /*读取串口收到的数据*/
        if (receive_num > 0)
        {
            printf("[nread=%d] ", receive_num);
            func_my_print(receive_buff, receive_num, 'c'); /*打印接收到的数据*/
        }
        //组织发送数据
        if ((1 == loopback_send_mode) && (receive_num > 0)) //数据回环处理
        {
            send_num = receive_num;
            memcpy(send_buff, receive_buff, receive_num);
        }
        else if (1 == active_send_mode)
        {
            if (active_send_time_count >= active_send_time)
            {
                active_send_time_count = 0;
                send_num = active_send_num;
                memcpy(send_buff, active_send_buff, active_send_num);
            }
            else
            {
                active_send_time_count++;
            }
        }
        //数据发送
        if (send_num > 0)
        {
            real_send_num = func_send_frame(fd, send_buff, send_num);
            if (real_send_num > 0)
            {
                printf("[nwrite=%d] ", real_send_num); /*打印发送的数据*/
                func_my_print(send_buff, real_send_num, 'c');
            }
            memset(send_buff, 0, send_num);
            send_num = 0;
        }
        usleep(1000);
    }
    exit(0);
}
