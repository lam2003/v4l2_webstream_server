CC      =g++
TARGET  =main
DIR_INC =${PWD}/include
DIR_SRC =${PWD}/src
DIR_OBJ =${PWD}/obj
SRC	=$(wildcard ${DIR_SRC}/*.cpp)
OBJ	=$(patsubst %.cpp,${DIR_OBJ}/%.o,$(notdir ${SRC}))
CFLAGS	=-I$(DIR_INC) `pkg-config --cflags opencv --libs opencv` -levent -lavcodec -lavformat -lswscale -lliveMedia -lBasicUsageEnvironment -lgroupsock -lUsageEnvironment -lx264 
${TARGET}:${OBJ}
	$(CC)   -o      $@      $^      $(CFLAGS)
${DIR_OBJ}/%.o:${DIR_SRC}/%.cpp
	$(CC)   $(CFLAGS)       -c      $<      -o      $@
.PHONY:clean
clean:
	rm -rf  ${DIR_OBJ}/*.o
	rm -rf  ./main
