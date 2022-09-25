#!/bin/bash

IMPL="armv6 armv6_lowsize armv7m armv7m_lowsize armv7m_small bi32 bi32_armv6 bi32_armv7m bi32_armv7m_small bi32_lowreg bi32_lowsize bi8 opt32 opt32_lowsize opt64 opt64_lowsize opt8"

exec 3>&1 4>&2
exec 1>test-config.log 2>&1

mkdir test-config
cd test-config

cmake .. -DCMAKE_C_COMPILER=arm-linux-gnueabi-gcc \
         -DEMULATOR="qemu-arm;-L;/usr/arm-linux-gnueabi" \
         -DIMPL_LIST=""

for I in $IMPL; do
  echo $I | grep -vq bi
  BI=$?
  echo BI=$BI
  FLAGS="-O2;-fomit-frame-pointer;-march=armv7"
  for IM in 0 1; do
    for IP in 0 1; do
      for UL in 0 1; do
        for IB in $(seq 0 $BI); do
          cmake . -DIMPL_LIST="$I" -DREL_FLAGS="$FLAGS" \
            -DCOMPILE_DEFS="-DASCON_INLINE_MODE=${IM};-DASCON_INLINE_PERM=${IP};-DASCON_UNROLL_LOOPS=${UL};-DASCON_INLINE_BI=${IB}"
          RESULT=Passed
          cmake --build . --clean-first -- -k \
            | tee -a ../test-config.log | grep "Built target genkat" >&3
          echo "Test IMPL=$I IM=${IM} IP=${IP} UL=${UL} IB=${IB}:" \
            | tee -a ../test-config.log >&3
          ctest \
            | tee -a ../test-config.log | grep Test >&3
          echo \
            | tee -a ../test-config.log >&3
        done
      done
    done
  done
done

cd ..
rm -rf test-config

exit $(grep -c Failed test-config.log)
