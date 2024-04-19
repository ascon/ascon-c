#!/bin/bash

if [ -z $1 ]; then
  echo "Usage: $0 <factor> <resultsfile> <algorithm> <implementations>..."
  exit 1
fi

FACTOR=$1
shift
RESULTSFILE=$1
shift
ALG=$1
shift
IMPL="$*"

echo "factor: $FACTOR"
echo "resultsfile: $RESULTSFILE"
echo "algorithm: $ALG"
echo "implementations: $IMPL"
echo

rm $RESULTSFILE 2>/dev/null
cmake . -DALG_LIST=$ALG -DTEST_LIST=getcycles 2>/dev/null >/dev/null

HEADER="  size \t|     1 |     8 |    16 |    32 |    64 |  1536 |  long |        config       | implementation\n"
  TABLE="-------:|------:|------:|------:|------:|------:|------:|------:|:-------------------:|:---------------\n"
echo -n -e "${HEADER}${TABLE}"

for I in $IMPL; do
  for M in 0 1; do
    for P in 0 1; do
      for U in 0 1; do
        for B in 0 1; do
          rm *.a getcycles_* 2>/dev/null
          cmake . \
            -DCOMPILE_DEFS="-DASCON_INLINE_MODE=${M};-DASCON_INLINE_PERM=${P};-DASCON_UNROLL_LOOPS=${U};-DASCON_INLINE_BI=${B}" \
            -DIMPL_LIST=${I} 2>/dev/null >/dev/null
          cmake --build . 2>/dev/null >/dev/null
          CYCLES=$(./getcycles_* $FACTOR 2>/dev/null | tail -n 1)
          SIZE=$(size -t *.a 2>/dev/null | grep TOTALS | awk '{print $1}')
          echo -e " $SIZE \t$CYCLES IM=${M} IP=${P} UL=${U} IB=${B} | $I" | grep -v "^0" | tee -a $RESULTSFILE
        done
      done
    done
  done
done

sort -n -k8 -t\| -o $RESULTSFILE $RESULTSFILE
echo -e "${HEADER}${TABLE}$(cat $RESULTSFILE)" > $RESULTSFILE
echo
cat $RESULTSFILE
