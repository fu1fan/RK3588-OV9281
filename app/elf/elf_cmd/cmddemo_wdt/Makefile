CPP=aarch64-linux-gcc

wdt_demo:main.o
	$(CPP)	-Wall	main.o  -o cmddemo_wdt
main.o:main.c 
	$(CPP)	-c	-Wall	main.c	-o	main.o
clean:
	$(RM) *.o	cmddemo_wdt

