//===========================================================================
//
// This example from a prerelease of the Scalable HeterOgeneous Computing
// (SHOC) Benchmark Suite Alpha v1.1.1i for Intel MIC architecture
// Contact: Kyle Spafford <kys@ornl.gov>
//         Rezaur Rahman <rezaur.rahman@intel.com>
//
// Copyright (c) 2011, UT-Battelle, LLC
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//   
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of Oak Ridge National Laboratory, nor UT-Battelle, LLC, nor
//    the names of its contributors may be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// ==============================================================================

#include <math.h>
#include <stdlib.h>
#include <sys/time.h>
#include <cassert>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <sys/types.h>
#include <sys/sysctl.h>


#include <stdio.h>
#include "Timer.h"
#include "ResultDatabase.h"
#include "OptionParser.h"
#include "omp.h"
#include <string>
#include <iostream>

#include <mkl_service.h>
#include <mkl_blas.h>


#include "offload.h"

#ifdef __MIC__
        #include <lmmintrin.h>
#endif


using namespace std;




#define KNFFREQ 1200
#define CPUFREQ 3330
#define NTHREADS 124

int micFreq = KNFFREQ;
int micdev = 0;

using namespace std;

template <class T>
void RunTest(string testName, ResultDatabase &resultDB, OptionParser &op);

template <class T>
inline void devGEMM(char transa, char transb, int m, int n, int k, T alpha,
        const T *A, int lda, const T *B, int ldb, T beta, T *C, int ldc);



#include <stdio.h>
#include <sys/types.h>
#include <sys/sysctl.h>

__declspec(target(mic)) int getMICFreqMHz()
{


	#if 0
       		int freq=0;
        	int retval=0;	
        	size_t len=sizeof(freq);
 
		const char* sysctl_name="lrb.info.hw.gpu.freq";
	
		retval = sysctl(sysctl_name,strlen(sysctl_name), &freq, &len, 0L,0);
		if(retval)
              	printf("sysctl failed - getMICFreq");
        
		if(freq)
		{ 
                	printf("clk freq = %d \n", freq);
	   		return freq;
		}
	#endif
	#ifdef __MIC__
		return KNFFREQ;
	#else
        	return CPUFREQ;
	#endif

}

double second()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec / 1000000.0;
}


// ********************************************************
// Function: toString
//
// Purpose:
//   Simple templated function to convert objects into
//   strings using stringstream
//
// Arguments:
//   t: the object to convert to a string
//
// Returns:  a string representation of t
//
// Modifications:
//
// ********************************************************
template<class T> inline std::string toString(const T& t)
{
    stringstream ss;
    ss << t;
    return ss.str();
}

// ********************************************************
// Function: error
//
// Purpose:
//   Simple routine to print an error message and exit
//
// Arguments:
//   message: an error message to print before exiting
//
// ********************************************************
void error(char *message)
{
    cerr << "ERROR: " << message << endl;
    exit(1);
}

// ********************************************************
// Function: fill
//
// Purpose:
//   Simple routine to initialize input array
//
// Arguments:
//   A: pointer to the array to initialize
//   n: number of elements in the array
//
// ********************************************************
template <class T>
void fill(T *A, int n, int maxi)
{
    for (int j = 0; j < n; j++)
        A[j] = T((rand() % (maxi * 2 + 1)) - maxi) / (maxi + 1.);
}

// ****************************************************************************
// Function: addBenchmarkSpecOptions
//
// Purpose:
//   Add benchmark specific options parsing.  The user is allowed to specify
//   the size of the input data in kiB.
//
// Arguments:
//   op: the options parser / parameter database
//
// Programmer: Anthony Danalis
// Creation: September 08, 2009
// Returns:  nothing
//
// ****************************************************************************
void addBenchmarkSpecOptions(OptionParser &op)
{
    op.addOption("KiB", OPT_INT, "0", "data size (in Kibibytes)");
    op.addOption("N", OPT_INT, "0", "SQ Matrix Dimension");
}

// ****************************************************************************
// Function: runBenchmark
//
// Purpose:
//   This benchmark measures the performance of the single precision general
//   matrix multiplication (SGEMM) operation in GFLOPS.  Data transfer time
//   over the PCIe bus is not included in this measurement.
//
// Arguments:
//  resultDB: the benchmark stores its results in this ResultDatabase
//  op: the options parser / parameter database
//
// Returns:  nothing
//
// Programmer: Anthony Danalis
// Creation: September 08, 2009
//
// Modifications:
//
// ****************************************************************************

int MICFreq=0;

void
RunBenchmark(OptionParser &op, ResultDatabase &resultDB)
{
    

     typedef enum {KNF=1, KNC=2} MIC_DEVICE;
     MIC_DEVICE KNF_DEVICE=KNF;
     micdev = op.getOptionInt("target");

// setup openmp environment on the device
#pragma offload target(mic:micdev) inout(MICFreq)
        {
#ifdef __MIC__
                omp_set_num_threads(NTHREADS);
                kmp_set_defaults("KMP_AFFINITY=explicit,proclist=[4-127:1],granularity=fine");
                MICFreq =  getMICFreqMHz();
#endif
        }

    cout << "Running single precision test" << endl;
    RunTest<float>("SGEMM", resultDB, op);
    cout << "Running double precision test" << endl;
    RunTest<double>("DGEMM", resultDB, op);
 
/*
    if (KNF_DEVICE==KNC)
    {
        cout << "Running double precision test" << endl;
        RunTest<double>("DGEMM", resultDB, op);
    } else {
        cout << "Skipping double precision test" << endl;
        char atts[1024] = "DP_Not_Supported";
        resultDB.AddResult("DGEMM" , atts, "GFLOPS/s", 0);
    }
*/

}

template <class T>
void RunTest(string testName, ResultDatabase &resultDB, OptionParser &op)
{
    int passes = op.getOptionInt("passes");
    int N;

    if ((op.getOptionInt("KiB") == 0) && (op.getOptionInt("N") == 0))
    {
        int probSizes[4] = { 1, 4, 8, 16 };
        N = probSizes[op.getOptionInt("size")-1] * 1024 / sizeof(T);
    } else if((op.getOptionInt("KiB") == 0)){
        N = op.getOptionInt("N")/(sizeof(T)/sizeof(float));// double works with half the matrix size of floats 
    }else{
        N = op.getOptionInt("KiB") * 1024 / sizeof(T);
    }

    // Initialize host memory
	static     T *A;
	static     T *B;
	static    T *C;

     int LD = N % 512 ? N : N + 32;

     size_t matrix_elements = N * N;
     size_t matrix_bytes = matrix_elements * sizeof(T);
     // make size multiple of 64 bytes
     size_t allocation_size = ((matrix_bytes+4096.0)/4096.0)*4096;

	printf("matrix bytes = %ld, alloc=%ld\n",matrix_bytes, allocation_size);
	fflush(stdout);
	A=B=C=NULL;
    // Allocate memory for the matrices
    A = (T *)_mm_malloc(allocation_size,4096);
    B = (T *)_mm_malloc(allocation_size,4096);
    C = (T *)_mm_malloc(allocation_size,4096);

// for transfer efficiency, transfer 64x data bytes
    matrix_elements = allocation_size/sizeof(T); 
    
    if(!A || !B || !C){
       printf("memory allocaiton failed\n");
       return ;
    }
//
    fill<T>(A, N * N, 31);
    fill<T>(B, N * N, 31);
    fill<T>(C, N * N, 31);

// Allocate buffer on the device


// allocate memory on the card for the first call with largest array size and keep it around
#pragma offload target(mic:micdev) \
        in(A:length(matrix_elements) free_if(0) ) \
        in(B:length(matrix_elements) free_if(0))\
        in(C:length(matrix_elements) free_if(0))\
        out(C:length(matrix_elements) free_if(0))
        {
        }



    double startTime, dataSendTime, computeTime, memFreeTime;

    startTime = second();


#pragma offload target(mic:micdev) \
        in(A:length(matrix_elements) alloc_if(0) free_if(0) ) \
        in(B:length(matrix_elements) alloc_if(0) free_if(0)) \
        in(C:length(matrix_elements) alloc_if(0) free_if(0)) \
        out(C:length(matrix_elements)  alloc_if(0) free_if(0))
        {
        }

       dataSendTime = second() - startTime;
// move to a format needed by the benchmark
    float transferTime = (float)dataSendTime; //includes input and results output time

    printf("data transfer time=%lf, BW=%lf\n", dataSendTime, (matrix_elements*sizeof(T)/dataSendTime/1.0e9));
    fflush(stdout);
    for (; passes > 0; --passes)
    {
        for (int i = 0; i < 2; i++)
        {
            const char transa = 'N';
            const char transb = i ? 'T' : 'N';
            const int nb = 128;
            const int idim = N / nb;

            int dim = idim * nb;

            const int m = dim;
            const int n = dim;
            const int k = dim;
            const int lda = dim;
            const int ldb = dim;
            const int ldc = dim;
            const T alpha = 1;
            const T beta = -1; //benchmark has it as 0
            double blas_time;
#pragma offload target(mic:micdev) \
        in(transa,transb,m,n,k,lda,ldb,ldc)\
        nocopy(A:length(matrix_elements) alloc_if(0) free_if(0) ) \
        nocopy(B:length(matrix_elements) alloc_if(0) free_if(0)) \
        nocopy(C:length(matrix_elements) alloc_if(0) free_if(0)) \
        nocopy(C:length(matrix_elements)  alloc_if(0) free_if(0))\
        inout(blas_time)
        {
            const T alpha = 1;
            const T beta = -1; //benchmark has it as 0


            
            int freq = getMICFreqMHz();
    // warm up
          devGEMM<T>(transa, transb, m, n, k, alpha, A, lda, B, ldb, beta,
                  C, ldc);

            double t2 = ((double)__rdtsc())/((double)freq*1e6);

            for (int ii = 0; ii < 4; ++ii)
            {

        	devGEMM<T>(transa, transb, m, n, k, alpha, A, lda, B, ldb,
                           beta, C, ldc);
             }

            t2 = ((double)__rdtsc())/((double)freq*1e6) - t2;
 
            blas_time = (t2 / 4.0) ;
         }

            
            double blas_gflops = 2. * m * n * k / blas_time / 1e9;
            double pcie_gflops = 2. * m * n * k / (blas_time + transferTime)
                    / 1e9;
            resultDB.AddResult(testName+"-"+transb, toString(dim), "GFlops",
                    blas_gflops);
            resultDB.AddResult(testName+"-"+transb+"_PCIe", toString(dim),
                    "GFlops", pcie_gflops);
            resultDB.AddResult(testName+"-"+transb+"_Parity", toString(dim),
                    "N", transferTime / blas_time);

          //  printf("[RUN] SHOC-%s\n", testName+"-"+transb);
          //  printf("[DESCRIPTION] SHOC-%s, N=%d\n",testName+"-"+transb,dim);
          //  printf("[PERFORMACE] Task.Computaition.Avg %lf Gflops\n",pcie_gflops);
          //  printf("[PERFORMACE] Task.Computaition.Avg-No-PCIe %lf Gflops\n",blas_gflops);
 
        }
    }

    // Clean Up
   
#pragma offload target(mic:micdev) \
        in(A:length(matrix_elements) alloc_if(0) ) \
        in(B:length(matrix_elements) alloc_if(0)) \
        in(C:length(matrix_elements) alloc_if(0)) \
        out(C:length(matrix_elements)  alloc_if(0) )
        {
        }
if(A) _mm_free(A);
if(B) _mm_free(B); 
if (C) _mm_free(C);
}

template<>
inline void devGEMM<double>(char transa, char transb, int m, int n, int k,
        double alpha, const double *A, int lda, const double *B, int ldb,
        double beta, double *C, int ldc) {
                   
     dgemm(&transa, &transb, &m, &n, &k, &alpha,
                                       A, &lda, B, &ldb, &beta, C, &ldc);
}

template <>
inline void devGEMM<float>(char transa, char transb, int m, int n, int k,
        float alpha, const float *A, int lda, const float *B, int ldb,
        float beta, float *C, int ldc ){

     sgemm(&transa, &transb, &m, &n, &k, &alpha,
                                        A, &lda, B, &ldb, &beta, C, &ldc);
}
