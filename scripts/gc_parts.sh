#!/bin/bash

# usage from ${HDKNN_ROOT}/build:
#
# ${MPIEXEC} ${MPIARGS} $(../scripts/parts.sh ${EXE} ${YEAR} ${VERTEX_COUNT}) ${EXE_ARGS}

if [ -z ${1} ]; then
    echo "needs a first argument indicating the executable!"
    exit 1
else
    EXE=${1}
fi

if [ -z ${2} ]; then
    echo "needs a second argument indicating the year!"
    exit 1
else
    YEAR=${2}
fi

if [ -z ${3} ]; then
    echo "needs a third argument indicating the vertex count!"
    exit 1
else
    VERTEX_COUNT=${3}
fi

PATH_PREFIX="$(sh ../scripts/gc_prefix.sh ${YEAR})/${VERTEX_COUNT}/"

DATA_PATH="${PATH_PREFIX}parts.txt"
GT_PATH="${PATH_PREFIX}gt_parts.txt"

ARGS_PREFIX="-i ${DATA_PATH} -g ${GT_PATH} -c ${VERTEX_COUNT}"

echo "${EXE} ${ARGS_PREFIX}"
