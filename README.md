# ch32v003_daisy

CH32V003s can program by using single wire SWIO bus. Also, SWIO can be use to debug mcu using some registers like DMDATA0 and DMDATA1. With some bit-banging we can use V003 as master SWIO device. 
X035s have PIOC peripheral to do this instead of bit-banging. Also RP2040 and ESP32 can be used for master device. Check [PicoRVD](https://github.com/aappleby/picorvd) and [cnlohr's ESP32S2 Programmer](https://github.com/cnlohr/esp32s2-cookbook/tree/master/ch32v003programmer).

I try to create a simple protocol to use these V003s like neopixel. It can be used for chainable rotary encoder drivers, led drivers, relay drivers, button drives, etc.

Master <-> V003 (id: 0x0) <-> V003 (id: 0x1) <-> V003 (id: 0x2)
0x20AA0001 -> 0x04 = Write 0001 to AA register, device 2 

## Write Data
Send 0x (chip id: 0-f) (write state: 0) (2 bytes for internal register) (4 bytes for data) to DMDATA0.
Each slave device checks chip id. If it's zero, then use the command. Else decrement chip address by one then sends to next device.

### Examples
Master DMDATA0 -> Device 0 (0x20AA0001) -> Device 1 (0x10AA0001) -> Device 2 (0x00AA0001) -> Stop

## Read Data
### Master Device View
Send 0x (chip id: not checked) (read state clear: 3) (6 bytes zero) to DMDATA0.
Every chip clears its DMDATA1 to 0x00000000.

Send 0x (chip id: 0-f) (read state: 1) (2 bytes for internal register) (4 bytes zero) to DMDATA0.
Read DMDATA1 periodically, process it if its different than 0x00000000.
Parse DMDATA1, 0x (chip id: 0-f) (read complete: 2) (2 bytes for internal register) (4 bytes for data).

### Slave Device View
Each slave device checks DMDATA1 of next device and replaces its DMDATA1 if it's different than 0x00000000.
This feature runs repeatedly when read mode activated. If command is read state clear, then disable read mode.
Only requested device can modify its DMDATA1.

### Examples
Master DMDATA0 -> Device 0 (0x03000000) -> Device 1 (0x03000000) -> Device 2 (0x03000000) -> Stop
Master DMDATA0 -> Device 0 (0x11AA0000) -> Device 1 (0x01AA0000) -> Stop
Master DMDATA1 -> Check cont. different that 0x00000000 
