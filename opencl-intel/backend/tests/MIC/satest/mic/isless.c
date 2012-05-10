// RUN: pwd
// RUN: clang -cc1 -cl-kernel-arg-info -emit-llvm-bc -x cl -I ../../../../../../../src/backend/clang_headers/ \
// RUN: -include opencl_.h -o %s.bc %s
// RUN: python ../../bin/SATest.py -config=%s.cfg.xml -tsize=1

__kernel void isless_test(__global float3 *sourceA, __global float3 *sourceB, __global int3 *destValues, __global int3 *destValuesB)
{																			
    int  tid = 5; 
    float3 sampA = sourceA[tid];
    float3 sampB = sourceB[tid];
    destValues[tid] = isless(sampA, sampB); 
    destValuesB[tid] = islessequal(sampA, sampB); 	
}
