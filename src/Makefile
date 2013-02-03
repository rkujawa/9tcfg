CC=vc
RMRF=rm
GROFF=groff

CFLAGS=
LDFLAGS=-lamiga -lposix

OBJECTS=file.o cfgreg.o addmem.o rom.o 9tcfg.o
MANPAGE=9tcfg.1

EXECUTABLE=9tcfg
MANPAGEPS=9tcfg.ps

all: $(EXECUTABLE) 
man: $(MANPAGEPS) 

$(MANPAGEPS) : $(MANPAGE) 
	$(GROFF) -Tps -mandoc -c $< > $(MANPAGEPS) 

9tcfg : $(OBJECTS)
	$(CC) -o $(EXECUTABLE) $(LDFLAGS) $(OBJECTS)

%.o : %.c
	$(CC) $(CFLAGS) -c $<

clean :
	$(RM) -rf $(OBJECTS) $(EXECUTABLE) $(MANPAGEPS) 

