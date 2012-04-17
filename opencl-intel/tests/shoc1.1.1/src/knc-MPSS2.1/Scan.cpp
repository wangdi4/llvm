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

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <string>
#include "omp.h"

// #include "offload.h"
#include "Timer.h"

#include "OptionParser.h"
#include "ResultDatabase.h"
 
#ifdef __MIC2__
#include <immintrin.h>
#endif 0
#include "common/sampling_MIC.h"
// #include "sampling.h"
 
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <cassert>
#include <iostream>
#include <vector>

#include "OptionParser.h"
#include "ResultDatabase.h"
#include "Scan.h"

#define BLOCK 768
#define ERR 1.0e-4

using namespace std;
#define NUM_THREADS (236)

#define USE_TILING
#ifdef USE_TILING
#define KNC_IDEAL_L2_BUFFER (256*16)  // This was last tuned for KNC hardware.
__declspec(target(mic)) int ideal_buffer_size = KNC_IDEAL_L2_BUFFER;
// #define USE_TBB
#ifdef USE_TBB
#include "scan_tiling_tbb.h"
#pragma offload_attribute(pop)
#else
#include "scan_tiling_openmp.h"
#endif
#endif

template <class T>
__declspec(target(MIC)) void  scanKNF(T *data,  T* dev_result, const size_t size, T* ipblocksum, T* opblocksum, int numblocks);

// ****************************************************************************
// Function: addBenchmarkSpecOptions
//
// Purpose:
//   Add benchmark specific options parsing
//
// Arguments:
//   op: the options parser / parameter database
//
// Returns:  nothing
//
// Programmer: Kyle Spafford
// Creation: August 13, 2009
//
// Modifications:
//
// ****************************************************************************
void addBenchmarkSpecOptions(OptionParser &op)
{
    op.addOption("iterations", OPT_INT, "256", "specify scan iterations");
}

// ****************************************************************************
// Function: RunBenchmark
//
// Purpose:
//   Executes the scan (parallel prefix sum) benchmark
//
// Arguments:
//   resultDB: results from the benchmark are stored in this db
//   op: the options parser / parameter database
//
// Returns:  nothing
//
// Programmer: Kyle Spafford
// Creation: August 13, 2009
//
// Modifications:
//
// ****************************************************************************
void
RunBenchmark(OptionParser &op, ResultDatabase &resultDB)
{
    cout << "Running single precision test" << endl;
	RunTest<float>("Scan", resultDB, op);

    // Test to see if this device supports double precision
        cout << "Running double precision test" << endl;
        RunTest<double>("Scan-DP", resultDB, op);
}


#define ALIGN (4096)


template <class T>
void RunTest(string testName, ResultDatabase &resultDB, OptionParser &op)
{
    int probSizes[8] = { 1, 8, 32, 64 , 128 , 256 , 512 }; // increased the problem size to demonstrate that application will now scale.

    int size = probSizes[op.getOptionInt("size")-1];
    // Convert to MB
    size = (size *1024*1024)/sizeof(T);
    // create input data on CPU
    unsigned int bytes = size * sizeof(T);

    // Allocate Host Memory
__declspec(target(MIC)) static     T* h_idata;
__declspec(target(MIC)) static     T* reference;
__declspec(target(MIC)) static    T* h_odata;

     h_idata = (T*)_mm_malloc(bytes,ALIGN);
     reference = (T*)_mm_malloc(bytes,ALIGN);
     h_odata = (T*)_mm_malloc(bytes,ALIGN);

	   
    // Initialize host memory
    // cout << "Initializing host memory." << endl;
    for (int i = 0; i < size; i++) 
    { 
        h_idata[i] = i % 3; // Fill with some pattern
        h_odata[i] = 0.0; 
        reference[i]=0.0;
    }

    int micdev = op.getOptionInt("target");

    // allocate device memory

    // Allocate  data to CoProc 
    // cout << "Copying data to device." << endl;
    #pragma offload target(mic:micdev) in(h_idata:length(size)  free_if(0)) \
                                out(h_odata:length(size) free_if(0))
        {
        }

    double start = curr_second();
    // Get data transfer time
    #pragma offload target(mic:micdev) in(h_idata:length(size) alloc_if(0) free_if(0)) \
                                out(h_odata:length(size) alloc_if(0) free_if(0))
        {
        }


    float transferTime = curr_second()-start;

    int passes = op.getOptionInt("passes");
    int iters = op.getOptionInt("iterations");

    // cout << "Running benchmark with size " << size << endl;
    for (int k = 0; k < passes; k++)
    {
        double totalScanTime = 0.0f;
        start = curr_second();

    #pragma offload target(mic:micdev) nocopy(h_idata:length(size) alloc_if(0) free_if(0)) \
                                nocopy(h_odata:length(size) alloc_if(0) free_if(0))
        {
			int ThreadCount = NUM_THREADS;
#ifdef USE_TILING
#ifdef USE_TBB
			my_TBB_init(NUM_THREADS);
#endif
			T* ipblocksum = (T*)malloc((ThreadCount+1)*sizeof(T));
			for (int j = 0; j < iters; j++)
        	{	
				if (j % 10 == 0) VTResumeSampling();
              	scanTiling<T>(h_idata, h_odata, size, ipblocksum, ThreadCount);
				if (j % 10 == 0) VTPauseSampling();
        	}
			free ((void *)ipblocksum);
#else
	
			int numblocks;
			numblocks=(int)ceil((double)size/BLOCK);
			T* ipblocksum=(T*)malloc((numblocks+1)*sizeof(T));
			T* opblocksum=(T*)malloc((numblocks+1)*sizeof(T));
			for (int j = 0; j < iters; j++)
        	{	
				if (k == 2 && j % 10 == 0) VTResumeSampling();
                scanKNF<T>(h_idata, h_odata, size, ipblocksum, opblocksum, numblocks);
				if (k == 2 && j % 10 == 0) VTPauseSampling();
        	}
		    free(ipblocksum);
		    free(opblocksum);
#endif
	}	
        totalScanTime = curr_second()-start;
   
     #pragma offload target(mic:micdev) out(h_odata:length(size) alloc_if(0) free_if(0))
  	{
	}
        // If results aren't correct, don't report perf numbers
        if (! scanCPU<T>(h_idata, reference, h_odata, size))
        {
            return;
        }

#ifdef PRINT_PREFIX_SUM_LAST_ELEMENT
printf("reference[size]=%lf, odata[size]=%lf \n", reference[size-1], h_odata[size-1]);
#endif

        char atts[1024];
        double avgTime = (totalScanTime / (double) iters);
		printf ("avgTime = %f\n", avgTime);
        sprintf(atts, "%d items", size);
        double gb = (double)(size * sizeof(T)) / (1000. * 1000. * 1000.);
        resultDB.AddResult(testName, atts, "GB/s", gb / avgTime);
        resultDB.AddResult(testName+"_PCIe", atts, "GB/s",
                gb / (avgTime + transferTime));
        resultDB.AddResult(testName+"_Parity", atts, "N",
                transferTime / avgTime);
    }

//clean up
    #pragma offload target(mic:micdev) in(h_idata:length(size) alloc_if(0) ) \
                                out(h_odata:length(size) alloc_if(0))
        {
 
        }
    _mm_free(h_idata);
    _mm_free(h_odata);
    _mm_free(reference);

#if 0
    for (int i = 0; i < numLevelsAllocated; i++)
    {
        _mm_free(scanBlockSums[i]);
    }
    free(scanBlockSums);
#endif 

}

// ****************************************************************************
// Function: scanArrayRecursive
//
// Purpose:
//   Workhorse for the scan benchmark, this function recursively scans
//   arbitrary sized arrays, including those which are of a non power
//   of two length, or not evenly divisible by block size
//
// Arguments:
//     outArray: pointer to output memory on the device
//     inArray:  pointer to input memory on the device
//     numElements: the number of elements to scan
//     level: the current level of recursion, starting at 0
//     blockSums: pointer to device memory to store intermediate sums
//
// Returns:
//
// Programmer: Kyle Spafford
// Creation: August 13, 2009
//
// Modifications:
//
// ****************************************************************************
template <class T>
void scanArrayRecursive(T* outArray, T* inArray, int numElements,
        int level, T** blockSums)
{
    

#if 0
// Kernels handle 8 elems per thread
    unsigned int numBlocks = max(1,
    		(unsigned int)ceil((float)numElements/(4.f * BLOCK_SIZE)));
    unsigned int sharedEltsPerBlock = BLOCK_SIZE * 2;
    unsigned int sharedMemSize = sizeof(T) * sharedEltsPerBlock;

    bool fullBlock = (numElements == numBlocks * 4 * BLOCK_SIZE);

    dim3 grid(numBlocks, 1, 1);
    dim3 threads(BLOCK_SIZE, 1, 1);

    // execute the scan
    if (numBlocks > 1) 
    {
        scan<T, vecT><<<grid, threads, sharedMemSize>>>
            (outArray, inArray, blockSums[level], numElements, fullBlock, true);
    } else
    {
        scan<T, vecT><<<grid, threads, sharedMemSize>>>
           (outArray, inArray, blockSums[level], numElements, fullBlock, false);
    }
    if (numBlocks > 1)
    {
        scanArrayRecursive<T, vecT>(blockSums[level], blockSums[level],
                numBlocks, level + 1, blockSums);
        vectorAddUniform4<T><<< grid, threads >>>
                (outArray, blockSums[level], numElements);
    }
#endif
} 

template <class T>
__declspec(target(MIC)) void  scanKNF(T *input,  T* output, const size_t n, T* ipblocksum, T* opblocksum, int numblocks)
{
#define USE_BETTER_OPENMP
#ifdef USE_BETTER_OPENMP
#include "scan_openmp.h"
#else
#define USE_ORIGINAL_CODE
#include "scan_original.h"
#endif
}


// ****************************************************************************
// Function: scanCPU
//
// Purpose:
//   Simple cpu scan routine to verify device results
//
// Arguments:
//   data : the input data
//   reference : space for the cpu solution
//   dev_result : result from the device
//   size : number of elements
//
// Returns:  nothing, prints relevant info to stdout
//
// Programmer: Kyle Spafford
// Creation: August 13, 2009
//
// Modifications:
//
// ****************************************************************************
template <class T>
bool scanCPU(T *data, T* reference, T* dev_result, const size_t size)
{
	// BUG in OpenMP code: reference[0] is not 0!  It should be data[0].  Just lucky here because they initialized data[0] to zero!
    reference[0] = 0;
    bool passed = true;
	// You cannot validate beyond a certain buffer size because of rounding errors.
	if (size > 128) {
#ifdef USE_ORIGINAL_CODE
		for (unsigned int i = 1; i < size; ++i)
		{
			reference[i] = data[i - 1] + reference[i - 1];
		}
#else
		// This is an inclusive scan while the OpenMP code is an exclusive scan (I think that is what it is called.)
		reference[0] = data[0];
		for (unsigned int i = 1; i < size; ++i)
		{
			reference[i] = data[i] + reference[i - 1];
		}
#endif    
		for (unsigned int i = 0; i < size; ++i)
		{
			if (abs(reference[i] - dev_result[i]) > ERR )
			{
// #ifdef VERBOSE_OUTPUT

				cout << "Mismatch at i: " << i << " ref: " << reference[i - 1]
					 << " dev: " << dev_result[i] << endl;
// #endif
				passed = false;
			}
		}
	}
    cout << "Test ";
    if (passed)
        cout << "Passed" << endl;
    else
        cout << "---FAILED---" << endl;
    return passed;
}
