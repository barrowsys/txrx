dont expect anything to stay the same or even work at all im just fuckin around

https://youtu.be/D0HsFuK1VK4  
https://youtu.be/E8eAyrQ8ePQ  

## rewrite/
this is the most recent version of the code and therefore the only one u should care about  

the makefile helps you out with getting it compiled and uploaded  
be sure to set PORT, ARDUINO_FQBN, and ARDUINO_DIR to the correct values.  
you can change WATCH, or you can install watchexec with `cargo install watchexec`

`make upload` compiles the code and uploads it to PORT  
`make serial` opens PORT with cu(1)  
`make watch` reruns upload and serial automatically when any files in the directory change

`upload_2`, `serial_2`, and `watch_2` do the same things but to a second arduino. NN_ADDR1 and NN_ADDR2 are swapped, the code is compiled for ARDUINO2_FQBN, and is uploaded to PORT2.

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
Valid values are 0x06 ACK (indicating successful RX) or 0x15 NACK (requesting retransmission)

| Options     | Name* | Description                  |
|-------------|-------|------------------------------|
| Bit 7 (128) | TITY  | opTions In Transit Yet^^     |
| Bit 6 (64)  | RACK  | Request ACKnowledge          |
| Bit 5 (32)  |       |                              |
| Bit 4 (16)  |       |                              |
| Bit 3 (8)   |       |                              |
| Bit 2 (4)   |       |                              |
| Bit 1 (2)   | BSOM  | Break Some Obstinate Member^ |
| Bit 0 (1)   |       |                              |


(*): look i may be gay but have you SEEN women???? how can u not

(^): If any header bytes after the first two are 0x02 it will break compatibility with old firmwares, so a length of 2 is considered invalid.
The BSOM bit of options should always be 0 to prevent this, although any other bit being set also works.
If you fail to observe this warning, i will cry. it will be extremely sad.
Every time 0x02 is transmitted in a nanonet header a baby arduino running nanonet is abandoned by its parents.
And theres only, like, 4 arduinos running nanonet on the planet. so dont do that.
i just forced u 2 read this utterly stupid footnote. rekt.

^^: 0 if this is the last options byte, 1 if there will be another. i have no idea what i would use a second options byte for or what else i would use a successive byte for????? idk dawg i just wanted to joke about breasts

## old/
the subdirectories are arduino projects.  
the NanoNet.h files in them are hardlinked to the top directory's version.  
run ./update_links.sh to re-link them all.

###### Frame Structure
|      | Preamble | Destination | Source | Start Text | Payload | End Text | CRC     | Ending |
|------|----------|-------------|--------|------------|---------|----------|---------|--------|
| Data | 0xFF01   | 1 byte      | 1 byte | 0x02       |         | 0x03     | 2 bytes | 0x04   |
| CRC? | No       | Yes         | Yes    | No         | Yes     | No       | No      | No     |

## old2/
This was all the garbage i had in rewrite/ until i abstracted it all down into a makefile and preprocessor stuff  
Probably not very useful but im a hoarder  
