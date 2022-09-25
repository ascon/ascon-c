#!/bin/bash

for o in 0 1 2 3; do
  for a in aead hash auth; do
    for i in crypto_*$a*/ascon*v12/*armv6m*; do
      rm -rf *.o
      echo "arm-none-eabi-gcc -mcpu=cortex-m0 -O$o -fomit-frame-pointer -Itests -I$i $i/*.[cS] tests/genkat_$a.c -c"
      arm-none-eabi-gcc -mcpu=cortex-m0 -O$o -fomit-frame-pointer -Itests -I$i $i/*.[cS] tests/genkat_$a.c -c
      arm-none-eabi-gcc -mcpu=cortex-m0 -O$o -fomit-frame-pointer --specs=nosys.specs -Wl,--gc-sections *.o
    done
  done
done
