.SUFFIXES : .c .o

DATETIME = `date "+%Y%m%d%H%M%S"`

CC = gcc
SRCHOME =`echo $(HOME)/Blue-X-ray-Eraser/1.server`
APPHOME =`echo $(BXRG_HOME)`

INC = -I$(SRCHOME)/inc
LIB = -L$(SRCHOME)/lib

INILIB = -L$(SRCHOME)/lib/iniparser-4.1/lib64
INIINC = -I$(SRCHOME)/lib/iniparser-4.1/include

NNGLIB = -L$(SRCHOME)/lib/nng-1.4.0/lib64
NNGINC = -I$(SRCHOME)/lib/nng-1.4.0/include

JSNINC = -I$(SRCHOME)/lib/json-c-0.15/include
JSNLIB = -L$(SRCHOME)/lib/json-c-0.15/lib64

DBLIB = -L/usr/lib64 -ldl -lm -lcrypto -lz -lssl
DBINC = -I/usr/include/mysql -I/usr/include/mysql/mysql

LIBS = $(LIB) -lBxrUtils $(DBLIB) -lmysqlclient $(INILIB) -liniparser  $(JSNLIB) -ljson-c -lpthread -D_BXDBG
CFLAGS = -g $(INC) $(DBINC) $(INIINC) $(NNGINC) $(JSNINC)

OBJS = bxr_parser.o
SRCS = bxr_parser.c 

TARGET = bxr_parser

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

install : 
	@cp -arp $(TARGET) $(SRCHOME)/bin/.
	@cp -arp *.sh $(SRCHOME)/bin/.
	@cp -arp Properties/*.conf $(SRCHOME)/conf/.
#	@ls -al $(SRCHOME)/bin/$(TARGET)
	@echo -e "\n##-------------------------------------------##"
	@find $(APPHOME)/bin -maxdepth 1 -name $(TARGET) -exec mv {} {}.$(DATETIME) \;
	@find $(APPHOME)/bin -maxdepth 1 -name "$(TARGET).*" -exec mv {} $(APPHOME)/backup/. \;
	@cp -arp $(TARGET) $(APPHOME)/bin/.
	@cp -arp *.sh $(APPHOME)/bin/.
	@cp -arp Properties/*.conf $(APPHOME)/conf/.
	@ls -al $(APPHOME)/bin/$(TARGET)
	@ls -al $(APPHOME)/conf/*.conf
	@echo -e "##-------------------------------------------##\n"
