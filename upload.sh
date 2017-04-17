#!/bin/bash

rm -f vm_root/projet/our_mod.ko

umount vm_root/
mount root.img vm_root/



make clean all

cp our_mod.ko vm_root/projet/
cp our_mod.h  vm_root/projet/
cp main.c     vm_root/projet/
cp init.sh    vm_root/

sync

./qemu-run-externKernel.sh

