#!/bin/bash

if [[ $# -lt 1 ]]; then
  OPT=-O2
  TYPE=Release
elif [[ $# -lt 2 ]]; then
  OPT=$1
  TYPE=Release
else
  OPT=$1
  TYPE=$2
fi

exec 3>&1 4>&2
exec 1>test-all.log 2>&1

mkdir test-all
cd test-all

echo
echo "Test clang $OPT builds:" | tee -a ../test-all.log | grep Test >&3
cmake .. -DCMAKE_C_COMPILER=clang -DALG_LIST="" -DIMPL_LIST=""
cmake .. -UALG_LIST -UEMULATOR \
         -DCMAKE_BUILD_TYPE=$TYPE \
         -DREL_FLAGS="$OPT;-fomit-frame-pointer;-march=native;-mtune=native" \
         -DIMPL_LIST="bi32;bi32_lowreg;bi32_lowsize;bi8;esp32;opt32;opt32_lowsize;opt64;opt64_lowsize;opt8;opt8_lowsize;ref"
cmake --build . --clean-first -- -k | tee -a ../test-all.log | grep "Built target genkat" >&3
ctest | sed 's/[0-9.]* sec//g' | tee -a ../test-all.log | grep Test >&3
echo
../scripts/benchmark-size.sh

echo
echo "Test gcc $OPT builds:" | tee -a ../test-all.log | grep Test >&3
cmake .. -DCMAKE_C_COMPILER=gcc -DALG_LIST="" -DIMPL_LIST=""
cmake .. -UALG_LIST -UEMULATOR \
         -DCMAKE_BUILD_TYPE=$TYPE \
         -DREL_FLAGS="$OPT;-fomit-frame-pointer;-march=native;-mtune=native" \
         -DIMPL_LIST="bi32;bi32_lowreg;bi32_lowsize;bi8;esp32;opt32;opt32_lowsize;opt64;opt64_lowsize;opt8;opt8_lowsize;ref"
cmake --build . --clean-first -- -k | tee -a ../test-all.log | grep "Built target genkat" >&3
ctest | sed 's/[0-9.]* sec//g' | tee -a ../test-all.log | grep Test >&3
echo
../scripts/benchmark-size.sh

echo
echo "Test avx512 $OPT builds:" | tee -a ../test-all.log | grep Test >&3
cmake .. -DCMAKE_C_COMPILER=gcc -DALG_LIST="" -DIMPL_LIST=""
cmake .. -UALG_LIST -DEMULATOR="sde;--" \
         -DCMAKE_BUILD_TYPE=$TYPE \
         -DDBG_FLAGS="$OPT;-std=c99;-Wall;-Wextra;-Wshadow;-fsanitize=address,undefined;-march=icelake-client" \
         -DREL_FLAGS="$OPT;-fomit-frame-pointer;-march=icelake-client" \
         -DIMPL_LIST="avx512"
cmake --build . --clean-first -- -k | tee -a ../test-all.log | grep "Built target genkat" >&3
ctest | sed 's/[0-9.]* sec//g' | tee -a ../test-all.log | grep Test >&3
echo
../scripts/benchmark-size.sh

echo
echo "Test neon $OPT builds:" | tee -a ../test-all.log | grep Test >&3
cmake .. -DCMAKE_C_COMPILER=arm-linux-gnueabi-gcc -DALG_LIST="" -DIMPL_LIST=""
cmake .. -UALG_LIST -DEMULATOR="qemu-arm;-L;/usr/arm-linux-gnueabi" \
         -DCMAKE_BUILD_TYPE=$TYPE \
         -DDBG_FLAGS="$OPT;-std=c99;-Wall;-Wextra;-Wshadow;-fsanitize=address,undefined;-march=armv7-a" \
         -DREL_FLAGS="$OPT;-fomit-frame-pointer;-march=armv7-a" \
         -DIMPL_LIST="neon"
cmake --build . --clean-first -- -k | tee -a ../test-all.log | grep "Built target genkat" >&3
ctest | sed 's/[0-9.]* sec//g' | tee -a ../test-all.log | grep Test >&3
echo
../scripts/benchmark-size.sh

echo
echo "Test armv7m $OPT builds:" | tee -a ../test-all.log | grep Test >&3
cmake .. -DCMAKE_C_COMPILER=arm-linux-gnueabi-gcc -DALG_LIST="" -DIMPL_LIST=""
cmake .. -UALG_LIST -DEMULATOR="qemu-arm;-L;/usr/arm-linux-gnueabi" \
         -DCMAKE_BUILD_TYPE=$TYPE \
         -DREL_FLAGS="$OPT;-fomit-frame-pointer;-march=armv7" \
         -DDBG_FLAGS="$OPT;-std=c99;-Wall;-Wextra;-Wshadow;-fsanitize=address,undefined;-march=armv7" \
         -DIMPL_LIST="armv6;armv6_lowsize;armv7m;armv7m_lowsize;armv7m_small;bi32;bi32_armv6;bi32_armv7m;bi32_armv7m_small;bi32_lowreg;bi32_lowsize;bi8;esp32;opt32;opt32_lowsize;opt64;opt64_lowsize;opt8;opt8_lowsize;protected_bi32_armv6;protected_bi32_armv6_leveled;ref"
cmake --build . --clean-first -- -k | tee -a ../test-all.log | grep "Built target genkat" >&3
ctest | sed 's/[0-9.]* sec//g' | tee -a ../test-all.log | grep Test >&3
echo
../scripts/benchmark-size.sh

echo
echo "Test mipsel $OPT builds:" | tee -a ../test-all.log | grep Test >&3
cmake .. -DCMAKE_C_COMPILER=mipsel-linux-gnu-gcc -DALG_LIST="" -DIMPL_LIST=""
cmake .. -UALG_LIST -DEMULATOR="qemu-mipsel;-L;/usr/mipsel-linux-gnu" \
         -DCMAKE_BUILD_TYPE=$TYPE \
         -DREL_FLAGS="$OPT;-fomit-frame-pointer" \
         -DDBG_FLAGS="$OPT;-std=c99;-Wall;-Wextra;-Wshadow" \
         -DIMPL_LIST="bi32;bi32_lowreg;bi32_lowsize;bi8;esp32;opt32;opt32_lowsize;opt64;opt64_lowsize;opt8;opt8_lowsize;ref"
cmake --build . --clean-first -- -k | tee -a ../test-all.log | grep "Built target genkat" >&3
ctest | sed 's/[0-9.]* sec//g' | tee -a ../test-all.log | grep Test >&3
echo
../scripts/benchmark-size.sh

cd ..
rm -rf test-all

exit $(grep -c Failed test-all.log)
