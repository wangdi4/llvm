__kernel void main_kernel(__global float4 *inputImage, __global float4 *output,
                          const int width, const int height) {
  int2 imgSize = {width, height};

  float f = inputImage[6].w;
  int i;
  for (i = 0; i < width * height; ++i) {
    f += inputImage[i].w;
    output[i] = inputImage[i];
  }

  float4 fl8 = output[8];
  float4 fl9 = output[9];

  int temp = 1;
}
