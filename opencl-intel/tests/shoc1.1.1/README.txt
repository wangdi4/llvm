
The latest KNC code is in: https://subversion.jf.intel.com/ssg/drd/epe/ase/tc/workloads/trunk/OpenSource/SHOC/1.1.1/

As of now this version of SHOC KNC code W/Ls are not optimized for KNC hardware.The code is similar to KNF code. Some "#include<>" in the source code and  the build environment are changed to work with new compiler and driver for KNC. We are in the process of optimizing code for KNC.

The current binary and the source code is tested for:
Compiler: Composer_xe_2013.0.017 
Driver: MPSS 2.1-2430-2
MIC: KNC A0

The codes that are more  optimized for KNF/KNC are:
BusSpeed*
DeviceMemory
MaxFlops
FFT
Reduction
Scan
SGEMM
Spmv
MD
Sort

Issues:
There are total of 14 W/Ls available for SHOC. 11 W/Ls are woring fine.
As of now: The following workloads are not working with the new compiler/driver:
DeviceMemory (Intrinsic issue)
MaxFlops (Intrinsic issue)
S3D (SCIF write issue)

It is recommended to recompile the src code in your system. The binaries are available for the above compiler/driver.

 How to make:
Type make inside 1.1.1/src/  (it will make some common files. Some files for CUDA/OpenCL will fail to be compiled which we can ignore)
Type make  inside 1.1/src/knc-Mpss2.1/ (it will make all the WL except Stencil2D which is in seperate directory)
Type make inside 1.1.1/src/knc-Mpss2.1/Stencil2d/ (it will make Stencil2D, also some object files from the uppper level directory needed to be added)

How to run:
The binaries are produded in 1.1.1/bin/Serial/knc-MPSS2.1/.
cd 1.1.1/bin/Serial/knc-MPSS2.1/
For example, to run MD W/L
./MD -s 1
Argument for s is 1,2,3,4, which correspond problem size from smallest to the largest.

For the new Sort code and with compiler 2013.0.021 the best environment setting is:
export MIC_ENV_PREFIX=MIC
export MIC_OMP_NUM_THREADS=236
export MIC_USE_2MB_BUFFERS=32K
export MIC_KMP_AFFINITY="explicit,granularity=thread,proclist=[1-233:4,2-234:4,3-235:4,4-236:4]"
time ./Sort -s 4

For MD the best environment setting is
export MIC_ENV_PREFIX=MIC
export MIC_OMP_NUM_THREADS=236
export MIC_KMP_AFFINITY=granularity=thread,balanced
 export MIC_USE_2MB_BUFFERS=32K

To run SPmv code with new matrix:
export MIC_ENV_PREFIX=MIC
export MIC_OMP_NUM_THREADS=236
export MIC_KMP_AFFINITY=balanced,granularity=fine
folder=/nfs/pdx/home/mbhuiyan/mkl/__release_lnx/
export MKLROOT=${folder}/mkl
export LD_LIBRARY_PATH=${MKLROOT}/lib/intel64:${LD_LIBRARY_PATH}
export MIC_LD_LIBRARY_PATH=${MKLROOT}/lib/mic:${MIC_LD_LIBRARY_PATH}
export C_INCLUDE_PATH=${MKLROOT}/include:${C_INCLUDE_PATH} #/project/meguney/mkl_build/mkl_mic_nigh        tly/mkl/include:${        C_INCLUDE_PATH}
export MIC_BUFFERSIZE=128M
./Spmv --mm_filename ./cant/cant.mtx

To run GEMM code with new code:(It only runs on 2013.0.17 compiler, so you have to source 017)
environment is same as the Spmv
export MIC_OMP_NUM_THREADS=224
./GEMM -N 4096

To run FFT code with KNC:
environ is same is Spmv except:
export MIC_OMP_NUM_THREADS=128
./FFT --MB 256

Bob's new version of Scan runs on 2013 017 compiler
