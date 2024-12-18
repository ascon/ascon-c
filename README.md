# Reference, highly optimized, masked C and ASM implementations of Ascon (NIST SP 800-232)

Ascon will be standardized as **NIST SP 800-232**: https://csrc.nist.gov/pubs/sp/800/232/ipd 

Ascon is a family of lightweight cryptographic algorithms and consists of:
- Authenticated encryption schemes with associated data (AEAD)
- Hash functions (HASH) and extendible output functions (XOF)
- Message authentication codes (MAC) and pseudo-random functions (PRF)

For more information on Ascon visit: https://ascon.iaik.tugraz.at/

Most implementations in this repository use the "ECRYPT Benchmarking of Cryptographic Systems (eBACS)" interface:

- https://bench.cr.yp.to/call-aead.html for AEAD (Ascon-AEAD128)
- https://bench.cr.yp.to/call-hash.html for HASH and XOF (Ascon-Hash256, Ascon-XOF128)
- https://nacl.cr.yp.to/auth.html for MAC and PRF (Ascon-Mac, Ascon-Prf, Ascon-PrfShort)
- Except CXOF, which uses the interface defined in `tests/crypto_cxof.h` (Ascon-CXOF128)

New KAT files (known answer tests) for all NIST SP 800-232 algorithms can be found in `crypto_*/*/LWC*.txt`.

Old KAT files can still be found in the same path at [v1.3.0](https://github.com/ascon/ascon-c/tree/v1.3.0) for NIST SP 800-232 and at [v1.2.8](https://github.com/ascon/ascon-c/tree/v1.2.8) for Ascon v1.2.


## TL;DR

If you do not know where to start, use the reference implementations (self-contained, portable, very fast):

- **Ascon-AEAD128**: `crypto_aead/asconaead128/ref`
- **Ascon-Hash256**: `crypto_hash/asconhash256/ref`
- **Ascon-XOF128**:  `crypto_hash/asconxof128/ref`

An implementation can be built and run manually using the following commands (more details below):

```
gcc -march=native -O3 -Icrypto_aead/asconaead128/ref crypto_aead/asconaead128/ref/*.c -Itests tests/genkat_aead.c -o genkat && ./genkat
gcc -march=native -O3 -Icrypto_aead/asconaead128/ref crypto_aead/asconaead128/ref/*.c -DCRYPTO_AEAD -Itests tests/getcycles.c -o getcycles && ./getcycles
```

```
gcc -march=native -O3 -Icrypto_hash/asconhash256/ref crypto_hash/asconhash256/ref/*.c -Itests tests/genkat_hash.c -o genkat && ./genkat
gcc -march=native -O3 -Icrypto_hash/asconhash256/ref crypto_hash/asconhash256/ref/*.c -DCRYPTO_HASH -Itests tests/getcycles.c -o getcycles && ./getcycles
```

```
gcc -march=native -O3 -Icrypto_hash/asconxof128/ref crypto_hash/asconxof128/ref/*.c -Itests tests/genkat_hash.c -o genkat && ./genkat
gcc -march=native -O3 -Icrypto_hash/asconxof128/ref crypto_hash/asconxof128/ref/*.c -DCRYPTO_HASH -Itests tests/getcycles.c -o getcycles && ./getcycles
```

## Algorithms

This repository contains implementations of the following NIST SP 800-232 draft standards:

- `crypto_aead/asconaead128`:  **Ascon-AEAD128**     (authenticated encryption, formerly **Ascon-128a**)
- `crypto_hash/asconhash256`:  **Ascon-Hash256**     (hash function, formerly **Ascon-Hash**)
- `crypto_hash/asconxof128`:   **Ascon-XOF128**      (extendable output function, formerly **Ascon-Xof**)

and combined algorithm implementations supporting both AEAD and XOF:

- `crypto_aead_hash/asconaeadxof128`: **Ascon-AEAD128** combined with **Ascon-XOF128**

We also provide the following Ascon MAC and PRF algorithms (not standardized yet):

- `crypto_auth/asconmacv13`:   **Ascon-Mac**      (message authentication code)
- `crypto_auth/asconprfv13`:   **Ascon-Prf**      (pseudo-random function)
- `crypto_auth/asconprfsv13`:  **Ascon-PrfShort** (pseudo-random function for very short messages)


## Other Ascon algorithms

Note that for the draft NIST standard, the following changes have been made compared to the Ascon submission:

- The endianness has been switched from big endian to little endian
- The initial values have been updated to accommodate potential extensions

Other Ascon variants and versions can be found in different branches of this repository:

- git branch [v1.2](https://github.com/ascon/ascon-c/tree/v1.2): Many other Ascon algorithms as submitted to the NIST and CAESAR competitions
- git branch [v1.3](https://github.com/ascon/ascon-c/tree/v1.3): Many other Ascon algorithms using little endian notation and updated initial values


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
- `armv6`: 32-bit speed-optimized ARMv6, ARMv7-A
- `armv6m`: 32-bit speed-optimized ARMv6-M
- `armv7m`: 32-bit speed-optimized ARMv7-M, ARMv8, ARMv9
- `armv6_lowsize`: 32-bit size-optimized ARMv6, ARMv7-A
- `armv6m_lowsize`: 32-bit size-optimized ARMv6-M
- `armv7m_lowsize`: 32-bit size-optimized ARMv7-M, ARMv8, ARMv9
- `armv7m_small`: 32-bit small speed-optimized ARMv7-M, ARMv8, ARMv9
- `bi32_armv6`: 32-bit speed-optimized bit-interleaved ARMv6, ARMv7-A
- `bi32_armv6m`: 32-bit speed-optimized bit-interleaved ARMv6-M
- `bi32_armv7m`: 32-bit speed-optimized bit-interleaved ARMv7-M, ARMv8, ARMv9
- `bi32_armv7m_small`: 32-bit small bit-interleaved ARMv7-M, ARMv8, ARMv9
- `avr`: 8-bit size- and speed-optimized AVR
- `avr_lowsize`: 8-bit size-optimized AVR

the following ASM implementations (only `asm_rv32i` has been updated to NIST SP 800-232 so far):

- `asm_esp32`: 32-bit optimized ESP32 using funnel-shift instructions
- `asm_rv32i`: 32-bit optimized RV32I using the base instruction set
- `asm_rv32b`: 32-bit optimized RV32B using bitmanip base (Zbb)
- `asm_fsr_rv32b`: 32-bit optimized funnel-shift RV32B using bitmanip base and bitmanip terniary (ZbbZbt)
- `asm_bi32_rv32b`: 32-bit optimized bit-interleaved RV32B using bitmanip base and bitmanip permutations (ZbbZbp)

and the following high-level masked (shared) C with inline ASM implementations:

- `protected_bi32_armv6`: 32-bit masked bit-interleaved ARMv6, ARMv7
- `protected_bi32_armv6_leveled`: 32-bit masked and leveled bit-interleaved ARMv6, ARMv7

The masked C implementations can be used as a starting point to generate
device specific C/ASM implementations. Note that the masked C implementations
require a minimum amount of ASM instructions. Otherwise, the compiler may
heavily optimize the code and even combine shares. Obviously, the output
generated is very sensitive to compiler and environment changes and any
generated output needs to be security evaluated. A preliminary evaluation of
these implementations has been performed on some
[ChipWhisperer](https://www.newae.com/chipwhisperer) devices. The setup and
preliminary results can found at: https://github.com/ascon/simpleserial-ascon


# Code size on different CPU architectures (`-march`) in bytes

|                 | asconaead128    | asconxof128     | asconaeadxof128 | asconprfv13     | asconprfsv13    |
|:----------------|----------------:|----------------:|----------------:|----------------:|----------------:|
| x86-64-v4       | 1339            | 729             | 1708            | 1242            | 1087            |
| armv7-a         | 1492            | 792             | 1880            | 1548            | 1424            |
| armv7-m         | 1090            | 632             | 1346            | 800             | 732             |
| armv6-m         | 1162            | 664             | 1386            | 1100            | 780             |
| rv32i           | 1612            | 1000            | 2380            | 1928            | 1772            |
| esp32           | 1098            | 619             | 1436            | 1239            | 1020            |
| avr             | 3512            | 1662            | 4000            | 1984            | 1858            |

This table can be re-generated by running the following scripts (requires `gcc`, `clang` and `picolibc` variants to be installed):

```
scripts/size-all.sh asconaead128 asconxof128 asconaeadxof128 asconprfv13 asconprfsv13
```

# Estimated performance results on different CPUs in cycles per byte

## **Ascon-AEAD128** (NIST SP 800-232)

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


## **Ascon-Hash256** and **Ascon-XOF128** (NIST SP 800-232)

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


## **Ascon-Mac** and **Ascon-Prf**

| Message Length in Bytes  |    1 |    8 |   16 |   32 |   64 | 1536 | long |
|:-------------------------|-----:|-----:|-----:|-----:|-----:|-----:|-----:|
| Intel Core i5-6300U      |  369 |   46 |   24 |   18 | 11.7 |  6.4 |  6.3 |
| Intel Core i5-4200U      |  506 |   63 |   32 |   24 | 16.2 |  8.8 |  8.7 |
| ARM1176JZF-S (ARMv6)     | 1769 |  223 |  117 |   85 | 57.5 | 31.9 | 31.6 |


## **Ascon-PrfShort**

| Message Length in Bytes  |    1 |    8 |   16 |   32 |   64 | 1536 | long |
|:-------------------------|-----:|-----:|-----:|-----:|-----:|-----:|-----:|
| Intel Core i5-6300U      |  185 |   23 |   12 |    - |    - |    - |    - |
| Intel Core i5-4200U      |  257 |   33 |   17 |    - |    - |    - |    - |
| ARM1176JZF-S (ARMv6)     | 1057 |  132 |   69 |    - |    - |    - |    - |

\* Results taken from eBACS for Ascon v1.2: http://bench.cr.yp.to/

Performance results for other Ascon variants and versions can be found in git branch [v1.2](https://github.com/ascon/ascon-c/tree/v1.2) and [v1.3](https://github.com/ascon/ascon-c/tree/v1.3).


# Build and test

Build and test all Ascon C targets using release flags (`-O2 -fomit-frame-pointer -march=native -mtune=native`):

```
mkdir build && cd build
cmake ..
cmake --build .
ctest
```


Build and test all Ascon C targets on Windows:

```
mkdir build
cd build
cmake ..
cmake --build . --config Release
ctest -C Release
```

To print input/output values and intermediate Ascon states, add `-DASCON_PRINT_STATE` to the build via the cmake `-DCOMPILE_DEFS` option:

```
mkdir build && cd build
cmake .. -DCOMPILE_DEFS="-DASCON_PRINT_STATE"
cmake --build .
ctest
```

Build and test all Ascon C targets using debug flags (with NIST defined flags and sanitizers):

```
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .
ctest
```

Manually set the compiler and/or release flags (e.g. to disable `-march=native -mtune=native`).

```
mkdir build && cd build
cmake .. -DCMAKE_C_COMPILER=clang -DREL_FLAGS="-O2;-fomit-frame-pointer"
cmake --build .
ctest
```

Build and run only specific algorithms, implementations and tests:

```
mkdir build && cd build
cmake .. -DALG_LIST="asconaead128;asconhash256" -DIMPL_LIST="opt64;bi32" -DTEST_LIST="genkat"
cmake --build .
ctest
```

Note that cmake stores variables in a cache. Therefore, variables can be set
one-by-one, unset using e.g. `cmake . -UIMPL_LIST` and shown using `cmake . -L`:

```
mkdir build && cd build
cmake ..
cmake . -DALG_LIST="asconaead128;asconhash256"
cmake . -DIMPL_LIST="opt64;bi32"
cmake . -DTEST_LIST="genkat"
cmake . -L
cmake --build .
ctest
```

Compile and test using Intel SDE (use full path to `sde` or add to path variable):

```
mkdir build && cd build
cmake .. -DCMAKE_C_COMPILER=gcc -DIMPL_LIST=avx512 -DEMULATOR="sde;--" \
         -DREL_FLAGS="-O2;-fomit-frame-pointer;-march=icelake-client"
cmake --build .
ctest
```

Cross compile and test with custom emulator using e.g. `qemu-arm`:

```
mkdir build && cd build
cmake .. -DCMAKE_C_COMPILER="arm-linux-gnueabi-gcc" \
         -DREL_FLAGS="-O2;-fomit-frame-pointer;-march=armv7;-mtune=cortex-m4" \
         -DEMULATOR="qemu-arm;-L;/usr/arm-linux-gnueabi" \
         -DALG_LIST="asconaead128;asconhash256" -DIMPL_LIST="armv7m;bi32_armv7m"
cmake --build .
ctest
```

Compile and test for ARMv6-M on bare metal using `picolibc` and `qemu-system-arm`:

```
mkdir build && cd build
cmake .. -DCMAKE_C_COMPILER="arm-none-eabi-gcc" -DCMAKE_C_COMPILER_FORCED=ON \
         -DREL_FLAGS="-O2;-mcpu=cortex-m0;--specs=picolibc.specs;--oslib=semihost;-T../tests/microbit.ld" \
         -DEMULATOR="qemu-system-arm;-semihosting;-nographic;-machine;microbit;-kernel" \
         -DALG_LIST="asconaead128;asconhash256" -DIMPL_LIST="armv6m;armv6m_lowsize"
cmake --build .
ctest
```

Compile and test for RV32 on bare metal using `picolibc` and `qemu-system-riscv32`:

```
mkdir build && cd build
cmake .. -DCMAKE_C_COMPILER="riscv64-unknown-elf-gcc" -DCMAKE_C_COMPILER_FORCED=ON \
         -DREL_FLAGS="-O2;-march=rv32i;-mabi=ilp32;--specs=picolibc.specs;--oslib=semihost;-T../tests/rv32.ld" \
         -DEMULATOR="qemu-system-riscv32;-semihosting;-nographic;-machine;virt;-cpu;rv32;-bios;none;-kernel" \
         -DALG_LIST="asconaead128;asconhash256" -DIMPL_LIST="asm_rv32i"
cmake --build .
ctest
```


# Build and benchmark:

Build the getcycles test:

```
mkdir build && cd build
cmake .. -DALG_LIST="asconaead128;asconhash256" -DIMPL_LIST="opt32;opt32_lowsize" -DTEST_LIST="getcycles"
cmake --build .
```

Get the CPU cycle performance:

```
./getcycles_crypto_aead_asconaead128_opt32
./getcycles_crypto_aead_asconaead128_opt32_lowsize
./getcycles_crypto_hash_asconhash256_opt32
./getcycles_crypto_hash_asconhash256_opt32_lowsize
```

Get the implementation size:

```
size -t libcrypto_aead_asconaead128_opt32.a
size -t libcrypto_aead_asconaead128_opt32_lowsize.a
size -t libcrypto_hash_asconhash256_opt32.a
size -t libcrypto_hash_asconhash256_opt32_lowsize.a
```


# Manual builds

## Build and run native targets:

Build example for AEAD algorithms:

```
gcc -march=native -O3 -Icrypto_aead/asconaead128/opt64 crypto_aead/asconaead128/opt64/*.c -Itests tests/genkat_aead.c -o genkat
gcc -march=native -O3 -Icrypto_aead/asconaead128/opt64 crypto_aead/asconaead128/opt64/*.c -DCRYPTO_AEAD -Itests tests/getcycles.c -o getcycles
```

Build example for HASH/XOF algorithms:

```
gcc -march=native -O3 -Icrypto_hash/asconhash256/opt64 crypto_hash/asconhash256/opt64/*.c -Itests tests/genkat_hash.c -o genkat
gcc -march=native -O3 -Icrypto_hash/asconhash256/opt64 crypto_hash/asconhash256/opt64/*.c -DCRYPTO_HASH -Itests tests/getcycles.c -o getcycles
```

Build example for MAC/PRF algorithms:

```
gcc -march=native -O3 -Icrypto_auth/asconprfv13/opt64 crypto_auth/asconprfv13/opt64/*.c -Itests tests/genkat_auth.c -o genkat
gcc -march=native -O3 -Icrypto_auth/asconprfv13/opt64 crypto_auth/asconprfv13/opt64/*.c -DCRYPTO_AUTH -Itests tests/getcycles.c -o getcycles
```

Generate KATs and get CPU cycles:

```
./genkat
./getcycles
```

## Build and print intermediate values:

To print input/output values and intermediate Ascon states, add `-DASCON_PRINT_STATE` to the build:

```
gcc -DASCON_PRINT_STATE -Icrypto_aead/asconaead128/opt64 crypto_aead/asconaead128/opt64/*.c -Itests tests/genkat_aead.c -o genkat && ./genkat | less
```


## Build and run ARMv7-M targets:

Build example for AEAD algorithms:

```
arm-linux-gnueabi-gcc -march=armv7 -O2 -Icrypto_aead/asconaead128/armv7m crypto_aead/asconaead128/armv7m/*.c -Itests tests/genkat_aead.c -o genkat
arm-linux-gnueabi-gcc -march=armv7 -O2 -Icrypto_aead/asconaead128/armv7m crypto_aead/asconaead128/armv7m/*.c -DCRYPTO_AEAD -Itests tests/getcycles.c -o getcycles
```

Build example for HASH/XOF algorithms:

```
arm-linux-gnueabi-gcc -march=armv7 -O2 -Icrypto_aead/asconaead128/armv7m crypto_aead/asconaead128/armv7m/*.c -Itests tests/genkat_hash.c -o genkat
arm-linux-gnueabi-gcc -march=armv7 -O2 -Icrypto_aead/asconaead128/armv7m crypto_aead/asconaead128/armv7m/*.c -DCRYPTO_HASH -Itests tests/getcycles.c -o getcycles
```

Generate KATs and get CPU cycles:

```
qemu-arm -L /usr/arm-linux-gnueabi ./genkat
qemu-arm -L /usr/arm-linux-gnueabi ./getcycles
```


## Build and run ARMv6-M targets:


Setup:

```
sudo apt install gcc-arm-none-eabi picolibc-arm-none-eabi qemu-system-arm
```

Example to build, run and test an AEAD/HASH algorithm using `gcc`, `picolibc` and `qemu`:

```
arm-none-eabi-gcc -O2 -mcpu=cortex-m0 --specs=picolibc.specs --oslib=semihost -Ttests/microbit.ld \
    -Icrypto_aead/asconaead128/armv6m crypto_aead/asconaead128/armv6m/*.[cS] -Itests tests/genkat_aead.c -o genkat
qemu-system-arm -semihosting -nographic -machine microbit -kernel genkat
diff LWC_AEAD_KAT_128_128.txt crypto_aead/asconaead128/LWC_AEAD_KAT_128_128.txt
```

```
arm-none-eabi-gcc -O2 -mcpu=cortex-m0 --specs=picolibc.specs --oslib=semihost -Ttests/microbit.ld \
    -Icrypto_hash/asconhash256/opt32 crypto_hash/asconhash256/opt32/*.[cS] -Itests tests/genkat_hash.c -o genkat
qemu-system-arm -semihosting -nographic -machine microbit -kernel genkat
diff LWC_HASH_KAT_256.txt crypto_hash/asconhash256/LWC_HASH_KAT_256.txt
```


## Build and run RV32 targets:

Setup:

```
sudo apt install gcc-riscv64-unknown-elf picolibc-riscv64-unknown-elf qemu-system-misc
```

Example to build, run and test an AEAD/HASH algorithm using `gcc`, `picolibc` and `qemu`:

```
riscv64-unknown-elf-gcc -O2 -march=rv32i -mabi=ilp32 --specs=picolibc.specs --oslib=semihost -Ttests/rv32.ld \
    -Icrypto_aead/asconaead128/asm_rv32i crypto_aead/asconaead128/asm_rv32i/*.[cS] -Itests tests/genkat_aead.c -o genkat
qemu-system-riscv32 -semihosting -nographic -machine virt -cpu rv32 -bios none -kernel genkat
diff LWC_AEAD_KAT_128_128.txt crypto_aead/asconaead128/LWC_AEAD_KAT_128_128.txt
```

```
riscv64-unknown-elf-gcc -O2 -march=rv32i -mabi=ilp32 --specs=picolibc.specs --oslib=semihost -Ttests/rv32.ld \
    -Icrypto_hash/asconhash256/opt32 crypto_hash/asconhash256/opt32/*.[cS] -Itests tests/genkat_hash.c -o genkat
qemu-system-riscv32 -semihosting -nographic -machine virt -cpu rv32 -bios none -kernel genkat
diff LWC_HASH_KAT_256.txt crypto_hash/asconhash256/LWC_HASH_KAT_256.txt
```


## Build and run AVR targets:

Example to build, run and test an AEAD algorithm using `avr-gcc`, `avr-libc` and `simavr`.

Setup:

```
sudo apt install gcc-avr avr-libc simavr
git clone https://github.com/JohannCahier/avr_uart.git
```

Single test vector using `demo` and performance measurement using `getcycles`:

```
avr-gcc -mmcu=atmega128 -std=c99 -Os -Icrypto_aead/asconaead128/opt8 crypto_aead/asconaead128/opt8/*.[cS] \
    -DAVR_UART -Iavr_uart avr_uart/avr_uart.c -Wno-incompatible-pointer-types -Wno-cpp \
    -DCRYPTO_AEAD -Itests tests/demo.c -o demo
simavr -m atmega128 ./demo
```
```
avr-gcc -mmcu=atmega128 -std=c99 -Os -Icrypto_aead/asconaead128/opt8 crypto_aead/asconaead128/opt8/*.[cS] \
    -DAVR_UART -Iavr_uart avr_uart/avr_uart.c -Wno-incompatible-pointer-types -Wno-cpp \
    -DCRYPTO_AEAD -Itests tests/getcycles.c -o getcycles
simavr -t -m atmega128 ./getcycles
```

Generate all test vectors for AEAD/HASH and write result to a file. Press Ctrl-C to quit `simavr` after about a minute:

```
avr-gcc -mmcu=atmega128 -std=c99 -Os -Icrypto_aead/asconaead128/opt8 crypto_aead/asconaead128/opt8/*.[cS] \
    -DAVR_UART -Iavr_uart avr_uart/avr_uart.c -Wno-incompatible-pointer-types -Wno-cpp \
    -Itests tests/genkat_aead.c -o genkat_aead
echo "Press Ctrl-C to quit simavr after about a minute"
simavr -t -m atmega128 ./genkat_aead 2> LWC_AEAD_KAT_128_128.txt
sed -i -e 's/\x1b\[[0-9;]*m//g' -e 's/\.\.$//' LWC_AEAD_KAT_128_128.txt
diff LWC_AEAD_KAT_128_128.txt crypto_aead/asconaead128/LWC_AEAD_KAT_128_128.txt
```

```
avr-gcc -mmcu=atmega128 -std=c99 -Os -Icrypto_hash/asconhash256/opt8 crypto_hash/asconhash256/opt8/*.[cS] \
    -DAVR_UART -Iavr_uart avr_uart/avr_uart.c -Wno-incompatible-pointer-types -Wno-cpp \
    -Itests tests/genkat_hash.c -o genkat_hash
echo "Press Ctrl-C to quit simavr after about a minute"
simavr -t -m atmega128 ./genkat_hash 2> LWC_HASH_KAT_256.txt
sed -i -e 's/\x1b\[[0-9;]*m//g' -e 's/\.\.$//' LWC_HASH_KAT_256.txt
diff LWC_HASH_KAT_256.txt crypto_hash/asconhash256/LWC_HASH_KAT_256.txt
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
  - factor = actual_frequency / base_frequency

* Run a getcycles program using the frequency factor and watch the results:
  ```
  while true; do ./getcycles_crypto_aead_asconaead128_opt64 $factor; done
  ```

* Run the `benchmark-getcycles.sh` script with the frequency factor and a
  specific algorithm to benchmark all corresponding getcycles implementations:
  ```
  scripts/benchmark-getcycles.sh $factor asconaead
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


## Benchmark Ascon using supercop

Download supercop according to the website: http://bench.cr.yp.to/supercop.html

To test only Ascon, just run the following commands:

```
./do-part init
./do-part crypto_aead asconaead128
./do-part crypto_hash asconhash256
./do-part crypto_hash asconxof128
```

Show the cycles/Byte for a 1536 Byte long message:

```
cat bench/*/data | grep '_cycles 1536 ' | awk '{printf "%.1f\t%s\t%s\n", $9/$8, $6, $7}' | sort -nr
```


## Evaluate and optimize Ascon on constraint devices:

The script `build-test.sh` quickly test builds all implementations of an algorithm
for a given compiler with compile flags:
```
scripts/build-test.sh asconaead128 arm-none-eabi-gcc -march=armv6-m -O2
scripts/build-test.sh asconaead128 arm-linux-gnueabi-gcc -march=armv7-m -O2
scripts/build-test.sh asconaead128 clang --target=riscv32-unknown-elf -march=rv32i -mabi=ilp32 -I/usr/lib/picolibc/arm-none-eabi/include
```


The `size-build.sh` script builds and sorts the size of all implementations
of an algorithm for a given compiler with compile flags:
```
scripts/size-build.sh asconaead128 arm-none-eabi-gcc -march=armv6-m
scripts/size-build.sh asconaead128 arm-linux-gnueabi-gcc -march=armv7-m
scripts/size-build.sh asconaead128 clang --target=riscv32-unknown-elf -march=rv32i -mabi=ilp32 -I/usr/lib/picolibc/arm-none-eabi/include
```

The ascon-c code allows to set compile-time parameters `ASCON_INLINE_MODE`
(IM), `ASCON_INLINE_PERM` (IP), `ASCON_UNROLL_LOOPS` (UL), `ASCON_INLINE_BI`
(IB), via command line or in the `crypto_*/ascon*/*/config.h` files.

Use the `benchmark-config.sh` script to evaluate all combinations of these
parameters for a given list of Ascon implementations. The script is called
with a frequency factor, output file, the algorithm, and a list of
implementations to test:
```
scripts/benchmark-config.sh $factor results-config.md asconaead128 ref opt64 opt64_lowsize
```
The `results-config.md` file then contains a markup table with size and cycles
for each implementation and parameter set to evaluate several time-area
trade-offs.
