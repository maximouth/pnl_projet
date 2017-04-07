#!/bin/bash

cd projet
dmesg -C
insmod our_mod.ko
mknod /dev/hello c 245 0
dmesg
