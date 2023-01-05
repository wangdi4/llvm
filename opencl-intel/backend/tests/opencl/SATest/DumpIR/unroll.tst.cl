
__kernel void f(__global int *data) {
#pragma unroll
  for (int i = 0; i < 4; ++i) {
    if (data[i]) {
      data[i + 4] = sin((float)i);
    }
  }
}

__kernel void g(__global int *data) {
  for (int i = 0; i < 4; ++i) {
    if (data[i]) {
      data[i + 4] = cos((float)i);
    }
  }
}
