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


// ****************************************************************************
// File: GEMM.cpp
//
// Purpose:
//   Contains performance tests for a SGEMM and DGEMM MKL calls.
//
// ****************************************************************************

#include <iostream>
#include <sstream>
#include <iomanip>

#include "Timer.h"
#include "ResultDatabase.h"
#include "OptionParser.h"
#include "omp.h"
#include <string>
#include <iostream>

#include <mkl_service.h>
#include <mkl_blas.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/sysctl.h>

#include "offload.h"
#ifdef __MIC__
#include <lmmintrin.h>
#endif

#include "omp.h"

using namespace std;

#define NTHREADS 124

int micdev = 0;

using namespace std;

// Forward declarations
template <class T>
void RunTest(string testName, ResultDatabase &resultDB, OptionParser &op);

template <class T>
inline void devGEMM(char transa, char transb, int m, int n, int k, T alpha,
        const T *A, int lda, const T *B, int ldb, T beta, T *C, int ldc);


extern "C" double dsecnd();
__declspec(target(mic))
inline double curr_second() { return dsecnd(); }

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
//   This benchmark measures the performance of the general
//   matrix multiplication (GEMM) operation in GFLOPS.  Results are
//   reported with and without PCIe transfer time.
//
// Arguments:
//  resultDB: the benchmark stores its results in this ResultDatabase
//  op: the options parser / parameter database
//
// Returns:  nothing
//
// ****************************************************************************

// The following two methods are just a templatized call to GEMM.

template<> 
 inline __declspec(target(MIC))  void devGEMM<double>(char transa, char transb, int m, int n, int k,
        double alpha, const double *A, int lda, const double *B, int ldb,
        double beta, double *C, int ldc) {

   dgemm(&transa, &transb, &m, &n, &k, &alpha,
           A, &lda, B, &ldb, &beta, C, &ldc);
}

template <>
inline __declspec(target(MIC)) void devGEMM<float>(char transa, char transb, int m, int n, int k,
        float alpha, const float *A, int lda, const float *B, int ldb,
        float beta, float *C, int ldc ){

   sgemm(&transa, &transb, &m, &n, &k, &alpha,
           A, &lda, B, &ldb, &beta, C, &ldc);
}


#define ALIGNMENT (2*1024*1024)

void
RunBenchmark(OptionParser &op, ResultDatabase &resultDB)
{

    micdev = op.getOptionInt("target");

    // Setup openmp environment on the device
    #pragma offload target(MIC:micdev) 
    {
        omp_set_num_threads(NTHREADS);

		int max_threads = kmp_get_affinity_max_proc();

#pragma omp parallel num_threads(NTHREADS)
		{
			int ithr = omp_get_thread_num();
#if defined(__MIC__) && defined(__INTEL_OFFLOAD)
#ifdef __linux__
			ithr += 1;
#else
			ithr += 4;
#endif
#elif defined(__MIC__) && defined(__linux__)
			if (ithr < max_threads - 4)
				ithr += 1;
			else if (ithr == max_threads - 4)
				ithr = 0;
#endif
			kmp_affinity_mask_t m;
			kmp_create_affinity_mask(&m);
			kmp_set_affinity_mask_proc(ithr, &m);
			kmp_set_affinity(&m);
			kmp_destroy_affinity_mask(&m);
		}
    }
//#endif //I have commented out it

    // KS: Templates currently cause a bug, so for the time being
    //     only execute the double precision test.
     cout << "Running single precision test" << endl;
     RunTest<float>("SGEMM", resultDB, op);
    
    cout << "Running double precision test" << endl;
    RunTest<double>("DGEMM", resultDB, op);
}



template <class T>
void RunTest(string testName, ResultDatabase &resultDB, OptionParser &op)
{
    // Repeat the test multiple times
    int passes = op.getOptionInt("passes");
   
    // Dimension of the matrix
    int N;

    // Parse command line options
    // There are basically three options here
    // "-s [1-4]" pick a predefined size
    // "--N [number]" use a number x number square matrix
    // "--KiB [number]" use a large square matrix
    if ((op.getOptionInt("KiB") == 0) && (op.getOptionInt("N") == 0))
    {
        int probSizes[4] = { 1, 4, 8, 16 };
        N = probSizes[op.getOptionInt("size")-1] * 1024 / sizeof(T);
    }
    else if((op.getOptionInt("KiB") == 0)) 
    {
        N = op.getOptionInt("N"); 
        // For double we run half the size matrices
        N = N / (sizeof (T)/sizeof(float));
    }
    else 
    {
        N = op.getOptionInt("KiB") * 1024 / sizeof(T);
    }

#define FIX_LD(x) (((x) * sizeof(T)) % 1024 == 0 ? (x) + 128 : (x))

	int LDA = FIX_LD(N);

    // Initialize host memory
    // KS: In code received from Reza (knf.tar, this variable was declared as static
    //     which is presumably avoiding a bug).  Would like more info here.

__declspec(target(MIC))    static T *A;
__declspec(target(MIC))    static T *B;
__declspec(target(MIC))    static T *C;

    // Use a square matrix
    size_t matrix_elements = N * LDA;
    size_t matrix_bytes = matrix_elements * sizeof(T);

    printf("matrix bytes = %ld\n",matrix_bytes);
    fflush(stdout);
   
    // Allocate memory for the matrices
    A = (T *)_mm_malloc(matrix_bytes,ALIGNMENT);
    B = (T *)_mm_malloc(matrix_bytes,ALIGNMENT);
    C = (T *)_mm_malloc(matrix_bytes,ALIGNMENT);

    if(!A || !B || !C)
    {
        printf("memory allocaiton failed\n");
        return;
    }
   
    // Fill the matrices with some random data
    fill<T>(A, N * LDA, 31);
    fill<T>(B, N * LDA, 31);
    fill<T>(C, N * LDA, 31);

    // Allocate memory on the card and keep it around
    #pragma offload target(MIC:micdev) \
        in(A:length(matrix_elements)  alloc_if(1) free_if(0)) \
        in(B:length(matrix_elements)  alloc_if(1) free_if(0)) \
        in(C:length(matrix_elements)  alloc_if(1) free_if(0))
    {
    }

    // Timing variables
    double start_time, transfer_time;
    
    // Start the timer for the PCIe transfer
    // curr_second is a gettimeofday() timer
    start_time = curr_second();

    #pragma offload target(MIC:micdev) \
        in(A:length(matrix_elements)  alloc_if(0) free_if(0)) \
        in(B:length(matrix_elements)  alloc_if(0) free_if(0)) \
        in(C:length(matrix_elements)  alloc_if(0) free_if(0)) \
        out(C:length(matrix_elements) alloc_if(0) free_if(0))
    {
    }

    transfer_time = curr_second() - start_time;

    // Print out the transfer time in seconds for sanity check
    //printf("data transfer time=%lf\n", transfer_time);
    //fflush(stdout);
    
    // Begin main test loop
    for (; passes > 0; --passes)
    {
        for (int i = 0; i < 2; i++)
        {
            // Set up all the variables for the GEMM call
            const char transa = 'N';
            const char transb = 'N';
            const int nb = 128;
            const int idim = N / nb;
            int dim = idim * nb;
            const int m = dim;
            const int n = dim;
            const int k = dim;
            const int lda = FIX_LD(dim);
            const int ldb = FIX_LD(dim);
            const int ldc = FIX_LD(dim);


            #pragma offload target(MIC:micdev) \
                in(A:length(matrix_elements)  alloc_if(0) free_if(0))  \
                in(B:length(matrix_elements)  alloc_if(0) free_if(0))  \
                in(C:length(matrix_elements)  alloc_if(0) free_if(0))  \
                out(C:length(matrix_elements) alloc_if(0) free_if(0))
            {
                const T alpha = 1;
                const T beta = -1; //benchmark has it as 0


                // Warm up
                devGEMM<T>(transa, transb, m, n, k, alpha, A, lda, B, ldb, beta,
                           C, ldc);

            }

            // Time it takes for the actual gemm call
            double blas_time; 
            double startTime=curr_second();

            #pragma offload target(MIC:micdev) \
                nocopy(A: alloc_if(0) free_if(0))  \
                nocopy(B: alloc_if(0) free_if(0))  \
                nocopy(C: alloc_if(0) free_if(0))
	    {
                const T alpha = 1;
                const T beta = 1; //benchmark has it as 0


                // Do 4 iterations
                for (int ii = 0; ii < 4; ++ii)
                {
                   devGEMM<T>(transa, transb, m, n, k, alpha, A, lda, B, ldb,
                           beta, C, ldc);
                }

            }

            blas_time = (curr_second()-startTime)/4.0;

            // Calculate GFLOPS
            double blas_gflops = 2. * m * n * k / blas_time / 1e9;
            double pcie_gflops = 2. * m * n * k / (blas_time + transfer_time)
                / 1e9;
            resultDB.AddResult(testName+"-"+transb, toString(dim), "GFlops",
                    blas_gflops);
            resultDB.AddResult(testName+"-"+transb+"_PCIe", toString(dim),
                    "GFlops", pcie_gflops);
      }
   }

    // Clean Up
    #pragma offload target(MIC:micdev) \
        in(A:length(matrix_elements)  alloc_if(0) free_if(1)) \
        in(B:length(matrix_elements)  alloc_if(0) free_if(1)) \
        in(C:length(matrix_elements)  alloc_if(0) free_if(1)) \
        out(C:length(matrix_elements) alloc_if(0) free_if(1))
    {
    }

    _mm_free(A);
    _mm_free(B);
    _mm_free(C);
}


