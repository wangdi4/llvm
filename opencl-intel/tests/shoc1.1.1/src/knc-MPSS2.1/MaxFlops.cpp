//===========================================================================
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

/****************************************************************************
 *	Intel MIC(Many Integrated Core) version of SHOC MaxFlops program
 *
 *
 *	Zhi Ying(zhi.ying@intel.com),Jun Jin(jun.i.jin@intel.com)
 *
 *	Last modification: May 23, 2011
 *	
 *****************************************************************************/
#include <stdio.h>
#include <omp.h>
#include <math.h>
#include "Timer.h"
#include "ResultDatabase.h"
#include "OptionParser.h"
#include "ProgressBar.h"
#include "MaxFlops.h"

int micdev = 0;

template <class T>
void
RunTest(ResultDatabase &resultDB, int npasses, int verbose, int quiet, 
        float repeatF, ProgressBar &pb, const char* precision);

void addBenchmarkSpecOptions(OptionParser &op)
{  
}
template <class T>
void CheckResults(T* hostMem, int numFloats)
{
	int halfNumFloats = numFloats/2;
	for (int j=0 ; j<halfNumFloats; ++j)
	{
		if (hostMem[j] != hostMem[numFloats-j-1])
		{
			cout << "Error; hostMem[" << j << "]=" << hostMem[j]
				<< " is different from its twin element hostMem["
				<< (numFloats-j-1) << "]=" << hostMem[numFloats-j-1]
				<<"; stopping check\n";
			break;
		}
	}
}
template <class T>
void InitData(T *hostMem, int numFloats)
{
	int halfNumFloats = numFloats/2;
	srand((unsigned)time(NULL));
	for (int j=0; j<halfNumFloats; ++j)
	{
		hostMem[j] = hostMem[numFloats-j-1] = (T)((rand()/(float)RAND_MAX)*10.0);
	}
}

void RunBenchmark(  OptionParser &op, ResultDatabase &resultDB)
{
    bool verbose = op.getOptionBool("verbose");
    bool quiet = op.getOptionBool("quiet");
    const unsigned int passes = op.getOptionInt("passes");
    micdev = op.getOptionInt("target");


    // Thread configuration

#pragma offload target(mic)
{
	omp_set_num_threads(124);
	kmp_set_defaults("KMP_AFFINITY=explicit,proclist=[4-127:1],granularity=fine");
}

	double repeatF = 3;
    fprintf (stdout, "Adjust repeat factor = %lg\n", repeatF);

    // Initialize progress bar
    int totalRuns = 16*passes*2;
    ProgressBar pb(totalRuns);
    if (!verbose && !quiet)
       pb.Show(stdout);

    RunTest<float>(resultDB, passes, verbose, quiet,
             repeatF, pb, "-SP");
    
    RunTest<double>(resultDB, passes, verbose, quiet,
             repeatF, pb, "-DP");
			 
    if (!verbose)
        fprintf (stdout, "\n\n");
    
}
template <class T>
void WarmingUp(int threads)
{
	int warmRepeat = 1;
	T *testMem =  (T*)_mm_malloc(sizeof(T)*threads,64);
	InitData<T>(testMem,threads);
#pragma offload target(mic) in(testMem:length(threads))
	{
		Add1_MIC<T>(threads,testMem, warmRepeat, 10.0);
		Add2_MIC<T>(threads,testMem, warmRepeat, 10.0);
		Add4_MIC<T>(threads,testMem, warmRepeat, 10.0);
		Add8_MIC<T>(threads,testMem, warmRepeat, 10.0);
		Mul1_MIC<T>(threads,testMem, warmRepeat, 1.01);
		Mul2_MIC<T>(threads,testMem, warmRepeat, 1.01);
		Mul4_MIC<T>(threads,testMem, warmRepeat, 1.01);
		Mul8_MIC<T>(threads,testMem, warmRepeat, 1.01);
		MAdd1_MIC<T>(threads,testMem, warmRepeat, 10.0, 0.9899);
		MAdd2_MIC<T>(threads,testMem, warmRepeat, 10.0, 0.9899);
		MAdd4_MIC<T>(threads,testMem, warmRepeat, 10.0, 0.9899);
		MAdd8_MIC<T>(threads,testMem, warmRepeat, 10.0, 0.9899);
		MulMAdd1_MIC<T>(threads,testMem, warmRepeat, 3.75, 0.355);
		MulMAdd2_MIC<T>(threads,testMem, warmRepeat, 3.75, 0.355);
		MulMAdd4_MIC<T>(threads,testMem, warmRepeat, 3.75, 0.355);
		MulMAdd8_MIC<T>(threads,testMem, warmRepeat, 3.75, 0.355);
	}
	_mm_free(testMem);
}
template <class T>
void
RunTest(ResultDatabase &resultDB,
        int npasses,
        int verbose,
        int quiet,
        float repeatF,
        ProgressBar &pb,
        const char* precision)
{
    char sizeStr[128];
    static __declspec(target(mic)) T *hostMem;

    int realRepeats = (int)round(repeatF*20);
    if (realRepeats < 2)
       realRepeats = 2;
    
    // Alloc host memory
    //int halfNumFloats = 1024*124*32;
    int halfNumFloats = 1024*1024;
    int numFloats = 2*halfNumFloats;
	hostMem = (T*)_mm_malloc(sizeof(T)*numFloats,64);

    sprintf (sizeStr, "Size:%07d", numFloats);
    float t = 0.0f;
	double TH;
	double flopCount;
	double gflop;
	
	///Warming up
	WarmingUp<T>(124*16);
	
    for (int pass=0 ; pass<npasses ; ++pass)
	{
		////////// Add1 //////////
		InitData<T>(hostMem,numFloats);
#pragma offload target(mic) in(hostMem:length(numFloats) free_if(0))
		{}
		TH = curr_second();
#pragma offload target(mic) in(numFloats,realRepeats) nocopy(hostMem)
		{
			Add1_MIC<T>(numFloats,hostMem, realRepeats, 10.0);
		}
		t = curr_second()-TH;
		flopCount = (double)numFloats * 1 * realRepeats * 240 * 1;
		gflop = flopCount / (double)(t*1e9);
		resultDB.AddResult(string("Add1")+precision, sizeStr, "GFLOPS", gflop);
#pragma offload target(mic) out(hostMem:length(numFloats) alloc_if(0))
		{}
		CheckResults<T>(hostMem,numFloats);
		pb.addItersDone();
		if (!verbose && !quiet)pb.Show(stdout);

		////////// Add2 //////////
		InitData<T>(hostMem,numFloats);
#pragma offload target(mic) in(hostMem:length(numFloats) free_if(0))
		{}
		TH = curr_second();
#pragma offload target(mic) in(numFloats,realRepeats) nocopy(hostMem)
		{
			Add2_MIC<T>(numFloats,hostMem, realRepeats, 10.0);
		}
		t = curr_second()-TH;
		flopCount = (double)numFloats * 1 * realRepeats * 120 * 2;
		gflop = flopCount / (double)(t*1e9);
		resultDB.AddResult(string("Add2")+precision, sizeStr, "GFLOPS", gflop);
#pragma offload target(mic) out(hostMem:length(numFloats) alloc_if(0))
		{}
		CheckResults<T>(hostMem,numFloats);
		pb.addItersDone();
		if (!verbose && !quiet)pb.Show(stdout);

		////////// Add4 //////////
		InitData<T>(hostMem,numFloats);
#pragma offload target(mic) in(hostMem:length(numFloats) free_if(0))
		{}
		TH = curr_second();
#pragma offload target(mic) in(numFloats,realRepeats) nocopy(hostMem)
		{
			Add4_MIC<T>(numFloats,hostMem, realRepeats, 10.0);
		}
		t = curr_second()-TH;
		flopCount = (double)numFloats * 1 * realRepeats * 60 * 4;
		gflop = flopCount / (double)(t*1e9);
		resultDB.AddResult(string("Add4")+precision, sizeStr, "GFLOPS", gflop);
#pragma offload target(mic) out(hostMem:length(numFloats) alloc_if(0))
		{}
		CheckResults<T>(hostMem,numFloats);
		pb.addItersDone();
		if (!verbose && !quiet)pb.Show(stdout);
	
		////////// Add8 //////////
		InitData<T>(hostMem,numFloats);
#pragma offload target(mic) in(hostMem:length(numFloats) free_if(0))
		{}
		TH = curr_second();
#pragma offload target(mic) in(numFloats,realRepeats) nocopy(hostMem)
		{
			Add8_MIC<T>(numFloats,hostMem, realRepeats, 10.0);
		}
		t = curr_second()-TH;
		flopCount = (double)numFloats * 1 * realRepeats * 80 * 3;
		gflop = flopCount / (double)(t*1e9);
		resultDB.AddResult(string("Add8")+precision, sizeStr, "GFLOPS", gflop);
#pragma offload target(mic) out(hostMem:length(numFloats) alloc_if(0))
		{}
		CheckResults<T>(hostMem,numFloats);
		pb.addItersDone();
		if (!verbose && !quiet)pb.Show(stdout);
		
		////////// Mul1 //////////
		InitData<T>(hostMem,numFloats);
#pragma offload target(mic) in(hostMem:length(numFloats) free_if(0))
		{}
		TH = curr_second();
#pragma offload target(mic) in(numFloats,realRepeats) nocopy(hostMem)
		{
			Mul1_MIC<T>(numFloats,hostMem, realRepeats, 1.01);
		}
		t = curr_second()-TH;
		flopCount = (double)numFloats * 2 * realRepeats * 200 * 1;
		gflop = flopCount / (double)(t*1e9);
		resultDB.AddResult(string("Mul1")+precision, sizeStr, "GFLOPS", gflop);
#pragma offload target(mic) out(hostMem:length(numFloats) alloc_if(0))
		{}
		CheckResults<T>(hostMem,numFloats);
		pb.addItersDone();
		if (!verbose && !quiet)pb.Show(stdout);

		////////// Mul2 //////////
		InitData<T>(hostMem,numFloats);
#pragma offload target(mic) in(hostMem:length(numFloats) free_if(0))
		{}
		TH = curr_second();
#pragma offload target(mic) in(numFloats,realRepeats) nocopy(hostMem)
		{
			Mul2_MIC<T>(numFloats,hostMem, realRepeats, 1.01);
		}
		t = curr_second()-TH;
		flopCount = (double)numFloats * 2 * realRepeats * 100 * 2;
		gflop = flopCount / (double)(t*1e9);
		resultDB.AddResult(string("Mul2")+precision, sizeStr, "GFLOPS", gflop);
#pragma offload target(mic) out(hostMem:length(numFloats) alloc_if(0))
		{}
		CheckResults<T>(hostMem,numFloats);
		pb.addItersDone();
		if (!verbose && !quiet)pb.Show(stdout);
	
		////////// Mul4 //////////
		InitData<T>(hostMem,numFloats);
#pragma offload target(mic) in(hostMem:length(numFloats) free_if(0))
		{}
		TH = curr_second();
#pragma offload target(mic) in(numFloats,realRepeats) nocopy(hostMem)
		{
			Mul4_MIC<T>(numFloats,hostMem, realRepeats, 1.01);
		}
		t = curr_second()-TH;
		flopCount = (double)numFloats * 2 * realRepeats * 50 * 4;
		gflop = flopCount / (double)(t*1e9);
		resultDB.AddResult(string("Mul4")+precision, sizeStr, "GFLOPS", gflop);
#pragma offload target(mic) out(hostMem:length(numFloats) alloc_if(0))
		{}
		CheckResults<T>(hostMem,numFloats);
		pb.addItersDone();
		if (!verbose && !quiet)pb.Show(stdout);
			
		////////// Mul8 //////////
		InitData<T>(hostMem,numFloats);
#pragma offload target(mic) in(hostMem:length(numFloats) free_if(0))
		{}
		TH = curr_second();
#pragma offload target(mic) in(numFloats,realRepeats) nocopy(hostMem)
		{
			Mul8_MIC<T>(numFloats,hostMem, realRepeats, 1.01);
		}
		t = curr_second()-TH;
		flopCount = (double)numFloats * 2 * realRepeats * 25 * 8;
		gflop = flopCount / (double)(t*1e9);
		resultDB.AddResult(string("Mul8")+precision, sizeStr, "GFLOPS", gflop);
#pragma offload target(mic) out(hostMem:length(numFloats) alloc_if(0))
		{}
		CheckResults<T>(hostMem,numFloats);
		pb.addItersDone();
		if (!verbose && !quiet)pb.Show(stdout);
		
		////////// MAdd1 //////////
		InitData<T>(hostMem,numFloats);
#pragma offload target(mic) in(hostMem:length(numFloats) free_if(0))
		{}
		TH = curr_second();
#pragma offload target(mic) in(numFloats,realRepeats) nocopy(hostMem)
		{
			MAdd1_MIC<T>(numFloats,hostMem, realRepeats, 10.0, 0.9899);
		}
		t = curr_second()-TH;
		flopCount = (double)numFloats * 2 * realRepeats * 240 * 1;
		gflop = flopCount / (double)(t*1e9);
		resultDB.AddResult(string("MAdd1")+precision, sizeStr, "GFLOPS", gflop);
#pragma offload target(mic) out(hostMem:length(numFloats) alloc_if(0))
		{}
		CheckResults<T>(hostMem,numFloats);
		pb.addItersDone();
		if (!verbose && !quiet)pb.Show(stdout);
		
		////////// MAdd2 //////////
		InitData<T>(hostMem,numFloats);
#pragma offload target(mic) in(hostMem:length(numFloats) free_if(0))
		{}
		TH = curr_second();
#pragma offload target(mic) in(numFloats,realRepeats) nocopy(hostMem)
		{
			MAdd2_MIC<T>(numFloats,hostMem, realRepeats, 10.0, 0.9899);
		}
		t = curr_second()-TH;
		flopCount = (double)numFloats * 2 * realRepeats * 120 * 2;
		gflop = flopCount / (double)(t*1e9);
		resultDB.AddResult(string("MAdd2")+precision, sizeStr, "GFLOPS", gflop);
#pragma offload target(mic) out(hostMem:length(numFloats) alloc_if(0))
		{}
		CheckResults<T>(hostMem,numFloats);
		pb.addItersDone();
		if (!verbose && !quiet)pb.Show(stdout);
		
		////////// MAdd4 //////////
		InitData<T>(hostMem,numFloats);
#pragma offload target(mic) in(hostMem:length(numFloats) free_if(0))
		{}
		TH = curr_second();
#pragma offload target(mic) in(numFloats,realRepeats) nocopy(hostMem)
		{
			MAdd4_MIC<T>(numFloats,hostMem, realRepeats, 10.0, 0.9899);
		}
		t = curr_second()-TH;
		flopCount = (double)numFloats * 2 * realRepeats * 60 * 4;
		gflop = flopCount / (double)(t*1e9);
		resultDB.AddResult(string("MAdd4")+precision, sizeStr, "GFLOPS", gflop);
#pragma offload target(mic) out(hostMem:length(numFloats) alloc_if(0))
		{}
		CheckResults<T>(hostMem,numFloats);
		pb.addItersDone();
		if (!verbose && !quiet)pb.Show(stdout);
		
		////////// MAdd8 //////////
		InitData<T>(hostMem,numFloats);
#pragma offload target(mic) in(hostMem:length(numFloats) free_if(0))
		{}
		TH = curr_second();
#pragma offload target(mic) in(numFloats,realRepeats) nocopy(hostMem)
		{
			MAdd8_MIC<T>(numFloats,hostMem, realRepeats, 10.0, 0.9899);
		}
		t = curr_second()-TH;
		flopCount = (double)numFloats * 2 * realRepeats * 30 * 8;
		gflop = flopCount / (double)(t*1e9);
		resultDB.AddResult(string("MAdd8")+precision, sizeStr, "GFLOPS", gflop);
#pragma offload target(mic) out(hostMem:length(numFloats) alloc_if(0))
		{}
		CheckResults<T>(hostMem,numFloats);
		pb.addItersDone();
		if (!verbose && !quiet)pb.Show(stdout);

		////////// MulMAdd1 //////////
		InitData<T>(hostMem,numFloats);
#pragma offload target(mic) in(hostMem:length(numFloats) free_if(0))
		{}
		TH = curr_second();
#pragma offload target(mic) in(numFloats,realRepeats) nocopy(hostMem)
		{
			MulMAdd1_MIC<T>(numFloats,hostMem, realRepeats, 3.75, 0.355);
		}
		t = curr_second()-TH;
		flopCount = (double)numFloats * 3 * realRepeats * 160 * 1;
		gflop = flopCount / (double)(t*1e9);
		resultDB.AddResult(string("MulMAdd1")+precision, sizeStr, "GFLOPS", gflop);
#pragma offload target(mic) out(hostMem:length(numFloats) alloc_if(0))
		{}
		CheckResults<T>(hostMem,numFloats);
		pb.addItersDone();
		if (!verbose && !quiet)pb.Show(stdout);
		
		////////// MulMAdd2 //////////
		InitData<T>(hostMem,numFloats);
#pragma offload target(mic) in(hostMem:length(numFloats) free_if(0))
		{}
		TH = curr_second();
#pragma offload target(mic) in(numFloats,realRepeats) nocopy(hostMem)
		{
			MulMAdd2_MIC<T>(numFloats,hostMem, realRepeats, 3.75, 0.355);
		}
		t = curr_second()-TH;
		flopCount = (double)numFloats * 3 * realRepeats * 80 * 2;
		gflop = flopCount / (double)(t*1e9);
		resultDB.AddResult(string("MulMAdd2")+precision, sizeStr, "GFLOPS", gflop);
#pragma offload target(mic) out(hostMem:length(numFloats) alloc_if(0))
		{}
		CheckResults<T>(hostMem,numFloats);
		pb.addItersDone();
		if (!verbose && !quiet)pb.Show(stdout);
		
		////////// MulMAdd4 //////////
		InitData<T>(hostMem,numFloats);
#pragma offload target(mic) in(hostMem:length(numFloats) free_if(0))
		{}
		TH = curr_second();
#pragma offload target(mic) in(numFloats,realRepeats) nocopy(hostMem)
		{
			MulMAdd4_MIC<T>(numFloats,hostMem, realRepeats, 3.75, 0.355);
		}
		t = curr_second()-TH;
		flopCount = (double)numFloats * 3 * realRepeats * 40 * 4;
		gflop = flopCount / (double)(t*1e9);
		resultDB.AddResult(string("MulMAdd4")+precision, sizeStr, "GFLOPS", gflop);
#pragma offload target(mic) out(hostMem:length(numFloats) alloc_if(0))
		{}
		CheckResults<T>(hostMem,numFloats);
		pb.addItersDone();
		if (!verbose && !quiet)pb.Show(stdout);
		
		////////// MulMAdd8 //////////
		InitData<T>(hostMem,numFloats);
#pragma offload target(mic) in(hostMem:length(numFloats) free_if(0))
		{}
		TH = curr_second();
#pragma offload target(mic) in(numFloats,realRepeats) nocopy(hostMem)
		{
			MulMAdd8_MIC<T>(numFloats,hostMem, realRepeats, 3.75, 0.355);
		}
		t = curr_second()-TH;
		flopCount = (double)numFloats * 3 * realRepeats * 20 * 8;
		gflop = flopCount / (double)(t*1e9);
		resultDB.AddResult(string("MulMAdd8")+precision, sizeStr, "GFLOPS", gflop);
#pragma offload target(mic) out(hostMem:length(numFloats) alloc_if(0))
		{}
		CheckResults<T>(hostMem,numFloats);
		pb.addItersDone();
		if (!verbose && !quiet)pb.Show(stdout);
	}

    _mm_free(hostMem);
}


