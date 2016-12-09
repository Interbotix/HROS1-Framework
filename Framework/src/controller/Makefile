#############################################################################
#
#	Makefile for building PS3ControllerTest
#
#############################################################################

ifeq ($(DEBUG),y)
	CFLAGS += -O -g		# -O is need to expand inlines
else
	CFLAGS += -O2 #-I/root/gumstix-buildroot/build_arm_nofpu/staging_dir/usr/include 
endif

TARGET	= PS3Controller
PWD     := $(shell pwd)

CC = g++

#
# If you need additional serch paths for include files, then use the -I flag
# and add them to the CPPFLAGS variable
#

# CPPFLAGS += -I somedir

#
# If you need addional search paths for library files, then use the -L flag
# and add them to LDFLAGS.
#

LDFLAGS += -lbluetooth -lusb

#
# If you need additional libraries, then use -lxxx to search for libxxx.a
#

LDLIBS += -lpthread

.PHONY: all

all:	depend $(TARGET) 

#
# You can change the $(TARGET).c if that's not your main file and you can
# add additional .o files to the end of the line
#

SRCS	= JoystickController.cpp
OBJS	= $(SRCS:.cpp=.o)

$(TARGET) : $(OBJS)
clean:
	rm -rf $(OBJS) core .depend $(TARGET)

depend .depend dep:
		$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -M $(SRCS) > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif
