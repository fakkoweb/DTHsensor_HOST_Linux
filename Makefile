###########################################
# Simple Makefile for HIDAPI test program
#
# Alan Ott
# Signal 11 Software
# 2010-06-01
###########################################
####  USAGE  ####
# Type MAKE in this folder to compile
# Type MAKE CLEAN in this folder to delete object and exec file
##############


all: usb_host


###########  CONFIGURATION  #############
####    All values must be separated by space!!     ####
#####################################
LIBS      = `pkg-config libusb-1.0 libudev libcurl --libs` -lpthread -lboost_system		# Put HERE Lib flags for the Linker (-l)
INCLUDES ?=  -Ilibs/includes -Iincludes `pkg-config libusb-1.0 libcurl --cflags`			# Put HERE Include flags for the compilers (-I)

CFLAGS   ?= -Wall -g												# Put HERE other flags for the gcc compiler
CXXFLAGS ?= -Wall -g -std=c++11 									# Put HERE other flags for the g++ compiler

COBJS     = libs/hidapi/hid-libusb.o												# Where are C files?
CPPOBJS   = libs/jsoncpp/jsoncpp.o libs/p_sleep.o main.o program/control.o program/sensors.o program/sensors_math.o program/drivers.o program/functions.o program/OMV.o program/http_manager.o	# Where are C++ files?

#####################################

OBJS      = $(COBJS) $(CPPOBJS)
CC       ?= gcc
CXX      ?= g++


usb_host: $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ $(LIBS) -o usb_host

$(COBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $(INCLUDES) $< -o $@

$(CPPOBJS): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $(INCLUDES) $< -o $@

clean:
	rm -f $(OBJS) $(CPPOBJS) usb_host

.PHONY: clean
