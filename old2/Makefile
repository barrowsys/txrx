sync_header:
	rm -f USB0/NanoNet.h USB2/NanoNet.h new_USB0/NanoNet.old.h
	ln `pwd`/NanoNet.old.h USB0/NanoNet.h
	ln `pwd`/NanoNet.old.h USB2/NanoNet.h
	ln `pwd`/NanoNet.old.h new_USB0/NanoNet.old.h

verify_all: verify_USB0 verify_USB2 verify_new_USB0

verify_USB0:
	arduino --verify USB0/USB0.ino
verify_new_USB0:
	arduino --verify new_USB0/new_USB0.ino
verify_USB2:
	arduino --verify USB2/USB2.ino

serial_USB0:
	cu -s 9600 -l /dev/ttyUSB0
serial_USB2:
	cu -s 9600 -l /dev/ttyUSB2
