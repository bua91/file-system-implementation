IDIR =../include
CC=gcc
CFLAGS=-I$(IDIR)
ODIR=obj

_DEPS = serversh.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = serversh.o server.o 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))
$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

serversh: $(OBJ)
	gcc -g -pthread -o $@ $^ $(CFLAGS) -lcrypto -ggdb

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~
