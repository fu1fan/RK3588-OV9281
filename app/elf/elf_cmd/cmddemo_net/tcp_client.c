#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <termios.h> /*PPSIX 终端控制定义*/

/*
 * @description  : 检测TCP客户端连接状态
 * @param - fd   : 文件描述符
 * @return		 : 执行结果 链接断返回-1 正常返回0
 */
int func_detect_tcp_client_link(int fd)
{
    int info_len = 0, re_err = 0;
    struct tcp_info info;

    info_len = sizeof(struct tcp_info);

    getsockopt(fd, IPPROTO_TCP, TCP_INFO, (void *)&info, (socklen_t *)&info_len);
    if (info.tcpi_state != TCP_ESTABLISHED)
    {
        re_err = -1;
    }

    return re_err;
}

/*
 * @description  : TCP客户端与服务端建立链接
 * @param - *fd  : 链接文件描述符
 * @param - *ip  : 服务端ip，
 * @param - port : 服务端端口，
 * @return		 : 执行结果 链接成功返回0 连接失败返回-1
 */
int func_create_tcp_client_link(int *fd, char *ip, unsigned int port)
{
    int value = 0, Flags = 0;
    struct sockaddr_in servaddr;
    fd_set set;
    struct timeval timeout;

    //设置建立连接等待时间
    timeout.tv_sec = 30;
    timeout.tv_usec = 0;

    /*(1) 创建套接字*/
    if ((*fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        value = -1;
        return value;
    }

    /*(2) 设置链接服务器地址结构*/
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ip);
    servaddr.sin_port = htons(port);

    //设置为非阻塞
    Flags = fcntl(*fd, F_GETFL, 0);
    fcntl(*fd, F_SETFL, Flags | O_NONBLOCK);

    /*(3) 发送链接服务器请求*/
    if (connect(*fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        FD_ZERO(&set);
        FD_SET(*fd, &set);
        if (select((*fd) + 1, NULL, &set, NULL, &timeout) > 0)
        {
            if (func_detect_tcp_client_link(*fd) == -1)
                return -1;
            else
                value = 0;
        }
        else
        {
            value = -1;
            return value;
        }
    }
    return value;
}

/*
 * @description  : TCP客户端接收数据函数
 * @param - fd   : 链接文件描述符
 * @param - *p_receive_buff: 接收数据缓冲区首地址
 * @param - count: 最大接收长度
 * @return		 : 实际接收数据长度
 */
int func_tcp_client_receive(int fd, char *p_receive_buff, int count)
{
    int receive_numb = 0;

    memset(p_receive_buff, 0x0, count);
    receive_numb = recv(fd, p_receive_buff, count, 0);

    return receive_numb;
}

/*
 * @description  : TCP客户端发送数据函数
 * @param - fd   : 链接文件描述符
 * @param - *p_send_buff: 发送数据缓冲区首地址
 * @param - count: 发送数据长度
 * @return		 : 实际发送数据长度
 */
int func_tcp_client_send(int fd, char *p_send_buff, int count)
{
    int send_numb = 0;

    if (func_detect_tcp_client_link(fd) != 0)
    {
        printf("ReadCwnd judge connect break\n");
        return -1;
    }
    send_numb = write(fd, p_send_buff, count);
    if (count != send_numb)
    {
        printf("server terminated prematurely write err\n");
        return -1;
    }
    return send_numb;
}

/*
 * @description  : TCP客户端关闭链接
 * @param - fd   : 链接文件描述符
 * @return		 : 无
 */
void func_close_tcp_client_link(int fd)
{
    if (fd > 0)
    {
        close(fd);
    }
}
