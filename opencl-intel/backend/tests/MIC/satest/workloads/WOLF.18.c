#define BLOCK_WIDTH 8
#if 0  // comment out GPU version of kernel that could be used in future
//The scalar version, optimized for GPU
__kernel void DCT(__global float* output, __global float* input, __global float* dct, const uint width)
{
    uint globalIdx = get_global_id(0);
    uint globalIdy = get_global_id(1);
    uint groupIdx  = get_group_id(0);
    uint groupIdy  = get_group_id(1);
  __local float inter[BLOCK_WIDTH * BLOCK_WIDTH];
    uint i  = get_local_id(1);
  uint k1, k2, n1, n2;
    uint idx = 0;
  uint index1 = 0;
  uint index2 = 0;
  float acc[BLOCK_WIDTH] = {0.0f};
  k1 = i;
  //Calculate a single line from DCT^T * IN
  index1 =  k1 * BLOCK_WIDTH;
  index2 =  ( groupIdy * BLOCK_WIDTH ) * width + groupIdx * BLOCK_WIDTH;
  for( int ind = 0; ind < BLOCK_WIDTH; ind++ )
  {
    for(n1 = 0; n1 < BLOCK_WIDTH; n1++)
    {
      acc[ind] += dct[index1 + n1] * input[index2 + n1];  //acc += dct(k1,n1) * in(n1,n2)
    }
    index2 += width;
  }
  idx = k1 * BLOCK_WIDTH;
  inter[idx + 0] = acc[0];  //inter[k1][0]
  inter[idx + 1] = acc[1];  //inter[k1][1]
  inter[idx + 2] = acc[2];  //inter[k1][2]
  inter[idx + 3] = acc[3];  //inter[k1][3]
  inter[idx + 4] = acc[4];  //inter[k1][4]
  inter[idx + 5] = acc[5];  //inter[k1][5]
  inter[idx + 6] = acc[6];  //inter[k1][6]
  inter[idx + 7] = acc[7];  //inter[k1][7]
    barrier(CLK_LOCAL_MEM_FENCE);
  for(int ind = 0; ind < BLOCK_WIDTH; ind++)
  {
    acc[ind] = 0.0f;
  }
  k2 = i;
  //Calculate a single line from (DCT^T * IN) * DCT
  index1 =  0;
  index2 =  k2 * BLOCK_WIDTH;
  for( int ind = 0; ind < BLOCK_WIDTH; ind++ )
  {
    for(n2 = 0; n2 < BLOCK_WIDTH; n2++)
    {
      acc[ind] += inter[index1 + n2] * dct[index2 + n2];  //acc += dct(k2,n2) * inter(k1,n2)
    }
    index1 += BLOCK_WIDTH;
  }
  //write results to output
  idx = ( groupIdy * BLOCK_WIDTH + k2 ) * width + groupIdx * BLOCK_WIDTH;
    output[idx + 0] = acc[0];
  output[idx + 1] = acc[1];
  output[idx + 2] = acc[2];
  output[idx + 3] = acc[3];
  output[idx + 4] = acc[4];
  output[idx + 5] = acc[5];
  output[idx + 6] = acc[6];
  output[idx + 7] = acc[7];
}
//The vector version, optimized for GPU
__kernel void DCT_VECTOR(__global float8* output, __global float8* input, __global float8* dct, const uint width)
{
    uint globalIdx = get_global_id(0);
    uint globalIdy = get_global_id(1);
    uint groupIdx  = get_group_id(0);
    uint groupIdy  = get_group_id(1);
  __local float8 inter[BLOCK_WIDTH];
    uint i  = get_local_id(1);
  uint k1, k2, n1, n2;
    uint idx = 0;
    float8 acc = 0.0f;
  float8 temp;
  int step = width / BLOCK_WIDTH;
  k1 = i;
  //Calculate a single line from DCT^T * IN
  uint index1 = k1;
  uint index2 =  ( groupIdy * BLOCK_WIDTH ) * step + groupIdx;
  temp = dct[index1] * input[index2];
  acc.s0 = temp.s0 + temp.s1 + temp.s2 + temp.s3 + temp.s4 + temp.s5 + temp.s6 + temp.s7;
  index2 += step;
  temp = dct[index1] * input[index2];
  acc.s1 = temp.s0 + temp.s1 + temp.s2 + temp.s3 + temp.s4 + temp.s5 + temp.s6 + temp.s7;
  index2 += step;
  temp = dct[index1] * input[index2];
  acc.s2 = temp.s0 + temp.s1 + temp.s2 + temp.s3 + temp.s4 + temp.s5 + temp.s6 + temp.s7;
  index2 += step;
  temp = dct[index1] * input[index2];
  acc.s3 = temp.s0 + temp.s1 + temp.s2 + temp.s3 + temp.s4 + temp.s5 + temp.s6 + temp.s7;
  index2 += step;
  temp = dct[index1] * input[index2];
  acc.s4 = temp.s0 + temp.s1 + temp.s2 + temp.s3 + temp.s4 + temp.s5 + temp.s6 + temp.s7;
  index2 += step;
  temp = dct[index1] * input[index2];
  acc.s5 = temp.s0 + temp.s1 + temp.s2 + temp.s3 + temp.s4 + temp.s5 + temp.s6 + temp.s7;
  index2 += step;
  temp = dct[index1] * input[index2];
  acc.s6 = temp.s0 + temp.s1 + temp.s2 + temp.s3 + temp.s4 + temp.s5 + temp.s6 + temp.s7;
  index2 += step;
  temp = dct[index1] * input[index2];
  acc.s7 = temp.s0 + temp.s1 + temp.s2 + temp.s3 + temp.s4 + temp.s5 + temp.s6 + temp.s7;
  idx = k1;
    inter[idx] = acc; //inter[k1]
    barrier(CLK_LOCAL_MEM_FENCE);
  k2 = i;
  //Calculate a single line from (DCT^T * IN) * DCT
  index2 =  k2;
  temp = inter[0] * dct[index2];
  acc.s0 = temp.s0 + temp.s1 + temp.s2 + temp.s3 + temp.s4 + temp.s5 + temp.s6 + temp.s7;
  temp = inter[1] * dct[index2];
  acc.s1 = temp.s0 + temp.s1 + temp.s2 + temp.s3 + temp.s4 + temp.s5 + temp.s6 + temp.s7;
  temp = inter[2] * dct[index2];
  acc.s2 = temp.s0 + temp.s1 + temp.s2 + temp.s3 + temp.s4 + temp.s5 + temp.s6 + temp.s7;
  temp = inter[3] * dct[index2];
  acc.s3 = temp.s0 + temp.s1 + temp.s2 + temp.s3 + temp.s4 + temp.s5 + temp.s6 + temp.s7;
  temp = inter[4] * dct[index2];
  acc.s4 = temp.s0 + temp.s1 + temp.s2 + temp.s3 + temp.s4 + temp.s5 + temp.s6 + temp.s7;
  temp = inter[5] * dct[index2];
  acc.s5 = temp.s0 + temp.s1 + temp.s2 + temp.s3 + temp.s4 + temp.s5 + temp.s6 + temp.s7;
  temp = inter[6] * dct[index2];
  acc.s6 = temp.s0 + temp.s1 + temp.s2 + temp.s3 + temp.s4 + temp.s5 + temp.s6 + temp.s7;
  temp = inter[7] * dct[index2];
  acc.s7 = temp.s0 + temp.s1 + temp.s2 + temp.s3 + temp.s4 + temp.s5 + temp.s6 + temp.s7;
  //write results to output
  idx = ( groupIdy * BLOCK_WIDTH + k2 ) * step + groupIdx;
    output[idx] = acc;
}
//The same as DCT_VECTOR with "dot" builtin function
__kernel void DCT_VECTOR_DOT(__global float8* output, __global float8* input, __global float8* dct, const uint width)
{
    uint globalIdx = get_global_id(0);
    uint globalIdy = get_global_id(1);
    uint groupIdx  = get_group_id(0);
    uint groupIdy  = get_group_id(1);
  __local float8 inter[BLOCK_WIDTH];
    uint i  = get_local_id(1);
  uint k1, k2, n1, n2;
    uint idx = 0;
    float8 acc = 0.0f;
  int step = width / BLOCK_WIDTH;
  k1 = i;
  //Calculate a single line from DCT^T * IN
  uint index1 = k1;
  uint index2 =  ( groupIdy * BLOCK_WIDTH ) * step + groupIdx;
  acc.s0 = dot(dct[index1].lo, input[index2].lo) + dot(dct[index1].hi, input[index2].hi);
  index2 += step;
  acc.s1 = dot(dct[index1].lo, input[index2].lo) + dot(dct[index1].hi, input[index2].hi);
  index2 += step;
  acc.s2 = dot(dct[index1].lo, input[index2].lo) + dot(dct[index1].hi, input[index2].hi);
  index2 += step;
  acc.s3 = dot(dct[index1].lo, input[index2].lo) + dot(dct[index1].hi, input[index2].hi);
  index2 += step;
  acc.s4 = dot(dct[index1].lo, input[index2].lo) + dot(dct[index1].hi, input[index2].hi);
  index2 += step;
  acc.s5 = dot(dct[index1].lo, input[index2].lo) + dot(dct[index1].hi, input[index2].hi);
  index2 += step;
  acc.s6 = dot(dct[index1].lo, input[index2].lo) + dot(dct[index1].hi, input[index2].hi);
  index2 += step;
  acc.s7 = dot(dct[index1].lo, input[index2].lo) + dot(dct[index1].hi, input[index2].hi);
  idx = k1;
    inter[idx] = acc; //inter[k1]
    barrier(CLK_GLOBAL_MEM_FENCE);
  k2 = i;
  //Calculate a single line from (DCT^T * IN) * DCT
  index2 =  k2;
  acc.s0 = dot(inter[0].lo, dct[index2].lo) + dot(inter[0].hi, dct[index2].hi);
  acc.s1 = dot(inter[1].lo, dct[index2].lo) + dot(inter[1].hi, dct[index2].hi);
  acc.s2 = dot(inter[2].lo, dct[index2].lo) + dot(inter[2].hi, dct[index2].hi);
  acc.s3 = dot(inter[3].lo, dct[index2].lo) + dot(inter[3].hi, dct[index2].hi);
  acc.s4 = dot(inter[4].lo, dct[index2].lo) + dot(inter[4].hi, dct[index2].hi);
  acc.s5 = dot(inter[5].lo, dct[index2].lo) + dot(inter[5].hi, dct[index2].hi);
  acc.s6 = dot(inter[6].lo, dct[index2].lo) + dot(inter[6].hi, dct[index2].hi);
  acc.s7 = dot(inter[7].lo, dct[index2].lo) + dot(inter[7].hi, dct[index2].hi);
  //write results to output
  idx = ( groupIdy * BLOCK_WIDTH + k2 ) * step + groupIdx;
    output[idx] = acc;
}
#endif // end GPU version of kernels that could be used in the future
// The scalar version, optimized for CPU
__kernel void DCT_CPU(__global float *output, __global float *input,
                      __global float *dct, const uint width) {
  uint groupIdx = get_global_id(0);
  uint groupIdy = get_global_id(1);
  uint k1, k2, n1, n2;
  float inter[BLOCK_WIDTH * BLOCK_WIDTH] = {0};
  uint step = width;
  uint inputIndex = 0;
  uint dctIndex = 0;
  uint interIndex = 0;
  uint outputIndex = 0;
  inputIndex = (groupIdy * BLOCK_WIDTH) * width + groupIdx * BLOCK_WIDTH;
  // Calculate DCT^T * IN
  for (n2 = 0; n2 < BLOCK_WIDTH; n2++) {
    dctIndex = 0;
    interIndex = 0;
    for (k1 = 0; k1 < BLOCK_WIDTH; k1++) {
      for (n1 = 0; n1 < BLOCK_WIDTH; n1++) {
        // inter(n2,k1) += dct(k1,n1) * in(n1,n2)
        inter[interIndex + n2] += dct[dctIndex + n1] * input[inputIndex + n1];
      }
      dctIndex += BLOCK_WIDTH;
      interIndex += BLOCK_WIDTH;
    }
    inputIndex += step;
  }
  dctIndex = 0;
  outputIndex = (groupIdy * BLOCK_WIDTH) * width + groupIdx * BLOCK_WIDTH;
  // Calculate (DCT^T * IN) * DCT
  for (k2 = 0; k2 < BLOCK_WIDTH; k2++) {
    interIndex = 0;
    for (k1 = 0; k1 < BLOCK_WIDTH; k1++) {
      output[outputIndex + k1] = 0;
      for (n2 = 0; n2 < BLOCK_WIDTH; n2++) {
        // out(k1,k2) = dct(k2,n2) * inter(n2,k1)
        output[outputIndex + k1] += dct[dctIndex + n2] * inter[interIndex + n2];
      }
      interIndex += BLOCK_WIDTH;
    }
    dctIndex += BLOCK_WIDTH;
    outputIndex += step;
  }
}
// The vector version, optimized for CPU
__kernel void DCT_CPU_VECTOR(__global float *output, __global float8 *input,
                             __global float8 *dct, const uint width) {
  uint groupIdx = get_global_id(0);
  uint groupIdy = get_global_id(1);
  uint k1, k2, n1, n2;
  float acc[BLOCK_WIDTH * BLOCK_WIDTH];
  float8 inter[BLOCK_WIDTH];
  float8 temp;
  uint step = width / BLOCK_WIDTH;
  uint inputIndex = 0;
  uint dctIndex = 0;
  uint interIndex = 0;
  uint outputIndex = 0;
  inputIndex = (groupIdy * BLOCK_WIDTH) * width / BLOCK_WIDTH + groupIdx;
  // Calculate DCT^T * IN
  for (n2 = 0; n2 < BLOCK_WIDTH; n2++) {
    interIndex = 0;
    for (k1 = 0; k1 < BLOCK_WIDTH; k1++) {
      // inter(n2,k1) += dct(k1,:) * in(:,n2)
      temp = dct[k1] * input[inputIndex];
      acc[interIndex + n2] = temp.s0 + temp.s1 + temp.s2 + temp.s3 + temp.s4 +
                             temp.s5 + temp.s6 + temp.s7;
      interIndex += BLOCK_WIDTH;
    }
    inputIndex += step;
  }
  for (int i = 0; i < BLOCK_WIDTH; i++) {
    inter[i] = vload8(i, &acc[0]);
  }
  outputIndex = (groupIdy * BLOCK_WIDTH) * width + groupIdx * BLOCK_WIDTH;
  // Calculate (DCT^T * IN) * DCT
  for (k2 = 0; k2 < BLOCK_WIDTH; k2++) {
    for (k1 = 0; k1 < BLOCK_WIDTH; k1++) {
      // out(k1,k2) = dct(k2,:) * inter(:,k1)
      temp = dct[k2] * inter[k1];
      output[outputIndex + k1] = temp.s0 + temp.s1 + temp.s2 + temp.s3 +
                                 temp.s4 + temp.s5 + temp.s6 + temp.s7;
    }
    outputIndex += width;
  }
}
__kernel void DCT_CPU_VECTOR_AVX(__global float *output, __global float8 *input,
                                 __global float8 *dct, const uint width) {
  uint groupIdx = get_global_id(0);
  uint groupIdy = get_global_id(1);
  uint k1, k2, n1, n2;
  float acc[BLOCK_WIDTH * BLOCK_WIDTH];
  float8 inter[BLOCK_WIDTH];
  float8 temp;
  uint step = width / BLOCK_WIDTH;
  uint inputIndex = 0;
  uint dctIndex = 0;
  uint interIndex = 0;
  uint outputIndex = 0;
  inputIndex = (groupIdy * BLOCK_WIDTH) * width / BLOCK_WIDTH + groupIdx;
  // Calculate DCT^T * IN
  for (n2 = 0; n2 < BLOCK_WIDTH; n2++) {
    interIndex = 0;
    for (k1 = 0; k1 < BLOCK_WIDTH; k1++) {
      float4 tmp0 = ((__global float4 *)dct)[k1 * 2];
      float4 tmp1 = ((__global float4 *)input)[inputIndex * 2];
      float4 tmp2 = ((__global float4 *)dct)[k1 * 2 + 1];
      float4 tmp3 = ((__global float4 *)input)[inputIndex * 2 + 1];
      acc[interIndex + n2] = dot(tmp0, tmp1) + dot(tmp2, tmp3);
      interIndex += BLOCK_WIDTH;
    }
    inputIndex += step;
  }
  for (int i = 0; i < BLOCK_WIDTH; i++) {
    inter[i] = vload8(i, &acc[0]);
  }
  outputIndex = (groupIdy * BLOCK_WIDTH) * width + groupIdx * BLOCK_WIDTH;
  // Calculate (DCT^T * IN) * DCT
  for (k2 = 0; k2 < BLOCK_WIDTH; k2++) {
    for (k1 = 0; k1 < BLOCK_WIDTH; k1++) {
      float4 tmp0 = ((__global float4 *)dct)[k2 * 2];
      float4 tmp1 = ((float4 *)inter)[k1 * 2];
      float4 tmp2 = ((__global float4 *)dct)[k2 * 2 + 1];
      float4 tmp3 = ((float4 *)inter)[k1 * 2 + 1];
      output[outputIndex + k1] = dot(tmp0, tmp1) + dot(tmp2, tmp3);
    }
    outputIndex += width;
  }
}
