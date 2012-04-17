// This example from a prerelease of the Scalable HeterOgeneous Computing
// (SHOC) Benchmark Suite Alpha v1.1.1i for Intel MIC architecture
// Contact: Kyle Spafford <kys@ornl.gov>
//         Rezaur Rahman <rezaur.rahman@intel.com>
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
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Timer.h"

#include "OptionParser.h"
#include "ResultDatabase.h"

#include "fftlib.h"

using namespace std;

template <class T2>
void runtest(const string& name,
	ResultDatabase &resultDb, OptionParser &op);
template <class T2>
void dump(OptionParser& op);

void 
addBenchmarkSpecOptions(OptionParser &op) 
{
    op.addOption("MB", OPT_INT, "0", "data size (in megabytes)");
    op.addOption("dump-sp", OPT_BOOL, "false", "dump result after SP fft/ifft");
    op.addOption("dump-dp", OPT_BOOL, "false", "dump result after DP fft/ifft");
}

static void
fillResultDB(const string& name, const string& reason, OptionParser &op, 
             ResultDatabase& resultDB)
{
    // resultDB requires neg entry for every possible result
    int passes = op.getOptionInt("passes");
    for (int k=0; k<passes; k++) {
        resultDB.AddResult(name , reason, "GB/s", FLT_MAX);
        resultDB.AddResult(name+"_PCIe" , reason, "GB/s", FLT_MAX);
        resultDB.AddResult(name+"_Parity" , reason, "GB/s", FLT_MAX);
        resultDB.AddResult(name+"-INV" , reason, "GB/s", FLT_MAX);
        resultDB.AddResult(name+"-INV_PCIe" , reason, "GB/s", FLT_MAX);
        resultDB.AddResult(name+"-INV_Parity" , reason, "GB/s", FLT_MAX);
    }
}

template <class T2> inline bool dp(void);
template <> inline bool dp<cplxflt>(void) { return false; }
template <> inline bool dp<cplxdbl>(void) { return true; }

template <class T2>
void runtest(const string& name, ResultDatabase &resultDB, OptionParser &op)
{
	int i;

	static __declspec(target(mic)) T2 *source;	
	int chk;
	unsigned long bytes = 0;

	if (op.getOptionInt("MB") == 0) {
        int probSizes[4] = { 1, 8, 96, 256 };
	int sizeIndex = op.getOptionInt("size")-1;
	if (sizeIndex < 0 || sizeIndex >= 4) {
	    cerr << "Invalid size index specified\n";
	    exit(-1);
	}
        bytes = probSizes[sizeIndex];
    } else {
        bytes = op.getOptionInt("MB");
    }
    // Convert to MB
    bytes *= 1024 * 1024;

    int passes = op.getOptionInt("passes");

	int fftsz = 512;	//The size of the transform computed is fixed at 512 complex elements

	int N = (bytes)/sizeof(T2);

	int half_n_cmplx = N/2;

	int n_ffts = N/fftsz;

	int half_n_ffts = n_ffts/2;

	//allocate space
	source = (T2*)MKL_malloc(bytes,  4096);

	//init test data
	srand((unsigned)time(NULL));
	for(i = 0; i < half_n_cmplx; i++){
		source[i].x = (rand()/(float)RAND_MAX)*2-1;
        source[i].y = (rand()/(float)RAND_MAX)*2-1;
        source[i+half_n_cmplx].x = source[i].x;
        source[i+half_n_cmplx].y = source[i].y;
	}

	//warm up
#pragma offload target(mic) in(fftsz)
{}
	//measure
	double pcie_TH = curr_second();
#pragma offload target(mic) in(source:length(N) align(4096) free_if(0))
{}
        double transfer_time = curr_second()- pcie_TH;
	
	const char *sizeStr;
	stringstream ss;
	ss << "N=" << (long)N;
	sizeStr = strdup(ss.str().c_str());
	
	for(int k = 0; k < passes; k++){
		//time fft
		double TH, t; 
#pragma offload target(mic) in(fftsz, n_ffts) nocopy(source)		
{
        TH  = curr_second(); 
		forward(source, fftsz, n_ffts);
        t = curr_second()-TH;
}		
		double Gflops = n_ffts*(5*fftsz*log2((float)fftsz))/(t*1e9f);//batch * N*log2(N) / time
		double gflopsPCIe = n_ffts*(5*fftsz*log2(fftsz)) /
                ((transfer_time+t)*1e9f);
		resultDB.AddResult(name, sizeStr, "GFLOPS", Gflops);
		resultDB.AddResult(name+"_PCIe", sizeStr, "GFLOPS", gflopsPCIe);
        resultDB.AddResult(name+"_Parity", sizeStr, "N", transfer_time / t);
		
		//time ifft
		TH = curr_second();
#pragma offload target(mic) in(fftsz, n_ffts) nocopy(source)		
{
         TH  = curr_second();		
		inverse(source, fftsz, n_ffts);
         t = curr_second()-TH;
}	
		Gflops = n_ffts*(5*fftsz*log2((float)fftsz))/(t*1e9f);//batch * N*log2(N) / time
		gflopsPCIe = n_ffts*(5*fftsz*log2(fftsz)) /
                ((transfer_time+t)*1e9f);
		resultDB.AddResult(name+"-INV", sizeStr, "GFLOPS", Gflops);
		resultDB.AddResult(name+"-INV_PCIe", sizeStr, "GFLOPS", gflopsPCIe);
        resultDB.AddResult(name+"-INV_Parity", sizeStr, "N", transfer_time / t);

		//check result
#pragma offload target(mic) in(half_n_cmplx) nocopy(source) out(chk)
{
		chk = checkDiff(source, half_n_cmplx);
}
		cout << "pass " << k << ((chk) ? ": failed\n" : ": passed\n");
	}
#pragma offload target(mic) out(source:length(N) alloc_if(0))
{}
	//free space
	//free(source);
    MKL_free(source);
}

void
RunBenchmark(OptionParser &op, ResultDatabase &resultDB)
{
/* I just comented the following part
#pragma offload target(mic)
{
	//omp_set_num_threads(62);
	//mkl_set_num_threads(2);
	//kmp_set_defaults("KMP_AFFINITY=explicit,proclist=[4-127:2],granularity=fine");
	kmp_set_defaults("MKL_DYNAMIC=false,KMP_AFFINITY=scatter,--MB 256");
}
*/
	if (op.getOptionBool("dump-sp")) {
        dump<cplxflt>(op);
    }
    else if (op.getOptionBool("dump-dp")) {
        dump<cplxdbl>(op);
    }
	else{
		//Single precision test
		runtest<cplxflt>("SP-FFT", resultDB, op);
		//Double precision test
		runtest<cplxdbl>("DP-FFT",resultDB, op);
	}
}

template <class T2>
void dump(OptionParser &op)
{
	int i;
	static __declspec(target(mic)) T2 *source;
	int chk;

	unsigned long bytes = 0;

	if (op.getOptionInt("MB") == 0) {
        int probSizes[4] = { 1, 8, 96, 256 };
	int sizeIndex = op.getOptionInt("size")-1;
	if (sizeIndex < 0 || sizeIndex >= 4) {
	    cerr << "Invalid size index specified\n";
	    exit(-1);
	}
        bytes = probSizes[sizeIndex];
	}
	else {
		bytes = op.getOptionInt("MB");
	}
	// Convert to MB
	bytes *= 1024 * 1024;

	int passes = op.getOptionInt("passes");

	int fftsz = 512;	//The size of the transform computed is fixed at 512 complex elements

	int N = (bytes)/sizeof(T2);

	int half_n_cmplx = N/2;

	int n_ffts = N/fftsz;

	int half_n_ffts = n_ffts/2;

	//allocate space
	source = (T2*)malloc(bytes);

	//init test data
	for(i = 0; i < half_n_cmplx; i++){
		source[i].x = (rand()/(float)RAND_MAX)*2-1;
        source[i].y = (rand()/(float)RAND_MAX)*2-1;
        source[i+half_n_cmplx].x = source[i].x;
        source[i+half_n_cmplx].y = source[i].y;
	}

	fprintf(stdout, "INITIAL:\n");
	for (i = 0; i < N; i++) {
		fprintf(stdout, "(%g, %g)\n", source[i].x, source[i].y);
	}
#pragma offload target(mic) in(fftsz, n_ffts) in(source:length(N) free_if(0))		
{
	forward(source, fftsz, n_ffts);
}

	fprintf(stdout, "FORWARD:\n");
	for (i = 0; i < N; i++) {
		fprintf(stdout, "(%g, %g)\n", source[i].x, source[i].y);
	}
#pragma offload target(mic) in(fftsz, n_ffts) out(source:length(N) alloc_if(0))	
{		
	inverse(source, fftsz, n_ffts);
}
	fprintf(stdout, "\nINVERSE:\n");
	for (i = 0; i < N; i++) {
		fprintf(stdout, "(%g, %g)\n", source[i].x, source[i].y);
	}

	//free space
	free(source);
}
