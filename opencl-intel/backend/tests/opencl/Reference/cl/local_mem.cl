// Test for local memory in SATest OpenCL Reference
// Computes sum of array using local memory
// CL source code taken from Conformance test_conformance\basic\test_local.c
// RUN: SATest -OCL -REF -config=%s.cfg -neat=0 >%t
// RUN: FileCheck %s <%t

__kernel void compute_sum_with_localmem(__global int *a, int n, __global int *sum)
{
	 __local int tmp_sum[8];
    int  tid = get_local_id(0);
    int  lsize = get_local_size(0);
    int  i;

    tmp_sum[tid] = 0;
    for (i=tid; i<n; i+=lsize)
        tmp_sum[tid] += a[i];

    if( lsize == 1 )
    {
       if( tid == 0 )
           *sum = tmp_sum[0];
       return;
    }

    do
    {
       barrier(CLK_LOCAL_MEM_FENCE);
       if (tid < lsize/2)
       {
           int sum = tmp_sum[tid];
           if( (lsize & 1) && tid == 0 )
               sum += tmp_sum[tid + lsize - 1];
           tmp_sum[tid] = sum + tmp_sum[tid + lsize/2];
       }
       lsize = lsize/2; 
    }while( lsize );

    if( tid == 0 )
	{
       *sum = tmp_sum[0];
	   printf("%d\n", *sum);
// CHECK: 36
	}
} 
