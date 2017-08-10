CC      =g++
TARGET  =main
DIR_INC =${PWD}/include
DIR_SRC =${PWD}/src
DIR_OBJ =${PWD}/obj
SRC	=$(wildcard ${DIR_SRC}/*.c)
OBJ	=$(patsubst %.c,${DIR_OBJ}/%.o,$(notdir ${SRC}))
CFLAGS	=-I$(DIR_INC) `pkg-config --cflags opencv --libs opencv` -levent 
${TARGET}:${OBJ}
	$(CC)   -o      $@      $^      $(CFLAGS)
${DIR_OBJ}/%.o:${DIR_SRC}/%.c
	$(CC)   $(CFLAGS)       -c      $<      -o      $@
.PHONY:clean
clean:
	rm -rf  ${DIR_OBJ}/*.o
	rm -rf  ./main
