CFLAGS = -g -Wall
CC = gcc
LIBS = 
INCLUDES =
OBJS = sortmain.o readandwrite.o data.o
SRCS = sortmain.c readandwrite.c data.c
HDRS = data.h io.h 

all: sortmain

sortmain: ${OBJS}
	${CC} ${CFLAGS} ${INCLUDES} -o $@ ${OBJS} ${LIBS}

.c.o:
	${CC} ${CFLAGS} ${INCLUDES} -c $<

depend:
	makedepend ${SRCS}

clean:
	rm *.o core *~