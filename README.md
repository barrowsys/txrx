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
