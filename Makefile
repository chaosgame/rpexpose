INCLUDE=`pkg-config --cflags x11` rpexpose.h
LIBS=`pkg-config --libs x11` -lm
OBJECTS=rpexpose.o parse.o thumbnail.o gui.o
DESTDIR=/usr/local/bin

all: rpexpose

rpexpose: ${OBJECTS}
	${CC} ${CFLAGS} ${OBJECTS} ${LIBS} -g -o $@ 

.c.o:
	${CC} ${CFLAGS} ${INCLUDE} -g -c $^

install: rpexpose
	cp rpthumb rpselect ${BINARY} ${DESTDIR}
	@echo
	@echo Add the following lines to your .ratpoisonrc file
	@echo addhook exec switchwin rpthumb
	@echo addhook quit exec rpexpose --clean
	@echo bind \<key\> exec rpselect

clean:
	rm -f rpexpose *.gch *.o
