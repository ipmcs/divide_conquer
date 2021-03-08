#!/bin/bash

L1i_module="SN4L"
simulation_name="SN4L_ISCA2020"
idx=v5
cores=48
traces=~/ipc1_public/*.xz

#CHECK simulator
BINARY_NAME=hashed_perceptron-$L1i_module-next_line-spp_dev-no-lru-1core-remote

rm bin/$BINARY_NAME


#CHECK BUILD
./build_champsim.sh hashed_perceptron $L1i_module next_line spp_dev no lru 1 remote

#CHECK ID
baseline=v1

mkdir -p results_50M/$idx

#CHECK prefetcher name
cp prefetcher/$L1i_module.l1i_pref results_50M/$idx/.
cp src/ooo_cpu.cc results_50M/$idx/.
cp inc/cache.h results_50M/$idx/.
cp inc/champsim.h results_50M/$idx/.
cp prefetcher/BTB.h results_50M/$idx/.


# USE IT IF SOME UNFINISHED JOBS MAKE TROUBLE
kill -9 `pidof $BINARY_NAME`

#ls -q ./ipc1_public/*.xz | while read trace
for trace in $traces
do

    sleep 2
    array=($(pidof $BINARY_NAME))
    echo "${#array[@]}"
    while [ ${#array[@]} -ge $cores ]
    do
        sleep 5
        #echo "waiting"
        array=($(pidof $BINARY_NAME))
    done

    echo "${trace##*/}" 

    #CHECK RUN
    ./run_champsim.sh $BINARY_NAME 50 50 ${trace##*/} _${simulation_name}_$idx &

done

while [ ${#array[@]} -ge 1 ]
do
        sleep 5
        array=($(pidof $BINARY_NAME))
done

mv ./results_50M/*$idx* ./results_50M/$idx/.
rm -r experiments
mkdir experiments
cp -r ./results_50M/$baseline ./experiments/.
mkdir experiments/$idx
cp ./results_50M/$idx/*$idx* ./experiments/$idx/.

rm speedup.txt
python speedup.py
cp speedup.txt ./results_50M/$idx/.
rm speedup.txt

rm predictor.txt
python predictor_accuracy.py
cp predictor.txt ./results_50M/$idx/.
rm predictor.txt

rm acc_cov_over.txt
python acc_cov_over.py
cp acc_cov_over.txt ./results_50M/$idx/.
rm acc_cov_over.txt

wait
exit 0

