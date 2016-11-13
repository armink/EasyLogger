#!/bin/bash
gcc -I "easylogger/inc" -I "../../../easylogger/inc" -O0 -g3 -Wall -c "../../../easylogger/src/elog.c" -o "out/elog.o" 
gcc -I "easylogger/inc" -I "../../../easylogger/inc" -O0 -g3 -Wall -c "../../../easylogger/src/elog_async.c" -o "out/elog_async.o" 
gcc -I "easylogger/inc" -I "../../../easylogger/inc" -O0 -g3 -Wall -c "../../../easylogger/src/elog_buf.c" -o "out/elog_buf.o" 
gcc -I "easylogger/inc" -I "../../../easylogger/inc" -O0 -g3 -Wall -c "easylogger/port/elog_port.c" -o "out/elog_port.o"
gcc -I "easylogger/inc" -I "../../../easylogger/inc" -O0 -g3 -Wall -c  "../../../easylogger/src/elog_utils.c" -o "out/elog_utils.o"
gcc -I "easylogger/inc" -I "../../../easylogger/inc" -O0 -g3 -Wall -c "main.c" -o "out/main.o"
gcc -o out/EasyLoggerLinuxDemo "out/elog.o" "out/elog_async.o" "out/elog_buf.o" "out/elog_port.o" "out/elog_utils.o" "out/main.o" -lpthread
