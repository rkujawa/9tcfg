CC=vc
RMRF=rm
GROFF=groff

CFLAGS=
LDFLAGS=-lamiga

OBJECTS=cpu.o file.o cfgreg.o addmem.o rom.o 9tcfg.o
ADDHEADERS=config.h hardware.h
MANPAGE=9tcfg.1

EXECUTABLE=9tcfg
MANPAGEPS=9tcfg.ps

all: $(EXECUTABLE) 
man: $(MANPAGEPS) 

$(MANPAGEPS) : $(MANPAGE) 
	$(GROFF) -Tps -mandoc -c $< > $(MANPAGEPS) 

9tcfg : $(OBJECTS)
	$(CC) -o $(EXECUTABLE) $(LDFLAGS) $(OBJECTS)

%.o : %.c %.h $(ADDHEADERS)
	$(CC) $(CFLAGS) -c $<

clean :
	$(RM) -rf $(OBJECTS) $(EXECUTABLE) $(MANPAGEPS) 

