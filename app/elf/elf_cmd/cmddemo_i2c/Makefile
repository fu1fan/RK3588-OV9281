CPP=aarch64-linux-gcc

i2c_demo:main.o i2c.o
	$(CPP)	-Wall	main.o i2c.o -o	cmddemo_i2c
main.o:main.c i2c.h
	$(CPP)	-c	-Wall	main.c	-o	main.o
i2c.o:i2c.c
	$(CPP)	-c	-Wall	i2c.c	-o	i2c.o
clean:
	$(RM) *.o	cmddemo_i2c

