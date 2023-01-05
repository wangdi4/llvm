#define SUB_GROUP_SIZE 8

__attribute__((intel_reqd_sub_group_size(SUB_GROUP_SIZE))) __kernel void
fmax_fmin_test(__global float *data1, __global float *data2) {
  int sglid = get_sub_group_local_id();

  float temp_f = 1.0f;

  unsigned int nan_temp_u = 0xffd9ac74;
  float nan_temp_f = *(float *)(&nan_temp_u);

  data1[sglid] = fmax(data2[sglid] + temp_f, data1[sglid] + nan_temp_f);
  data2[sglid] = fmin(data2[sglid] + temp_f, data1[sglid] + nan_temp_f);
}
