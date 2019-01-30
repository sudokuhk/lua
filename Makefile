SHELL=/bin/sh

BASEDIR=.

TARGET=runlua

LIB_DIR=$(BASEDIR) 
OBJECT_DIR=$(BASEDIR)
SRC_DIR=$(BASEDIR)
INC_DIR=$(BASEDIR)


#$(shell mkdir -p ${LIB_DIR})
#$(shell mkdir -p ${BIN_DIR})
#$(shell mkdir -p ${OBJECT_DIR})

RM=rm -rf

#------------------

CC=gcc
CXX=g++
SHARED=-shared -o
FPIC=-c

LD_LIBS = -ldl -lm -llua
#-------------------

SRCS = main.c cmath.c cJSON.c json.c network.c memory.c url.c file.c
SRC_OBJECT := $(patsubst %.c,%.o,$(SRCS))

.PHONY:all
all:$(TARGET)

$(TARGET):$(SRC_OBJECT) 
	$(CC) -g -O0 $^ $(LD_LIBS) -I. -o $@

%.o : %.c
	$(CC) -g -O0 -I. -c $^ -o $@
    
clean:
	-$(RM) $(TARGET)
	-$(RM) *.o
