CPP=aarch64-linux-gcc
tty_demo:main.o serial.o
	$(CPP)	-Wall	main.o serial.o	-o	cmddemo_serialport
main.o:main.c serial.h
	$(CPP)	-c	-Wall	main.c	-o	main.o
serial.o:serial.c
	$(CPP)	-c	-Wall	serial.c	-o	serial.o
clean:
	$(RM) *.o	cmddemo_serialport

