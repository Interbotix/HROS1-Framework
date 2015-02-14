SHELL := /bin/sh

.SUFFIXES:
.SUFFIXES: .c .o .cpp

DESTDIR = 

prefix = /usr/local
bindir = $(prefix)/bin
srcdir = .

INSTALL = install
INSTALL_PROGRAM = $(INSTALL)

PROGRAM := CommanderTest
OBJFILES := CommanderTest.o SerialInputCommander.o

CXX = g++
INCLUDE_DIRS = -I../include -I../../Framework/include 
INCLUDE_DIRS = -I../../../include -I../../../../Framework/include
CXXFLAGS +=	-O2 -DLINUX -g -Wall -fmessage-length=0 $(INCLUDE_DIRS)
LIBS += -lpthread -lrt

OBJS =		../../Framework/src/commander/SerialInputCommander.o	\



CommanderTest: $(OBJFILES) 

CommanderTest.o: CommanderTest.cpp SerialInputCommander.h

SerialInputCommander.o: SerialInputCommander.cpp SerialInputCommander.h
 
CFLAGS = -Wall -O2

XCPLAY_CFLAGS := -pthread -g
XCPLAY_LDFLAGS := -pthread -g

all: $(PROGRAM)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS) $(XCPLAY_CFLAGS)

%.o: %.cpp
	g++ -o $@ -c $< $(CFLAGS) $(XCPLAY_CFLAGS)


$(PROGRAM): $(OBJFILES)
	$(CC) -o $@ $(LDFLAGS) $(XCPLAY_LDFLAGS) $(OBJFILES)

clean:
	rm -rf $(PROGRAM) dist *.o *.tar.gz
