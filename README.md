# Reference, optimized, masked C and ASM implementations of Ascon

Ascon is a family of lightweight cryptographic algorithms and consists of:
- Authenticated encryption schemes with associated data (AEAD)
- Hash functions (HASH) and extendible output functions (XOF)
- Pseudo-random functions (PRF) and message authentication codes (MAC)

All implementations use the "ECRYPT Benchmarking of Cryptographic Systems (eBACS)" interface:

- https://bench.cr.yp.to/call-aead.html for AEAD (Ascon-128, Ascon-128a, Ascon-80pq)
- https://bench.cr.yp.to/call-hash.html for HASH and XOF (Ascon-Hash, Ascon-Hasha, Ascon-Xof, Ascon-Xofa)
- https://nacl.cr.yp.to/auth.html for PRF and MAC (Ascon-Mac, Ascon-Prf, Ascon-PrfShort)

For more information on Ascon visit: https://ascon.iaik.tugraz.at/


## Algorithms

This repository contains implementations of the following 10 Ascon v1.2 algorithms:

- `crypto_aead/ascon128v12`: Ascon-128
- `crypto_aead/ascon128av12`: Ascon-128a
- `crypto_aead/ascon80pqv12`: Ascon-80pq
- `crypto_hash/asconhashv12`: Ascon-Hash
- `crypto_hash/asconhashav12`: Ascon-Hasha
- `crypto_hash/asconxofv12`: Ascon-Xof
- `crypto_hash/asconxofav12`: Ascon-Xofa
- `crypto_auth/asconmacv12`: Ascon-Mac
- `crypto_auth/asconprfv12`: Ascon-Prf
- `crypto_auth/asconprfsv12`: Ascon-PrfShort

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
- `opt8`: 8-bit optimized
- `bi8`: 8-bit optimized bit-interleaved

the following C with inline ASM implementations:

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

the following ASM implementations:

- `asm_esp32`: 32-bit optimized funnel-shift ESP32
- `asm_rv32i`: 32-bit optimized RV32I (base instructions)
- `asm_rv32b`: 32-bit optimized RV32B (bitmanip Zbb)
- `asm_fsr_rv32b`: 32-bit optimized funnel-shift RV32B (bitmanip ZbbZbt)
- `asm_bi32_rv32b`: 32-bit optimized bit-interleaved RV32B (bitmanip ZbbZbp)

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

## Ascon-128 and Ascon-80pq

| Message Length in Bytes  |    1 |    8 |   16 |   32 |   64 | 1536 | long |
|:-------------------------|-----:|-----:|-----:|-----:|-----:|-----:|-----:|
| AMD Ryzen 7 1700\*       |      |      |      |      | 14.5 |  8.8 |  8.6 |
| Intel Xeon E5-2609 v4\*  |      |      |      |      | 17.3 | 10.8 | 10.5 |
| Cortex-A53 (ARMv8)\*     |      |      |      |      | 18.3 | 11.3 | 11.0 |
| Intel Core i5-6300U      |  367 |   58 |   35 |   23 | 17.6 | 11.9 | 11.4 |
| Intel Core i5-4200U      |  521 |   81 |   49 |   32 | 23.9 | 16.2 | 15.8 |
| Cortex-A15 (ARMv7)\*     |      |      |      |      | 69.8 | 36.2 | 34.6 |
| Cortex-A7 (NEON)         | 2182 |  249 |  148 |   97 | 71.7 | 47.5 | 46.5 |
| Cortex-A7 (ARMv7)        | 1871 |  292 |  175 |  115 | 86.6 | 58.3 | 57.2 |
| ARM1176JZF-S (ARMv6)     | 1921 |  277 |  167 |  112 | 83.7 | 57.2 | 56.8 |


## Ascon-128a

| Message Length in Bytes  |    1 |    8 |   16 |   32 |   64 | 1536 | long |
|:-------------------------|-----:|-----:|-----:|-----:|-----:|-----:|-----:|
| AMD Ryzen 7 1700\*       |      |      |      |      | 12.0 |  6.0 |  5.7 |
| Intel Xeon E5-2609 v4\*  |      |      |      |      | 14.1 |  7.3 |  6.9 |
| Cortex-A53 (ARMv8)\*     |      |      |      |      | 15.1 |  7.6 |  7.3 |
| Intel Core i5-6300U      |  365 |   47 |   31 |   19 | 13.5 |  8.0 |  7.8 |
| Intel Core i5-4200U      |  519 |   67 |   44 |   27 | 18.8 | 11.0 | 10.6 |
| Cortex-A15 (ARMv7)\*     |      |      |      |      | 60.3 | 25.3 | 23.8 |
| Cortex-A7 (NEON)         | 2204 |  226 |  132 |   82 | 55.9 | 31.7 | 30.7 |
| Cortex-A7 (ARMv7)        | 1911 |  255 |  161 |  102 | 71.3 | 42.3 | 41.2 |
| ARM1176JZF-S (ARMv6)     | 1908 |  235 |  156 |   99 | 70.4 | 43.0 | 42.9 |


## Ascon-Hash and Ascon-Xof

| Message Length in Bytes  |    1 |    8 |   16 |   32 |    64 | 1536 | long |
|:-------------------------|-----:|-----:|-----:|-----:|------:|-----:|-----:|
| Intel Core i5-6300U      |  747 |  114 |   69 |   46 |  34.2 | 23.2 | 23.1 |
| Intel Core i5-4200U      |  998 |  153 |   92 |   61 |  45.5 | 30.9 | 30.7 |
| ARM1176JZF-S (ARMv6)     | 3051 |  462 |  277 |  184 | 137.3 | 92.6 | 92.2 |


## Ascon-Hasha and Ascon-Xofa

| Message Length in Bytes  |    1 |    8 |   16 |   32 |    64 | 1536 | long |
|:-------------------------|-----:|-----:|-----:|-----:|------:|-----:|-----:|
| Intel Core i5-6300U      |  550 |   83 |   49 |   33 |  23.7 | 15.6 | 15.5 |
| Intel Core i5-4200U      |  749 |  112 |   67 |   44 |  31.8 | 20.8 | 20.7 |
| ARM1176JZF-S (ARMv6)     | 2390 |  356 |  211 |  138 | 100.7 | 65.7 | 65.3 |


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

Build and test all Ascon C targets using performance flags:

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


Build and test all Ascon C targets using NIST flags and sanitizers:

```
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .
ctest
```

Manually set the compiler and compiler flags.

```
mkdir build && cd build
cmake .. -DCMAKE_C_COMPILER=clang -DREL_FLAGS="-O2;-fomit-frame-pointer;-march=native;-mtune=native"
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

Example to build, run and test an AEAD algorithm using `gcc`, `picolibc` and `qemu`:

```
sudo apt install gcc-riscv64-unknown-elf picolibc-riscv64-unknown-elf qemu-system-misc
riscv64-unknown-elf-gcc -O2 -march=rv32i -mabi=ilp32 --specs=picolibc.specs --oslib=semihost --crt0=hosted -Ttests/rv32.ld \
    -Icrypto_aead/ascon128v12/asm_rv32i crypto_aead/ascon128v12/asm_rv32i/*.[cS] -Itests tests/genkat_aead.c -o genkat
qemu-system-riscv32 -semihosting-config enable=on -monitor none -serial none -nographic -machine virt,accel=tcg -cpu rv32 -bios none -kernel genkat
diff LWC_AEAD_KAT_128_128.txt crypto_aead/ascon128v12/LWC_AEAD_KAT_128_128.txt
```


## Manually build and run an AVR target:

Example to build, run and test an AEAD algorithm using `avr-gcc`, `avr-libc` and `simavr`:

```
sudo apt install gcc-avr avr-libc simavr
git clone https://github.com/JohannCahier/avr_uart.git
avr-gcc -mmcu=atmega128 -std=c99 -Os -Icrypto_aead/ascon128v12/opt8 crypto_aead/ascon128v12/opt8/*.[cS] -Itests -DCRYPTO_AEAD tests/demo.c -o demo \
    -Iavr_uart -DAVR_UART avr_uart/avr_uart.c
simavr -m atmega128 ./demo
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
  ./benchmark-getcycles.sh $factor ascon128
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


## Evaluate and optimize Ascon on constraint devices:

* The ascon-c code allows to set compile-time parameters `ASCON_INLINE_MODE`
  (IM), `ASCON_INLINE_PERM` (IP), `ASCON_UNROLL_LOOPS` (UL), `ASCON_INLINE_BI`
  (IB), via command line or in the `crypto_*/ascon*/*/config.h` files.
* Use the `benchmark-config.sh` script to evaluate all combinations of these
  parameters for a given list of Ascon implementations. The script is called
  with an output file, frequency factor, the algorithm, and the list of
  implementations to test:
  ```
  ./benchmark-config.sh results-config.md $factor ascon128 ref opt64 opt64_lowsize
  ```
* The `results-config.md` file then contains a markup table with size and cycles
  for each implementation and parameter set to evaluate several time-area
  trade-offs.
* The `benchmark-all.sh` and `benchmark-size.sh` scripts provides a time/size
  and size-only table of all currently compiled implementations:
  ```
  ./benchmark-all.sh results-all.md
  ./benchmark-size.sh results-size.md
  ```
