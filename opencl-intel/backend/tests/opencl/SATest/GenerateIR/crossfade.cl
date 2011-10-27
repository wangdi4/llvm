// Phil Kerly

float4 evaluatePixel(float4 fColor, float4 bColor, const float intensity)
{
#if 0
	// Fails on Intel SDK alpha (intel_ocl_sdk_vista_win7_ia32_1.0.1.6154)
	return mix(fColor, bColor, intensity);
#else
	float4 vIntensity = (float4)intensity;
	return mix(fColor, bColor, vIntensity);
#endif
}


__kernel void
crossfade_GPU(
    __global float4 *front, 
    __global float4 *back, 
    __global float4 *output,
    const float intensity
    )
{
	int gid0_curPix = get_global_id(0);

	output[gid0_curPix] = evaluatePixel(front[gid0_curPix], back[gid0_curPix], intensity);
}

__kernel void
crossfade(
    __global float4 *front, 
    __global float4 *back, 
    __global float4 *output,
    const float intensity,
    const uint pixelCountPerGlobalID
    )
{
	size_t global_id = get_global_id(0);
	size_t index = pixelCountPerGlobalID*global_id;
	
#if 0
	printf("global_size = %d\n", get_global_size(0));
	printf("global_id = %d\n", global_id);		
	
	printf("working on %d pixels at index %d\n\n", pixelCountPerGlobalID, index);
	
	printf("input pointer = %p, output pointer = %p\n", front, output);
#endif

	size_t lastIndex = index+pixelCountPerGlobalID;
	for (; index < lastIndex; ++index)	{
		output[index] = evaluatePixel(front[index], back[index], intensity); //mix(front[index], back[index], vIntensity);
#if 0
		if (index < 20) printf("front = %f, back = %f, output = %f\n", front[index].x, back[index].x, output[index].x);
#endif
	}
	
}
