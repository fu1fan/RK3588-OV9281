#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <termios.h> /*PPSIX 终端控制定义*/

/*
 * @description : UDP客户端创建文件描述符
 * @param - *fd : 文件描述符
 * @param - *ip : UDP服务端IP
 * @param - port: UDP服务端端口
 * @return		: 成功返回0 失败返回-1
 */
int func_create_udp_client_link(int *fd, char *ip, unsigned int port)
{
    int value = 0, Flags = 0;

    /*(1) 创建套接字*/
    if ((*fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        value = -1;
        return value;
    }

    //设置为非阻塞
    Flags = fcntl(*fd, F_GETFL, 0);
    fcntl(*fd, F_SETFL, Flags | O_NONBLOCK);

    return value;
}

/*
 * @description  : UDP客户端接收数据函数
 * @param - fd   : 链接文件描述符
 * @param - *p_receive_buff: 接收数据缓冲区首地址
 * @param - count: 数据最大接收长度
 * @return		 : 实际接收数据长度
 */
int func_udp_client_receive(int fd, char *p_receive_buff, int count)
{
    int receive_numb = 0;
    struct sockaddr_in servaddr;
    socklen_t len;

    memset(&servaddr, 0, sizeof(servaddr));
    len = sizeof(struct sockaddr_in);

    memset(p_receive_buff, 0x0, count);
    receive_numb = recvfrom(fd, p_receive_buff, count, 0, (struct sockaddr *)&servaddr, &len);

    return receive_numb;
}

/*
 * @description  : UDP客户端发送数据函数
 * @param - fd   : 链接文件描述符
 * @param - *p_send_buff: 发送数据缓冲区首地址
 * @param - count: 发送数据长度
 * @param - *ip  : UDP服务端IP
 * @param - port : UDP服务端端口
 * @return		 : 实际发送数据长度
 */
int func_udp_client_send(int fd, char *p_send_buff, int count, char *ip, unsigned int port)
{
    struct sockaddr_in servaddr;

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ip);
    servaddr.sin_port = htons(port);

    sendto(fd, p_send_buff, count, 0, (struct sockaddr *)&servaddr, sizeof(servaddr));

    return count;
}

/*
 * @description  : UDP客户端关闭链接
 * @param - fd   : 链接文件描述符
 * @return		 : 无
 */
void func_close_udp_client_link(int fd)
{
    if (fd > 0)
    {
        close(fd);
    }
}
