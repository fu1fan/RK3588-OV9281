#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <termios.h> /*PPSIX 终端控制定义*/
#include "net.h"

char ver[20] = {"ver01.001"};

int sockfd, listenfd;
pthread_t net_thread;

struct_net_param net_param = {"192.168.0.2", 1001, 2404};
enum_run_mode run_mode = TCP_SERVER;

char send_buff[500], receive_buff[500];
int send_num = 0, receive_num = 0;
unsigned int connect_state = DISCONNECT;
unsigned int establish_listen = CREATE_FALSE;

//数据收发应用
char active_send_buff[500] = "test network";
int active_send_mode = 0, active_send_num = 12, active_send_time = 1000, active_send_time_count = 0;
int real_send_num = 0;
int loopback_send_mode = 0;

/*
 * @description : 自定义打印函数
 * @param - buff:  打印数据缓冲区
 * @param - lens:  打印数据长度
 * @param - mode:  打印格式
 * @return		: 无
 */
void func_my_print(char *buff, unsigned int lens, unsigned char mode)
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
    printf("Usage: %s [-ip ip] [-p port] [-t] [-u] [-c] [-s listen_port] [-T data time] [-L] [-v] "
           "\n\t'-ip' for IP example 192.168.0.1"
           "\n\t'-p' for port form 1 to 65535"
           "\n\t'-t' for tcp mode "
           "\n\t'-u' for udp mode"
           "\n\t'-c' for client mode"
           "\n\t'-s' for server mode  and listen_port"
           "\n\t'-T data time' for interval set time actively sends the data,unit is s"
           "\n\t'-L' data loopback,send the received data"
           "\n\t'-v' show version"
           "\n\texample :  tcp client  --> ./cmddemo_net -ip 192.168.2.97 -p 1001 -t -c -T 123 5 "
           "\n\texample :  tcp server  --> ./cmddemo_net -ip 192.168.2.98 -t -s 1001 -L"
           "\n\texample :  udp client  --> ./cmddemo_net -ip 192.168.2.97 -p 1001 -u -c -T 123 5 "
           "\n\texample :  udp server  --> ./cmddemo_net -ip 192.168.2.98 -u -s 1001 -L\n ",
           pname);
}

/*
 * @description : 解析函数带入参数
 * @param - numb: 参数个数
 * @param - *param: 带入参数数组指针
 * @param - *net: 网络应用参数
 * @return		: 无
 */
void get_param(int numb, char *param[], struct_net_param *net)
{
    int i = 0, len = 0;
    unsigned int udp = 0, server = 0;

    if (numb <= 2)
        return;

    for (i = 1; i < numb; i++)
    {
        if (!strcmp(param[i], "-ip"))
        {
            i++;
            printf("ip = %s    ", param[i]);
            strcpy(net->ip, param[i]);
            continue;
        }
        if (!strcmp(param[i], "-p"))
        {
            i++;
            net->port = atoi(param[i]);
            printf("port = %d\n", net->port);
            continue;
        }
        if (!strcmp(param[i], "-t"))
        {
            udp = 0;
            continue;
        }
        if (!strcmp(param[i], "-u"))
        {
            udp = 1;
            continue;
        }
        if (!strcmp(param[i], "-c"))
        {
            server = 0;
            continue;
        }
        if (!strcmp(param[i], "-s"))
        {
            server = 1;

            i++;
            net->listen_port = atoi(param[i]);
            printf("listen port = %d\n", net->listen_port);
            continue;
        }
        if (!strcmp(param[i], "-T"))
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
            printf("net_demo ver:  %s\n", ver);
            continue;
        }
    }

    if (udp == 1)
    {
        if (server == 1)
            run_mode = UDP_SERVER;
        else
            run_mode = UDP_CLIENT;
    }
    else
    {
        if (server == 1)
            run_mode = TCP_SERVER;
        else
            run_mode = TCP_CLIENT;
    }
}

/*
 * @description : 网络连接管理线程
 * @param - arg : 参数
 * @return		: 无
 */
void *net_manage(void *arg)
{
    int result = 0;

    while (1)
    {
        switch (run_mode)
        {
        case TCP_CLIENT:
            if (connect_state == DISCONNECT)
            {
                // TCP做客户端，主动跟服务端建立链接
                result = func_create_tcp_client_link(&sockfd, net_param.ip, net_param.port);
                if (result >= 0)
                {
                    connect_state = CONNECT;
                    printf("tcp connected\n");
                }
            }
            else
            {
                //检测TCP链接状态
                result = func_detect_tcp_client_link(sockfd);
                if (result < 0)
                {
                    connect_state = DISCONNECT;
                    close(sockfd);
                    printf("ReadCwnd judge connect break\n");
                }
            }
            break;
        case TCP_SERVER:
            if (establish_listen == CREATE_FALSE)
            {
                // TCP做服务端，建立监听
                result = func_create_tcp_server_listen(&listenfd, net_param.listen_port);
                if (result > 0)
                {
                    establish_listen = CREATE_TURE;
                    printf("tcp create server listen ok\n");
                }
                else
                {
                    if (listenfd > 0)
                    {
                        close(listenfd);
                        listenfd = 0;
                    }
                }
            }
            else
            {
                //接受客户端建立链接
                result = func_tcp_server_accept(listenfd, &sockfd, net_param.ip);
                if (result > 0)
                {
                    connect_state = CONNECT;
                    printf("tcp connected\n");
                }

                //检测TCP链接状态
                if (connect_state == CONNECT)
                {
                    result = func_detect_tcp_server_link(sockfd);
                    if (result < 0)
                    {
                        connect_state = DISCONNECT;
                        close(sockfd);
                        sockfd = 0;
                        printf("ReadCwnd judge connect braek\n");
                    }
                }
            }
            break;
        case UDP_CLIENT:
            if (connect_state == DISCONNECT)
            {
                // UDP做客户端，主动跟服务端建立链接
                result = func_create_udp_client_link(&sockfd, net_param.ip, net_param.port);
                if (result >= 0)
                {
                    connect_state = CONNECT;
                    printf("udp connected\n");

                    //需要主动发送数据，告知UDP服务端自己的IP和端口
                    send_num = 5;
                }
            }
            else
            {
                // UDP无法检测链接断开，请应用程序用超时无数据作为检测标准
            }
            break;
        case UDP_SERVER:
            if (establish_listen == CREATE_FALSE)
            {
                // UDP创建服务
                result = func_create_udp_server(&sockfd, net_param.listen_port);
                if (result >= 0)
                {
                    establish_listen = CREATE_TURE;
                    connect_state = CONNECT;
                    printf("udp create server ok\n");
                }
                else
                {
                    if (sockfd > 0)
                    {
                        close(sockfd);
                        listenfd = 0;
                    }
                }
            }
            else
            {
                // UDP无法检测链接断开，请应用程序用超时无数据作为检测标准
            }
            break;
        }
        usleep(50000);
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
    //检测是否有参数
    if (argc < 2)
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
    //获取通信参数
    get_param(argc, argv, &net_param);

    //创建链接管理线程
    if (pthread_create(&net_thread, NULL, net_manage, (void *)NULL))
    {
        printf("Pthread net_manage create error\n");
        exit(1);
    }

    while (1)
    {
        if (connect_state == DISCONNECT)
            continue;

        switch (run_mode)
        {
        case TCP_CLIENT:
            //接收数据
            receive_num = func_tcp_client_receive(sockfd, receive_buff, sizeof(receive_buff));
            if (receive_num > 0)
            {
                printf("[nread=%d ] ", receive_num);
                func_my_print(receive_buff, receive_num, 'c');
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
                real_send_num = func_tcp_client_send(sockfd, send_buff, send_num);
                if (real_send_num > 0)
                {
                    printf("[nwrite=%d] ", real_send_num);
                    func_my_print(send_buff, real_send_num, 'c');
                }
                else
                {
                    close(sockfd);
                    connect_state = DISCONNECT;
                }
                memset(send_buff, 0, send_num);
                send_num = 0;
            }
            break;
        case TCP_SERVER:
            //接收数据
            receive_num = func_tcp_server_receive(sockfd, receive_buff, sizeof(receive_buff));
            if (receive_num > 0)
            {
                printf("[nread=%d ] ", receive_num);
                func_my_print(receive_buff, receive_num, 'c');
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

                real_send_num = func_tcp_server_send(sockfd, send_buff, send_num);
                if (real_send_num > 0)
                {
                    printf("[nwrite=%d] ", real_send_num);
                    func_my_print(send_buff, real_send_num, 'c');
                }
                else
                {
                    close(sockfd);
                    connect_state = DISCONNECT;
                }
                memset(send_buff, 0, send_num);
                send_num = 0;
            }
            break;
        case UDP_CLIENT:
            //接收数据
            receive_num = func_udp_client_receive(sockfd, receive_buff, sizeof(receive_buff));
            if (receive_num > 0)
            {
                printf("[nread=%d ] ", receive_num);
                func_my_print(receive_buff, receive_num, 'c');
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
                real_send_num = func_udp_client_send(sockfd, send_buff, send_num, net_param.ip, net_param.port);
                if (real_send_num > 0)
                {
                    printf("[nwrite=%d] ", real_send_num);
                    func_my_print(send_buff, real_send_num, 'c');
                }
                memset(send_buff, 0, send_num);
                send_num = 0;
            }
            break;
        case UDP_SERVER:
            //接收数据
            receive_num = func_udp_server_receive(sockfd, receive_buff, sizeof(receive_buff), net_param.ip, &net_param.port);
            if (receive_num > 0)
            {
                printf("[nread=%d ] ", receive_num);
                func_my_print(receive_buff, receive_num, 'c');
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
                real_send_num = func_udp_server_send(sockfd, send_buff, send_num, net_param.ip, net_param.port);
                if (real_send_num > 0)
                {
                    printf("[nwrite=%d] ", real_send_num);
                    func_my_print(send_buff, real_send_num, 'c');
                }
                memset(send_buff, 0, send_num);
                send_num = 0;
            }
            break;
        }
        usleep(1000); // 1ms
    }
    exit(0);
}
