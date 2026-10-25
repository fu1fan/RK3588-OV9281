#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>
#include <linux/watchdog.h>
#include <string.h>       
#include <unistd.h>  
int main(int argc,char **argv)
{
		int fd;
		//./wdt settimeout 10 #SEC
		//.wdt keepalive 10 #SEC
		int sec = atoi(argv[3]);
		fd = open(argv[1],O_RDWR);
		if(fd < 0){
				perror("open");
		}
//		write(fd,"V",1);
		if(strcmp(argv[2],"settimeout") == 0){
				if(ioctl(fd,WDIOC_SETTIMEOUT,&sec) < 0){
						perror("ioctl set timeout");
				}
		} else if(strcmp(argv[2],"keepalive") == 0){
				if(ioctl(fd,WDIOC_SETTIMEOUT,&sec) < 0){
						perror("ioctl set timeout");
				}
				while(1){
						if(ioctl(fd,WDIOC_KEEPALIVE) < 0){
								perror("ioctl keep alive");
						}
						sleep(2);
				} 
		}
		while(1);
		//loop
		return 0;
}
