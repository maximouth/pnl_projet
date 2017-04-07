#!/bin/bash

rm -f vm_root/projet/our_mod.ko

make clean all

cp our_mod.ko vm_root/projet/

./qemu-run-externKernel.sh
