# Reference, highly optimized, masked C and ASM implementations of Ascon

Ascon is a family of lightweight cryptographic algorithms and consists of:
- Authenticated encryption schemes with associated data (AEAD)
- Hash functions (HASH) and extendible output functions (XOF)
- Pseudo-random functions (PRF) and message authentication codes (MAC)

All implementations use the "ECRYPT Benchmarking of Cryptographic Systems (eBACS)" interface:

- https://bench.cr.yp.to/call-aead.html for AEAD (Ascon-128, Ascon-128a, Ascon-80pq)
- https://bench.cr.yp.to/call-hash.html for HASH and XOF (Ascon-Hash, Ascon-Hasha, Ascon-Xof, Ascon-Xofa)
- https://nacl.cr.yp.to/auth.html for PRF and MAC (Ascon-Mac, Ascon-Prf, Ascon-PrfShort)

For more information on Ascon visit: https://ascon.iaik.tugraz.at/


## TL;DR

If you do not know where to start, use the reference implementations (self-contained, portable, very fast):

- `crypto_aead/ascon128v12/ref`
- `crypto_aead/ascon128av12/ref`
- `crypto_hash/asconxofv12/ref`
- `crypto_hash/asconxofav12/ref`


## Algorithms

This repository contains implementations of the following 10 Ascon v1.2 algorithms:

- `crypto_aead/ascon128v12`:   **Ascon-128**      (authenticated encryption)
- `crypto_aead/ascon128av12`:  **Ascon-128a**     (authenticated encryption)
- `crypto_aead/ascon80pqv12`:  **Ascon-80pq**     (authenticated encryption)
- `crypto_hash/asconhashv12`:  **Ascon-Hash**     (hash function)
- `crypto_hash/asconhashav12`: **Ascon-Hasha**    (hash function)
- `crypto_hash/asconxofv12`:   **Ascon-Xof**      (hash function with extendable output, XOF)
- `crypto_hash/asconxofav12`:  **Ascon-Xofa**     (hash function with extendable output, XOF)
- `crypto_auth/asconmacv12`:   **Ascon-Mac**      (message authentication code)
- `crypto_auth/asconprfv12`:   **Ascon-Prf**      (message authentication code with extendable output, PRF)
- `crypto_auth/asconprfsv12`:  **Ascon-PrfShort** (message authentication code for very short messages)

We also provide two combined algorithm implementations supporting both AEAD and
hashing:

- `crypto_aead_hash/asconv12`: Ascon-128 combined with Ascon-Hash
- `crypto_aead_hash/asconav12`: Ascon-128a combined with Ascon-Hasha

The following algorithms demonstrate the performance improvement of Ascon on
32-bit platforms without bit interleaving overhead. Bit interleaving could be
performed externally on the host side or using a dedicated instruction (e.g.
using the ARM Custom Datapath Extension). Note that a similar performance
improvement could be achieved using funnel shift instructions (available on some
32-bit RISC-V extensions).

- `crypto_aead/ascon128bi32v12`: Ascon-128 (+17% on ARM1176JZF-S)
- `crypto_aead/ascon128abi32v12`: Ascon-128a (+23% on ARM1176JZF-S)
- `crypto_hash/asconhashbi32v12`: Ascon-Hash (+5% on ARM1176JZF-S)
- `crypto_hash/asconhashabi32v12`: Ascon-Hasha (+8% on ARM1176JZF-S)
- `crypto_aead_hash/asconbi32v12`: Ascon-128 combined with Ascon-Hash
- `crypto_aead_hash/asconabi32v12`: Ascon-128a combined with Ascon-Hasha


## Implementations

For most algorithms, we provide the following pure C implementations:

- `ref`: reference implementation
- `opt64`: 64-bit speed-optimized
- `opt32`: 32-bit speed-optimized
- `opt64_lowsize`: 64-bit size-optimized
- `opt32_lowsize`: 32-bit size-optimized
- `bi32`: 32-bit speed-optimized bit-interleaved
- `bi32_lowreg`: 32-bit speed-optimized bit-interleaved (low register usage)
- `bi32_lowsize`: 32-bit size-optimized bit-interleaved
- `esp32`: 32-bit ESP32 optimized
- `opt8`: 8-bit size- and speed-optimized
- `bi8`: 8-bit optimized bit-interleaved

the following C with inline or partial ASM implementations:

- `avx512`: 320-bit speed-optimized AVX512
- `neon`: 64-bit speed-optimized ARM NEON
- `armv6`: 32-bit speed-optimized ARMv6
- `armv6m`: 32-bit speed-optimized ARMv6-M
- `armv7m`: 32-bit speed-optimized ARMv7-M
- `armv6_lowsize`: 32-bit size-optimized ARMv6
- `armv6m_lowsize`: 32-bit size-optimized ARMv6-M
- `armv7m_lowsize`: 32-bit size-optimized ARMv7-M
- `armv7m_small`: 32-bit small speed-optimized ARMv7-M
- `bi32_armv6`: 32-bit speed-optimized bit-interleaved ARMv6
- `bi32_armv6m`: 32-bit speed-optimized bit-interleaved ARMv6-M
- `bi32_armv7m`: 32-bit speed-optimized bit-interleaved ARMv7-M
- `bi32_armv7m_small`: 32-bit small bit-interleaved ARMv7-M
- `avr`: 8-bit size- and speed-optimized AVR
- `avr_lowsize`: 8-bit size-optimized AVR

the following ASM implementations:

- `asm_esp32`: 32-bit optimized ESP32 using funnel-shift instructions
- `asm_rv32i`: 32-bit optimized RV32I using the base instruction set
- `asm_rv32b`: 32-bit optimized RV32B using bitmanip base (Zbb)
- `asm_fsr_rv32b`: 32-bit optimized funnel-shift RV32B using bitmanip base and bitmanip terniary (ZbbZbt)
- `asm_bi32_rv32b`: 32-bit optimized bit-interleaved RV32B using bitmanip base and bitmanip permutations (ZbbZbp)

and the following high-level masked (shared) C with inline ASM implementations:

- `protected_bi32_armv6`: 32-bit masked bit-interleaved ARMv6
- `protected_bi32_armv6_leveled`: 32-bit masked and leveled bit-interleaved ARMv6

The masked C implementations can be used as a starting point to generate
device specific C/ASM implementations. Note that the masked C implementations
require a minimum amount of ASM instructions. Otherwise, the compiler may
heavily optimize the code and even combine shares. Obviously, the output
generated is very sensitive to compiler and environment changes and any
generated output needs to be security evaluated. A preliminary evaluation of
these implementations has been performed on some
[ChipWhisperer](https://www.newae.com/chipwhisperer) devices. The setup and
preliminary results can found at: https://github.com/ascon/simpleserial-ascon


# Performance results on different CPUs in cycles per byte

## Ascon-128a

| Message Length in Bytes  |    1 |    8 |   16 |   32 |   64 | 1536 | long |
|:-------------------------|-----:|-----:|-----:|-----:|-----:|-----:|-----:|
| AMD EPYC 7742\*          |      |      |      |      |  7.4 |  4.4 |  4.2 |
| AMD Ryzen 9 5950X\*      |      |      |      |      |  8.1 |  5.3 |  5.2 |
| Apple M1 (ARMv8)\*       |      |      |      |      |  9.4 |  6.3 |  6.3 |
| Cortex-A72 (ARMv8)\*     |      |      |      |      | 10.9 |  7.2 |  7.0 |
| Intel Xeon E5-2609 v4\*  |      |      |      |      | 11.3 |  7.4 |  7.2 |
| Intel Core i5-6300U      |  365 |   47 |   31 |   19 | 13.5 |  8.0 |  7.8 |
| Intel Core i5-4200U      |  519 |   67 |   44 |   27 | 18.8 | 11.0 | 10.6 |
| Cortex-A9 (ARMv7)\*      |      |      |      |      | 42.8 | 24.6 | 24.0 |
| Cortex-A7 (NEON)         | 2204 |  226 |  132 |   82 | 55.9 | 31.7 | 30.7 |
| Cortex-A7 (ARMv7)\*      |      |      |      |      | 55.5 | 38.2 | 37.5 |
| ARM1176JZF-S (ARMv6)     | 1908 |  235 |  156 |   99 | 70.4 | 43.0 | 42.9 |


## Ascon-128 and Ascon-80pq

| Message Length in Bytes  |    1 |    8 |   16 |   32 |   64 | 1536 | long |
|:-------------------------|-----:|-----:|-----:|-----:|-----:|-----:|-----:|
| AMD EPYC 7742\*          |      |      |      |      |  8.1 |  6.6 |  6.5 |
| AMD Ryzen 9 5950X\*      |      |      |      |      | 11.0 |  8.2 |  8.1 |
| Apple M1 (ARMv8)\*       |      |      |      |      | 12.5 |  9.5 |  9.3 |
| Cortex-A72 (ARMv8)\*     |      |      |      |      | 13.8 | 10.7 | 10.5 |
| Intel Xeon E5-2609 v4\*  |      |      |      |      | 14.9 | 10.8 | 10.6 |
| Intel Core i5-6300U      |  367 |   58 |   35 |   23 | 17.6 | 11.9 | 11.4 |
| Intel Core i5-4200U      |  521 |   81 |   49 |   32 | 23.9 | 16.2 | 15.8 |
| Cortex-A9 (ARMv7)\*      |      |      |      |      | 51.7 | 34.1 | 33.3 |
| Cortex-A7 (NEON)         | 2182 |  249 |  148 |   97 | 71.7 | 47.5 | 46.5 |
| Cortex-A7 (ARMv7)\*      |      |      |      |      | 69.6 | 52.0 | 51.6 |
| ARM1176JZF-S (ARMv6)     | 1921 |  277 |  167 |  112 | 83.7 | 57.2 | 56.8 |


## Ascon-Hasha and Ascon-Xofa

| Message Length in Bytes  |    1 |    8 |   16 |   32 |    64 | 1536 | long |
|:-------------------------|-----:|-----:|-----:|-----:|------:|-----:|-----:|
| AMD EPYC 7742\*          |      |      |      |      |       |      |      |
| AMD Ryzen 7 1700\*       |      |      |      |      |  22.0 | 12.1 | 11.7 |
| Apple M1 (ARMv8)\*       |      |      |      |      |       |      |      |
| Cortex-A72 (ARMv8)\*     |      |      |      |      |  22.2 | 14.5 | 14.2 |
| Intel Xeon E5-2609 v4\*  |      |      |      |      |  23.3 | 14.4 | 14.0 |
| Intel Core i5-6300U      |  550 |   83 |   49 |   33 |  23.7 | 15.6 | 15.5 |
| Intel Core i5-4200U      |  749 |  112 |   67 |   44 |  31.8 | 20.8 | 20.7 |
| Cortex-A9 (ARMv7)\*      |      |      |      |      |  87.5 | 45.6 | 44.0 |
| Cortex-A7 (ARMv7)\*      |      |      |      |      | 102.3 | 63.5 | 61.8 |
| ARM1176JZF-S (ARMv6)     | 2390 |  356 |  211 |  138 | 100.7 | 65.7 | 65.3 |


## Ascon-Hash and Ascon-Xof

| Message Length in Bytes  |    1 |    8 |   16 |   32 |    64 | 1536 | long |
|:-------------------------|-----:|-----:|-----:|-----:|------:|-----:|-----:|
| AMD EPYC 7742\*          |      |      |      |      |  21.1 | 13.3 | 12.4 |
| AMD Ryzen 9 5950X\*      |      |      |      |      |  24.1 | 16.1 | 15.8 |
| Apple M1 (ARMv8)\*       |      |      |      |      |  29.2 | 19.6 | 18.5 |
| Cortex-A72 (ARMv8)\*     |      |      |      |      |  30.5 | 20.5 | 20.0 |
| Intel Xeon E5-2609 v4\*  |      |      |      |      |  31.9 | 21.4 | 21.2 |
| Intel Core i5-6300U      |  747 |  114 |   69 |   46 |  34.2 | 23.2 | 23.1 |
| Intel Core i5-4200U      |  998 |  153 |   92 |   61 |  45.5 | 30.9 | 30.7 |
| Cortex-A9 (ARMv7)\*      |      |      |      |      |  95.8 | 55.5 | 53.9 |
| Cortex-A7 (ARMv7)\*      |      |      |      |      | 138.1 | 89.9 | 88.8 |
| ARM1176JZF-S (ARMv6)     | 3051 |  462 |  277 |  184 | 137.3 | 92.6 | 92.2 |


## Ascon-Mac and Ascon-Prf

| Message Length in Bytes  |    1 |    8 |   16 |   32 |   64 | 1536 | long |
|:-------------------------|-----:|-----:|-----:|-----:|-----:|-----:|-----:|
| Intel Core i5-6300U      |  369 |   46 |   24 |   18 | 11.7 |  6.4 |  6.3 |
| Intel Core i5-4200U      |  506 |   63 |   32 |   24 | 16.2 |  8.8 |  8.7 |
| ARM1176JZF-S (ARMv6)     | 1769 |  223 |  117 |   85 | 57.5 | 31.9 | 31.6 |


## Ascon-PrfShort

| Message Length in Bytes  |    1 |    8 |   16 |   32 |   64 | 1536 | long |
|:-------------------------|-----:|-----:|-----:|-----:|-----:|-----:|-----:|
| Intel Core i5-6300U      |  185 |   23 |   12 |    - |    - |    - |    - |
| Intel Core i5-4200U      |  257 |   33 |   17 |    - |    - |    - |    - |
| ARM1176JZF-S (ARMv6)     | 1057 |  132 |   69 |    - |    - |    - |    - |

\* Results taken from eBACS: http://bench.cr.yp.to/


# Build and test

Build and test all Ascon C targets using release flags (-O2 -fomit-frame-pointer -march=native -mtune=native):

```
mkdir build && cd build
cmake ..
cmake --build .
ctest
```


Build and test all Ascon C targets on Windows:

```
mkdir build && cd build
cmake ..
cmake --build . --config Release
ctest -C Release
```


Build and test all Ascon C targets using debug flags (with NIST defined flags and sanitizers):

```
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .
ctest
```

Manually set the compiler and/or release flags (e.g. to disable -march=native -mtune=native).

```
mkdir build && cd build
cmake .. -DCMAKE_C_COMPILER=clang -DREL_FLAGS="-O2;-fomit-frame-pointer"
cmake --build .
ctest
```

Build and run only specific algorithms, implementations and tests:

```
mkdir build && cd build
cmake .. -DALG_LIST="ascon128;asconhash" -DIMPL_LIST="opt64;bi32" -DTEST_LIST="genkat"
cmake --build .
ctest
```

Note that cmake stores variables in a cache. Therefore, variables can be set
one-by-one, unset using e.g. `cmake . -UIMPL_LIST` and shown using `cmake . -L`:

```
mkdir build && cd build
cmake ..
cmake . -DALG_LIST="ascon128;asconhash"
cmake . -DIMPL_LIST="opt64;bi32"
cmake . -DTEST_LIST="genkat"
cmake . -L
cmake --build .
ctest
```

Cross compile and test with custom emulator using e.g. `qemu-arm`:

```
mkdir build && cd build
cmake .. -DCMAKE_C_COMPILER="arm-linux-gnueabi-gcc" \
         -DREL_FLAGS="-O2;-fomit-frame-pointer;-march=armv7;-mtune=cortex-m4" \
         -DEMULATOR="qemu-arm;-L;/usr/arm-linux-gnueabi" \
         -DALG_LIST="ascon128;ascon128a" -DIMPL_LIST="armv7m;bi32_armv7m"
cmake --build .
ctest
```

or using Intel SDE (use full path to `sde` or add to path variable):

```
mkdir build && cd build
cmake .. -DCMAKE_C_COMPILER=gcc -DIMPL_LIST=avx512 -DEMULATOR="sde;--" \
         -DREL_FLAGS="-O2;-fomit-frame-pointer;-march=icelake-client"
cmake --build .
ctest
```


# Build and benchmark:

Build the getcycles test:

```
mkdir build && cd build
cmake .. -DALG_LIST="ascon128;asconhash" -DIMPL_LIST="opt32;opt32_lowsize" -DTEST_LIST="getcycles"
cmake --build .
```

Get the CPU cycle performance:

```
./getcycles_crypto_aead_ascon128v12_opt32
./getcycles_crypto_aead_ascon128v12_opt32_lowsize
./getcycles_crypto_hash_asconhashv12_opt32
./getcycles_crypto_hash_asconhashv12_opt32_lowsize
```

Get the implementation size:

```
size -t libcrypto_aead_ascon128v12_opt32.a
size -t libcrypto_aead_ascon128v12_opt32_lowsize.a
size -t libcrypto_hash_asconhashv12_opt32.a
size -t libcrypto_hash_asconhashv12_opt32_lowsize.a
```


# Manually build and run a single Ascon target:

Build example for AEAD algorithms:

```
gcc -march=native -O3 -Icrypto_aead/ascon128v12/opt64 crypto_aead/ascon128v12/opt64/*.c -Itests tests/genkat_aead.c -o genkat
gcc -march=native -O3 -Icrypto_aead/ascon128v12/opt64 crypto_aead/ascon128v12/opt64/*.c -DCRYPTO_AEAD -Itests tests/getcycles.c -o getcycles
```

Build example for HASH algorithms:

```
gcc -march=native -O3 -Icrypto_hash/asconhashv12/opt64 crypto_hash/asconhashv12/opt64/*.c -Itests tests/genkat_hash.c -o genkat
gcc -march=native -O3 -Icrypto_hash/asconhashv12/opt64 crypto_hash/asconhashv12/opt64/*.c -DCRYPTO_HASH -Itests tests/getcycles.c -o getcycles
```

Generate KATs and get CPU cycles:

```
./genkat
./getcycles
```


## Manually build and run an RV32 target:


Setup:

```
sudo apt install gcc-riscv64-unknown-elf picolibc-riscv64-unknown-elf qemu-system-misc
```

Example to build, run and test an AEAD/HASH algorithm using `gcc`, `picolibc` and `qemu`:

```
riscv64-unknown-elf-gcc -O2 -march=rv32i -mabi=ilp32 --specs=picolibc.specs --oslib=semihost --crt0=hosted -Ttests/rv32.ld \
    -Icrypto_aead/ascon128v12/asm_rv32i crypto_aead/ascon128v12/asm_rv32i/*.[cS] -Itests tests/genkat_aead.c -o genkat
qemu-system-riscv32 -semihosting-config enable=on -monitor none -serial none -nographic -machine virt,accel=tcg -cpu rv32 -bios none -kernel genkat
diff LWC_AEAD_KAT_128_128.txt crypto_aead/ascon128v12/LWC_AEAD_KAT_128_128.txt
```

```
riscv64-unknown-elf-gcc -O2 -march=rv32i -mabi=ilp32 --specs=picolibc.specs --oslib=semihost --crt0=hosted -Ttests/rv32.ld \
    -Icrypto_hash/asconhashv12/opt32 crypto_hash/asconhashv12/opt32/*.[cS] -Itests tests/genkat_hash.c -o genkat
qemu-system-riscv32 -semihosting-config enable=on -monitor none -serial none -nographic -machine virt,accel=tcg -cpu rv32 -bios none -kernel genkat
diff LWC_HASH_KAT_256.txt crypto_hash/asconhashv12/LWC_HASH_KAT_256.txt
```


## Manually build and run an AVR target:

Example to build, run and test an AEAD algorithm using `avr-gcc`, `avr-libc` and `simavr`.

Setup:

```
sudo apt install gcc-avr avr-libc simavr
git clone https://github.com/JohannCahier/avr_uart.git
```

Single test vector using `demo` and performance measurement using `getcycles`:

```
avr-gcc -mmcu=atmega128 -std=c99 -Os -Icrypto_aead/ascon128v12/opt8 crypto_aead/ascon128v12/opt8/*.[cS] \
    -DAVR_UART -Iavr_uart avr_uart/avr_uart.c -Wno-incompatible-pointer-types -Wno-cpp \
    -DCRYPTO_AEAD -Itests tests/demo.c -o demo
simavr -m atmega128 ./demo
```
```
avr-gcc -mmcu=atmega128 -std=c99 -Os -Icrypto_aead/ascon128v12/opt8 crypto_aead/ascon128v12/opt8/*.[cS] \
    -DAVR_UART -Iavr_uart avr_uart/avr_uart.c -Wno-incompatible-pointer-types -Wno-cpp \
    -DCRYPTO_AEAD -Itests tests/getcycles.c -o getcycles
simavr -t -m atmega128 ./getcycles
```

Generate all test vectors for AEAD/HASH and write result to a file. Press Ctrl-C to quit `simavr` after about a minute:

```
avr-gcc -mmcu=atmega128 -std=c99 -Os -Icrypto_aead/ascon128v12/opt8 crypto_aead/ascon128v12/opt8/*.[cS] \
    -DAVR_UART -Iavr_uart avr_uart/avr_uart.c -Wno-incompatible-pointer-types -Wno-cpp \
    -Itests tests/genkat_aead.c -o genkat_aead
echo "Press Ctrl-C to quit simavr after about a minute"
simavr -t -m atmega128 ./genkat_aead 2> LWC_AEAD_KAT_128_128.txt
sed -i -e 's/\x1b\[[0-9;]*m//g' -e 's/\.\.$//' LWC_AEAD_KAT_128_128.txt
diff LWC_AEAD_KAT_128_128.txt crypto_aead/ascon128v12/LWC_AEAD_KAT_128_128.txt
```

```
avr-gcc -mmcu=atmega128 -std=c99 -Os -Icrypto_hash/asconhashv12/opt8 crypto_hash/asconhashv12/opt8/*.[cS] \
    -DAVR_UART -Iavr_uart avr_uart/avr_uart.c -Wno-incompatible-pointer-types -Wno-cpp \
    -Itests tests/genkat_hash.c -o genkat_hash
echo "Press Ctrl-C to quit simavr after about a minute"
simavr -t -m atmega128 ./genkat_hash 2> LWC_HASH_KAT_256.txt
sed -i -e 's/\x1b\[[0-9;]*m//g' -e 's/\.\.$//' LWC_HASH_KAT_256.txt
diff LWC_HASH_KAT_256.txt crypto_hash/asconhashv12/LWC_HASH_KAT_256.txt
```


# Benchmarking

## Hints to get more reliable getcycles results on Intel/AMD CPUs:

* Determine the processor base frequency (also called design frequency):
  - e.g. using the Intel/AMD website
  - or using `lscpu` listed under model name

* Disable turbo boost (this should lock the frequency to the next value
  below the processor base frequency):
  ```
  echo 1 | sudo tee /sys/devices/system/cpu/intel_pstate/no_turbo
  ```

* If the above does not work, manually set the frequency using e.g. `cpufreq-set`.

* Determine the actual frequency (under load):
  - e.g. by watching the frequency using `lscpu` or `cpufreq-info`

* Determine the scaling factor between the actual and base frequency:
  - factor = actual frequency / base frequency

* Run a getcycles program using the frequency factor and watch the results:
  ```
  while true; do ./getcycles_crypto_aead_ascon128v12_opt64 $factor; done
  ```

* Run the `benchmark-getcycles.sh` script with the frequency factor and a
  specific algorithm to benchmark all corresponding getcycles implementations:
  ```
  scripts/benchmark-getcycles.sh $factor ascon128
  ```


## Hints to activate the performance monitor unit (PMU) on ARM CPUs:

* First try to install `linux-tools` and see if it works.

* On many ARM platforms, the PMU has to be enabled using a kernel module:
  - Source code for Armv6 (32-bit):
    <http://sandsoftwaresound.net/raspberry-pi/raspberry-pi-gen-1/performance-counter-kernel-module/>
  - Source code for Armv7 (32-bit):
    <https://github.com/thoughtpolice/enable_arm_pmu>
  - Source code for Armv8/Aarch64 (64-bit):
    <https://github.com/rdolbeau/enable_arm_pmu>

* Steps to compile the kernel module on the raspberry pi:
  - Find out the kernel version using `uname -a`
  - Download the kernel header files, e.g. `raspberrypi-kernel-header`
  - Download the source code for the Armv6 kernel module
  - Build, install and load the kernel module


## Benchmark Ascon v1.2 using supercop

Download supercop according to the website: http://bench.cr.yp.to/supercop.html

To test only Ascon, just run the following commands:

```
./do-part init
./do-part crypto_aead ascon128v12
./do-part crypto_aead ascon128av12
./do-part crypto_aead ascon80pqv12
./do-part crypto_hash asconhashv12
./do-part crypto_hash asconxofv12
```

Show the cycles/Byte for a 1536 Byte long message:

```
cat bench/*/data | grep '_cycles 1536 ' | awk '{printf "%.1f\t%s\t%s\n", $9/$8,
$6, $7}' | sort -nr
```


## Evaluate and optimize Ascon on constraint devices:

* The ascon-c code allows to set compile-time parameters `ASCON_INLINE_MODE`
  (IM), `ASCON_INLINE_PERM` (IP), `ASCON_UNROLL_LOOPS` (UL), `ASCON_INLINE_BI`
  (IB), via command line or in the `crypto_*/ascon*/*/config.h` files.
* Use the `benchmark-config.sh` script to evaluate all combinations of these
  parameters for a given list of Ascon implementations. The script is called
  with an output file, frequency factor, the algorithm, and the list of
  implementations to test:
  ```
  scripts/benchmark-config.sh results-config.md $factor ascon128 ref opt64 opt64_lowsize
  ```
* The `results-config.md` file then contains a markup table with size and cycles
  for each implementation and parameter set to evaluate several time-area
  trade-offs.
* The `benchmark-all.sh` and `benchmark-size.sh` scripts provides a time/size
  and size-only table of all currently compiled implementations:
  ```
  scripts/benchmark-all.sh results-all.md
  scripts/benchmark-size.sh results-size.md
  ```
