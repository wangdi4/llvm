// Check results for uniform work_group add/min/max builtin

#define LOCAL_WORK_SIZE 4

__kernel void GroupBuiltinAdd(__global int *wg_reduce_add_i,
                              __global float *wg_reduce_add_f,
                              __global double *wg_reduce_add_d,
                              __global uint *wg_reduce_add_ui,
                              __global long *wg_reduce_add_l,
                              __global ulong *wg_reduce_add_ul) {
  int lid = get_local_id(0) + get_group_id(0) * LOCAL_WORK_SIZE;

  int temp_i = 1 + lid;
  float temp_f = 1.1 + lid;
  double temp_d = 1.1 + lid;
  uint temp_ui = 2 + lid;
  long temp_l = 3 + lid;
  ulong temp_ul = 4 + lid;

  // reduce builtin
  wg_reduce_add_i[lid] = work_group_reduce_add(temp_i);   // int
  wg_reduce_add_f[lid] = work_group_reduce_add(temp_f);   // float
  wg_reduce_add_d[lid] = work_group_reduce_add(temp_d);   // double
  wg_reduce_add_ui[lid] = work_group_reduce_add(temp_ui); // uint
  wg_reduce_add_l[lid] = work_group_reduce_add(temp_l);   // long
  wg_reduce_add_ul[lid] = work_group_reduce_add(temp_ul); // ulong
}

__kernel void GroupBuiltinMax(__global int *wg_reduce_min_i,
                              __global float *wg_reduce_min_f,
                              __global double *wg_reduce_min_d,
                              __global uint *wg_reduce_min_ui,
                              __global long *wg_reduce_min_l,
                              __global ulong *wg_reduce_min_ul) {
  int lid = get_local_id(0) + get_group_id(0) * LOCAL_WORK_SIZE;

  int temp_i = 2 + lid;
  float temp_f = 1.1 - lid;
  double temp_d = 1.1 - lid;
  uint temp_ui = lid + 1;
  long temp_l = 3 + lid;
  ulong temp_ul = 4 + lid;

  // reduce builtin
  wg_reduce_min_i[lid] = work_group_reduce_min(temp_i);   // int
  wg_reduce_min_f[lid] = work_group_reduce_min(temp_f);   // float
  wg_reduce_min_d[lid] = work_group_reduce_min(temp_d);   // double
  wg_reduce_min_ui[lid] = work_group_reduce_min(temp_ui); // uint
  wg_reduce_min_l[lid] = work_group_reduce_min(temp_l);   // long
  wg_reduce_min_ul[lid] = work_group_reduce_min(temp_ul); // ulong
}

__kernel void GroupBuiltinMin(__global int *wg_reduce_max_i,
                              __global float *wg_reduce_max_f,
                              __global double *wg_reduce_max_d,
                              __global uint *wg_reduce_max_ui,
                              __global long *wg_reduce_max_l,
                              __global ulong *wg_reduce_max_ul) {
  int lid = get_local_id(0) + get_group_id(0) * LOCAL_WORK_SIZE;

  int temp_i = 2 + lid;
  float temp_f = 1.1 - lid;
  double temp_d = 1.1 - lid;
  uint temp_ui = lid + 1;
  long temp_l = 3 + lid;
  ulong temp_ul = 4 + lid;

  // reduce builtin
  wg_reduce_max_i[lid] = work_group_reduce_max(temp_i);   // int
  wg_reduce_max_f[lid] = work_group_reduce_max(temp_f);   // float
  wg_reduce_max_d[lid] = work_group_reduce_max(temp_d);   // double
  wg_reduce_max_ui[lid] = work_group_reduce_max(temp_ui); // uint
  wg_reduce_max_l[lid] = work_group_reduce_max(temp_l);   // long
  wg_reduce_max_ul[lid] = work_group_reduce_max(temp_ul); // ulong
}
