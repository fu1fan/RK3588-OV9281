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
 * @description  : 检测TCP服务端连接状态
 * @param - fd   : 文件描述符
 * @return		 : 执行结果 链接断返回-1 正常返回0
 */
int func_detect_tcp_server_link(int fd)
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
 * @description  : 建立TCP服务端监听
 * @param - *fd  : 监听文件描述符
 * @param - listen_port: 监听端口
 * @return		 : 执行结果 创建失败返回-1 正常返回0
 */
int func_create_tcp_server_listen(int *fd, unsigned int listen_port)
{
    int value = 0, Flags = 0;
    struct sockaddr_in servaddr;

    /*(1) 创建套接字*/
    if ((*fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        return -1;
    }

    //设置为非阻塞
    Flags = fcntl(*fd, F_GETFL, 0);
    fcntl(*fd, F_SETFL, Flags | O_NONBLOCK);
    setsockopt(*fd, SOL_SOCKET, SO_REUSEADDR, &Flags, sizeof(int));

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

    /*(4) 监听客户请求*/
    if (listen(*fd, 10) < 0)
    {
        printf("listen error%s\n", strerror(errno));
        value = -1;
        return value;
    }

    value = 1;
    return value;
}

/*
 * @description  : TCP服务端与客户端建立链接
 * @param - listen_fd: 监听文件描述符
 * @param - *fd  : 链接文件描述符
 * @param - *ip  : 客户端ip，用于ip过滤
 * @return		 : 执行结果 无错误返回1 出现错误返回-1
 */
int func_tcp_server_accept(int listen_fd, int *fd, char *ip)
{
    struct sockaddr_in clientddr;
    socklen_t clilen = 0;
    int tempfd = 0, Flags = 0, value = 0;
    char temp_ip[16] = {0};
    unsigned int temp_port;

    bzero(&clientddr, sizeof(clientddr));
    clilen = sizeof(clientddr);
    //检测跟客户端建立链接是否成功
    if ((tempfd = accept(listen_fd, (struct sockaddr *)&clientddr, &clilen)) < 0)
    {
        return value;
    }

    //检测建立链接的客户端IP是否与设置的相同，此处不判别，将接收所有IP请求建立链接。
    memcpy(temp_ip, inet_ntoa(clientddr.sin_addr), 16);
    temp_port = ntohs(clientddr.sin_port);
    printf("client: IP=%s, port=%d\n", temp_ip, temp_port);
    if (memcmp(ip, temp_ip, 16) != 0)
    {
        close(tempfd);
        return value;
    }

    if (*fd > 0)
    {
        close(*fd);
    }
    //设置为非阻塞
    Flags = fcntl(tempfd, F_GETFL, 0);
    fcntl(tempfd, F_SETFL, Flags | O_NONBLOCK);

    *fd = tempfd;
    value = 1;
    return value;
}

/*
 * @description  : TCP服务端接收数据函数
 * @param - fd   : 链接文件描述符
 * @param - *p_receive_buff: 接收数据缓冲区首地址
 * @param - count: 最大接收长度
 * @return		 : 实际接收数据长度
 */
int func_tcp_server_receive(int fd, char *p_receive_buff, int count)
{
    int receive_numb = 0;

    memset(p_receive_buff, 0x0, count);
    receive_numb = recv(fd, p_receive_buff, count, 0);

    return receive_numb;
}

/*
 * @description  : TCP服务端发送数据函数
 * @param - fd   : 链接文件描述符
 * @param - *p_send_buff: 发送数据缓冲区首地址
 * @param - count: 发送数据长度
 * @return		 : 实际发送数据长度
 */
int func_tcp_server_send(int fd, char *p_send_buff, int count)
{
    int send_numb = 0;

    if (func_detect_tcp_server_link(fd) != 0) /*发送数据之前先检查链接状态*/
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
 * @description  : TCP服务端关闭监听
 * @param - fd   : 监听文件描述符
 * @return		 : 无
 */
void func_close_tcp_server_listen(int listen_fd)
{
    if (listen_fd > 0)
    {
        close(listen_fd);
    }
}

/*
 * @description  : TCP服务端关闭链接
 * @param - fd   : 链接文件描述符
 * @return		 : 无
 */
void func_close_tcp_server_link(int fd)
{
    if (fd > 0)
    {
        close(fd);
    }
}
