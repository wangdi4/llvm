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

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "OptionParser.h"
#include "ResultDatabase.h"
#include "Timer.h"


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
// Creation: December 15, 2009
//
// Modifications:
//
// ****************************************************************************
void addBenchmarkSpecOptions(OptionParser &op)
{
    ;
}

// ****************************************************************************
// Function: triad
//
// Purpose:
//   A simple vector addition kernel
//   C = A + s*B
//
// Arguments:
//   A,B - input vectors
//   C - output vectors
//   s - scalar
//
// Returns:  nothing
//
// Programmer: Kyle Spafford
// Creation: December 15, 2009
//
// Modifications:
//
// ****************************************************************************
__declspec(target(MIC)) void Triad( float * A,   float* B,   float* C, float s, int length)
{
   #pragma omp parallel for
   #pragma ivdep
    for(int idx=0; idx<length; idx++){
       C[idx] = A[idx] + s*B[idx];
    }
}

#define ALIGNMENT 4096
__declspec(target(MIC)) float  *A0, *B0, *C0,*A1, *B1, *C1;


// ****************************************************************************
// Function: RunBenchmark
//
// Purpose:
//   Implements the Stream Triad benchmark in CUDA.  This benchmark
//   is designed to test CUDA's overall data transfer speed. It executes
//   a vector addition operation with no temporal reuse. Data is read
//   directly from the global memory. This implementation tiles the input
//   array and pipelines the vector addition computation with
//   the data download for the next tile. However, since data transfer from
//   host to device is much more expensive than the simple vector computation,
//   data transfer operations should completely dominate the execution time.
//
// Arguments:
//   resultDB: results from the benchmark are stored in this db
//   op: the options parser (contains input parameters)
//
// Returns:  nothing
//
// Programmer: Kyle Spafford
// Creation: December 15, 2009
//
// Modifications:
//
// ****************************************************************************
void RunBenchmark( OptionParser &op,ResultDatabase &resultDB)
{
    const bool verbose = op.getOptionBool("verbose");
    const int n_passes = op.getOptionInt("passes");
    int micdev = op.getOptionInt("target");


    // 256k through 8M bytes
    const int nSizes = 9;
    const size_t blockSizes[] = { 64, 128, 256, 512, 1024, 2048, 4096, 8192,
            16384 };
    const size_t memSize =  blockSizes[nSizes - 1];

    const size_t numMaxFloats = 1024 * memSize / 4;
    const size_t halfNumFloats = numMaxFloats / 2;

    // Create some host memory pattern
    __declspec(target(MIC)) float *h_mem;
    h_mem = (float *) _mm_malloc(sizeof(float)*numMaxFloats,ALIGNMENT);

   // Allocate some  memory
    A0 =  (float *)_mm_malloc( blockSizes[nSizes - 1] * 1024, ALIGNMENT);
    B0 =  (float *)_mm_malloc( blockSizes[nSizes - 1] * 1024, ALIGNMENT);
    C0 =  (float *)_mm_malloc( blockSizes[nSizes - 1] * 1024, ALIGNMENT);

    A1 =  ( float *)_mm_malloc( blockSizes[nSizes - 1] * 1024, ALIGNMENT);
    B1 =  ( float *)_mm_malloc( blockSizes[nSizes - 1] * 1024, ALIGNMENT);
    C1 =  ( float *)_mm_malloc( blockSizes[nSizes - 1] * 1024, ALIGNMENT);

//initialize / allocate data on the card
    #pragma offload target(MIC:micdev) \
                in(A0:length(numMaxFloats) free_if(0) ) \
                in(B0:length(numMaxFloats) free_if(0) ) \
                inout(C0:length(numMaxFloats) free_if(0) )
    {

    }

    #pragma offload target(MIC:micdev) \
                in(A1:length(numMaxFloats) free_if(0) ) \
                in(B1:length(numMaxFloats) free_if(0) ) \
                inout(C1:length(numMaxFloats) free_if(0) )
    {
  
    }



    float scalar = 1.75f;

//    const size_t blockSize = 128;

    // Number of passes. Use a large number for stress testing.
    // A small value is sufficient for computing sustained performance.
    char sizeStr[256];
    for (int pass = 0; pass < n_passes; ++pass)
    {
        // Step through sizes forward
        for (int i = 0; i < nSizes; ++i)
        {
            int elemsInBlock = blockSizes[i] * 1024 / sizeof(float);
            for (int j = 0; j < halfNumFloats; ++j)
                h_mem[j] = h_mem[halfNumFloats + j]
                                 = (float) (drand48() * 10.0);

            // Copy input memory to the device
            if (verbose)
                cout << ">> Executing Triad with vectors of length "
                << numMaxFloats << " and block size of "
                << elemsInBlock << " elements." << "\n";
            sprintf(sizeStr, "Block:%05ldKB", blockSizes[i]);

// start submitting blocks of data of size elemsInBlock
            // overlap the computation of one block with the data
            // download for the next block and the results upload for
            // the previous block
            int crtIdx = 0;


           double startTime = curr_second();

            memcpy(A0, (void const*) h_mem, blockSizes[i] * 1024);
            memcpy(B0, (void const*) h_mem, blockSizes[i] * 1024);


 	    #pragma offload target(MIC:micdev) \
                in(A0:length(elemsInBlock) free_if(0) alloc_if(0) ) \
                in(B0:length(elemsInBlock) free_if(0) alloc_if(0)) \
                inout(C0:length(elemsInBlock) free_if(0) alloc_if(0) )
            {
                            Triad(A0, B0, C0, scalar, elemsInBlock );
            }



            if (elemsInBlock < numMaxFloats)
            {
                // start downloading data for next block
                memcpy(A1, (void const*)(h_mem+elemsInBlock), blockSizes[i] * 1024);
                memcpy(B1, (void const*)(h_mem+elemsInBlock), blockSizes[i] * 1024);
            }

            int blockIdx = 1;
            unsigned int currStream = 1;
            while (crtIdx < numMaxFloats)
            {
                currStream = blockIdx & 1;
                // Start copying back the answer from the last kernel
                if (currStream)
                {
                    memcpy(h_mem + crtIdx, (void const*)C0, elemsInBlock
                        * sizeof(float));
                }
                else
                {
                    memcpy(h_mem + crtIdx, (void const*)C1, elemsInBlock
                        * sizeof(float));
                }

                crtIdx += elemsInBlock;

                if (crtIdx < numMaxFloats)
                {
                    // Execute the kernel
                    if (currStream)
                    {
		             #pragma offload target(MIC:micdev) \
                		in(A1:length(elemsInBlock) alloc_if(0) free_if(0) ) \
                		in(B1:length(elemsInBlock) alloc_if(0) free_if(0) ) \
                		inout(C1:length(elemsInBlock) alloc_if(0) free_if(0) )
            			{
                        		Triad(A1, B1, C1,  scalar, elemsInBlock);
				}
                    }
                    else
                    {



                             #pragma offload target(MIC:micdev) \
                                in(A0:length(elemsInBlock) alloc_if(0) free_if(0) ) \
                                in(B0:length(elemsInBlock) alloc_if(0) free_if(0) ) \
                                inout(C0:length(elemsInBlock) alloc_if(0) free_if(0) )
                                {
                                        Triad(A0, B0, C0,  scalar, elemsInBlock);
                                }
   

                    }
                }
                if (crtIdx+elemsInBlock < numMaxFloats)
                {
                    // Download data for next block
                    if (currStream)
                    {
                        memcpy(A0, (void const*)(h_mem+crtIdx+elemsInBlock),
                                blockSizes[i]*1024);
                        memcpy(B0, (void const*)(h_mem+crtIdx+elemsInBlock),
                                blockSizes[i]*1024);
                    }
                    else
                    {

                        memcpy(A1, (void const*)(h_mem+crtIdx+elemsInBlock),
                                blockSizes[i]*1024);
                        memcpy(B1, (void const*)(h_mem+crtIdx+elemsInBlock),
                                blockSizes[i]*1024);
 
                    }
                }
                blockIdx += 1;
                currStream = !currStream;
            }

            double time = curr_second()-startTime;

            double triadFlops = ((double)numMaxFloats * 2.0) / (time*1e9);
            resultDB.AddResult("TriadFlops", sizeStr, "GFLOP/s", triadFlops);

            double bdwth = ((double)numMaxFloats*sizeof(float)*3.0)
                / (time*1000.*1000.*1000.);
            resultDB.AddResult("TriadBdwth", sizeStr, "GB/s", bdwth);

            // Checking memory for correctness. The two halves of the array
            // should have the same results.
            if (verbose) cout << ">> checking memory\n";
            for (int j=0; j<halfNumFloats; ++j)
            {
                if (h_mem[j] != h_mem[j+halfNumFloats])
                {
                    cout << "Error; hostMem[" << j << "]=" << h_mem[j]
                         << " is different from its twin element hostMem["
                         << (j+halfNumFloats) << "]: "
                         << h_mem[j+halfNumFloats] << "stopping check\n";
                    break;
                }
            }
            if (verbose) cout << ">> finish!" << endl;

            // Zero out the test host memory
            for (int j=0; j<numMaxFloats; ++j)
                h_mem[j] = 0.0f;
        }
    }

// free the data allocated on the card
    #pragma offload target(MIC:micdev) \
                in(A0:length(numMaxFloats) alloc_if(0) ) \
                in(B0:length(numMaxFloats) alloc_if(0) ) \
                inout(C0:length(numMaxFloats) alloc_if(0) )
    {

    }

    #pragma offload target(MIC:micdev) \
                in(A1:length(numMaxFloats) alloc_if(0) ) \
                in(B1:length(numMaxFloats) alloc_if(0) ) \
                inout(C1:length(numMaxFloats) alloc_if(0) )
    {

    }

    // Cleanup
_mm_free(h_mem);

    _mm_free(A0);
    _mm_free(B0);
    _mm_free(C0);


    _mm_free(A1);
    _mm_free(B1);
    _mm_free(C1);

}




