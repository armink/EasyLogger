CC = cc

ROOTPATH=../../..
INCLUDE = -I./easylogger/inc  -I./easylogger/plugins/ -I$(ROOTPATH)/easylogger/plugins/ -I$(ROOTPATH)/easylogger/inc
LIB=-lpthread

OBJ += $(patsubst %.c, %.o, $(wildcard *.c))
OBJ += $(patsubst %.c, %.o, $(wildcard $(ROOTPATH)/easylogger/src/*.c))
OBJ += $(patsubst %.c, %.o, $(wildcard $(ROOTPATH)/easylogger/plugins/file/elog_file.c))
OBJ += $(patsubst %.c, %.o, $(wildcard easylogger/port/*.c))
OBJ += $(patsubst %.c, %.o, $(wildcard easylogger/plugins/file/*.c))

CFLAGS = -O0 -g3 -Wall
target = EasyLoggerLinuxDemo

all:$(OBJ)
	$(CC) out/*.o -o $(target) $(LIB)
	mv $(target) out
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@ $(INCLUDE)
	mv $@ out
clean:
	rm -rf out/*
