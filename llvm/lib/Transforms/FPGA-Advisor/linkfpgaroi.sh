#!/bin/sh

BOOST_ROOT=/usr/intel/pkgs/boost/1.59.0/
TBBLIBDIR=/nfs/hd/proj/asim/x86_64_linux26/bld/tbb/tbb41_20121003oss/build/linux_intel64_gcc_cc4.4.2_libc2.4_kernel2.6.16.60_release

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
export LD_LIBRARY_PATH=$TBBLIBDIR:$BOOST_ROOT/lib/

csa-opt -load=$CSA_TOP/tools/install/lib/LLVMBasicBlockNumberer.so -bbnumber -bbn:o $prefix.bbnumber.txt < $prefix.bc > $prefix.bbn.bc

csa-opt -load=$CSA_TOP/tools/install/lib/LLVMFPGA-Advisor.so -fpga-advisor-instrument < $prefix.bbn.bc > $prefix.bbn.fpgaadv.bc

csa-opt -load=$CSA_TOP/tools/install/lib/LLVMROIController.so -roi -roi:i $roifile < $prefix.bbn.fpgaadv.bc > $prefix.bbn.fpgaadv.roi.bc

clang++ $prefix.bbn.fpgaadv.roi.bc $$CSA_TOP/tools/src/llvm/examples/FPGA-Advisor/common/rdtsc.ll $$CSA_TOP/tools/src/llvm/lib/Transforms/LLVMROIController/ROIanalysis.bc $$CSA_TOP/tools/src/llvm/lib/Transforms/FPGA-Advisor/ROIhandler.bc -o $prefix.roicontroller.fpgaadvisor -L$BOOST_ROOT/lib -lboost_system -lboost_serialization

echo "$prefix.roicontroller.fpgaadvisor  generated!"
