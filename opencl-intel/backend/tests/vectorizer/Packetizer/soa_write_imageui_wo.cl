__kernel void soa_read_imageui(__write_only image2d_t img, __global uint4 * in, uint randomId) {
  uint lid0 = get_local_id(0);
  uint lid1 = get_local_id(1);
  uint4 color = in[lid0 + get_local_size(0)*lid1];
  // non-masked read_imageui
  write_imageui(img, (int2)(lid0, lid1), color);
  // masked read_imageui
  if(lid0 % randomId == 0) {
    write_imageui(img, (int2)(lid0, lid1), color * 2);
  }
}
