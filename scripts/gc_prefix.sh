#!/bin/bash

if [ -z ${1} ]; then
    YEAR="2017"
else
    YEAR=${1}
fi

if [ ${HOSTNAME} = "kang" ]; then
    echo "/Users/priest2/workspace/nisenemarks/data/${YEAR}"
else
    echo "/p/lustre1/salta/benchmarks/graphchallenge/${YEAR}"
fi
