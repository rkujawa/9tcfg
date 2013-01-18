#CC=cc
CC=vc
CFLAGS=
LDFLAGS=-lamiga -lposix

OBJECTS=9tcfg.o

all: 9tcfg 

9tcfg : $(OBJECTS)
	$(CC) -o 9tcfg $(LDFLAGS) $(OBJECTS)

%.o : %.c
	$(CC) $(CFLAGS) -c $<

clean :
	rm -rf $(OBJECTS) 9tcfg 

