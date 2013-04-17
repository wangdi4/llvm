__kernel void sample_kernel( read_only image3d_t input, sampler_t sampler, __global int *results )
{
   int tidX = get_global_id(0), tidY = get_global_id(1), tidZ = get_global_id(2);
   int offset = tidZ*get_image_width(input)*get_image_height(input) + tidY*get_image_width(input) + tidX;
   int4 coords = (int4)( tidX, tidY, tidZ, 0 );
   float4 clr = read_imagef( input, coords );
   int4 test = (clr != read_imagef( input, sampler, coords ));
   if ( test.x || test.y || test.z || test.w )
      results[offset] = -1;
   else
      results[offset] = 0;
}