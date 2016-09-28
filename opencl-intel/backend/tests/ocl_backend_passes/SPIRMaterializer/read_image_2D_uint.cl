typedef read_only image2d_t image2d_uint;     //image2d_t where channel_data_type is CL_UNSIGNED_INT32

__kernel void sample_kernel( image2d_uint input, sampler_t sampler, __global int *results )
{
   int tidX = get_global_id(0), tidY = get_global_id(1);
   int offset = tidY*get_image_width(input) + tidX;
   int2 coords = (int2)(tidX, tidY);
   uint4 clr = read_imageui( input, coords );
   int4 test = (clr != read_imageui( input, sampler, coords ));
   if ( test.x || test.y || test.z || test.w )
      results[offset] = -1;
   else
      results[offset] = 0;
}
