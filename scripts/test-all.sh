#!/bin/bash

OPT=$1

exec 3>&1 4>&2
exec 1>test-all.log 2>&1

mkdir test-all
cd test-all

echo
echo "Test clang -$OPT builds:" | tee -a ../test-all.log | grep Test >&3
cmake .. -DCMAKE_C_COMPILER=clang -DALG_LIST="" -DIMPL_LIST=""
cmake .. -UALG_LIST -UEMULATOR \
         -DREL_FLAGS="-$OPT;-fomit-frame-pointer;-march=native;-mtune=native" \
         -DIMPL_LIST="bi32;bi32_lowreg;bi32_lowsize;bi8;esp32;opt32;opt32_lowsize;opt64;opt64_lowsize;opt8;ref"
cmake --build . --clean-first -- -k | tee -a ../test-all.log | grep "Built target genkat" >&3
ctest | tee -a ../test-all.log | grep Test >&3

echo
echo "Test gcc -$OPT builds:" | tee -a ../test-all.log | grep Test >&3
cmake .. -DCMAKE_C_COMPILER=gcc -DALG_LIST="" -DIMPL_LIST=""
cmake .. -UALG_LIST -UEMULATOR \
         -DREL_FLAGS="-$OPT;-fomit-frame-pointer;-march=native;-mtune=native" \
         -DIMPL_LIST="bi32;bi32_lowreg;bi32_lowsize;bi8;esp32;opt32;opt32_lowsize;opt64;opt64_lowsize;opt8;ref"
cmake --build . --clean-first -- -k | tee -a ../test-all.log | grep "Built target genkat" >&3
ctest | tee -a ../test-all.log | grep Test >&3

echo
echo "Test avx512 -$OPT builds:" | tee -a ../test-all.log | grep Test >&3
cmake .. -DCMAKE_C_COMPILER=gcc -DALG_LIST="" -DIMPL_LIST=""
cmake .. -UALG_LIST -DEMULATOR="sde;--" \
         -DREL_FLAGS="-$OPT;-fomit-frame-pointer;-march=icelake-client" \
         -DIMPL_LIST="avx512"
cmake --build . --clean-first -- -k | tee -a ../test-all.log | grep "Built target genkat" >&3
ctest | tee -a ../test-all.log | grep Test >&3

echo
echo "Test neon -$OPT builds:" | tee -a ../test-all.log | grep Test >&3
cmake .. -DCMAKE_C_COMPILER=arm-linux-gnueabi-gcc -DALG_LIST="" -DIMPL_LIST=""
cmake .. -UALG_LIST -DEMULATOR="qemu-arm;-L;/usr/arm-linux-gnueabi" \
         -DREL_FLAGS="-$OPT;-fomit-frame-pointer;-march=armv7-a" \
         -DIMPL_LIST="neon"
cmake --build . --clean-first -- -k | tee -a ../test-all.log | grep "Built target genkat" >&3
ctest | tee -a ../test-all.log | grep Test >&3

echo "Test armv6m -$OPT builds:" | tee -a ../test-all.log | grep Test >&3
cmake .. -DCMAKE_C_COMPILER=arm-linux-gnueabi-gcc -DALG_LIST="" -DIMPL_LIST=""
cmake .. -UALG_LIST -DEMULATOR="qemu-arm;-L;/usr/arm-linux-gnueabi" \
         -DREL_FLAGS="-$OPT;-fomit-frame-pointer;-march=armv6" \
         -DIMPL_LIST="armv6m;armv6m_lowsize;bi32_armv6m"
cmake --build . --clean-first -- -k | tee -a ../test-all.log | grep "Built target genkat" >&3
ctest | tee -a ../test-all.log | grep Test >&3

echo
echo "Test armv7m -$OPT builds:" | tee -a ../test-all.log | grep Test >&3
cmake .. -DCMAKE_C_COMPILER=arm-linux-gnueabi-gcc -DALG_LIST="" -DIMPL_LIST=""
cmake .. -UALG_LIST -DEMULATOR="qemu-arm;-L;/usr/arm-linux-gnueabi" \
         -DREL_FLAGS="-$OPT;-fomit-frame-pointer;-march=armv7" \
         -DIMPL_LIST="armv6;armv6_lowsize;armv7m;armv7m_lowsize;armv7m_small;bi32;bi32_armv6;bi32_armv7m;bi32_armv7m_small;bi32_lowreg;bi32_lowsize;bi8;esp32;opt32;opt32_lowsize;opt64;opt64_lowsize;opt8;protected_bi32_armv6;protected_bi32_armv6_leveled;ref"
cmake --build . --clean-first -- -k | tee -a ../test-all.log | grep "Built target genkat" >&3
ctest | tee -a ../test-all.log | grep Test >&3

cd ..
rm -rf test-all

exit $(grep -c Failed test-all.log)
