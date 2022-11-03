__kernel void f(__global double *data, long scalar) {
  int lid = get_local_id(0);
  data[lid] = data[lid] * scalar;
}
