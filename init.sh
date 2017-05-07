#!/bin/bash

cd projet
dmesg -C

insmod our_mod.ko
mknod /dev/hello c 245 0

dmesg

make -f Makefile_app
gcc toto.c

./a.out &

gcc -o sleep sleep.c

./sleep &


ps

./Projet.x /dev/hello
