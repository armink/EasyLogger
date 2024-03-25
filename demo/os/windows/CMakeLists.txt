cmake_minimum_required(VERSION 3.26)
project(easylogger_demo)

set(CMAKE_C_STANDARD 99)

include_directories(
        ${PROJECT_SOURCE_DIR}
        easylogger/inc
        ../../../easylogger/inc
        ../../../easylogger/plugins/file
)

file(GLOB SOURCES
        main.c
        easylogger/port/*.c
        ../../../easylogger/src/*.c
        ../../../easylogger/plugins/file/elog_file.c
)

add_executable(${PROJECT_NAME} ${SOURCES})