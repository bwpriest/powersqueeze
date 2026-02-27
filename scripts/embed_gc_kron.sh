#!/bin/bash

export BASE_PATH=/p/lustre1/salta/benchmarks/graphchallenge/embeddings/

export TASK_COUNT=112
export ACCOUNT=clmshls
export PARTITION=pbatch
export EXE=./examples/pi_kron_interleaved_dump
export TIME=1:00:00

export NODE_COUNTS=(32 64 128 256)

export RANGE_SIZES=(8)

export REPLICATION_COUNTS=(1)

export EXPONENTS=(4)

#export VERTEX_COUNTS=(1000 5000 20000 50000)
export VERTEX_COUNTS=(50000)
export LEN=${#VERTEX_COUNTS[@]}
echo "Counts: ${LEN}"

#export YEARS=(2022a 2022d)
export YEARS=(2022a)

for N in ${NODE_COUNTS[@]}; do
    for r in ${RANGE_SIZES[@]}; do
	for R in ${REPLICATION_COUNTS[@]}; do
	    for E in ${EXPONENTS[@]}; do
		export DIRNAME=N${N}_r${r}_R${R}
		export SEED=$((1000 * ${N} + 10 * ${r} + ${R}))
		for Y in ${YEARS[@]}; do
		    for ((i=0; i<${LEN};++i)); do
			export V=${VERTEX_COUNTS[i]}
			for ((j=$i; j < ${LEN}; ++j)); do
			    export U=${VERTEX_COUNTS[j]}
			    echo "starting job (${i}, ${j}), (${V}, ${U}) for ${Y}"
			    mkdir -p ${BASE_PATH}${Y}/${V}_${U}
			    export TARGET=${BASE_PATH}${Y}/${V}_${U}/${DIRNAME}
			    mkdir -p ${TARGET}
			    srun -N ${N} --tasks-per-node ${TASK_COUNT} -A ${ACCOUNT} -p ${PARTITION} -t ${TIME} $(../scripts/gc_kron.sh ${EXE} ${Y} ${V} ${Y} ${U}) -r ${r} -R ${R} -e ${E} -z ${SEED} -u -V -f ${TARGET} &> ${TARGET}/log.txt &
			done
		    done
		done
	    done
	done
    done
done
