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

#include <stdio.h>
#include "Timer.h"
#include "ResultDatabase.h"
#include "OptionParser.h"
#include "omp.h"
#include <string>
#include <iostream>

// For heterogeneous features include "offload.h"
#include "offload.h"

#ifdef __MIC__
#include <lmmintrin.h>
#endif


// Memory Benchmarks
#define VECSIZE_SP 480000
#define REPS_SP 1000

float __declspec(target(MIC)) testICC_read(int reps, int eversion);
float __declspec(target(MIC)) testICC_write(int reps, int eversion, float value);

float __declspec(target(MIC)) testIntrinsics_read(int reps, int eversion);
float __declspec(target(MIC)) testIntrinsics_write(int reps, int eversion, float value);

// L2 & L1 Benchmarks
#define VECSIZE_SP_L2 4864
#define REPS_SP_L2 1000000
#define VECSIZE_SP_L1 1024
#define REPS_SP_L1 1000000

float __declspec(target(MIC)) testICC_read_caches(int reps, int eversion, int worksize);
float __declspec(target(MIC)) testICC_write_caches(int reps, int eversion, float value, int worksize);

float __declspec(target(MIC)) testIntrinsics_read_caches(int reps, int eversion, int worksize);
float __declspec(target(MIC)) testIntrinsics_write_caches(int reps, int eversion, float value, int worksize);

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
// Programmer: Alexander Heinecke
// Creation: July 23, 2010
//
// Modifications:
//
// ****************************************************************************
void addBenchmarkSpecOptions(OptionParser &op)
{

}

// ****************************************************************************
// Function: runBenchmark
//
// Purpose:
//
// Arguments:
//  resultDB: the benchmark stores its results in this ResultDatabase
//  op: the options parser / parameter database
//
// Returns:  nothing
//
// Programmer: Alexander Heinecke
// Creation: July 23, 2010
//
// Modifications:
//
// ****************************************************************************
void RunBenchmark( OptionParser &op, ResultDatabase &resultDB)
{
    bool verbose = op.getOptionBool("verbose");
    const unsigned int passes = op.getOptionInt("passes");
    //int eversion = op.getOptionInt("eversion");
    int eversion = 1;

    double t = 0.0f;
    double startTime;
    unsigned int w;
    unsigned int reps;
	double nbytes;
	float res = 0.0;
	float input = 1.0;

	// Determine number of available threads on installed KNF
	int numThreads = 124;

	// E1
	if (eversion == 1)
	{
		numThreads = 124;
	}
	// E2
	else if (eversion == 2)
	{
		numThreads = 108;
	}

	double dThreads = static_cast<double>(numThreads);

    for (int p = 0; p < passes; p++)
    {
		//////////////////////////////////////////
		// Test Read - ICC Code
		//////////////////////////////////////////
        w = VECSIZE_SP;
        reps = REPS_SP;
    	t = 0.0f;

    	res = 0.0;

		startTime= curr_second();

		#pragma offload target (MIC)
		res = testICC_read(reps, eversion);

		t = curr_second()-startTime;

		// Add result
    	nbytes= ((double)w)*((double)reps)*((double)sizeof(float))*dThreads;

		resultDB.AddResult("ICC_R_MEM", "", "GB/s",
				(((double)nbytes) / (t*1.e9)));

		//////////////////////////////////////////
		// Test Read - Intrinsics Code
		//////////////////////////////////////////
        w = VECSIZE_SP;
        reps = REPS_SP;
    	t = 0.0f;

    	res = 0.0;

		startTime= curr_second();

		#pragma offload target (MIC)
		res = testIntrinsics_read(reps, eversion);

		t = curr_second()-startTime;

		// Add result
    	nbytes= ((double)w)*((double)reps)*((double)sizeof(float))*dThreads;

		resultDB.AddResult("INT_R_MEM", "", "GB/s",
				(((double)nbytes) / (t*1.e9)));

		//////////////////////////////////////////
		// Test Wrtie - ICC Code
		//////////////////////////////////////////
        w = VECSIZE_SP;
        reps = REPS_SP;
    	t = 0.0f;

    	res = 0.0;


		startTime= curr_second();

		#pragma offload target (MIC)
		res = testICC_write(reps, eversion, input);

		t = curr_second()-startTime;

		// Add result
    	nbytes= ((double)w)*((double)reps)*((double)sizeof(float))*dThreads;

		resultDB.AddResult("ICC_W_MEM", "", "GB/s",
				(((double)nbytes) / (t*1.e9)));

		//////////////////////////////////////////
		// Test Wrtie - ICC Code
		//////////////////////////////////////////
        w = VECSIZE_SP;
        reps = REPS_SP;
    	t = 0.0f;

    	res = 0.0;


		startTime= curr_second();

		#pragma offload target (MIC)
		res = testIntrinsics_write(reps, eversion, input);

		t = curr_second()-startTime;

		// Add result
    	nbytes= ((double)w)*((double)reps)*((double)sizeof(float))*dThreads;

		resultDB.AddResult("INT_W_MEM", "", "GB/s",
				(((double)nbytes) / (t*1.e9)));


		//////////////////////////////////////////
		// Test Read L1 - ICC Code
		//////////////////////////////////////////
        w = VECSIZE_SP_L1;
        reps = REPS_SP_L1;
    	t = 0.0f;

    	res = 0.0;

		startTime= curr_second();

		#pragma offload target (MIC)
		res = testICC_read_caches(reps, eversion, w);

		t =curr_second()-startTime; 

		// Add result
    	nbytes= ((double)w)*((double)reps)*((double)sizeof(float))*4.0;

		resultDB.AddResult("ICC_R_L1", "", "GB/s",
				(((double)nbytes) / (t*1.e9)));

		//////////////////////////////////////////
		// Test Read L1 - Intrinsics Code
		//////////////////////////////////////////
        w = VECSIZE_SP_L1;
        reps = REPS_SP_L1;
    	t = 0.0f;

    	res = 0.0;

		startTime= curr_second();

		#pragma offload target (MIC)
		res = testIntrinsics_read_caches(reps, eversion, w);

		t = curr_second()-startTime;

		// Add result
    	nbytes= ((double)w)*((double)reps)*((double)sizeof(float))*4.0;

		resultDB.AddResult("INT_R_L1", "", "GB/s",
				(((double)nbytes) / (t*1.e9)));


		//////////////////////////////////////////
		// Test Write L1 - ICC Code
		//////////////////////////////////////////
        w = VECSIZE_SP_L1;
        reps = REPS_SP_L1;
    	t = 0.0f;

    	res = 0.0;

		startTime= curr_second();

		#pragma offload target (MIC)
		res = testICC_write_caches(reps, eversion, input, w);

		t = curr_second()-startTime;

		// Add result
    	nbytes= ((double)w)*((double)reps)*((double)sizeof(float))*4.0;

		resultDB.AddResult("ICC_W_L1", "", "GB/s",
				(((double)nbytes) / (t*1.e9)));

		//////////////////////////////////////////
		// Test Write L1 - Intrinsics Code
		//////////////////////////////////////////
        w = VECSIZE_SP_L1;
        reps = REPS_SP_L1;
    	t = 0.0f;

    	res = 0.0;

		startTime= curr_second();

		#pragma offload target (MIC)
		res = testIntrinsics_write_caches(reps, eversion, input, w);

		t = curr_second()-startTime;


		// Add result
    	nbytes= ((double)w)*((double)reps)*((double)sizeof(float))*4.0;

		resultDB.AddResult("INT_W_L1", "", "GB/s",
				(((double)nbytes) / (t*1.e9)));

		//////////////////////////////////////////
		// Test Read L2- ICC Code
		//////////////////////////////////////////
        w = VECSIZE_SP_L2;
        reps = REPS_SP_L2;
    	t = 0.0f;

    	res = 0.0;

		startTime= curr_second();

		#pragma offload target (MIC)
		res = testICC_read_caches(reps, eversion, w);

		t =curr_second()-startTime; 


		// Add result
    	nbytes= ((double)w)*((double)reps)*((double)sizeof(float))*4.0;

		resultDB.AddResult("ICC_R_L2", "", "GB/s",
				(((double)nbytes) / (t*1.e9)));

		//////////////////////////////////////////
		// Test Read L2 - Intrinsics Code
		//////////////////////////////////////////
        w = VECSIZE_SP_L2;
        reps = REPS_SP_L2;
    	t = 0.0f;

    	res = 0.0;

		startTime= curr_second();

		#pragma offload target (MIC)
		res = testIntrinsics_read_caches(reps, eversion, w);

		t =curr_second()-startTime; 

		// Add result
    	nbytes= ((double)w)*((double)reps)*((double)sizeof(float))*4.0;

		resultDB.AddResult("INT_R_L2", "", "GB/s",
				(((double)nbytes) / (t*1.e9)));

		//////////////////////////////////////////
		// Test Write L2 - ICC Code
		//////////////////////////////////////////
        w = VECSIZE_SP_L2;
        reps = REPS_SP_L2;
    	t = 0.0f;

    	res = 0.0;

		startTime= curr_second();

		#pragma offload target (MIC)
		res = testICC_write_caches(reps, eversion, input, w);

		t = curr_second()-startTime;

		// Add result
    	nbytes= ((double)w)*((double)reps)*((double)sizeof(float))*4.0;

		resultDB.AddResult("ICC_W_L2", "", "GB/s",
				(((double)nbytes) / (t*1.e9)));

		//////////////////////////////////////////
		// Test Write L2 - Intrinsics Code
		//////////////////////////////////////////
        w = VECSIZE_SP_L2;
        reps = REPS_SP_L2;
    	t = 0.0f;

    	res = 0.0;

		startTime= curr_second();

		#pragma offload target (MIC)
		res = testIntrinsics_write_caches(reps, eversion, input, w);

		t = curr_second()-startTime;

		// Add result
    	nbytes= ((double)w)*((double)reps)*((double)sizeof(float))*4.0;

		resultDB.AddResult("INT_W_L2", "", "GB/s",
				(((double)nbytes) / (t*1.e9)));
    }
}
//Hi
float __declspec(target(MIC)) testICC_read(int reps, int eversion)
{
#ifdef __MIC__
	#define aligned_malloc _mm_malloc
	#define aligned_free(addr) _mm_free(addr)

	size_t numElements;

	std::string schedule = "";
	if (eversion == 1)
	{
		schedule = "KMP_AFFINITY=proclist=[4-127],granularity=thread,explicit";
		numElements = VECSIZE_SP*124;
	}
	else if (eversion == 2)
	{
		schedule = "KMP_AFFINITY=proclist=[4-111],granularity=thread,explicit";
		numElements = VECSIZE_SP*108;
	}
	else
	{
		schedule = "KMP_AFFINITY=proclist=[4-95],granularity=thread,explicit";
		numElements = VECSIZE_SP*92;
	}
	kmp_set_defaults("KMP_WARNINGS=0");
	kmp_set_defaults(schedule.c_str());

	float* a = (float*)_mm_malloc(sizeof(float)*numElements, 64);
	__declspec(aligned(64))float res = 0.0;

	#pragma vector aligned
	#pragma ivdep
	#pragma omp parallel for shared(a)
	for (int q = 0; q < numElements; q++)
	{
		a[q] = 1.0;
	}

	#pragma omp parallel shared(res)
	{
		__declspec(aligned(64))float b = 0.0;
		int offset = VECSIZE_SP * omp_get_thread_num();

		for (int m = 0; m < reps; m++)
		{
			#pragma vector aligned
			#pragma ivdep
			for (int q = offset; q < offset+VECSIZE_SP; q++)
			{
				b += a[q];
			}
			b += 1.0;
		}


		#pragma omp critical
		{
			res += b;
		}
	}

	_mm_free(a);

	return res;
#else
	return 0.0;
#endif
}

float __declspec(target(MIC)) testIntrinsics_read(int reps, int eversion)
{
#ifdef __MIC__
	#define aligned_malloc _mm_malloc
	#define aligned_free(addr) _mm_free(addr)

	size_t numElements;

	std::string schedule = "";

	if (eversion == 1)
	{
		schedule = "KMP_AFFINITY=proclist=[4-127],granularity=thread,explicit";
		numElements = VECSIZE_SP*124;
	}
	else if (eversion == 2)
	{
		schedule = "KMP_AFFINITY=proclist=[4-111],granularity=thread,explicit";
		numElements = VECSIZE_SP*108;
	}
	else
	{
		schedule = "KMP_AFFINITY=proclist=[4-95],granularity=thread,explicit";
		numElements = VECSIZE_SP*92;
	}

	kmp_set_defaults("KMP_WARNINGS=0");
	kmp_set_defaults(schedule.c_str());

	float* a = (float*)_mm_malloc(sizeof(float)*numElements, 64);
	__declspec(aligned(64))float res = 0.0;

	#pragma vector aligned
	#pragma ivdep
	#pragma omp parallel for shared(a)
	for (int q = 0; q < numElements; q++)
	{
		a[q] = 1.0;
	}

	#pragma omp parallel shared(res)
	{
		int offset = VECSIZE_SP * omp_get_thread_num();

		__m512 b_0;
		__m512 b_1;
		__m512 b_2;
		__m512 b_3;
		__m512 b_4;
		__m512 b_5;
		__m512 b_6;
		__m512 b_7;

		for (int m = 0; m < reps; m++)
		{
			b_0 = _mm512_setzero();
			b_1 = _mm512_setzero();
			b_2 = _mm512_setzero();
			b_3 = _mm512_setzero();
			b_4 = _mm512_setzero();
			b_5 = _mm512_setzero();
			b_6 = _mm512_setzero();
			b_7 = _mm512_setzero();

			#pragma vector aligned
			#pragma ivdep
			for (int q = offset; q < offset+VECSIZE_SP; q+=128)
			{
				_mm_vprefetch1(&(a[q+128]), _MM_PFHINT_NT);
				_mm_vprefetch1(&(a[q+144]), _MM_PFHINT_NT);
				_mm_vprefetch1(&(a[q+160]), _MM_PFHINT_NT);
				_mm_vprefetch1(&(a[q+176]), _MM_PFHINT_NT);
				_mm_vprefetch1(&(a[q+192]), _MM_PFHINT_NT);
				_mm_vprefetch1(&(a[q+208]), _MM_PFHINT_NT);
				_mm_vprefetch1(&(a[q+224]), _MM_PFHINT_NT);
				_mm_vprefetch1(&(a[q+240]), _MM_PFHINT_NT);
/*
				_mm_vprefetch2(&(a[q+(128*10)]), _MM_PFHINT_NT_MISS);
				_mm_vprefetch2(&(a[q+((128*10)+16)]), _MM_PFHINT_NT_MISS);
				_mm_vprefetch2(&(a[q+((128*10)+32)]), _MM_PFHINT_NT_MISS);
				_mm_vprefetch2(&(a[q+((128*10)+48)]), _MM_PFHINT_NT_MISS);
				_mm_vprefetch2(&(a[q+((128*10)+64)]), _MM_PFHINT_NT_MISS);
				_mm_vprefetch2(&(a[q+((128*10)+80)]), _MM_PFHINT_NT_MISS);
				_mm_vprefetch2(&(a[q+((128*10)+96)]), _MM_PFHINT_NT_MISS);
				_mm_vprefetch2(&(a[q+((128*10)+112)]), _MM_PFHINT_NT_MISS);
*/
				__m512 a_0 = _mm512_loadd(&(a[q]), _MM_FULLUPC_NONE, _MM_BROADCAST_16X16, _MM_HINT_NT);
				__m512 a_1 = _mm512_loadd(&(a[q+16]), _MM_FULLUPC_NONE, _MM_BROADCAST_16X16, _MM_HINT_NT);
				__m512 a_2 = _mm512_loadd(&(a[q+32]), _MM_FULLUPC_NONE, _MM_BROADCAST_16X16, _MM_HINT_NT);
				__m512 a_3 = _mm512_loadd(&(a[q+48]), _MM_FULLUPC_NONE, _MM_BROADCAST_16X16, _MM_HINT_NT);
				__m512 a_4 = _mm512_loadd(&(a[q+64]), _MM_FULLUPC_NONE, _MM_BROADCAST_16X16, _MM_HINT_NT);
				__m512 a_5 = _mm512_loadd(&(a[q+80]), _MM_FULLUPC_NONE, _MM_BROADCAST_16X16, _MM_HINT_NT);
				__m512 a_6 = _mm512_loadd(&(a[q+96]), _MM_FULLUPC_NONE, _MM_BROADCAST_16X16, _MM_HINT_NT);
				__m512 a_7 = _mm512_loadd(&(a[q+112]), _MM_FULLUPC_NONE, _MM_BROADCAST_16X16, _MM_HINT_NT);

				b_0 = _mm512_add_ps(b_0, a_0);
				b_1 = _mm512_add_ps(b_1, a_1);
				b_2 = _mm512_add_ps(b_2, a_2);
				b_3 = _mm512_add_ps(b_3, a_3);
				b_4 = _mm512_add_ps(b_4, a_4);
				b_5 = _mm512_add_ps(b_5, a_5);
				b_6 = _mm512_add_ps(b_6, a_6);
				b_7 = _mm512_add_ps(b_7, a_7);
			}
			b_0 = _mm512_add_ps(b_0, b_1);
			b_2 = _mm512_add_ps(b_2, b_3);
			b_4 = _mm512_add_ps(b_4, b_5);
			b_6 = _mm512_add_ps(b_6, b_7);
			b_0 = _mm512_add_ps(b_0, b_2);
			b_4 = _mm512_add_ps(b_4, b_6);
			b_0 = _mm512_add_ps(b_0, b_4);
		}


		#pragma omp critical
		{
			res += _mm512_reduce_add_ps(b_0);
		}
	}

	_mm_free(a);

	return res;
#else
	return 0.0;
#endif
}

float __declspec(target(MIC)) testICC_write(int reps, int eversion, float value)
{
#ifdef __MIC__
	#define aligned_malloc _mm_malloc
	#define aligned_free(addr) _mm_free(addr)

	size_t numElements;

	std::string schedule = "";
	if (eversion == 1)
	{
		schedule = "KMP_AFFINITY=proclist=[4-127],granularity=thread,explicit";
		numElements = VECSIZE_SP*124;
	}
	else if (eversion == 2)
	{
		schedule = "KMP_AFFINITY=proclist=[4-111],granularity=thread,explicit";
		numElements = VECSIZE_SP*108;
	}
	else
	{
		schedule = "KMP_AFFINITY=proclist=[4-95],granularity=thread,explicit";
		numElements = VECSIZE_SP*92;
	}
	kmp_set_defaults("KMP_WARNINGS=0");
	kmp_set_defaults(schedule.c_str());

	float* a = (float*)_mm_malloc(sizeof(float)*numElements, 64);
	__declspec(aligned(64))float res = 0.0;

	#pragma vector aligned
	#pragma ivdep
	for (int q = 0; q < numElements; q++)
	{
		a[q] = 1.0;
	}

	#pragma omp parallel shared(res)
	{
		int offset = VECSIZE_SP * omp_get_thread_num();
		__declspec(aligned(64))float writeData = value + static_cast<float>(omp_get_thread_num());

		for (int m = 0; m < reps; m++)
		{
			#pragma vector aligned
			#pragma ivdep
			for (int q = offset; q < offset+VECSIZE_SP; q++)
			{
				a[q] += writeData;
			}
			writeData += 1.0;
		}
	}

	// sum something in a, avoid compiler optimizations
	res = a[0] + a[numElements-1];

	_mm_free(a);

	return res;
#else
	return 0.0;
#endif
}

float __declspec(target(MIC)) testIntrinsics_write(int reps, int eversion, float value)
{
#ifdef __MIC__
	#define aligned_malloc _mm_malloc
	#define aligned_free(addr) _mm_free(addr)

	size_t numElements;

	std::string schedule = "";

	if (eversion == 1)
	{
		schedule = "KMP_AFFINITY=proclist=[4-127],granularity=thread,explicit";
		numElements = VECSIZE_SP*124;
	}
	else if (eversion == 2)
	{
		schedule = "KMP_AFFINITY=proclist=[4-111],granularity=thread,explicit";
		numElements = VECSIZE_SP*108;
	}
	else
	{
		schedule = "KMP_AFFINITY=proclist=[4-95],granularity=thread,explicit";
		numElements = VECSIZE_SP*92;
	}

	kmp_set_defaults("KMP_WARNINGS=0");
	kmp_set_defaults(schedule.c_str());

	float* a = (float*)_mm_malloc(sizeof(float)*numElements, 64);
	__declspec(aligned(64))float res = 0.0;

	#pragma omp parallel shared(res)
	{
		int offset = VECSIZE_SP * omp_get_thread_num();
		__declspec(aligned(64))float writeData = value + static_cast<float>(omp_get_thread_num());

		__m512 toWrite = _mm512_loadd(&(writeData), _MM_FULLUPC_NONE, _MM_BROADCAST_1X16, _MM_HINT_NONE);
		__m512 one = _mm512_set_1to16_ps(1.0);

		for (int m = 0; m < reps; m++)
		{
			#pragma vector aligned
			#pragma ivdep
			for (int q = offset; q < offset+VECSIZE_SP; q+=128)
			{
				_mm_vprefetch1(&(a[q+128]), _MM_PFHINT_EX_NT);
				_mm_vprefetch1(&(a[q+144]), _MM_PFHINT_EX_NT);
				_mm_vprefetch1(&(a[q+160]), _MM_PFHINT_EX_NT);
				_mm_vprefetch1(&(a[q+176]), _MM_PFHINT_EX_NT);
				_mm_vprefetch1(&(a[q+192]), _MM_PFHINT_EX_NT);
				_mm_vprefetch1(&(a[q+208]), _MM_PFHINT_EX_NT);
				_mm_vprefetch1(&(a[q+224]), _MM_PFHINT_EX_NT);
				_mm_vprefetch1(&(a[q+240]), _MM_PFHINT_EX_NT);

				//_mm_vprefetch2(&(a[q+(128*10)]), _MM_PFHINT_EX_NT_MISS);
				//_mm_vprefetch2(&(a[q+((128*10)+16)]), _MM_PFHINT_EX_NT_MISS);
				//_mm_vprefetch2(&(a[q+((128*10)+32)]), _MM_PFHINT_EX_NT_MISS);
				//_mm_vprefetch2(&(a[q+((128*10)+48)]), _MM_PFHINT_EX_NT_MISS);
				//_mm_vprefetch2(&(a[q+((128*10)+64)]), _MM_PFHINT_EX_NT_MISS);
				//_mm_vprefetch2(&(a[q+((128*10)+80)]), _MM_PFHINT_EX_NT_MISS);
				//_mm_vprefetch2(&(a[q+((128*10)+96)]), _MM_PFHINT_EX_NT_MISS);
				//_mm_vprefetch2(&(a[q+((128*10)+112)]), _MM_PFHINT_EX_NT_MISS);

				__m512 a_0 = toWrite;
				__m512 a_1 = toWrite;
				__m512 a_2 = toWrite;
				__m512 a_3 = toWrite;
				__m512 a_4 = toWrite;
				__m512 a_5 = toWrite;
				__m512 a_6 = toWrite;
				__m512 a_7 = toWrite;

				_mm512_stored(&(a[q]), a_0, _MM_DOWNC_NONE, _MM_SUBSET32_16, _MM_HINT_NT);
				_mm512_stored(&(a[q+16]), a_1, _MM_DOWNC_NONE, _MM_SUBSET32_16, _MM_HINT_NT);
				_mm512_stored(&(a[q+32]), a_2, _MM_DOWNC_NONE, _MM_SUBSET32_16, _MM_HINT_NT);
				_mm512_stored(&(a[q+48]), a_3, _MM_DOWNC_NONE, _MM_SUBSET32_16, _MM_HINT_NT);
				_mm512_stored(&(a[q+64]), a_4, _MM_DOWNC_NONE, _MM_SUBSET32_16, _MM_HINT_NT);
				_mm512_stored(&(a[q+80]), a_5, _MM_DOWNC_NONE, _MM_SUBSET32_16, _MM_HINT_NT);
				_mm512_stored(&(a[q+96]), a_6, _MM_DOWNC_NONE, _MM_SUBSET32_16, _MM_HINT_NT);
				_mm512_stored(&(a[q+112]), a_7, _MM_DOWNC_NONE, _MM_SUBSET32_16, _MM_HINT_NT);
			}

			toWrite = _mm512_add_ps(toWrite, one);
		}
	}

	// sum something in a, avoid compiler optimizations
	res = a[0] + a[numElements-1];

	_mm_free(a);

	return res;
#else
	return 0.0;
#endif
}

float __declspec(target(MIC)) testICC_read_caches(int reps, int eversion, int worksize)
{
#ifdef __MIC__
	#define aligned_malloc _mm_malloc
	#define aligned_free(addr) _mm_free(addr)

	size_t numElements;

	std::string schedule = "";

	schedule = "KMP_AFFINITY=proclist=[4-7],granularity=thread,explicit";
	numElements = worksize*4;

	kmp_set_defaults("KMP_WARNINGS=0");
	kmp_set_defaults(schedule.c_str());

	float* a = (float*)_mm_malloc(sizeof(float)*numElements, 64);
	__declspec(aligned(64))float res = 0.0;

	#pragma vector aligned
	#pragma ivdep
	for (int q = 0; q < numElements; q++)
	{
		a[q] = 1.0;
	}

	#pragma omp parallel num_threads(4) shared(res)
	{
		__declspec(aligned(64))float b = 0.0;
		int offset = worksize * omp_get_thread_num();

		for (int m = 0; m < reps; m++)
		{
			#pragma vector aligned
			#pragma ivdep
			for (int q = offset; q < offset+worksize; q++)
			{
				b += a[q];
			}
			b += 1.0;
		}


		#pragma omp critical
		{
			res += b;
		}
	}

	_mm_free(a);

	return res;
#else
	return 0.0;
#endif
}

float __declspec(target(MIC)) testICC_write_caches(int reps, int eversion, float value, int worksize)
{
#ifdef __MIC__
	#define aligned_malloc _mm_malloc
	#define aligned_free(addr) _mm_free(addr)

	size_t numElements;

	std::string schedule = "";

	schedule = "KMP_AFFINITY=proclist=[4-7],granularity=thread,explicit";
	numElements = worksize*4;

	kmp_set_defaults("KMP_WARNINGS=0");
	kmp_set_defaults(schedule.c_str());

	float* a = (float*)_mm_malloc(sizeof(float)*numElements, 64);
	__declspec(aligned(64))float res = 0.0;

	#pragma omp parallel num_threads(4) shared(res)
	{
		int offset = worksize * omp_get_thread_num();
		__declspec(aligned(64))float writeData = value + static_cast<float>(omp_get_thread_num());

		for (int m = 0; m < reps; m++)
		{
			#pragma vector aligned
			#pragma ivdep
			for (int q = offset; q < offset+worksize; q++)
			{
				a[q] += writeData;
			}
			writeData += 1.0;
		}
	}

	// sum something in a, avoid compiler optimizations
	res = a[0] + a[numElements-1];

	_mm_free(a);

	return res;
#else
	return 0.0;
#endif
}

float __declspec(target(MIC)) testIntrinsics_read_caches(int reps, int eversion, int worksize)
{
#ifdef __MIC__
	#define aligned_malloc _mm_malloc
	#define aligned_free(addr) _mm_free(addr)

	size_t numElements;

	std::string schedule = "";

	schedule = "KMP_AFFINITY=proclist=[4-7],granularity=thread,explicit";
	numElements = worksize*4;

	kmp_set_defaults("KMP_WARNINGS=0");
	kmp_set_defaults(schedule.c_str());

	float* a = (float*)_mm_malloc(sizeof(float)*numElements, 64);
	__declspec(aligned(64))float res = 0.0;

	#pragma vector aligned
	#pragma ivdep
	for (int q = 0; q < numElements; q++)
	{
		a[q] = 1.0;
	}

	#pragma omp parallel num_threads(4) shared(res)
	{
		int offset = worksize * omp_get_thread_num();

		__m512 b_0 = _mm512_setzero();
		__m512 b_1 = _mm512_setzero();
		__m512 b_2 = _mm512_setzero();
		__m512 b_3 = _mm512_setzero();
		__m512 b_4 = _mm512_setzero();
		__m512 b_5 = _mm512_setzero();
		__m512 b_6 = _mm512_setzero();
		__m512 b_7 = _mm512_setzero();

		for (int m = 0; m < reps; m++)
		{
			#pragma vector aligned
			#pragma ivdep
			for (int q = offset; q < offset+worksize; q+=128)
			{
				__m512 a_0 = _mm512_loadd(&(a[q]), _MM_FULLUPC_NONE, _MM_BROADCAST_16X16, _MM_HINT_NONE);
				__m512 a_1 = _mm512_loadd(&(a[q+16]), _MM_FULLUPC_NONE, _MM_BROADCAST_16X16, _MM_HINT_NONE);
				__m512 a_2 = _mm512_loadd(&(a[q+32]), _MM_FULLUPC_NONE, _MM_BROADCAST_16X16, _MM_HINT_NONE);
				__m512 a_3 = _mm512_loadd(&(a[q+48]), _MM_FULLUPC_NONE, _MM_BROADCAST_16X16, _MM_HINT_NONE);
				__m512 a_4 = _mm512_loadd(&(a[q+64]), _MM_FULLUPC_NONE, _MM_BROADCAST_16X16, _MM_HINT_NONE);
				__m512 a_5 = _mm512_loadd(&(a[q+80]), _MM_FULLUPC_NONE, _MM_BROADCAST_16X16, _MM_HINT_NONE);
				__m512 a_6 = _mm512_loadd(&(a[q+96]), _MM_FULLUPC_NONE, _MM_BROADCAST_16X16, _MM_HINT_NONE);
				__m512 a_7 = _mm512_loadd(&(a[q+112]), _MM_FULLUPC_NONE, _MM_BROADCAST_16X16, _MM_HINT_NONE);

				b_0 = _mm512_add_ps(b_0, a_0);
				b_1 = _mm512_add_ps(b_1, a_1);
				b_2 = _mm512_add_ps(b_2, a_2);
				b_3 = _mm512_add_ps(b_3, a_3);
				b_4 = _mm512_add_ps(b_4, a_4);
				b_5 = _mm512_add_ps(b_5, a_5);
				b_6 = _mm512_add_ps(b_6, a_6);
				b_7 = _mm512_add_ps(b_7, a_7);
			}
			b_0 = _mm512_add_ps(b_0, b_1);
			b_2 = _mm512_add_ps(b_2, b_3);
			b_4 = _mm512_add_ps(b_4, b_5);
			b_6 = _mm512_add_ps(b_6, b_7);
			b_0 = _mm512_add_ps(b_0, b_2);
			b_4 = _mm512_add_ps(b_4, b_6);
			b_0 = _mm512_add_ps(b_0, b_4);
		}


		#pragma omp critical
		{
			res += _mm512_reduce_add_ps(b_0);
		}
	}

	_mm_free(a);

	return res;
#else
	return 0.0;
#endif
}

float __declspec(target(MIC)) testIntrinsics_write_caches(int reps, int eversion, float value, int worksize)
{
#ifdef __MIC__
	#define aligned_malloc _mm_malloc
	#define aligned_free(addr) _mm_free(addr)

	size_t numElements;

	std::string schedule = "";

	schedule = "KMP_AFFINITY=proclist=[4-7],granularity=thread,explicit";
	numElements = worksize*4;

	kmp_set_defaults("KMP_WARNINGS=0");
	kmp_set_defaults(schedule.c_str());

	float* a = (float*)_mm_malloc(sizeof(float)*numElements, 64);
	__declspec(aligned(64))float res = 0.0;

	#pragma omp parallel num_threads(4) shared(res)
	{
		int offset = worksize * omp_get_thread_num();
		__declspec(aligned(64))float writeData = value + static_cast<float>(omp_get_thread_num());

		__m512 toWrite = _mm512_loadd(&(writeData), _MM_FULLUPC_NONE, _MM_BROADCAST_1X16, _MM_HINT_NONE);
		__m512 one = _mm512_set_1to16_ps(1.0);

		for (int m = 0; m < reps; m++)
		{
			#pragma vector aligned
			#pragma ivdep
			for (int q = offset; q < offset+worksize; q+=128)
			{
				__m512 a_0 = toWrite;
				__m512 a_1 = toWrite;
				__m512 a_2 = toWrite;
				__m512 a_3 = toWrite;
				__m512 a_4 = toWrite;
				__m512 a_5 = toWrite;
				__m512 a_6 = toWrite;
				__m512 a_7 = toWrite;

				_mm512_stored(&(a[q]), a_0, _MM_DOWNC_NONE, _MM_SUBSET32_16, _MM_HINT_NONE);
				_mm512_stored(&(a[q+16]), a_1, _MM_DOWNC_NONE, _MM_SUBSET32_16, _MM_HINT_NONE);
				_mm512_stored(&(a[q+32]), a_2, _MM_DOWNC_NONE, _MM_SUBSET32_16, _MM_HINT_NONE);
				_mm512_stored(&(a[q+48]), a_3, _MM_DOWNC_NONE, _MM_SUBSET32_16, _MM_HINT_NONE);
				_mm512_stored(&(a[q+64]), a_4, _MM_DOWNC_NONE, _MM_SUBSET32_16, _MM_HINT_NONE);
				_mm512_stored(&(a[q+80]), a_5, _MM_DOWNC_NONE, _MM_SUBSET32_16, _MM_HINT_NONE);
				_mm512_stored(&(a[q+96]), a_6, _MM_DOWNC_NONE, _MM_SUBSET32_16, _MM_HINT_NONE);
				_mm512_stored(&(a[q+112]), a_7, _MM_DOWNC_NONE, _MM_SUBSET32_16, _MM_HINT_NONE);
			}

			toWrite = _mm512_add_ps(toWrite, one);
		}
	}

	// sum something in a, avoid compiler optimizations
	res = a[0] + a[numElements-1];

	_mm_free(a);

	return res;
#else
	return 0.0;
#endif
}
