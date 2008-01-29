INCLUDE=`pkg-config --cflags x11` rpexpose.h
LIBS=`pkg-config --libs x11` -lm
OBJECTS=rpexpose.o parse.o thumbnail.o gui.o
BINARY=rpexpose

all: ${BINARY}

${BINARY}: ${OBJECTS}
	${CC} ${CFLAGS} ${OBJECTS} ${LIBS} -g -o $@ 

.c.o:
	${CC} ${CFLAGS} ${INCLUDE} -g -c $^

install: ${BINARY}
	cp ${BINARY} /usr/local/bin

clean:
	rm -f ${OBJECTS} ${BINARY} *.gch
