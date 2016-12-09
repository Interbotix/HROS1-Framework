###############################################################
#
# Purpose: Makefile for "DARwIn/HROS Linux Framework"
# Author.: robotis & interbotix
# Version: 0.1
# License: GPL
#
###############################################################

CC = g++ -fPIC
AR = ar
ARFLAGS = cr

TARGET = darwin.a

INCLUDE_DIRS = -I../include -I../../Framework/include 

CXXFLAGS += -fPIC -shared -O2 -DLINUX -D_GNU_SOURCE -Wall -g $(INCLUDE_DIRS) 
#CXXFLAGS += -O2 -DDEBUG -DLINUX -D_GNU_SOURCE -Wall -shared $(INCLUDE_DIRS)
LFLAGS += -g -lpthread -ldl -lbluetooth -lncurses

OBJS =  ../../Framework/src/ArbotixPro.o     	\
        ../../Framework/src/math/Matrix.o   \
        ../../Framework/src/math/Plane.o    \
        ../../Framework/src/math/Point.o    \
        ../../Framework/src/math/Vector.o   \
		../../Framework/src/math/QuadraticStateTransform.o   \
		../../Framework/src/math/MotionState.o   \
        ../../Framework/src/motion/JointData.o  	\
        ../../Framework/src/motion/Kinematics.o 	\
        ../../Framework/src/motion/MotionManager.o  \
        ../../Framework/src/motion/MotionStatus.o   \
		../../Framework/src/motion/AngleEstimator.o \
        ../../Framework/src/motion/modules/Action.o \
        ../../Framework/src/motion/modules/Head.o   \
        ../../Framework/src/motion/modules/Walking.o\
        ../../Framework/src/vision/BallFollower.o   \
        ../../Framework/src/vision/PS3BallFollower.o   \
        ../../Framework/src/vision/LineFollower.o   \
        ../../Framework/src/vision/RobotFollower.o   \
        ../../Framework/src/vision/ConnectRegions.o   \
        ../../Framework/src/vision/RadonTransform.o   \
        ../../Framework/src/vision/BallTracker.o    \
        ../../Framework/src/vision/ColorFinder.o    \
        ../../Framework/src/vision/Image.o  		\
        ../../Framework/src/vision/ImgProcess.o 	\
        ../../Framework/src/minIni/minIni.o	\
        ../../Framework/src/controller/JoystickController.o	\
        streamer/httpd.o           \
        streamer/jpeg_utils.o      \
        streamer/mjpg_streamer.o   \
        LinuxActionScript.o   \
        LinuxCamera.o   \
        LinuxArbotixPro.o    \
        LinuxMotionTimer.o    \
        LinuxNetwork.o

$(TARGET): $(OBJS)
	mkdir -p ../lib/
	$(AR) $(ARFLAGS) ../lib/$(TARGET) $(OBJS) 
#	$(CC) -o $(TARGET) $(OBJS) $(LFLAGS)

all: $(TARGET)

clean:
	rm -f $(OBJS) ../lib/$(TARGET)

