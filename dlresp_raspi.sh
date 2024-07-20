#!/bin/bash

if [ "$EUID" -ne 0 ]
  then echo "Please run as root"
  exit
fi

if [ -z "$1" ]
  then echo "No EDID file supplied"
  exit
fi

modprobe dwc2
modprobe libcomposite

mkdir -p /sys/kernel/config/usb_gadget/g0

pushd .
cd /sys/kernel/config/usb_gadget/g0

echo 0x17e9 > idVendor
echo 0x0211 > idProduct
echo 255 > bDeviceClass
echo 0x0002 > bcdDevice

# mkdir strings/0x409
# echo "0211-001337" > strings/0x409/serialnumber
# echo "DisplayLink" > strings/0x409/manufacturer
# echo "USB Display Adapter" > strings/0x409/product

mkdir functions/ffs.dlresp
mkdir configs/c.1
ln -s functions/ffs.dlresp configs/c.1/.

popd
mkdir -p /dev/ffs-dlresp
mount -t functionfs dlresp /dev/ffs-dlresp
./displaylink-responder /dev/ffs-dlresp $1 &

ls /sys/class/udc | sudo tee /sys/kernel/config/usb_gadget/g0/UDC

echo "Done! Now use displaylink-framedump to extract frame."
