#!/bin/bash
arm-linux-gnueabi-gcc-10 -mcpu=cortex-m0 -O2 -fomit-frame-pointer -Itests -Icrypto_aead/ascon128v12/bi32_armv6m crypto_aead/ascon128v12/bi32_armv6m/*.c -c -DASCON_UNROLL_LOOPS=0
