INCLUDE=`pkg-config --cflags x11 xpm` 
LIBS=`pkg-config --libs x11 xpm` -lm

all: rpexpose

rpexpose: rpexpose.o parse.o thumbnail.o gui.o
	gcc -g rpexpose.o parse.o thumbnail.o gui.o -o rpexpose  $(LIBS)

rpexpose.o: rpexpose.c rpexpose.h
	gcc -g $(INCLUDE) -c rpexpose.c 

parse.o: parse.c rpexpose.h
	gcc -g $(INCLUDE) -c parse.c 

thumbnail.o: thumbnail.c rpexpose.h
	gcc -g $(INCLUDE) -c thumbnail.c

gui.o: gui.c rpexpose.h
	gcc -g $(INCLUDE) -c gui.c 

install: rpexpose
	cp rpexpose /usr/local/bin

clean:
	rm -f *.o rpexpose
