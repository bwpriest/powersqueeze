#!/bin/bash

FILE=${1}
DST_DIR=${2}
PREFIX=${3}
NUM_PARTS=${4}

wc -l ${FILE} > tmp.txt
read LINES FN < tmp.txt
rm -f tmp.txt

SUB_LINES=$((${LINES} / ${NUM_PARTS} + 1))

if [ ! -d "${DST_DIR}" ]; then
  mkdir ${DST_DIR}
fi

if [ ${HOSTNAME} = "kang"]; then
  gsplit -l ${SUB_LINES} -a ${#NUM_PARTS} -d ${FILE} ${DST_DIR}/${PREFIX}
else
  split --lines=${SUB_LINES} --suffix-length=${#NUM_PARTS} --numeric-suffixes ${FILE} ${DST_DIR}/${PREFIX}
fi
realpath ${DST_DIR}/* > ${DST_DIR}.txt
