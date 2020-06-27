CC = cc

ROOTPATH=../../..
INCLUDE = -I./easylogger/inc -I$(ROOTPATH)/easylogger/inc -I$(ROOTPATH)/easylogger/plugins/file
LIB=-lpthread

OBJ += $(patsubst %.c, %.o, $(wildcard *.c))
OBJ += $(patsubst %.c, %.o, $(wildcard $(ROOTPATH)/easylogger/src/*.c))
OBJ += $(patsubst %.c, %.o, $(wildcard $(ROOTPATH)/easylogger/plugins/file/elog_file.c))
OBJ += $(patsubst %.c, %.o, $(wildcard easylogger/port/*.c))

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
