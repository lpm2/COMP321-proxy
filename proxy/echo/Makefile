COURSE = /clear/www/htdocs/comp321
CC = gcc
CFLAGS = -Wall -Wextra -I${COURSE}/include -g
LDFLAGS = -lpthread -lnsl -lrt

PROGS = echoclient echoserveri echoservert echoservert_pre

all: ${PROGS}

echoclient: echoclient.o csapp.o
	${CC} ${CFLAGS} -o echoclient echoclient.o csapp.o ${LDFLAGS}

echoserveri: echoserveri.o echo.o csapp.o
	${CC} ${CFLAGS} -o echoserveri echoserveri.o echo.o csapp.o ${LDFLAGS}

echoservert: echoservert.o echo.o csapp.o
	${CC} ${CFLAGS} -o echoservert echoservert.o echo.o csapp.o ${LDFLAGS}

echoservert_pre: echoservert_pre.o echo_cnt.o sbuf.o csapp.o
	${CC} ${CFLAGS} -o echoservert_pre echoservert_pre.o echo_cnt.o sbuf.o csapp.o ${LDFLAGS}

sbuf.o: sbuf.c sbuf.h

echoservert_pre.o: echoservert_pre.c sbuf.h

csapp.o: ${COURSE}/src/csapp.c ${COURSE}/include/csapp.h
	${CC} ${CFLAGS} -c ${COURSE}/src/csapp.c -o csapp.o

clean:
	rm -f ${PROGS} *.o *~


