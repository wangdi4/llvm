typedef struct __attribute__((packed)) {
  int a;
  int b;
  uchar c;
} KernelArg;

__kernel void main_kernel(
    __global uchar *buf_in, __global uchar *buf_out,
    __read_only image2d_t image2df_in, __write_only image2d_t image2df_out,
    __read_only image3d_t image3df_in, __read_only image2d_t image2di_in,
    __write_only image2d_t image2di_out, __read_only image3d_t image3di_in,
    __read_only image2d_t image2dui_in, __write_only image2d_t image2dui_out,
    __read_only image3d_t image3dui_in, __constant KernelArg *struct_in,
    __global KernelArg *struct_out, __constant int4 *vector_in,
    __global int4 *vector_out) {
  if (get_global_id(0) > 0) {
    return;
  }
  // read struct_in and write to struct_out
  int a = struct_in->a;
  int b = struct_in->b;
  uchar c = struct_in->c;
  struct_out->a = a;
  struct_out->b = b;
  struct_out->c = c;
  // read vector_in and write to vector_out
  int4 kernel_int_vector = (int4)*vector_in;
  *vector_out = (int4)kernel_int_vector;

  // reads to image2d_in, image3d_in and writes image2d_out
  int i, j, k;

  float4 read_float_vector;
  int4 read_int_vector;
  uint4 read_uint_vector;
  for (i = 0; i < 2; i++) {
    for (j = 0; j < 2; j++) {
      int2 coord = (int2)(i, j);
      const sampler_t samplerNearest =
          CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;
      read_float_vector = read_imagef(image2df_in, samplerNearest, coord);
      read_int_vector = read_imagei(image2di_in, samplerNearest, coord);
      read_uint_vector = read_imageui(image2dui_in, samplerNearest, coord);
      write_imagef(image2df_out, coord, read_float_vector);
      write_imagei(image2di_out, coord, read_int_vector);
      write_imageui(image2dui_out, coord, read_uint_vector);
      for (k = 0; k < 2; k++)

      {
        int4 coord2 = (int4)(i, j, k, 0);
        read_float_vector = read_imagef(image3df_in, samplerNearest, coord2);
        read_int_vector = read_imagei(image3di_in, samplerNearest, coord2);
        read_uint_vector = read_imageui(image3dui_in, samplerNearest, coord2);
      }
    }
  }
}