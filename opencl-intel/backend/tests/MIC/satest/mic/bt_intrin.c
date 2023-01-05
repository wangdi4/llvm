__kernel void bt_intrin(__global int *shift, __global int *mask,
                        __global int *out) {
  int id = get_global_id(0);
  int num = 1 << shift[id];
  int result = num & mask[id];
  out[id] = result == 0;
}
