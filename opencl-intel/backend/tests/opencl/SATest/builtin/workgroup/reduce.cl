int __attribute__((overloadable)) work_group_reduce_bitwise_and(int);
int __attribute__((overloadable)) work_group_reduce_bitwise_or(int);
int __attribute__((overloadable)) work_group_reduce_bitwise_xor(int);

int __attribute__((overloadable)) work_group_reduce_logical_and(int);
int __attribute__((overloadable)) work_group_reduce_logical_or(int);
int __attribute__((overloadable)) work_group_reduce_logical_xor(int);

kernel void reduce_integer(__global int *bit_and_data,
                           __global int *bit_or_data,
                           __global int *bit_xor_data,
                           __global int *lgc_and_data,
                           __global int *lgc_or_data,
                           __global int *lgc_xor_data, const __global int *in) {
  int gid = get_global_id(0);

  bit_and_data[gid] = work_group_reduce_bitwise_and(in[gid]);
  bit_or_data[gid] = work_group_reduce_bitwise_or(in[gid]);
  bit_xor_data[gid] = work_group_reduce_bitwise_xor(in[gid]);

  lgc_and_data[gid] = work_group_reduce_logical_and(in[gid]);
  lgc_or_data[gid] = work_group_reduce_logical_or(in[gid]);
  lgc_xor_data[gid] = work_group_reduce_logical_xor(in[gid]);
}
