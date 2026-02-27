#!/bin/bash

if [ -z ${1} ]; then
    echo "requires a first argument indicating the executable!"
    exit 1
else
    EXE=${1}
fi

if [ -z ${2} ]; then
    echo "requires a second argument indicating the data year!"
    exit 1
else
    YEAR=${2}
fi

if [ -z ${3} ]; then
    echo "requires a third argument indicating the vertex count!"
    exit 1
else
    VERTEX_COUNT=${3}
fi

PATH_PREFIX="$(sh ../scripts/gc_prefix.sh ${YEAR})/${VERTEX_COUNT}/"

DATA_PATH="${PATH_PREFIX}nodes.tsv"
GT_PATH="${PATH_PREFIX}truePartition.tsv"

ARGS_PREFIX="-i ${DATA_PATH} -g ${GT_PATH} -c ${VERTEX_COUNT}"

echo "${EXE} ${ARGS_PREFIX}"
