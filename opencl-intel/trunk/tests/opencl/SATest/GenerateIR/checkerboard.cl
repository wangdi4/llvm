// Eric Palmer - OpenCL port of the example from the Adobe Pixel Bender Toolkit
/*****************************************************************************
 *
 * Copyright (C) 2008 Adobe Systems Incorporated
 * All Rights Reserved.
 *
 * NOTICE:  All information contained  herein is,  and remains the property of
 * Adobe Systems Incorporated and its suppliers, if any.  The intellectual and
 * technical  concepts  contained  herein  are  proprietary  to  Adobe Systems
 * Incorporated  and  its suppliers  and may  be covered  by U.S.  and Foreign
 * Patents, patents in process, and are protected by trade secret or copyright
 * law.  Dissemination of this information or reproduction of this material is
 * strictly forbidden  unless prior  written permission is obtained from Adobe
 * Systems Incorporated.
 *
 *****************************************************************************/
// checkerboard: A simple example to demonstrate the use of the kernel language to 
//               generate images.

float4
evaluatePixel(
	float2 outCrd,
	const float2 checkerSize,
	const float4 color1,
	const float4 color2
	)
{
    float2 checkerLocation = floor(outCrd / checkerSize);

	const float2 f2_2 = (float2)2.0f;
    float2 modLocation = fmod(checkerLocation, f2_2);

	const float2 f0_0 = (float2)0.0f;
	const float2 f1_1 = (float2)1.0f;
	
    uint setColor1 = all(isequal(modLocation, f0_0)) || 
					 all(isequal(modLocation, f1_1));

    float4 dst = setColor1 ? color1 : color2;

    return dst;
}

__kernel void
checkerboard2D(
    __global float4 *output,
	const float2 checkerSize,
	const float4 color1,
	const float4 color2
	)
{

	// Pixel-level parallelism
	int gid0_col = (int)get_global_id(0);
	int gid1_row = (int)get_global_id(1);
	int imgWidth = (int)get_global_size(0);

	int index = gid1_row*imgWidth + gid0_col;
	
	float2 curCrd = { (float)gid0_col, (float)gid1_row };
	output[index] = evaluatePixel(curCrd, checkerSize, color1, color2);
}
