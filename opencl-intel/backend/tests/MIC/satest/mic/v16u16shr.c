
__kernel void test(__global ushort16 *in) {
  int id = get_global_id(0);
  in[id] = in[id] >> in[id];
}
