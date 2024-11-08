#!/bin/bash

if [ -z $1 ]; then
  echo "Usage: $0 <cflags>..."
  exit 1
fi

CFLAGS=$*

echo
echo "gcc -march=native $CFLAGS"
echo
scripts/build-test.sh asconaead gcc -march=native $CFLAGS
echo
echo "arm-linux-gnueabi-gcc -march=armv7-a $CFLAGS"
echo
scripts/build-test.sh asconaead arm-linux-gnueabi-gcc -march=armv7-a $CFLAGS
echo
echo "arm-linux-gnueabi-gcc -march=armv7-m $CFLAGS"
echo
scripts/build-test.sh asconaead arm-linux-gnueabi-gcc -march=armv7-m $CFLAGS
echo
echo "arm-none-eabi-gcc -march=armv6-m $CFLAGS"
echo
scripts/build-test.sh asconaead arm-none-eabi-gcc -march=armv6-m --specs=picolibc.specs $CFLAGS
echo
echo "riscv64-unknonw-elf-gcc -march=rv32i $CFLAGS"
echo
scripts/build-test.sh asconaead riscv64-unknown-elf-gcc -march=rv32i -mabi=ilp32 --specs=picolibc.specs $CFLAGS
echo
echo "xtensa-lx106-elf-gcc $CFLAGS"
echo
scripts/build-test.sh asconaead xtensa-lx106-elf-gcc $CFLAGS
echo
echo "avr-gcc -mmcu=atmega2560 $CFLAGS"
echo
scripts/build-test.sh asconaead avr-gcc -mmcu=atmega2560 $CFLAGS
echo
echo "clang -march=native $CFLAGS"
echo
scripts/build-test.sh asconaead clang -march=native
echo
echo "clang -march=armv7-a $CFLAGS"
echo
scripts/build-test.sh asconaead clang --target=arm-linux-gnueabi -march=armv7-a $CFLAGS
echo
echo "clang -march=armv7-m $CFLAGS"
echo
scripts/build-test.sh asconaead clang --target=arm-linux-gnueabi -march=armv7-m $CFLAGS
echo
echo "clang -march=armv6-m $CFLAGS"
echo
scripts/build-test.sh asconaead clang --target=arm-none-eabi -march=armv6-m -I/usr/lib/picolibc/arm-none-eabi/include $CFLAGS
echo
echo "clang -march=rv32i $CFLAGS"
echo
scripts/build-test.sh asconaead clang --target=riscv64-unknown-elf -march=rv32i -mabi=ilp32 -I/usr/lib/picolibc/arm-none-eabi/include $CFLAGS
