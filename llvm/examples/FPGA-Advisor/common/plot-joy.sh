#!/usr/intel/bin/bash
#SRC=matrixmult
SRC=$1

# Verify that the program has been compiled to bitcode
if [ ! -e $SRC.opt.bc ]; then
    echo "Please build $SRC first"
    exit
fi

gnuplot_ver=$( gnuplot --version | cut -d' ' -f2 )
gnuplot_major=$( echo $gnuplot_ver | cut -d. -f1 )

echo "# Area Latency" > values.dat
for i in $( seq 100 100 2000 ); do
    echo "Area $i..."
    values=$( csa-opt -load LLVMFPGA-Advisor.so -fpga-advisor-analysis -use-threads=20 -parallelize-one-zero=1 -serial-cutoff=10 -rapid-convergence=5 -trace-file=trace.log $SRC.opt.bc -o /dev/null -area-constraint $i 2>&1 | awk "/Final Latency/{ print $i, \$NF }" )
    echo "$values" >> values.dat
done

echo "set xlabel 'Area'" > plot.plt
echo "set ylabel 'Latency'" >> plot.plt
echo "set title '$SRC'" >> plot.plt
echo "plot 'values.dat' with linespoints" >> plot.plt
if [ $gnuplot_major -lt "5" ]; then
    echo "pause -1 'Hit any key to continue'" >> plot.plt
    gnuplot plot.plt
else
    gnuplot -p plot.plt &
fi



