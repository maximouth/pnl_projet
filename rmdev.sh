#!/bin/sh

module=$(echo $1 | cut -f 1 -d '.')

/sbin/rmmod $module || exit 1
rm -f /dev/$module
dmesg
echo "=> Device /dev/$module removed"
echo ""
dmesg -C
