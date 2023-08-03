__kernel void test(__global int *res, long y, long x) {
  if ((long)get_global_id(0) + y < x)
    atom_inc(res);
}
