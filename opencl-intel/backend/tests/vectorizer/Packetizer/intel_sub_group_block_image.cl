__kernel void test(read_write image2d_t image) {
  int2 coord = (int2)(1,1);
  uint2 res = intel_sub_group_block_read2(image, coord);
  intel_sub_group_block_write2(image, coord, res);
  ushort2 res1 = intel_sub_group_block_read_us2(image, coord);
  intel_sub_group_block_write_us2(image, coord, res1);
}
