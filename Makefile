INCLUDE=`pkg-config --cflags x11` 
LIBS=`pkg-config --libs x11` -lm
OBJECTS=rpexpose.o parse.o thumbnail.o gui.o assert.o
DESTDIR=/usr/local/bin

all: rpexpose

rpexpose: ${OBJECTS}
	 ${CC} ${CFLAGS} ${OBJECTS} ${LIBS} -g -o $@ 

.c.o: rpexpose.h
	 ${CC} ${CFLAGS} ${INCLUDE} -g -c $^

install: rpexpose
	 cp rpthumb rpselect rpexpose ${DESTDIR}
	 @echo
	 @echo Add the following lines to your .ratpoisonrc file
	 @echo addhook switchwin exec rpthumb
	 @echo addhook quit exec rpexpose --clean
	 @echo bind \<key\> exec rpselect

clean:
	 rm -f rpexpose *.gch *.o
