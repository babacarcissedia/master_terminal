IDIR=./include
ODIR=./obj
LDIR=./lib
SDIR=./src
CC=gcc
CFLAGS=-I$(IDIR)
LFLAGS=-L$(LDIR)

DEPS=$(ODIR)/%.h
OBJ=$(ODIR)/%.o


slave: src/slave.c include/slave.h src/explode.c include/explode.h
	$(CC) -g -o bin/slave src/slave.c src/explode.c $(CFLAGS) $(LFLAGS) -lpthread


master: src/master.c include/master.h
	$(CC) -g -o bin/master src/master.c $(CFLAGS) $(LFLAGS) -lpthread


clean:
	rm -f $(ODIR)/*.o