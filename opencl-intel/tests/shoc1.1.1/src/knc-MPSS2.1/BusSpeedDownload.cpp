
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


#include "OptionParser.h"
#include "ResultDatabase.h"
#include "Timer.h"

// ****************************************************************************
// Function: addBenchmarkSpecOptions
//
// Purpose:
//   Add benchmark specific command line argument parsing.
//
//   -nopinned
//   This option controls whether page-locked or "pinned" memory is used.
//   The use of pinned memory typically results in higher bandwidth for data
//   transfer between host and device.
//
// Arguments:
//   op: the options parser / parameter database
//
// Returns:  nothing
//
// Programmer: Jeremy Meredith
// Creation: September 08, 2009
//
// Modifications:
//
// ****************************************************************************
void addBenchmarkSpecOptions(OptionParser &op)
{
    op.addOption("nopinned", OPT_BOOL, "",
                 "disable usage of pinned (pagelocked) memory", 'p');
}

// ****************************************************************************
// Function: runBenchmark
//
// Purpose:
//   Measures the bandwidth of the bus connecting the host processor to the
//   OpenCL device.  This benchmark repeatedly transfers data chunks of various
//   sizes across the bus to the OpenCL device, and calculates the bandwidth.
//
//
// Arguments:
//  resultDB: the benchmark stores its results in this ResultDatabase
//  op: the options parser / parameter database
//
// Returns:  nothing
//
// Programmer: Jeremy Meredith
// Creation: September 08, 2009
//
// Modifications:
//
// ****************************************************************************
//#define VALIDATE_DATA

#define ALIGN (2*1024*1024)

__declspec(target(MIC)) float *hostMem=NULL;


void RunBenchmark(OptionParser &op, ResultDatabase &resultDB)
{
    const bool verbose = op.getOptionBool("verbose");
    const bool pinned = !op.getOptionBool("nopinned");

    // Sizes are in kb
    const int nSizes  = 17;
    int sizes[nSizes] = {1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,
           32768, 65536};
    long long numMaxFloats = 1024 * (sizes[nSizes-1]) / 4;

    // Create some host memory pattern
    //
    hostMem = (float*)_mm_malloc(numMaxFloats*sizeof(float),ALIGN);


    if( hostMem==NULL ){
      printf("couldn't allocate CPU memory hostMem=%p \n",hostMem);
      printf("  TEST FAILED!\n"); fflush(NULL);
      return;
    }



    for (int i = 0; i < numMaxFloats; i++)
    {
        hostMem[i] = i % 77;
    }

    const unsigned int passes = op.getOptionInt("passes");
    int micdev = op.getOptionInt("target");


    // Three passes, forward and backward both
    for (int pass = 0; pass < passes; pass++)
    {
        // Step through sizes forward on even passes and backward on odd
        for (int i = 0; i < nSizes; i++)
        {
            int sizeIndex;
            if ((pass % 2) == 0)
                sizeIndex = i;
            else
                sizeIndex = (nSizes - 1) - i;

            int nbytes = sizes[sizeIndex] * 1024;

	//allocate memory on the card
	    #pragma offload target(mic:micdev) \
        	in(hostMem:length(1024*sizes[sizeIndex]/4) free_if(0) align(ALIGN) ) 
        	{
        	}

            double start = curr_second();
		//Actual transferring data from host to card
	    #pragma offload target(mic:micdev) \
        	in(hostMem:length((1024*sizes[sizeIndex]/4)) free_if(0) alloc_if(0)  )
        	{
        	}

            double t = curr_second()-start;


            // Convert to GB/sec
            if (verbose)
            {
                cerr << "size " << sizes[sizeIndex] << "k took " << t <<
                        " sec\n";
            }
            double speed = (double(sizes[sizeIndex]) / (1024. * 1024.)) / t;
            char sizeStr[256];
            sprintf(sizeStr, "% 6dkB", sizes[sizeIndex]);
            resultDB.AddResult("DownloadSpeed", sizeStr, "GiB/sec", speed);
	    resultDB.AddResult("DownloadTime", sizeStr, "ms", t);


// free memory allocated on the board
	    #pragma offload target(mic:micdev) \
               	 in(hostMem:length((1024*sizes[sizeIndex]/4)) alloc_if(0)  )
                 {
                 }

         }
    }

    // Cleanup
   //    delete[] hostMem;
    _mm_free(hostMem);
}
