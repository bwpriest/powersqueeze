#!/bin/bash

if [[ -z ${1} ]]; then
    echo "requires a first argument indicating the executable!"
    exit 1
else
    EXE=${1}
fi

if [[ -z ${2} ]]; then
    echo "requires a second argument indicating the year of the first graph!"
    exit 1
else
    DATA_YEAR=${2}
    DATA_COUNT=${2}
fi

if [[ -z ${3} ]]; then
    echo "requires a third argument indicating the number of vertices in the first graph!"
    exit 1
else
    DATA_COUNT=${3}
fi

if [[ -z ${4} ]]; then
    echo "requires a fourth argument indicating the year of the second graph!"
    exit 1
else
    KRON_YEAR=${4}
fi

if [[ -z ${5} ]]; then
    echo "requires a fifth argument indicating the number of vertices in the second graph!"
    exit 1
else
    KRON_COUNT=${5}
fi

VERTEX_COUNT=$((${DATA_COUNT} * ${KRON_COUNT}))

DATA_PATH_PREFIX="$(sh ../scripts/gc_prefix.sh ${DATA_YEAR})/${DATA_COUNT}/"
KRON_PATH_PREFIX="$(sh ../scripts/gc_prefix.sh ${KRON_YEAR})/${KRON_COUNT}/"

DATA_PATH="${DATA_PATH_PREFIX}nodes.tsv"
GT_PATH="${DATA_PATH_PREFIX}truePartition.tsv"

KRON_PATH="${KRON_PATH_PREFIX}nodes.tsv"
KT_PATH="${KRON_PATH_PREFIX}truePartition.tsv"


ARGS_PREFIX="-i ${DATA_PATH} -I ${KRON_PATH} -g ${GT_PATH} -G ${KT_PATH} -c ${VERTEX_COUNT}"
#ARGS_PREFIX="-i ${DATA_PATH} -g ${GT_PATH} -c ${VERTEX_COUNT}"

echo "${EXE} ${ARGS_PREFIX}"
