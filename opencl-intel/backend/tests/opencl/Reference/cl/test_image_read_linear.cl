__kernel void sample_kernel_1D( read_only image1d_t input, sampler_t imageSampler, __global float *xOffsets, __global float4 *results )
{
   int tidX = get_global_id(0);
   int offset = tidX;
   float coord = (float)xOffsets[offset];
   results[offset] = read_imagef( input, imageSampler, coord );
}

__kernel void sample_kernel_1Darr( read_only image1d_array_t input, sampler_t imageSampler, __global float *xOffsets, __global float *yOffsets, __global float4 *results )
{
   int tidX = get_global_id(0), tidY = get_global_id(1);
   int offset = tidY*get_image_width(input) + tidX;
   float2 coords = (float2)( (float)( xOffsets[offset] ), (float)( yOffsets[offset] ) );
   results[offset] = read_imagef( input, imageSampler, coords );
}

__kernel void sample_kernel_2D( read_only image2d_t input, sampler_t imageSampler, __global float *xOffsets, __global float *yOffsets, __global float4 *results )
{
   int tidX = get_global_id(0), tidY = get_global_id(1);
   int offset = tidY*get_image_width(input) + tidX;
   float2 coords = (float2)( (float)( xOffsets[offset] ), (float)( yOffsets[offset] ) );
   results[offset] = read_imagef( input, imageSampler, coords );
}

__kernel void sample_kernel_2Darr( read_only image2d_array_t input, sampler_t imageSampler, __global float *xOffsets, __global float *yOffsets, __global float *zOffsets,  __global float4 *results )
{
   int tidX = get_global_id(0), tidY = get_global_id(1), tidZ = get_global_id(2);
   int offset = tidZ*get_image_width(input)*get_image_height(input) + tidY*get_image_width(input) + tidX;
   float4 coords = (float4)( xOffsets[offset], yOffsets[offset], zOffsets[offset], 0.0f );
   results[offset] = read_imagef( input, imageSampler, coords );
}

__kernel void sample_kernel_3D( read_only image3d_t input, sampler_t imageSampler, __global float *xOffsets, __global float *yOffsets, __global float *zOffsets,  __global float4 *results )
{
   int tidX = get_global_id(0), tidY = get_global_id(1), tidZ = get_global_id(2);
   int offset = tidZ*get_image_width(input)*get_image_height(input) + tidY*get_image_width(input) + tidX;
   float4 coords = (float4)( xOffsets[offset], yOffsets[offset], zOffsets[offset], 0.0f );
   results[offset] = read_imagef( input, imageSampler, coords );
}