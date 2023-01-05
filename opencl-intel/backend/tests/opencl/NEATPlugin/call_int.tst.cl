int func(__global int *b) {
  uint tid = get_global_id(0);
  b[tid] = 3;
  return 3;
}

__kernel void call(__global int *a) { func(a); }
