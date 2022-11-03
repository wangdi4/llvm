__kernel void main_kernel(__global uchar *buf_in, __global uchar *buf_out) {
  float4 fl4 = (float4)(1.1f, 2.2f, 3.3f, 4.4f);
  int2 ii2 = (int2)(get_global_id(0), 9);
  fl4.w += (float)get_global_id(0);

  if (get_global_id(0) == 3)
    printf("%d %6.2v4hlf - that's all\n", ii2.x, fl4);
}
