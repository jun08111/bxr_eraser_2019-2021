.SUFFIXES : .c .o
CC = gcc

DATETIME = `date "+%Y%m%d%H%M%S"`

SRCHOME = /home/bluexg/Blue-X-ray-Eraser/1.server
APPHOME = /usr/bin
INC = -I$(SRCHOME)/inc
LIB = -L$(SRCHOME)/lib

INIINC = -I$(SRCHOME)/lib/iniparser-4.1/include
INILIB = -L$(SRCHOME)/lib/iniparser-4.1/lib64

NNGINC = -I$(SRCHOME)/lib/nng-1.4.0/include
NNGLIB = -L$(SRCHOME)/lib/nng-1.4.0/lib

JSNINC = -I$(SRCHOME)/lib/json-c-0.15
JSNLIB = -L$(SRCHOME)/lib/json-c-0.15

LIBS = -llsf $(LIB) -lBxrUtils  $(INILIB) -liniparser $(NNGLIB) -lnng $(JSNLIB) -ljson-c -lpthread -D_BXDBG
CFLAGS = -g $(INC) $(INIINC) $(NNGINC) $(JSNINC)

OBJS = bxr_tracer.o
SRCS = bxr_tracer.c 

TARGET = bxr_tracer

all : $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) -o $@ $(OBJS) $(INC) $(LIBS)

dep :
	gccmakedep $(INC) $(SRCS)

clean :
	rm -rf $(OBJS) $(TARGET) core

new : 
	$(MAKE) clean 
	$(MAKE) 
	$(MAKE) install 

install : 
	cp -arp $(TARGET) $(SRCHOME)/bin/.
	cp -arp Properties/BlueXrayG.conf $(SRCHOME)/conf/.
	sudo find $(APPHOME) -maxdepth 1 -name $(TARGET) -exec mv {} {}.$(DATETIME) \;
	@sudo find $(APPHOME) -maxdepth 1 -name "$(TARGET).*" -exec mv {} $(SRCHOME)/backup/. \;
	sudo cp -arp $(TARGET) /usr/bin/.
	sudo cp -arp Properties/BlueXrayG.conf /etc/.
#	sudo ln -s /usr/share/bluexg/$(TARGET) /usr/bin/$(TARGET)
