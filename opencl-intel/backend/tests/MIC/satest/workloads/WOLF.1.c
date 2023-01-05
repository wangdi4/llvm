#pragma OPENCL EXTENSION cl_khr_fp64 : enable
/* Each work-item computes 1 output float value*/
__kernel void matrixMultiplicationScalar(const __global float *matrixA,
                                         const __global float *matrixB,
                                         __global float *matrixC, uint widthA,
                                         uint widthB) {
  int x = get_global_id(0);
  int y = get_global_id(1);
  float sum = 0;
  for (int i = 0; i < widthA; i++) {
    sum += matrixA[i + y * widthA] * matrixB[x + i * widthB];
  }
  matrixC[x + y * widthB] = sum;
}
/* Each work-item computes 1 output double value*/
__kernel void matrixMultiplicationScalar_Double(const __global double *matrixA,
                                                const __global double *matrixB,
                                                __global double *matrixC,
                                                uint widthA, uint widthB) {
  int x = get_global_id(0);
  int y = get_global_id(1);
  double sum = 0;
  for (int i = 0; i < widthA; i++) {
    sum += matrixA[i + y * widthA] * matrixB[x + i * widthB];
  }
  matrixC[x + y * widthB] = sum;
}
/* Each work-item still computes 1 output float value*/
__kernel void matrixMultiplicationVector(const __global float *matrixA,
                                         const __global float *matrixB,
                                         __global float *matrixC, uint widthA,
                                         uint widthB) {
  int x = get_global_id(0);
  int y = get_global_id(1);
  float4 sum = (float4)0.0f;
  /* Vectorization reduces the width by a factor of 4 */
  for (int i = 0; i < widthA; i++) {
    float4 tempA0 = (float4)matrixA[i + y * widthA];
    float4 tempB0 = vload4(x, matrixB + i * widthB);
    sum += tempA0 * tempB0;
  }
  vstore4(sum, x, matrixC + y * widthB);
}
__kernel void matrixMultiplicationVector8(const __global float *matrixA,
                                          const __global float *matrixB,
                                          __global float *matrixC, uint widthA,
                                          uint widthB) {
  int x = get_global_id(0);
  int y = get_global_id(1);
  float8 sum = (float8)0.0f;
  /* Vectorization reduces the width by a factor of 8 */
  for (int i = 0; i < widthA; i++) {
    float8 tempA0 = (float8)matrixA[i + y * widthA];
    float8 tempB0 = vload8(x, matrixB + i * widthB);
    sum += tempA0 * tempB0;
  }
  vstore8(sum, x, matrixC + y * widthB);
}
#if 0
/* Each work-item computes 1 output float value*/
__kernel void matrixMultiplicationScalar_Img(
    __read_only image2d_t matrixA,
    __read_only image2d_t matrixB,
    __global float* matrixC,
    uint widthA, uint widthB)
{
    const sampler_t samplerType = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_LINEAR | CLK_ADDRESS_CLAMP;
    int x= get_global_id(0);
    int y= get_global_id(1);
    float sum = 0;
    for(int i = 0; i < widthA; i++)
    {
        //sum+=matrixA[i + y*widthA]*matrixB[x + i*widthB];
        float4  A = read_imagef( matrixA, samplerType, (float2)(i+0.5f,y+0.5f) );
        float4  B = read_imagef( matrixB, samplerType, (float2)(x+0.5f,i+0.5f) );
        sum += A.x*B.x;
    }
    matrixC[x + y* widthB] = sum;
}
/* Each work-item still computes 1 output float value*/
__kernel void matrixMultiplicationVector_Img(
    __read_only image2d_t matrixA,
    __read_only image2d_t matrixB,
    __global float* matrixC,
    uint widthA, uint widthB)
{
    const sampler_t samplerType = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_LINEAR | CLK_ADDRESS_CLAMP;
    int x = get_global_id(0);
    int y = get_global_id(1);
    float4 sum = (float4)0.0f;
    /* Vectorization reduces the width by a factor of 4 */
    for(int i = 0; i < widthA; i++)
    {
//        float4 tempA0 = (float4)matrixA[i + y* widthA];
//        float4 tempB0 = vload4(x,matrixB + i * widthB);
//        sum += tempA0*tempB0;
        float4  A = read_imagef( matrixA, samplerType, (float2)(i+0.5f,y+0.5f) );
        float4  B = read_imagef( matrixB, samplerType, (float2)(x+0.5f,i+0.5f) );
        sum += A.x*B;
    }
    vstore4(sum,x,matrixC+ y* widthB);
}
__kernel void matrixMultiplicationVector8_Img(
    __read_only image2d_t matrixA,
    __read_only image2d_t matrixB,
    __global float* matrixC,
    uint widthA, uint widthB)
{
    const sampler_t samplerType = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_LINEAR | CLK_ADDRESS_CLAMP;
    int x = get_global_id(0);
    int y = get_global_id(1);
    float8 sum = (float8)0.0f;
    uint8 mask = (uint8)(0, 1, 2, 3, 4, 5, 6, 7);
    /* Vectorization reduces the width by a factor of 4 */
    for(int i = 0; i < widthA; i++)
    {
//        float4 tempA0 = (float4)matrixA[i + y* widthA];
//        float4 tempB0 = vload4(x,matrixB + i * widthB);
//        sum += tempA0*tempB0;
        float4  A = read_imagef( matrixA, samplerType, (float2)(i+0.5f,y+0.5f) );
        float4  B = read_imagef( matrixB, samplerType, (float2)(x+0.5f,i+0.5f) );
        float4  A1 = read_imagef( matrixA, samplerType, (float2)(i+1.5f,y+0.5f) );
        float4  B1 = read_imagef( matrixB, samplerType, (float2)(x+1.5f,i+0.5f) );
        float8  AA1 = shuffle2(A, A1, mask);
        float8  BB1 = shuffle2(B, B1, mask);
        sum += AA1.s0*BB1;
    }
    vstore8(sum,x,matrixC+ y* widthB);
}
#endif
