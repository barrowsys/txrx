PORT = /dev/ttyUSB0
PORT_SPEED = 9600

PROJ_FILE = USB0.ino

verify:
	arduino --verify $(PROJ_FILE) --port $(PORT)

upload:
	arduino --upload $(PROJ_FILE) --port $(PORT)

serial:
	cu -s $(PORT_SPEED) -l $(PORT)

watch:
	watchexec --exts h,ino,cpp --on-busy-update restart 'make upload serial'
