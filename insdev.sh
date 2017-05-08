#!/bin/sh

module=$(echo "$1" | cut -f 1 -d '.')

dmesg -C
/sbin/insmod ./$module.ko || exit 1
rm -f /dev/$module
major=$(awk "\$2==\"$module\" {print \$1;exit}" /proc/devices)
mknod /dev/$module c $major 0
chmod 666 /dev/$module
dmesg
echo "=> Device /dev/$module created with major=$major"
echo ""
