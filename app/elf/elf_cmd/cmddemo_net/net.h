#include <stdio.h>

#define CONNECT 1
#define DISCONNECT 0

#define CREATE_TURE 1
#define CREATE_FALSE 0

/*网络参数结构体 */
typedef struct
{
    char ip[16];              /*目的IP*/
    unsigned int port;        /*目的端口*/
    unsigned int listen_port; /*目的端口*/
} struct_net_param;

/*工作模式*/
typedef enum
{
    TCP_CLIENT = 0, /*tcp客户端*/
    TCP_SERVER = 1, /*tcp服务端*/
    UDP_CLIENT = 2, /*udp客户端*/
    UDP_SERVER = 3, /*udp服务端*/
} enum_run_mode;

int func_detect_tcp_client_link(int fd);
int func_create_tcp_client_link(int *fd, char *ip, unsigned int port);
int func_tcp_client_receive(int fd, char *p_receive_buff, int count);
int func_tcp_client_send(int fd, char *p_send_buff, int count);
void func_close_tcp_client_link(int fd);

int func_detect_tcp_server_link(int fd);
int func_create_tcp_server_listen(int *fd, unsigned int listen_port);
int func_tcp_server_accept(int listen_fd, int *fd, char *ip);
int func_tcp_server_receive(int fd, char *p_receive_buff, int count);
int func_tcp_server_send(int fd, char *p_send_buff, int count);
void func_close_tcp_server_listen(int fd);
void func_close_tcp_server_link(int fd);

int func_create_udp_client_link(int *fd, char *ip, unsigned int port);
int func_udp_client_receive(int fd, char *p_receive_buff, int count);
int func_udp_client_send(int fd, char *p_send_buff, int count, char *ip, unsigned int port);
void func_close_udp_client_link(int fd);

int func_create_udp_server(int *fd, unsigned int listen_port);
int func_udp_server_receive(int fd, char *p_receive_buff, int count, char *ip, unsigned int *port);
int func_udp_server_send(int fd, char *p_send_buff, int count, char *ip, unsigned int port);
void func_close_udp_server(int fd);
