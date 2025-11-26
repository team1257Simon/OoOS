#!/bin/sh
dd if=/dev/zero of=$1 bs=512 count=750000
parted $1 -s -a minimal mklabel gpt		\
	mkpart EFI FAT32 2048s 93716s		\
	mkpart primary ext4 46MiB 366MiB	\
	toggle 1 boot
dd if=/dev/zero of=/tmp/part0.img bs=512 count=91669
mformat -F -i /tmp/part0.img -h 32 -t 32 -n 64 -c 1
mcopy -s -i /tmp/part0.img  $2/* ::
dd if=/tmp/part0.img of=$1 bs=512 count=91669 seek=2048 conv=notrunc
rm /tmp/part0.img
offs=$((46*2048*512))
size=$(((366-46)*2048*512))
touch /tmp/part1
fuseloop -O "$offs" -S "$size" $1 /tmp/part1
mkfs.ext4 -O ^dir_index -d $3 /tmp/part1
fusermount -u /tmp/part1
rm /tmp/part1