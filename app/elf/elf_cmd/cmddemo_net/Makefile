# CC=/usr/local/arm/cross/am335xt3/devkit/bin/arm-arago-linux-gnueabi-gcc
CPP=aarch64-linux-gcc
net_demo:main.o tcp_client.o tcp_server.o udp_client.o udp_server.o
	$(CPP)	-Wall	main.o tcp_client.o tcp_server.o udp_client.o udp_server.o	-o	cmddemo_net -lpthread
main.o:main.c net.h
	$(CPP)	-c	-Wall	main.c	-o	main.o -lpthread
tcp_client.o:tcp_client.c
	$(CPP)	-c	-Wall	tcp_client.c	-o	tcp_client.o
tcp_server.o:tcp_server.c
	$(CPP)	-c	-Wall	tcp_server.c	-o	tcp_server.o
udp_client.o:udp_client.c
	$(CPP)	-c	-Wall	udp_client.c	-o	udp_client.o
udp_server.o:udp_server.c
	$(CPP)	-c	-Wall	udp_server.c	-o	udp_server.o
clean:
	$(RM) *.o	cmddemo_net

