#!/bin/bash

if [ -z $1 ]; then
  echo "Usage: $0 <algorithms>..."
  exit 1
fi

mkdir -p size

echo
for alg in $*; do
  echo ${alg}
  echo "gcc -march=x86-64-v4"
  scripts/benchmark-size.sh ${alg} gcc -march=native \
    > size/${alg}_gcc_x86-64-v4.md
  echo "arm-linux-gnueabi-gcc -march=armv7-a"
  scripts/benchmark-size.sh ${alg} arm-linux-gnueabi-gcc -march=armv7-a \
    > size/${alg}_gcc_armv7-a.md
  echo "arm-linux-gnueabi-gcc -march=armv7-m"
  scripts/benchmark-size.sh ${alg} arm-linux-gnueabi-gcc -march=armv7-m \
    > size/${alg}_gcc_armv7-m.md
  echo "arm-none-eabi-gcc -march=armv6-m"
  scripts/benchmark-size.sh ${alg} arm-none-eabi-gcc -march=armv6-m --specs=picolibc.specs \
    > size/${alg}_gcc_armv6-m.md
  echo "riscv64-unknown-elf-gcc -march=rv32i"
  scripts/benchmark-size.sh ${alg} riscv64-unknown-elf-gcc -march=rv32i -mabi=ilp32 --specs=picolibc.specs \
    > size/${alg}_gcc_rv32i.md
  echo "xtensa-lx106-elf-gcc"
  scripts/benchmark-size.sh ${alg} xtensa-lx106-elf-gcc \
    > size/${alg}_gcc_esp32.md
  echo "avr-gcc -mmcu=atmega2560"
  scripts/benchmark-size.sh ${alg} avr-gcc -mmcu=atmega2560 \
    > size/${alg}_gcc_avr.md
  echo "clang -march=x86-64-v4"
  scripts/benchmark-size.sh ${alg} clang -march=native \
    > size/${alg}_clang_x86-64-v4.md
  echo "clang -march=armv7-a"
  scripts/benchmark-size.sh ${alg} clang --target=arm-linux-gnueabi -march=armv7-a \
    > size/${alg}_clang_armv7-a.md
  echo "clang -march=armv7-m"
  scripts/benchmark-size.sh ${alg} clang --target=arm-linux-gnueabi -march=armv7-m \
    > size/${alg}_clang_armv7-m.md
  echo "clang -march=armv6-m"
  scripts/benchmark-size.sh ${alg} clang --target=arm-none-eabi -march=armv6-m -I/usr/lib/picolibc/arm-none-eabi/include \
    > size/${alg}_clang_armv6-m.md
  echo "clang -march=rv32i"
  scripts/benchmark-size.sh ${alg} clang --target=riscv64-unknown-elf -march=rv32i -mabi=ilp32 -I/usr/lib/picolibc/arm-none-eabi/include \
    > size/${alg}_gcc_rv32i.md
  echo
done
