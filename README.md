dont expect anything to stay the same or even work at all im just fuckin around

https://youtu.be/D0HsFuK1VK4  
https://youtu.be/E8eAyrQ8ePQ  

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
