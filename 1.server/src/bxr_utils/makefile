CC = /usr/bin/gcc
LD = /usr/bin/ld

DATETIME = `date "+%Y%m%d%H%M%S"`

LIBNAME = libBxrUtils.so
LIBVER = 0.0.5

SRCHOME =`echo $(HOME)/Blue-X-ray-Eraser/1.server`
APPHOME =`echo $(BXRG_HOME)`

INC = -I$(SRCHOME)/inc
LIB = -L$(SRCHOME)/lib

INILIB = -L$(SRCHOME)/lib/iniparser-4.1/lib64
INIINC = -I$(SRCHOME)/lib/iniparser-4.1/include

# application compiler options
CFLAGS = -g -fPIC -m64 -D_BXDBG

# library linker options
LFLAGS = -shared
LLIBS = -lpthread

LIBS = $(LIB) $(INILIB) -liniparser 
INCS = $(INI) $(INIINC) 

# application linker options
LDFLAGS = $(LIB)

#.SUFFIXES: .c .cpp .cxx .o
CORULE = $(CC) -c $(@F:.o=.c) $(DEBUG) $(CFLAGS) $(INC) $(INIINC)

.c.o:
	$(CORULE)

sources = bxr_utils.o

out=$(LIBNAME).$(LIBVER)

all: $(out)
$(out): $(sources)
	$(CC) $(LFLAGS) -o $(out) $(sources) $(INCS) $(LIBS) $(LLIBS)
	ln -sf $(out) $(LIBNAME)

install:
	cp -arp $(LIBNAME)* $(SRCHOME)/lib/.
	sudo find $(APPHOME)/lib -maxdepth 1 -name $(LIBNAME) -exec mv {} {}.$(DATETIME) \;
	sudo cp -arp $(LIBNAME)* $(APPHOME)/lib/.

clean :
	rm -rf *.o $(LIBNAME)*

