<p align="center">
  <h1 align="center"> Divide and Conquer Frontend Bottleneck </h1>
  <p> This repository is the implementation of Divide and Conquer (D&C) instruction prefetcher also known as SN4L+Dis+BTB on ChampSim.  
</p>

# Citation
If you use this work, please cite our paper published in ISCA 2020, in which we introduced and evaluated the divide-and-conquer approach. We also urge you to include the following in the acknowledgment section: "We used the publicly-released source code of the divide-and-conquer approach [cite the GitHub page]."

Ali Ansari, Pejman Lotfi-Kamran, and Hamid Sarbazi-Azad, "Divide and Conquer Frontend Bottleneck," in the proceedings of ISCA 2020, pp. 65-78, June 2020


# License
The license is a free non-exclusive, non-transferable license to reproduce, use, modify and display the source code version of the Software, with or without modifications for commercial and non-commercial purposes. The license does not entitle Licensee to technical support, telephone assistance, enhancements, or updates to the Software. 

Maintained by Ali Ansari

Please forward all your inquiries to:  ali.ansari@epfl.ch

# Clone D&C repository
```
git clone https://github.com/ipmcs/divide_conquer.git
```

# Compile

ChampSim takes eight parameters: Branch predictor, L1I prefetcher, L1D prefetcher, L2C prefetcher, LLC prefetcher, LLC replacement policy, the number of cores, and a naming flag. 
For example, `./build_champsim.sh hashed_perceptron SN4L next_line spp_dev no lru 1 16K_SeqTable` builds a single-core processor with hashed_perceptron branch predictor, SN4L L1 prefetcher, next_line L1D prefetcher, spp_dev L2 prefetchers, no prefetcher for LLC, and the baseline LRU replacement policy for the LLC. The naming flag indicates that SN4L has 16 K entries for its SeqTable in this build.
```
$ ./build_champsim.sh hashed_perceptron SN4L next_line spp_dev no lru 1 16K_SeqTable

$ ./build_champsim.sh ${BRANCH} ${L1I_PREFETCHER} ${L1D_PREFETCHER} ${L2C_PREFETCHER} ${LLC_PREFETCHER} ${LLC_REPLACEMENT} ${NUM_CORE} ${FLAG}
```

# Download IPC-1 trace

The first instruction prefetching championship provided 50 traces that can be downloaded from:
```
https://drive.google.com/file/d/1qs8t8-YWc7lLoYbjbH_d3lf1xdoYBznf/view?usp=sharing
```


# Download DPC-3 trace

Professor Daniel Jimenez at Texas A&M University kindly provided traces for DPC-3. Use the following script to download these traces (~20GB size and max simpoint only).
```
$ cd scripts

$ ./download_dpc3_traces.sh
```

# Run simulation

Execute `run_champsim.sh` with proper input arguments. The default `TRACE_DIR` in `run_champsim.sh` is set to `~/ipc1_public`. <br>

* Single-core simulation: Run simulation with `run_champsim.sh` script.

```
Usage: ./run_champsim.sh [BINARY] [N_WARM] [N_SIM] [TRACE] [OPTION]
$ ./run_champsim.sh hashed_perceptron-SN4L-next_line-spp_dev-no-lru-1core-16K_SeqTable 50 50 server_036.champsimtrace.xz

${BINARY}: ChampSim binary compiled by "build_champsim.sh" (hashed_perceptron-SN4L-next_line-spp_dev-no-lru-1core-16K_SeqTable)
${N_WARM}: number of instructions for warmup (50 million)
${N_SIM}:  number of instructinos for detailed simulation (50 million)
${TRACE}: trace name (server_036.champsimtrace.xz)
${OPTION}: extra option for "-low_bandwidth" (src/main.cc)
```
Simulation results will be stored under "results_${N_SIM}M" as a form of "${TRACE}-${BINARY}-${OPTION}.txt".<br> 

* Multi-core simulation: D&C prefetcher is not yet tested and verified for multi-core simulations. However, baseline ChampSim provides the following by default: Run simulation with `run_4core.sh` script. <br>
```
Usage: ./run_4core.sh [BINARY] [N_WARM] [N_SIM] [N_MIX] [TRACE0] [TRACE1] [TRACE2] [TRACE3] [OPTION]
$ ./run_4core.sh bimodal-no-no-no-lru-4core 1 10 0 400.perlbench-41B.champsimtrace.xz \\
  401.bzip2-38B.champsimtrace.xz 403.gcc-17B.champsimtrace.xz 410.bwaves-945B.champsimtrace.xz
```
Note that we need to specify multiple trace files for `run_4core.sh`. `N_MIX` is used to represent a unique ID for mixed multi-programmed workloads. 

# Run multiple simulations
Execute `run_all_1core.sh` to utilize all available cores on your machine executing the traces stored in the folder containing your traces. You should set the following inputs:
```
L1i_module="SN4L": the name of the prefetcher file in the prefetcher directory, SN4L.l1i_pref
simulation_name="SN4L_ISCA2020": your desired name for your simulation
idx=v4: the name for a folder in the "results_${N_SIM}M" directory that will keep simulation output of all simulations
cores=48: the number of utilized cores
traces=~/ipc1_public/*.xz: the location where the traces will be found
```

# Extracting Results
Execute `parse_results.py` giving the name of folder in the "results_${N_SIM}M", for example, `v4`, and it will create an excel file with the same name, `v4.xlsx` in the `excels` folder.
Note that this script looks for special patterns marked with `SHEET` in the simulation output files. 

# Add your own branch predictor, data prefetchers, and replacement policy
**Copy an empty template**
```
$ cp branch/branch_predictor.cc branch/mybranch.bpred
$ cp prefetcher/l1d_prefetcher.cc prefetcher/mypref.l1d_pref
$ cp prefetcher/l2c_prefetcher.cc prefetcher/mypref.l2c_pref
$ cp prefetcher/llc_prefetcher.cc prefetcher/mypref.llc_pref
$ cp replacement/llc_replacement.cc replacement/myrepl.llc_repl
```

**Work on your algorithms with your favorite text editor**
```
$ vim branch/mybranch.bpred
$ vim prefetcher/mypref.l1d_pref
$ vim prefetcher/mypref.l2c_pref
$ vim prefetcher/mypref.llc_pref
$ vim replacement/myrepl.llc_repl
```

**Compile and test**
```
$ ./build_champsim.sh mybranch mypref mypref mypref myrepl 1
$ ./run_champsim.sh mybranch-mypref-mypref-mypref-myrepl-1core 1 10 bzip2_183B
```

# How to create traces

We have included only 4 sample traces, taken from SPEC CPU 2006. These 
traces are short (10 million instructions), and do not necessarily cover the range of behaviors your 
replacement algorithm will likely see in the full competition trace list (not
included).  We STRONGLY recommend creating your own traces, covering
a wide variety of program types and behaviors.

The included Pin Tool champsim_tracer.cpp can be used to generate new traces.
We used Pin 3.2 (pin-3.2-81205-gcc-linux), and it may require 
installing libdwarf.so, libelf.so, or other libraries, if you do not already 
have them. Please refer to the Pin documentation (https://software.intel.com/sites/landingpage/pintool/docs/81205/Pin/html/)
for working with Pin 3.2.

Get this version of Pin:
```
wget http://software.intel.com/sites/landingpage/pintool/downloads/pin-3.2-81205-gcc-linux.tar.gz
```

**Note on compatibility**: If you are using newer linux kernels/Ubuntu versions (eg. 20.04LTS), you might run into issues (such as [[1](https://github.com/ChampSim/ChampSim/issues/102)],[[2](https://stackoverflow.com/questions/55698095/intel-pin-tools-32-bit-processsectionheaders-560-assertion-failed)],[[3](https://stackoverflow.com/questions/43589174/pin-tool-segmentation-fault-for-ubuntu-17-04)]) with the PIN3.2. ChampSim tracer works fine with newer PIN tool versions that can be downloaded from [here](https://software.intel.com/content/www/us/en/develop/articles/pin-a-binary-instrumentation-tool-downloads.html). PIN3.17 is [confirmed](https://github.com/ChampSim/ChampSim/issues/102) to work with Ubuntu 20.04.1 LTS.

Once downloaded, open tracer/make_tracer.sh and change PIN_ROOT to Pin's location.
Run ./make_tracer.sh to generate champsim_tracer.so.

**Use the Pin tool like this**
```
pin -t obj-intel64/champsim_tracer.so -- <your program here>
```

The tracer has three options you can set:
```
-o
Specify the output file for your trace.
The default is default_trace.champsim

-s <number>
Specify the number of instructions to skip in the program before tracing begins.
The default value is 0.

-t <number>
The number of instructions to trace, after -s instructions have been skipped.
The default value is 1,000,000.
```
For example, you could trace 200,000 instructions of the program ls, after
skipping the first 100,000 instructions, with this command:
```
pin -t obj/champsim_tracer.so -o traces/ls_trace.champsim -s 100000 -t 200000 -- ls
```
Traces created with the champsim_tracer.so are approximately 64 bytes per instruction,
but they generally compress down to less than a byte per instruction using xz compression.

# Evaluate Simulation

ChampSim measures the IPC (Instruction Per Cycle) value as a performance metric. <br>
There are some other useful metrics printed out at the end of simulation. <br>

Good luck and be a champion! <br>
