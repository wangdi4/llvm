
__kernel void fma(__global int *result, __global const int *a,
                  __global const int *b, __global const int *c) {

  int index = get_global_id(0);
  result[index] = a[index] * b[index] + c[index];
}
