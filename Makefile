###############################################################
# THIS FILE IS LICENSED UNDER THE FOLLOWING TERMS             #
#                                                             #
# this code may not be used for any purpose. be gay, do crime #
#                                                             #
# THE FOLLOWING MESSAGE IS NOT A LICENSE                      #
#                                                             #
# <barrow@tilde.team> wrote this file.                        #
# by reading this message, you are reading "TRANS RIGHTS".    #
# this file and the content within it is the queer agenda.    #
# if we meet some day, and you think this stuff is worth it,  #
# you can buy me a beer, tea, or something stronger.          #
# -Ezra Barrow                                                #
###############################################################

CLOCK_PIN 	= 2
DATA_PIN 	= 4
STATUS_PIN 	= 13
TX_RATE 	= 20
DEBUG_C 	= -1 #compiletime debug level
DEBUG_R 	= 4 #runtime debug level

ifndef DEVICE
	DEVICE = 1
endif
ifndef DEVICE_LOADED
	include Makefile.$(DEVICE)
endif

ARDUINO_DIR = $(HOME)/Apps/arduino
ARDUINO_CLI = $(ARDUINO_DIR)/bin/arduino-cli
COMPILE_FLAGS = -b $(ARDUINO_FQBN) --build-path $(BUILD_DIR) --clean
UPLOAD_FLAGS  = $(COMPILE_FLAGS) --upload --verify -p $(PORT)

FLAGS += -DDEVICE=$(DEVICE)
FLAGS += -DMAKE_OVERRIDE
FLAGS += -DMY_ADDR=$(NN_ADDR1)
FLAGS += -DNN_CLOCK_PIN=$(CLOCK_PIN)
FLAGS += -DNN_DATA_PIN=$(DATA_PIN)
FLAGS += -DNN_STATUS_PIN=$(STATUS_PIN)
FLAGS += -DNN_TX_RATE=$(TX_RATE)
FLAGS += -DLOG_LEVEL_C=$(DEBUG_C)
FLAGS += -DLOG_LEVEL_R=$(DEBUG_R)
BUILD_FLAGS = $(FLAGS)

WATCH = watchexec
WATCH_FLAGS = --on-busy-update restart #--exts h,ino,Makefile,cpp
WATCH_COMMAND = make upload serial

default:

mktemp:
	$(eval BUILD_DIR := $(shell mktemp -d /tmp/arduino.XXXXXXXX))

compile: mktemp
	$(ARDUINO_CLI) compile $(COMPILE_FLAGS) --build-property "build.extra_flags=$(BUILD_FLAGS)"

upload: mktemp
	$(ARDUINO_CLI) compile $(UPLOAD_FLAGS) --build-property "build.extra_flags=$(BUILD_FLAGS)"

serial:
	cu -s $(PORT_SPEED) -l $(PORT)

watch:
	$(WATCH) $(WATCH_FLAGS) "$(WATCH_COMMAND)"
