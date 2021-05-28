kernel void fma(global int *result, global const int *a, global const int *b,
                global const int *c) {
  int index = get_global_id(0);
  result[index] = a[index] * b[index] + c[index];
}
