#############################################################
#
# Purpose: Makefile for "apiWrapper"
# Author.: Andrew Alter & Daniel Alner
# Version: 0.1
#
###############################################################
TARGET = apiwrapper.so

CXX = g++ -fPIC
INCLUDE_DIRS = -I../../../include -I../../../../Framework/include
CXXFLAGS += -fPIC -shared -O2 -DLINUX -g -Wall -fmessage-length=0 $(INCLUDE_DIRS)
LIBS += -lpthread -lncurses -lrt -lbluetooth

OBJS = main.o

all: darwin.a $(TARGET)

darwin.a:
	make -C ../../../build

$(TARGET): $(OBJS) ../../../lib/darwin.a
	$(CXX)  -fPIC -shared -o $(TARGET) $(OBJS) ../../../lib/darwin.a $(LIBS)
	$(info *****  api_wrapper.so generated *****)
        
clean:
	rm -f $(OBJS) $(TARGET)





