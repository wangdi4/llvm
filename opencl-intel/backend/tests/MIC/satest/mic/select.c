// TODO: Enable test_select function when conditional move instruction is fixed.
#if 0
__kernel void test_select(__global long *out, __global const long *a, __global const long *b) {
    int index = get_global_id(0);
    if (a[index] < b[index])
    {
        out[index] = a[index];
    }
    else
    {
        out[index] = b[index];
    }
}
#endif

__kernel void test_select2(__global long2 *out, __global const long2 *a,
                           __global const long2 *b) {
  int index = get_global_id(0);
  if (a[index].x < b[index].x) {
    out[index] = a[index];
  } else {
    out[index] = b[index];
  }
}

__kernel void test_select3(__global long3 *out, __global const long3 *a,
                           __global const long3 *b) {
  int index = get_global_id(0);
  if (a[index].x < b[index].x) {
    out[index] = a[index];
  } else {
    out[index] = b[index];
  }
}

__kernel void test_select4(__global long4 *out, __global const long4 *a,
                           __global const long4 *b) {
  int index = get_global_id(0);
  if (a[index].x < b[index].x) {
    out[index] = a[index];
  } else {
    out[index] = b[index];
  }
}

__kernel void test_select8(__global long8 *out, __global const long8 *a,
                           __global const long8 *b) {
  int index = get_global_id(0);
  if (a[index].x < b[index].x) {
    out[index] = a[index];
  } else {
    out[index] = b[index];
  }
}
