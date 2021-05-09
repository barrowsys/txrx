PORT = /dev/ttyUSB2
PORT_SPEED = 9600

PROJ_FILE = new_USB2.ino

echo_config:
	@echo "PORT = $(PORT)"
	@echo "PORT_SPEED = $(PORT_SPEED)"
	@echo "PROJ_FILE = $(PROJ_FILE)"

verify:
	arduino --verify $(PROJ_FILE) --port $(PORT)

upload:
	arduino --upload $(PROJ_FILE) --port $(PORT)

serial:
	cu -s $(PORT_SPEED) -l $(PORT)

watch:
	watchexec --exts h,ino,cpp --on-busy-update restart 'make PORT=$(PORT) PORT_SPEED=$(PORT_SPEED) PROJ_FILE=$(PROJ_FILE) upload serial'
