#!/bin/sh
CSALLVMSRC=/nfs/site/home/hgpatil/links/ThreadPoints/DesignForward2/CSA-LPU/Leap-FPGA-Advisor/llvm-3.6.1.src/
CSALLVMBUILD=/nfs/site/home/hgpatil/links/ThreadPoints/DesignForward2/CSA-LPU/Leap-FPGA-Advisor/llvm-build/
BOOST_ROOT=/usr/intel/pkgs/boost/1.59.0/

ERROR()
{
    echo "Usage: $0 bitcode-file roi-file"
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

if [ ! -e $2 ];
then
    echo "roi-file $2 does not exist"
    ERROR
fi

if [[ $1 == *".bc" ]]
then
    prefix=`echo $1 | sed '/.bc/s///'`
else
    echo "the bit-code file must have '.bc' suffix"
    ERROR
fi

roifile=$2
export LD_LIBRARY_PATH=$BOOST_ROOT/lib/

$CSALLVMBUILD/bin/opt -load=$CSALLVMBUILD/lib/LLVMBasicBlockNumberer.so -bbnumber -bbn:o $prefix.bbnumber.txt < $prefix.bc > $prefix.bbn.bc

$CSALLVMBUILD/bin/opt -load=$CSALLVMBUILD/lib/LLVMROIController.so -roi -roi:i $roifile < $prefix.bbn.bc > $prefix.roi.bc

clang++ $prefix.roi.bc $CSALLVMSRC/lib/Transforms/LLVMROIController/ROIanalysis.bc $CSALLVMSRC/lib/Transforms/LLVMROIController/ROIhandler.bc -o $prefix.roicontroller -L$BOOST_ROOT/lib -lboost_system -lboost_serialization

echo "$prefix.roicontroller generated!"
