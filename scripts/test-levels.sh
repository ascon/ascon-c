#!/bin/bash

exec 3>&1 4>&2
exec 1>test-levels.log 2>&1

mkdir test-levels
cd test-levels

cmake .. -DCMAKE_C_COMPILER=arm-linux-gnueabi-gcc \
         -DEMULATOR="qemu-arm;-L;/usr/arm-linux-gnueabi" \
         -DREL_FLAGS="-O2;-fomit-frame-pointer;-march=armv7" \
         -DIMPL_LIST="protected_bi32_armv6_leveled"

for k in $(seq 4); do
  for n in $(seq $k); do
    for m in $(seq $k); do
      for c in $(seq $m); do
        for a in $(seq $m); do
          RESULT=Passed
          SHARES="-DNUM_SHARES_KEY=$k;-DNUM_SHARES_NPUB=$n;-DNUM_SHARES_M=$m;-DNUM_SHARES_C=$c;-DNUM_SHARES_AD=$a"
          cmake . -DCOMPILE_DEFS="$SHARES" || RESULT=Failed
          cmake --build . --clean-first || RESULT=Failed
          ctest || RESULT=Failed
          echo "k:$k n:$n m:$m c:$c a:$a ... $RESULT" | tee -a ../test-levels.log >&3
        done
      done
    done
  done
done

cd ..
rm -rf test-levels

exit $(grep -c Failed test-levels.log)
