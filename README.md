# dlresp (displaylink responder)

Tested on Raspberry Pi Zero W with Windows 11 host.

Run `dlresp_raspi.sh` on Raspberry Pi with USB gadget support.

Packet decryption is not implemented, but latest Windows driver sends unencrypted packets.

![screendump](https://github.com/user-attachments/assets/10b0e568-bcea-47c6-9849-65c8403c8378)

## Usage

    ./dlresp <FFSPATH> <EDIDFILE>
    ./dlresp-dumpscreen [OUTFILEPATH]

## Resources

 - [displaylink-compression](https://github.com/kjy00302/displaylink-compression)
 - [tubecable](https://github.com/floe/tubecable)
