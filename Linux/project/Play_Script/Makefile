###############################################################
#
# Purpose: Makefile for "play_script"
# Author.: robotis
# Version: 0.1
# License: GPL
#
###############################################################

TARGET = play_script

CXX = g++
INCLUDE_DIRS = -I../../include -I../../../Framework/include
CXXFLAGS +=  -std=c++0x -O2 -DLINUX -g -Wall -fmessage-length=0 $(INCLUDE_DIRS)
LIBS += -lpthread -lncurses -lrt -ljpeg

OBJS =	./cmd_process.o \
		./main.o 


all: darwin.a $(TARGET)

darwin.a:
	make -C ../../build

$(TARGET): $(OBJS) ../../lib/darwin.a
	$(CXX) -o $(TARGET) $(OBJS) ../../lib/darwin.a $(LIBS)
	
clean:
	rm -f $(OBJS) $(TARGET)





