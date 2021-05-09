PORT = /dev/ttyUSB2
PORT_SPEED = 9600

PROJ_FILE = USB2.ino

verify:
	arduino --verify $(PROJ_FILE) --port $(PORT)

upload:
	arduino --upload $(PROJ_FILE) --port $(PORT)

serial:
	cu -s $(PORT_SPEED) -l $(PORT)

watch:
	watchexec --exts h,ino,cpp --on-busy-update restart 'make upload serial'
