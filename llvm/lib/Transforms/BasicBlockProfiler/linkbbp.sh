#!/bin/sh
CSALLVMSRC=/nfs/site/home/hgpatil/links/ThreadPoints/DesignForward2/CSA-LPU/Leap-FPGA-Advisor/llvm-3.6.1.src/
CSALLVMBUILD=/nfs/site/home/hgpatil/links/ThreadPoints/DesignForward2/CSA-LPU/Leap-FPGA-Advisor/llvm-build/
BOOST_ROOT=/usr/intel/pkgs/boost/1.59.0/

ERROR()
{
    echo "Usage: $0 bitcode-file slice-size"
    exit 1
}

if  [ $# != 2 ];  then
    echo "Not enough arguments!"
    ERROR
fi

if [ ! -e $1 ];
then
    echo "bit-code file $1 does not exist"
    ERROR
fi

if [[ $1 == *".bc" ]]
then
    prefix=`echo $1 | sed '/.bc/s///'`
else
    echo "the bit-code file must have '.bc' suffix"
    ERROR
fi

slicesize=$2

export LD_LIBRARY_PATH=$BOOST_ROOT/lib/

$CSALLVMBUILD/bin/opt -load=$CSALLVMBUILD/lib/LLVMBasicBlockNumberer.so -bbnumber -bbn:o $prefix.bbnumber.txt < $prefix.bc > $prefix.bbn.bc

$CSALLVMBUILD/bin/opt -load=$CSALLVMBUILD/lib/LLVMBasicBlockProfiler.so -bbprofile -bbp:slice_size $slicesize -bbp:o $prefix < $prefix.bbn.bc > $prefix.bbp.bc

clang++ $prefix.bbp.bc $CSALLVMSRC/lib/Transforms/BasicBlockProfiler/bbanalysis.bc $CSALLVMSRC/lib/Transforms/BasicBlockProfiler/bbprofile.bc -o $prefix.bbprofiler -L$BOOST_ROOT/lib -lboost_system -lboost_serialization

echo "$prefix.bbprofiler generated!"
