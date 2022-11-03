// XFAIL: win32

__kernel void test(__global short16 *in) {
  int id = get_global_id(0);
  in[id] = in[id] * in[id];
}
