PORT = /dev/ttyUSB0
PORT_SPEED = 9600
ARDUINO_FQBN = "arduino:avr:nano"
PORT2 = /dev/ttyUSB1
PORT2_SPEED = 9600
ARDUINO2_FQBN = "arduino:avr:nano"

NN_ADDR1 = 0x01
NN_ADDR2 = 0x02
DEBUG_LEVEL = 0
TX_RATE = 20

ARDUINO_DIR = $(HOME)/Downloads/arduino-1.8.13
ARDUINO_CLI = $(ARDUINO_DIR)/bin/arduino-cli
COMPILE_FLAGS = -b $(ARDUINO_FQBN) --build-path $(BUILD_DIR) --clean
UPLOAD_FLAGS  = $(COMPILE_FLAGS) --upload --verify -p $(PORT)

FLAGS += -DMY_ADDR=$(NN_ADDR1)
FLAGS += -DOTHER_ADDR=$(NN_ADDR2)
FLAGS += -D_DEBUG_LEVEL=$(DEBUG_LEVEL)
FLAGS += -DDEVICE=$(PORT)
FLAGS += -D_TX_RATE=$(TX_RATE)
BUILD_FLAGS = $(FLAGS)

WATCH = watchexec
WATCH_FLAGS = --on-busy-update restart 
WATCH_COMMAND = make upload serial

DEVICE_2 = compile_2 upload_2 watch_2 serial_2
$(DEVICE_2) : OLD_NN_ADDR1 := $(NN_ADDR1)
$(DEVICE_2) : OLD_NN_ADDR2 := $(NN_ADDR2)
$(DEVICE_2) : NN_ADDR1 := $(OLD_NN_ADDR2)
$(DEVICE_2) : NN_ADDR2 := $(OLD_NN_ADDR1)
$(DEVICE_2) : WATCH_COMMAND = make upload_2 serial_2
$(DEVICE_2) : PORT = $(PORT2)
$(DEVICE_2) : PORT_SPEED = $(PORT2_SPEED)
$(DEVICE_2) : ARDUINO_FQBN = $(ARDUINO2_FQBN)

default:

mktemp:
	$(eval BUILD_DIR := $(shell mktemp -d /tmp/arduino.XXXXXXXX))

compile compile_2: mktemp
	$(ARDUINO_CLI) compile $(COMPILE_FLAGS) --build-property "build.extra_flags=$(BUILD_FLAGS)"

upload upload_2: mktemp
	$(ARDUINO_CLI) compile $(UPLOAD_FLAGS) --build-property "build.extra_flags=$(BUILD_FLAGS)"

serial serial_2:
	cu -s $(PORT_SPEED) -l $(PORT)

watch watch_2:
	$(WATCH) $(WATCH_FLAGS) "$(WATCH_COMMAND)"
