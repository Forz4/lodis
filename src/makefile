TARGET = liblodis.a console
CC = @gcc
AR = @ar -ruc
RM = @rm -f
CCFLAG = -Wall

all:	$(TARGET) 

clean:
	$(RM) *.o

liblodis.a:		lodis.o
	$(AR) liblodis.a 	lodis.o

lodis.o:	lodis.h lodis.c lodis_api.h	
	$(CC) -c lodis.c $(CCFLAG)

console:		console.o liblodis.a
	$(CC) -o console console.c -L./ -llodis -I./ $(CCFLAG)
