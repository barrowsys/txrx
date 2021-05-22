dont expect anything to stay the same or even work at all im just fuckin around

https://youtu.be/D0HsFuK1VK4  
https://youtu.be/E8eAyrQ8ePQ  

the main makefile (Makefile) helps you out with getting it compiled and uploaded  
you can change WATCH, or you can install watchexec with `cargo install watchexec`  

`make upload` compiles the code and uploads it to PORT  
`make serial` opens PORT with cu(1)  
`make watch` reruns upload and serial automatically when any files in the directory change

devices are defined in their own makefiles (see Makefile.1 and Makefile.2)  
be sure to set PORT, ARDUINO_FQBN, and ARDUINO_DIR to the correct values.  
device 1 is defined by Makefile.1, and can be loaded into the primary makefile with `make DEVICE=1` (default).  
device 2 is defined by Makefile.2, and can be loaded with `make DEVICE=2`.  
setting make to load the device files directly (`make -f Makefile.2`) also works.  
u can make as many device files as u want.  

###### Frame Structure
|      | Preamble | Header  | STX  | Payload     | ETX  | Trailer | EOT  |
|------|----------|---------|------|-------------|------|---------|------|
| Data | 0xFF01   | 4 bytes | 0x02 | 0-255 bytes | 0x03 | 3 bytes | 0x04 |
| CRC  | No       | Yes     | No   | Yes         | No   | No      | No   |

| Header | Destination | Source | Length  | Options   |
|--------|-------------|--------|---------|-----------|
| Data   | 1 byte      | 1 byte | 1 byte^ | 1+ bytes^ |

| Trailer | CRC     | Reply  |
|---------|---------|--------|
| Data    | 2 bytes | 1 byte |
Reply is transmitted by the reciever of a frame if the RACK bit was set.
Otherwise, if RACK bit is unset, the byte is skipped.
Valid values are 0x06 ACK (indicating successful RX) or 0x15 NACK (requesting retransmission)

| Options     | Name* | Description                  |
|-------------|-------|------------------------------|
| Bit 7 (128) |       |                              |
| Bit 6 (64)  | RACK  | Request ACKnowledge          |
| Bit 5 (32)  |       |                              |
| Bit 4 (16)  |       |                              |
| Bit 3 (8)   |       |                              |
| Bit 2 (4)   |       |                              |
| Bit 1 (2)   |       |                              |
| Bit 0 (1)   |       |                              |
