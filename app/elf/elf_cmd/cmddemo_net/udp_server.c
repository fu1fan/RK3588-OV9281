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
 * @description  : UDP服务端创建监听文件描述符
 * @param - *fd  : 文件描述符
 * @param - listen_port: UDP服务端监听端口
 * @return		 : 成功返回0 失败返回-1
 */
int func_create_udp_server(int *fd, unsigned int listen_port)
{
    int value = 0, Flags = 0;
    struct sockaddr_in servaddr;

    /*(1) 创建套接字*/
    if ((*fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        value = -1;
        return value;
    }

    //设置为非阻塞
    Flags = fcntl(*fd, F_GETFL, 0);
    fcntl(*fd, F_SETFL, Flags | O_NONBLOCK);

    /*(2) 设置服务器sockaddr_in结构*/
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); //
    servaddr.sin_port = htons(listen_port);

    /*(3) 绑定套接字和端口*/
    if (bind(*fd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr)) == -1)
    {
        printf("Bind error%s\n", strerror(errno));
        value = -1;
        return value;
    }

    return value;
}

/*
 * @description  : UDP服务端接收数据函数
 * @param - fd   : 文件描述符
 * @param - *p_receive_buff: 接收数据缓冲区首地址
 * @param - count: 数据最大接收长度
 * @param - *ip  : UDP客户端IP
 * @param - *port: UDP客户端端口
 * @return		 : 实际接收数据长度
 */
int func_udp_server_receive(int fd, char *p_receive_buff, int count, char *ip, unsigned int *port)
{
    int receive_numb = 0;
    struct sockaddr_in cliaddr;
    socklen_t len;

    memset(&cliaddr, 0, sizeof(cliaddr));
    len = sizeof(struct sockaddr_in);

    memset(p_receive_buff, 0x0, count);
    receive_numb = recvfrom(fd, p_receive_buff, count, 0, (struct sockaddr *)&cliaddr, &len);
    if (receive_numb > 0)
    {
        //获得客户端的IP地址和端口号
        ip = inet_ntoa(cliaddr.sin_addr);
        *port = ntohs(cliaddr.sin_port);
        printf("client ip = %s ,client port = %d\n", ip, *port);
    }
    return receive_numb;
}

/*
 * @description  : UDP服务端发送数据函数
 * @param - fd   : 文件描述符
 * @param - *p_send_buff: 发送数据缓冲区首地址
 * @param - count: 发送数据长度
 * @param - *ip  : UDP客户端IP
 * @param - port : UDP客户端端口
 * @return		 : 实际发送数据长度
 */
int func_udp_server_send(int fd, char *p_send_buff, int count, char *ip, unsigned int port)
{
    struct sockaddr_in cliaddr;

    memset(&cliaddr, 0, sizeof(cliaddr));
    cliaddr.sin_family = AF_INET;
    cliaddr.sin_addr.s_addr = inet_addr(ip);
    cliaddr.sin_port = htons(port);

    sendto(fd, p_send_buff, count, 0, (struct sockaddr *)&cliaddr, sizeof(cliaddr));

    return count;
}

/*
 * @description  : UDP服务端关闭链接
 * @param - fd   : 链接文件描述符
 * @return		 : 无
 */
void func_close_udp_server(int fd)
{
    if (fd > 0)
    {
        close(fd);
    }
}
