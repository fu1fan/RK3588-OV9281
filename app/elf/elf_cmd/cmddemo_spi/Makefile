CPP=aarch64-linux-gcc
spi_demo:main.o spi.o file.o
	$(CPP)	-Wall	main.o spi.o file.o	-o	cmddemo_spi
main.o:main.c spi.h file.h
	$(CPP)	-c	-Wall	main.c	-o	main.o
spi.o:spi.c
	$(CPP)	-c	-Wall	spi.c	-o	spi.o
file.o:file.c file.h
	$(CPP)	-c	-Wall	file.c	-o	file.o
clean:
	$(RM) *.o	cmddemo_spi

