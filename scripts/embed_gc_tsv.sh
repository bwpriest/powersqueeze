#!/bin/bash

export BASE_PATH=/p/lustre1/salta/benchmarks/graphchallenge/embeddings/

export TASK_COUNT=112
export EXE=./examples/pi_tsv_interleaved_dump
export ACCOUNT=seq
export PARTITION=pdebug
export TIME=5:00

#export NODE_COUNTS=(1 2 4 8 16)
export NODE_COUNTS=(1)

export RANGE_SIZES=(8 16 32)
#export RANGE_SIZES=(8)

export REPLICATION_COUNTS=(1 2 4 8)
#export REPLICATION_COUNTS=(1)

export EXPONENTS=(4)

#export YEARS=(2022a 2022d)
export YEARS=(2022a)

#export COUNTS=(1000000 5000000 20000000)
export VERTEX_COUNTS=(20000000)

for N in ${NODE_COUNTS[@]}; do
    for r in ${RANGE_SIZES[@]}; do
	for R in ${REPLICATION_COUNTS[@]}; do
	    for e in ${EXPONENTS[@]}; do
		export DIRNAME=N${N}_r${r}_R${R}
		export SEED=$((1000 * ${N} + 10 * ${r} + ${R}))
		for Y in ${YEARS[@]}; do
		    for V in ${VERTEX_COUNTS[@]}; do
			mkdir -p ${BASE_PATH}${Y}/${V}
			export TARGET=${BASE_PATH}${Y}/${V}/${DIRNAME}
			mkdir -p ${TARGET}
			srun -N ${N} --tasks-per-node ${TASK_COUNT} -A ${ACCOUNT} -p ${PARTITION} -t ${TIME} $(../scripts/gc_parts.sh ${EXE} ${Y} ${V}) -r ${r} -R ${R} -e ${e} -z ${SEED} -u -V -f ${TARGET} &> ${TARGET}/log.txt &
		    done
		done
	    done
	done
    done
done

