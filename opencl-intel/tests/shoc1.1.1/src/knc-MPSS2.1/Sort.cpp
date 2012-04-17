//===========================================================================
////
//// This example from a prerelease of the Scalable HeterOgeneous Computing
//// (SHOC) Benchmark Suite Alpha v1.1.1i for Intel MIC architecture
//// Contact: Kyle Spafford <kys@ornl.gov>
////         Rezaur Rahman <rezaur.rahman@intel.com>
////
//// Copyright (c) 2011, UT-Battelle, LLC
//// All rights reserved.
//// 
//// Redistribution and use in source and binary forms, with or without
//// modification, are permitted provided that the following conditions are met:
////   
////  * Redistributions of source code must retain the above copyright
////    notice, this list of conditions and the following disclaimer.
////  * Redistributions in binary form must reproduce the above copyright
////    notice, this list of conditions and the following disclaimer in the
////    documentation and/or other materials provided with the distribution.
////  * Neither the name of Oak Ridge National Laboratory, nor UT-Battelle, LLC, nor
////    the names of its contributors may be used to endorse or promote products
////    derived from this software without specific prior written permission.
////
//// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
//// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//// ==============================================================================

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <string>
#include "omp.h"

#include "offload.h"
#include "Timer.h"

#include "OptionParser.h"
#include "ResultDatabase.h"

#ifdef TARGET_ARCH_LRB
#include <lmmintrin.h>
#include <pthread.h>
#endif



#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <cassert>
#include <iostream>
#include <vector>

#include "OptionParser.h"
#include "ResultDatabase.h"
#include "Sort.h"
#include "sortKernel.h"

#define BLOCK 768


using namespace std;


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
    op.addOption("nthreads", OPT_INT, "64", "specify number of threads");

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
    int device;

    cout << "Running test with unsigned int" << endl;
    RunTest<unsigned int>("Sort", resultDB, op);

}


#define ALIGN (4096)

template <class T>
void RunTest(string testName, ResultDatabase &resultDB, OptionParser &op)
{
    int probSizes[4] = { 1, 8, 48, 96 };

    int size = probSizes[op.getOptionInt("size")-1];
    // Convert to MB
    size = (size *1024*1024)/sizeof(T);
    // create input data on CPU
    unsigned int bytes = size * sizeof(T);

    // Allocate Host Memory
    __declspec(target(MIC)) static     T *hkey, *outkey;
    __declspec(target(MIC)) static     T *hvalue, *outvalue;

     hkey = (T*)_mm_malloc(bytes,ALIGN);
     hvalue = (T*)_mm_malloc(bytes,ALIGN);

     outkey = (T*)_mm_malloc(bytes,ALIGN);
     outvalue = (T*)_mm_malloc(bytes,ALIGN);


    // Initialize host memory
    cout << "Initializing host memory." << endl;
    for (int i = 0; i < size; i++)
    {
        hkey[i] = hvalue[i]= i % 255; // Fill with some pattern
    }

    int micdev = op.getOptionInt("target");
    int iters = op.getOptionInt("passes");
    int numThreads = op.getOptionInt("nthreads");

    cout << "Running benchmark" << endl;
    for(int it=0;it<iters;it++)
    {
    //cout << "Copying data to device." << endl;
    #pragma offload target(mic:micdev) in(hkey:length(size)  free_if(0)) \
                                in(hvalue:length(size) free_if(0))\
                                out(outkey:length(size) free_if(0))\
                                out(outvalue:length(size) free_if(0))
        {
        }

    double start = curr_second();
    // Get data transfer time
    #pragma offload target(mic:micdev) in(hkey:length(size) alloc_if(0) free_if(0)) \
                                in(hvalue:length(size) alloc_if(0) free_if(0))\
                                out(outkey:length(size) alloc_if(0) free_if(0))\
                                out(outvalue:length(size)alloc_if(0)  free_if(0))\
                                in(numThreads)

        {
        }


    float transferTime = curr_second()-start;

    double totalRunTime = 0.0f;
    start = curr_second();
    #pragma offload target(mic:micdev) nocopy(hkey:length(size) alloc_if(0) free_if(0)) \
                                nocopy(hvalue:length(size) alloc_if(0) free_if(0)) \
                                nocopy(outkey:length(size) alloc_if(0) free_if(0))\
                                nocopy(outvalue:length(size) alloc_if(0) free_if(0))\ 
                                in(numThreads)
    	{

#if 0
	T *tvalue,*tkey,*tarr;
	tvalue=(T*)malloc(sizeof(T)*bytes);
	tkey=(T*)malloc(sizeof(T)*bytes);
	tarr=(T*)malloc(sizeof(T)*bytes);
       	for (unsigned int j = 0; j < BITS; j++)
        {	
			radixoffset<T>(hkey,tkey,size,j);
              		scanArray<T>(tkey, tvalue, size);
			rearrange<T>(&hkey,&hvalue,&tkey,&tvalue,&tarr,size);			
        }
	free(tvalue);
	free(tkey);
	free(tarr);
#endif

          sortKernel<T>(hkey, hvalue, outkey, outvalue, size, numThreads);
    	}
        totalRunTime = curr_second()-start;

    #pragma offload target(mic:micdev) nocopy(hkey:length(size) alloc_if(0) free_if(1)) \
                                nocopy(hvalue:length(size) alloc_if(0) free_if(1)) \
                                out(outkey:length(size) alloc_if(0)) \
                                out(outvalue:length(size) alloc_if(0))
    	{
    	}


    // If results aren't correct, don't report perf numbers
    	if (! verifyResult<T>(outkey, outvalue, size))
        {
            return;
        }

        char atts[1024];
        double avgTime = (totalRunTime / (double) iters);
        sprintf(atts, "%d items", size);
        double gb = (double)(size * sizeof(T)) / (1000. * 1000. * 1000.);
        resultDB.AddResult(testName, atts, "GB/s", gb / avgTime);
        resultDB.AddResult(testName+"_PCIe", atts, "GB/s",
                gb / (avgTime + transferTime));
        resultDB.AddResult(testName+"_Parity", atts, "N",
                transferTime / avgTime);

    }
//clean up
    _mm_free(hkey);
    _mm_free(hvalue);
    
}




template <class T>
void radixoffset(T* hkey, T* tkey, const size_t n,const unsigned int iter)
{
        int i;
        #pragma omp parallel for\
                private(i)\
                shared(hkey,tkey)
        for(i=0;i<n;i++)
	{
                tkey[i]=!((hkey[i]&(1<<iter))>>iter);
	}
}

template <class T>
void rearrange(T** key, T** value, T** tkey, T** tvalue, T** tarr, const size_t n)
{

        unsigned int i;
        unsigned int totalfalses=(*tkey)[n-1]+(*tvalue)[n-1];

        #pragma omp parallel for\
                private(i)\
                shared(tkey,tvalue)
        for(i=0;i<n;i++)
        {
                unsigned int t;
                if((*tkey)[i]==0)
                        (*tkey)[i]=i-(*tvalue)[i]+totalfalses;
                else
                        (*tkey)[i]=(*tvalue)[i];
        }

        #pragma omp parallel for\
                private(i)\
                shared(tarr,tkey,tvalue,key,value)
        for(i=0;i<n;i++)
        {
                (*tarr)[(*tkey)[i]]=(*key)[i];
                (*tvalue)[(*tkey)[i]]=(*value)[i];
        }

        T* temp;
        temp= *key;
        *key= *tarr;
        *tarr= temp;

        temp= *value;
        *value= *tvalue;
        *tvalue=temp;
}

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
__declspec(target(MIC)) void  scanArray(T *input,  T* output, const size_t n)
{

	//######## Algorithm as described by NVIDIA tutorials #################################
	#if 0
	int d;
        unsigned k;

	#pragma omp parallel for\
		private(k)\
		shared(input,output)
        for(k=0;k<n;k+=2)
        {
		output[k+1]=input[k]+input[k+1];
                output[k]=input[k];
        }
        for(d=1;d<log2(n);d++)
        {
		#pragma omp parallel for\
	                private(k)\
        	        shared(input,output)
                for(k=(1<<d)-1;k<n;k=k+(1<<(d+1)))
                {
                        output[k+(1<<d)]=output[k]+output[k+(1<<d)];
                }
        }
        output[n-1]=0;
        for(d=log2(n)-1;d >= 0 ;d--)
        {
		#pragma omp parallel for\
	                private(k)\
        	        shared(input,output)
                for(k=(1<<d)-1;k<n;k=k+(1<<(d+1)))
                {
                        float t=output[k];
                        output[k]=output[k+(1<<d)];
                        output[k+(1<<d)]=t+output[k+(1<<d)];
                }
        }
	#endif
	
	int numblocks;
        numblocks=(int)ceil((double)n/BLOCK);
        float* ipblocksum=(float*)malloc(numblocks*sizeof(float));
        float* opblocksum=(float*)malloc(numblocks*sizeof(float));

        output[0]=0.0;

        #pragma omp parallel for\
                shared(input,output)
        for(int i=0;i<numblocks;i++)
        {
                int offset=i*BLOCK;
                int end=(i==numblocks-1)?(n-offset):BLOCK;
                int j;

                output[offset]=0.0;
                for(j=1;j<end;j++)
                {
                        output[offset+j]=output[offset+j-1]+input[offset+j-1];
                }
                ipblocksum[i]=output[offset+j-1]+input[offset+j-1];
        }

        opblocksum[0]=ipblocksum[0];
        for(int i=1;i<numblocks;i++)
        {
                opblocksum[i]=opblocksum[i-1]+ipblocksum[i];
        }

	#pragma omp parallel for\
                shared(output,opblocksum)
        for(int i=1;i<numblocks;i++)
        {

                int offset=i*BLOCK;
                int end=(i==numblocks-1)?(n-offset):BLOCK;
                float value=opblocksum[i-1];
                for(int j=0;j<end;j++)
                {
                        output[offset+j]=output[offset+j]+value;
                }
        }

        free(opblocksum);
        free(ipblocksum);
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
bool verifyResult(T *key,  T* val, const size_t size)
{
    bool passed = true;

    for (unsigned int i = 0; i < size-1; ++i)
    {
        if (key[i] > key[i+1])
        {
            passed = false;
        }
       if (val[i] > val[i+1])
        {
            passed = false;
        }


    }
    cout << "Test ";
    if (passed)
        cout << "Passed" << endl;
    else
        cout << "---FAILED---" << endl;
    return passed;
}
