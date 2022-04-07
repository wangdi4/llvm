__kernel void soa_read_imageui(image2d_t img, __global uint4 * out, uint randomId) {
  uint lid0 = get_local_id(0);
  uint lid1 = get_local_id(1);
  sampler_t smp = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;
  // non-masked read_imageui
  uint4 res  = read_imageui(img, (int2)(lid0, lid1));
  res += read_imageui(img, smp, (int2)(lid0, lid1));
  // masked read_imageui
  if(lid0 % randomId == 0) {
    res  += read_imageui(img, (int2)(lid1, lid0));
    res += read_imageui(img, smp, (int2)(lid1, lid0));
  }

  out[lid0 + get_local_size(0)*lid1] = res;
}
