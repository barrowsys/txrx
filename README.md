the interesting stuff is in transmitframe and recieveframe,, all three NanoNet.h files are hardlinked on my system

dont expect anything to stay the same ever im just fuckin around

https://youtu.be/D0HsFuK1VK4

# Frame Structure
|      | Preamble | Destination | Source | Start Text | Payload | End Text | CRC     | Ending |
| Data | 0xFF01   | 1 byte      | 1 byte | 0x02       |         | 0x03     | 2 bytes | 0x04   |
| CRC? | No       | Yes         | Yes    | No         | Yes     | No       | No      | No     |
