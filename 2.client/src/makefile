# change application name here (executable output name)
TARGET=bxr_eraser

# compiler
CC=gcc
# debug
DEBUG=-g
# optimisation
OPT=-O0
# warnings
WARN=-Wall

PTHREAD=-pthread

CCFLAGS=$(DEBUG) $(OPT) $(WARN) $(PTHREAD) -pipe -D_BXDBG

GTKLIB=`pkg-config --cflags --libs gtk+-3.0`

# linker
LD=gcc
LDFLAGS=$(PTHREAD) $(GTKLIB) -ljson-c -lsqlite3 -llsf -export-dynamic

OBJS=bxr_eraser.o

all: $(OBJS)
	$(LD) -o $(TARGET) $(OBJS) $(LDFLAGS)

bxr_eraser.o: bxr_eraser.c
	$(CC) -c $(CCFLAGS) bxr_eraser.c $(GTKLIB) -o bxr_eraser.o

clean:
	rm -f *.o $(TARGET)

