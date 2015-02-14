###############################################################
#
# Purpose: Makefile for "camera_tutorial"
# Author.: robotis
# Version: 0.1
# License: GPL
#
###############################################################

TARGET = camera_tutorial

INCLUDE_DIRS  = -I../../../include -I../../../../Framework/include
#INCLUDE_DIRS = -I../../include   -I../../../Framework/include



CXX = g++
#CXXFLAGS += -O2 -DLINUX -Wall $(INCLUDE_DIRS)
CXXFLAGS +=	-O2 -DLINUX -g -Wall -fmessage-length=0 $(INCLUDE_DIRS)
LIBS += -lpthread -lncurses -lrt -lbluetooth




#CXXFLAGS += -O2 -DDEBUG -DLINUX -Wall $(INCLUDE_DIRS)
LFLAGS += -lpthread -ljpeg

OBJECTS =   main.o

all: darwin.a $(TARGET)
#all: $(TARGET)

clean:
	rm -f *.a *.o $(TARGET) core *~ *.so *.lo

darwin.a:
	make -C ../../../build
#	make -C ../../build

$(TARGET): darwin.a $(OBJECTS)
	$(CXX) $(CFLAGS) $(OBJECTS) ../../lib/darwin.a -o $(TARGET) $(LFLAGS)
	chmod 755 $(TARGET)

# useful to make a backup "make tgz"
tgz: clean
	mkdir -p backups
	tar czvf ./backups/camera_`date +"%Y_%m_%d_%H.%M.%S"`.tgz --exclude backups *
