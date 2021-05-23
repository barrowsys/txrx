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
DEBUG_C 	= -1 #compiletime debug level
DEBUG_R 	= 0 #runtime debug level
TX_RATE 	= 80

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

FLAGS += -DMY_ADDR=$(NN_ADDR1)
FLAGS += -DOTHER_ADDR=$(NN_ADDR2)
FLAGS += -DDEVICE=$(DEVICE)
FLAGS += -DMAKE_OVERRIDE
FLAGS += -D_CLOCK_PIN=$(CLOCK_PIN)
FLAGS += -D_DATA_PIN=$(DATA_PIN)
FLAGS += -D_STATUS_PIN=$(STATUS_PIN)
FLAGS += -D_TX_RATE=$(TX_RATE)
FLAGS += -D_MAX_DEBUG=$(DEBUG_C)
FLAGS += -D_DEBUG_LEVEL=$(DEBUG_R)
BUILD_FLAGS = $(FLAGS)

TEST_FILE = "28k.h"

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

upload_test: mktemp
	$(ARDUINO_CLI) compile $(UPLOAD_FLAGS) --build-property "build.extra_flags=$(BUILD_FLAGS) -DRUN_TEST=\"tests/$(TEST_FILE)\""

serial:
	cu -s $(PORT_SPEED) -l $(PORT)

watch:
	$(WATCH) $(WATCH_FLAGS) "$(WATCH_COMMAND)"

watch_test:
	$(WATCH) $(WATCH_FLAGS) "make upload_test serial"
