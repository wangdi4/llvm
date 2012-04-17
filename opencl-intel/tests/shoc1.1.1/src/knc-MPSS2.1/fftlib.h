#ifndef FFTLIB_H
#define FFTLIB_H

#include <omp.h>
#include <math.h>

#pragma offload_attribute(push, target(mic))
#include <mkl.h>
#include <mkl_dfti.h>
#pragma offload_attribute(pop)

struct cplxflt{
	float x;
	float y;
};

struct cplxdbl{
	double x;
	double y;
};

#pragma offload_attribute(push, target(mic))
template <class T2>
__declspec(target(mic))
inline bool micDp(void);
template <> 
inline bool micDp<cplxflt>(void) { return false; }
template <>
inline bool micDp<cplxdbl>(void) { return true; }
template <class T2>
__declspec(target(mic))
void forward(T2* source, int fftsz, int n_ffts);
template <class T2>
__declspec(target(mic))
void inverse(T2* source, int fftsz, int n_ffts);
template <class T2>
__declspec(target(mic))
int checkDiff(T2 *source, int half_n_cmplx);
#pragma offload_attribute(pop)


template<class T2>
__declspec(target(mic))
void forward_orig(T2* source, int fftsz, int n_ffts)
{
	int test;
	int threads;
#pragma omp parallel
	threads = omp_get_num_threads();
	int fft_per_thread = (n_ffts + threads - 1)/threads;
#pragma omp parallel for shared(source)
	for(int i = 0; i < threads; i++)
	{
		DFTI_DESCRIPTOR_HANDLE plan;
		if(micDp<T2>()){
			DftiCreateDescriptor(&plan, DFTI_DOUBLE,DFTI_COMPLEX, 1,fftsz);
			DftiCommitDescriptor(plan);
			for(int j = 0; j < fft_per_thread; j++)
			{
				int workIndex = i * fft_per_thread + j;
				if(workIndex < n_ffts)DftiComputeForward(plan, &(source[workIndex*fftsz]));
			}
		}else{
			DftiCreateDescriptor(&plan, DFTI_SINGLE,DFTI_COMPLEX, 1,fftsz);
			DftiCommitDescriptor(plan);
			for(int j = 0; j < fft_per_thread; j++)
			{
				int workIndex = i * fft_per_thread + j;
				if(workIndex < n_ffts)DftiComputeForward(plan, &(source[workIndex*fftsz]));			
			}
		}		
		DftiFreeDescriptor(&plan);
	}
}

template<class T2>
__declspec(target(mic))
void forward(T2* source, int fftsz, int n_ffts)
{
    static __declspec(target(mic)) DFTI_DESCRIPTOR_HANDLE plan;
    if (!plan)
    {
        if (micDp<T2>())
        {
            DftiCreateDescriptor(&plan, DFTI_DOUBLE, DFTI_COMPLEX, 1, (MKL_LONG)fftsz);
        }
        else
        {
            DftiCreateDescriptor(&plan, DFTI_SINGLE, DFTI_COMPLEX, 1, (MKL_LONG)fftsz);
        }
        DftiSetValue(plan, DFTI_NUMBER_OF_TRANSFORMS, (MKL_LONG)n_ffts);
        DftiSetValue(plan, DFTI_INPUT_DISTANCE, (MKL_LONG)fftsz);
        DftiSetValue(plan, DFTI_OUTPUT_DISTANCE, (MKL_LONG)fftsz);
        DftiCommitDescriptor(plan);
    }
__declspec(target(mic)) __declspec(align(4096)) int a;
    DftiComputeForward(plan, source, &a);
    //DftiFreeDescriptor(&plan);
    //    //plan = NULL;
    //    
}

template<class T2>
__declspec(target(mic))
void inverse_orig(T2* source, int fftsz, int n_ffts)
{
	int threads;
#pragma omp parallel
	threads = omp_get_num_threads();
	int fft_per_thread = (n_ffts + threads - 1)/threads;
#pragma omp parallel for shared(source)
	for(int i = 0; i < threads; i++)
	{
		DFTI_DESCRIPTOR_HANDLE plan;
		if(micDp<T2>()){
			DftiCreateDescriptor(&plan, DFTI_DOUBLE,DFTI_COMPLEX, 1,fftsz);
			DftiSetValue(plan, DFTI_BACKWARD_SCALE, 1.0/fftsz);
			DftiCommitDescriptor(plan);
			for(int j = 0; j < fft_per_thread; j++)
			{
				int workIndex = i * fft_per_thread + j;
				if(workIndex < n_ffts)DftiComputeBackward(plan, &source[workIndex*fftsz]);
			}
		}else{
			DftiCreateDescriptor(&plan, DFTI_SINGLE,DFTI_COMPLEX, 1,fftsz);
			DftiSetValue(plan, DFTI_BACKWARD_SCALE, 1.0/fftsz);
			DftiCommitDescriptor(plan);
			for(int j = 0; j < fft_per_thread; j++)
			{
				int workIndex = i * fft_per_thread + j;
				if(workIndex < n_ffts)DftiComputeBackward(plan, &source[workIndex*fftsz]);
			}
		}		
		DftiFreeDescriptor(&plan);
	}

}

template<class T2>
__declspec(target(mic))
void inverse(T2* source, int fftsz, int n_ffts)
{
    static __declspec(target(mic)) DFTI_DESCRIPTOR_HANDLE plan;
    if (!plan)
    {
        if (micDp<T2>())
        {
            DftiCreateDescriptor(&plan, DFTI_DOUBLE, DFTI_COMPLEX, 1, (MKL_LONG)fftsz);
        }
        else
        {
            DftiCreateDescriptor(&plan, DFTI_SINGLE, DFTI_COMPLEX, 1, (MKL_LONG)fftsz);
        }
        DftiSetValue(plan, DFTI_NUMBER_OF_TRANSFORMS, (MKL_LONG)n_ffts);
        DftiSetValue(plan, DFTI_INPUT_DISTANCE, (MKL_LONG)fftsz);
        DftiSetValue(plan, DFTI_OUTPUT_DISTANCE, (MKL_LONG)fftsz);
        DftiSetValue(plan, DFTI_BACKWARD_SCALE, 1.0/fftsz);
        DftiCommitDescriptor(plan);
    }
    DftiComputeBackward(plan, source);
    DftiFreeDescriptor(&plan);
    plan = NULL;
}



template <class T2>
__declspec(target(mic))
int checkDiff(T2 *source, int half_n_cmplx)
{
	int diff = 0;
	int threads;
#pragma omp parallel
	threads = omp_get_num_threads();
	int check_per_thread = (half_n_cmplx + threads - 1)/threads;
#pragma omp parallel for shared(source, diff)
	for(int i = 0; i < threads; i++)
	{
		for(int j = 0; j < check_per_thread; j++)
		{
			int workIndex = i * check_per_thread + j;
			if(workIndex < half_n_cmplx){
				T2 a = source[workIndex];
				T2 b = source[workIndex+half_n_cmplx];
				if (fabs(a.x - b.x)>1e-6 || fabs(a.y - b.y)>1e-6) {
					diff = workIndex;
					break;
				}
			}
		}
	}
	return diff;
}

#endif
