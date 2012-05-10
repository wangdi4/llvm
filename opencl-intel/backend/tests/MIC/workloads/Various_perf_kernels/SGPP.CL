#define OCL_MULT_N_DATAPREFETCH_BLOCKSIZE_DP 128
#define OCL_DATAPREFETCH_SIZE_DP 64
#define dims 3

#pragma OPENCL EXTENSION cl_khr_fp64 : enable
__kernel void multTransModOCL(__global double* ptrSource,
						__global double* ptrData,
						__global double* ptrLevel,
						__global double* ptrIndex,
						__global double* ptrResult,
						uint sourceSize,
						uint offset)
{
	int globalIdx = get_global_id(0);
	int localIdx = get_local_id(0);
	globalIdx = globalIdx + offset;

	double eval, index_calc, abs, last, localSupport, curSupport;
	double myResult = 0.0f;
	__local double locData[dims*OCL_MULT_N_DATAPREFETCH_BLOCKSIZE_DP/2];
	__local double locSource[OCL_MULT_N_DATAPREFETCH_BLOCKSIZE_DP/2];
double level_d;
double index_d; 
	for (size_t d = 0; d < dims; d++)
	{
		level_d = ptrLevel[(globalIdx*dims)+ d];
		index_d = ptrIndex[(globalIdx*dims)+ d];
	}

// Iterate over all grid points
	for(int i = 0; i < sourceSize; i+=OCL_MULT_N_DATAPREFETCH_BLOCKSIZE_DP/2 )
	{
		for (size_t d = 0; d < dims; d++)
		{
			locData[(localIdx*dims)+d] = ptrData[((i+localIdx)*dims)+d];
		}
		locSource[localIdx] = ptrSource[i+localIdx];
		barrier(CLK_LOCAL_MEM_FENCE);
		for(int k = 0; k < OCL_MULT_N_DATAPREFETCH_BLOCKSIZE_DP/2; k++)
		{

		curSupport = locSource[k];
		for (size_t d = 0; d < dims; d++)
		{
			if (level_d == 2.0)
			{
			curSupport *= 1.0;
			}
			else if (index_d == 1.0)
			{
			curSupport *= max(2.0 - ( (level_d) * (locData[(k*dims)+d])) , 0.0);
			}
			else if ((index_d) == (level_d) - 1.0)
			{
			curSupport *= max((level_d) * (locData[(k*dims)+d]) - (index_d) + 1.0, 0.0);
			}
			else
			{
			curSupport *= max(1.0 - fabs( ( (level_d) * (locData[(k*dims)+d]) ) - (index_d) ), 0.0);
			}
		}
		myResult += curSupport;
		}
		barrier(CLK_LOCAL_MEM_FENCE);
	}
		ptrResult[globalIdx] = myResult;
}
		
#if 1
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
		__kernel void multModOCL(__global double* ptrAlpha,
								__global double* ptrData,
								__global double* ptrLevel,
								__global double* ptrIndex,
								__global double* ptrResult,
								uint fastStorageSize,
								uint storageSize,
								uint offset)
		{
		int globalIdx = get_global_id(0);
		int localIdx = get_local_id(0);
		globalIdx = globalIdx + offset;

		__local double locLevel[dims * OCL_DATAPREFETCH_SIZE_DP];
		__local double locIndex[dims * OCL_DATAPREFETCH_SIZE_DP];
		__local double locAlpha[OCL_DATAPREFETCH_SIZE_DP];

		double eval, index_calc, abs, last, localSupport, curSupport;
		double myResult = 0.0;
		// Create registers for the data
double data_d;
		for (size_t d = 0; d < dims; d++)
		{
			data_d  = ptrData[(globalIdx*dims)+d];
		}
	
		// Iterate over all grid points (fast ones, with cache)
		for(int j = 0; j < fastStorageSize; j+=OCL_DATAPREFETCH_SIZE_DP)
		{
		for (size_t d = 0; d < dims; d++)
		{
			locLevel[(localIdx*dims)+d] = ptrLevel[((j+localIdx)*dims)+ d];
			locIndex[(localIdx*dims)+d] = ptrIndex[((j+localIdx)*dims)+ d];
		}
		locAlpha[localIdx] = ptrAlpha[j+localIdx];
		barrier(CLK_LOCAL_MEM_FENCE);
		
		for(int k = 0; k < OCL_DATAPREFETCH_SIZE_DP; k++)
		{
		curSupport = locAlpha[k];
		for (size_t d = 0; d < dims; d++)
		{
			if ((locLevel[(k*dims)+d]) == 2.0)
			{
			curSupport *= 1.0;
			}
			else if ((locIndex[(k*dims)+d]) == 1.0)
			{
			curSupport *= max(2.0 - ( (locLevel[(k*dims)+d]) * (data_d) ), 0.0) ;
			}
			else if ((locIndex[(k*dims)+d]) == ((locLevel[(k*dims)+d]) - 1.0) )
			{
			curSupport *= max(( (locLevel[(k*dims)+d]) * (data_d) ) - (locIndex[(k*dims)+d]) + 1.0, 0.0);
			}
			else
			{
			curSupport *= max(1.0 - fabs( ( (locLevel[(k*dims)+d]) * (data_d) ) - (locIndex[(k*dims)+d]) ), 0.0);
			}
		}
		myResult += curSupport;
		}

		barrier(CLK_LOCAL_MEM_FENCE);
		}

		// Iterate over all grid points (slow ones, without cache)
		for(int m = fastStorageSize; m < storageSize; m++)
		{
		curSupport = ptrAlpha[m];
		for (size_t d = 0; d < dims; d++)
		{
			if ((ptrLevel[(m*dims)+d]) == 2.0)
			{
			curSupport *= 1.0;
			}
			else if ((ptrIndex[(m*dims)+d]) == 1.0)
			{
			curSupport *= max(2.0 - ( (ptrLevel[(m*dims)+d]) * (data_d) ), 0.0) ;
			}
			else if ((ptrIndex[(m*dims)+d]) == ((ptrLevel[(m*dims)+d]) - 1.0) )
			{
			curSupport *= max(( (ptrLevel[(m*dims)+d]) * (data_d) ) - (ptrIndex[(m*dims)+d]) + 1.0, 0.0);
			}
			else
			{
			curSupport *= max(1.0 - fabs( ( (ptrLevel[(m*dims)+d]) * (data_d) ) - (ptrIndex[(m*dims)+d]) ), 0.0);
			}
		}
		myResult += curSupport;
		}

		ptrResult[globalIdx] = myResult;
		}

#endif

#pragma OPENCL EXTENSION cl_khr_fp64 : enable
		__kernel void multTransOCL(__global double* ptrSource,
							__global double* ptrData,
								__global double* ptrLevel,
								__global double* ptrIndex,
								__global double* ptrResult,
							uint sourceSize,
							uint offset)
		{
		int globalIdx = get_global_id(0);
		int localIdx = get_local_id(0);
		globalIdx = globalIdx + offset;

		double eval, index_calc, abs, last, localSupport, curSupport;
		double myResult = 0.0f;
		__local double locData[dims*OCL_MULT_N_DATAPREFETCH_BLOCKSIZE_DP/2];
		__local double locSource[OCL_MULT_N_DATAPREFETCH_BLOCKSIZE_DP/2];

double level_d;
double index_d; 
		for (size_t d = 0; d < dims; d++)
		{
			level_d = ptrLevel[(globalIdx*dims)+d];
			index_d = ptrIndex[(globalIdx*dims)+d];
		}

		// Iterate over all grid points
		for(int i = 0; i < sourceSize; i+=OCL_MULT_N_DATAPREFETCH_BLOCKSIZE_DP/2)
		{
		for (size_t d = 0; d < dims; d++)
		{
			locData[(localIdx*dims)+d] = ptrData[((i+localIdx)*dims)+d];
		}
		locSource[localIdx] = ptrSource[i+localIdx];
		barrier(CLK_LOCAL_MEM_FENCE);
		for(int k = 0; k < OCL_MULT_N_DATAPREFETCH_BLOCKSIZE_DP/2; k++)
		{

		curSupport = locSource[k];
		for (size_t d = 0; d < dims; d++)
		{
			eval = ((level_d) * (locData[(k*dims)+d]));
			index_calc = eval - (index_d);
			abs = fabs(index_calc);
			last = 1.0 - abs;
			localSupport = fmax(last, 0.0);
			curSupport *= localSupport;
		}
		myResult += curSupport;
		}
		barrier(CLK_LOCAL_MEM_FENCE);
		}
		ptrResult[globalIdx] = myResult;
		}


#pragma OPENCL EXTENSION cl_khr_fp64 : enable
		__kernel void multOCL(__global double* ptrAlpha,
								__global double* ptrData,
								__global double* ptrLevel,
								__global double* ptrIndex,
								__global double* ptrResult,
								uint fastStorageSize,
								uint storageSize,
								uint offset)
		{
		int globalIdx = get_global_id(0);
		int localIdx = get_local_id(0);
		globalIdx = globalIdx + offset;

		__local double locLevel[dims * OCL_DATAPREFETCH_SIZE_DP];
		__local double locIndex[dims * OCL_DATAPREFETCH_SIZE_DP];
		__local double locAlpha[OCL_DATAPREFETCH_SIZE_DP];

		double eval, index_calc, abs, last, localSupport, curSupport;
		double myResult = 0.0;
		// Create registers for the data
double data_d;
		for (size_t d = 0; d < dims; d++)
		{
			data_d = ptrData[(globalIdx*dims)+d];
		}

		// Iterate over all grid points (fast ones, with cache)
		for(int j = 0; j < fastStorageSize; j+=OCL_DATAPREFETCH_SIZE_DP)
		{
		for (size_t d = 0; d < dims; d++)
		{
			locLevel[(localIdx*dims)+d] = ptrLevel[((j+localIdx)*dims)+d];
			locIndex[(localIdx*dims)+d] = ptrIndex[((j+localIdx)*dims)+d];
		}
		locAlpha[localIdx] = ptrAlpha[j+localIdx];
		barrier(CLK_LOCAL_MEM_FENCE);

		for(int k = 0; k < OCL_DATAPREFETCH_SIZE_DP; k++)
		{
		curSupport = locAlpha[k];
		for (size_t d = 0; d < dims; d++)
		{
			eval = ((locLevel[(k*dims)+ d ]) * (data_d));
			index_calc = eval - (locIndex[(k*dims)+d]);
			abs = fabs(index_calc);
			last = 1.0 - abs;
			localSupport = fmax(last, 0.0);
			curSupport *= localSupport;
		}
		myResult += curSupport;
		}

		barrier(CLK_LOCAL_MEM_FENCE);
		}

		// Iterate over all grid points (slow ones, without cache)
		for(int m = fastStorageSize; m < storageSize; m++)
		{
		curSupport = ptrAlpha[m];
		for (size_t d = 0; d < dims; d++)
		{
			eval = ((ptrLevel[(m*dims)+d]) * (data_d));
			index_calc = eval - (ptrIndex[(m*dims)+d]);
			abs = fabs(index_calc);
			last = 1.0 - abs;
			localSupport = fmax(last, 0.0);
			curSupport *= localSupport;
		}
		myResult += curSupport;
		}

		ptrResult[globalIdx] = myResult;
		}


