#!/bin/sh
mkdir -p $2/FILE
echo "Hello FAT32 World!" > $2/FILE/MESSAGE1.TXT
echo "1234.5678" > $2/FILE/MESSAGE2.TXT
dd if=/dev/zero of=$1 bs=512 count=93750
parted $1 -s -a minimal mklabel gpt
parted $1 -s -a minimal mkpart EFI FAT32 2048s 93716s
parted $1 -s -a minimal toggle 1 boot
dd if=/dev/zero of=/tmp/part.img bs=512 count=91669
mformat -F -i /tmp/part.img -h 32 -t 32 -n 64 -c 1
mcopy -s -i /tmp/part.img  $2/* ::
dd if=/tmp/part.img of=$1 bs=512 count=91669 seek=2048 conv=notrunc
rm /tmp/part.img