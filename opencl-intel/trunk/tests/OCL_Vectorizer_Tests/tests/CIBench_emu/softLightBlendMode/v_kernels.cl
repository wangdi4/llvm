// improve loop
__kernel void programMany_3(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, float4 l2, float4 l3, read_only image2d_t t0, sampler_t t_sampler0, read_only image2d_t t1, sampler_t t_sampler1)
{
	const int Many = 1024;
	const float4 p0  = (float4)( 0x1.0c6f7ap-20, 0x1p+0, 0x1p-1, 0x1p+1 );
	const float4 p1  = (float4)( 0x1.8p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
	float4       f0_x, o0_x, r0_x, r1_x, r2_x, r3_x, r4_x, r5_x, r6_x;
	float4       f0_y, o0_y, r0_y, r1_y, r2_y, r3_y, r4_y, r5_y, r6_y;
	float4       f0_z, o0_z, r0_z, r1_z, r2_z, r3_z, r4_z, r5_z, r6_z;
	float4       f0_w, o0_w, r0_w, r1_w, r2_w, r3_w, r4_w, r5_w, r6_w;
	float4     input_x0[Many/4], input_y0[Many/4], input_z0[Many/4], input_w0[Many/4], output_x[Many/4], output_y[Many/4], output_z[Many/4], output_w[Many/4];
	float4     input_x1[Many/4], input_y1[Many/4], input_z1[Many/4], input_w1[Many/4];

	int orig_loc_x = get_global_id(0)*Many;
	int tmp = orig_loc_x;
	int loc_y = get_global_id(1);
	int count = 0;

       	int4 loc_x = tmp + (int4)(0, 1, 2, 3);

	f0_x = st_origin.x + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.x + ((float)loc_y + 0.5f) * st_delta.z;
	f0_y = st_origin.y + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.y + ((float)loc_y + 0.5f) * st_delta.w;
	f0_z = 0.0f;
	f0_w = 0.0f;

	r5_x = f0_x;
	r5_y = f0_y;
	r5_z = f0_z;
	r5_w = f0_w;

	r1_x = r5_x * l0.x + r5_y * l0.y + l0.w;       
	r1_y = r5_x * l1.x + r5_y * l1.y + l1.w;

	r4_x = r1_x;
	r4_y = r1_y;
	r4_z = r1_z;
	r4_w = r1_w;

	{
		float2 stride;
		stride.x = r4_x.y - r4_x.x;
		stride.y = r4_y.y - r4_y.x;
		__async_work_group_stream_from_image(t0, t_sampler0, (float2)(r4_x.x, r4_y.x), stride, Many, input_x0, input_y0, input_z0, input_w0);
	}

        r6_x = r5_x * l2.x + r5_y * l2.y + l2.w;
        r6_y = r5_x * l3.x + r5_y * l3.y + l3.w;

	r0_x = r6_x;
	r0_y = r6_y;
	r0_z = r6_z;
	r0_w = r6_w;

	{
		float2 stride;
		stride.x = r0_x.y - r0_x.x;
		stride.y = r0_y.y - r0_y.x;
		__async_work_group_stream_from_image(t1, t_sampler1, (float2)(r0_x.x, r0_y.x), stride, Many, input_x1, input_y1, input_z1, input_w1);
	}

	//for (count = 0; count < Many/4; ++count)
	do
	{
		// extract the inputs you currently require
		r2_x = input_x0[count];
		r2_y = input_y0[count];
		r2_z = input_z0[count];
		r2_w = input_w0[count];
	
		// extract the inputs you currently require
		r3_x = input_x1[count];
		r3_y = input_y1[count];
		r3_z = input_z1[count];
		r3_w = input_w1[count];

		r1_w = max(r3_w, p0.x);
		r4_w = half_recip(r1_w);
		r5_w = r3_w;

		r5_x = r3_x*r4_w;
		r5_y = r3_y*r4_w;
		r5_z = r3_z*r4_w;

		r6_w = max(r2_w, p0.x);
		r0_w = half_recip(r6_w);
		r1_w = r2_w;

		r1_x = r2_x*r0_w;
		r1_y = r2_y*r0_w;
		r1_z = r2_z*r0_w;

		r4_x = p0.y-r1_w;
		r4_y = p0.y-r1_w;
		r4_z = p0.y-r1_w;
		r4_w = p0.y-r1_w;

		r6_x = r4_x*r3_w + r1_w;
		r6_y = r4_y*r3_w + r1_w;
		r6_z = r4_z*r3_w + r1_w;
		r6_w = r4_w*r3_w + r1_w;

		r0_w = r6_x;

		r2_x = r1_x-p0.z;
		r2_y = r1_y-p0.z;
		r2_z = r1_z-p0.z;
		r2_w = r1_w-p0.z;

		r4_x = p0.y-r1_x;
		r4_y = p0.y-r1_y;
		r4_z = p0.y-r1_z;
		r4_w = p0.y-r1_w;

		r6_x = r4_x*p0.w;
		r6_y = r4_y*p0.w;
		r6_z = r4_z*p0.w;
		r6_w = r4_w*p0.w;

		r4_x = half_powr(r5_x, r6_x);
		r4_y = half_powr(r5_y, r6_y);
		r4_z = half_powr(r5_z, r6_z);
		r4_w = half_powr(r5_w, r6_w);

		r6_x = p1.x-r1_x;
		r6_y = p1.x-r1_y;
		r6_z = p1.x-r1_z;
		r6_w = p1.x-r1_w;

		r5_x = half_powr(r5_x, r6_x);
		r5_y = half_powr(r5_y, r6_y);
		r5_z = half_powr(r5_z, r6_z);
		r5_w = half_powr(r5_w, r6_w);

		r4_x = clamp(select(r5_x,r4_x, isless(r2_x, 0.0f)), 0.0f, 1.0f);
		r4_y = clamp(select(r5_y,r4_y, isless(r2_y, 0.0f)), 0.0f, 1.0f);
		r4_z = clamp(select(r5_z,r4_z, isless(r2_z, 0.0f)), 0.0f, 1.0f);
		r4_w = clamp(select(r5_w,r4_w, isless(r2_w, 0.0f)), 0.0f, 1.0f);

		r2_x = fabs(r3_w);
		r2_y = fabs(r3_w);
		r2_z = fabs(r3_w);
		r2_w = fabs(r3_w);

		r4_x = mix(r1_x,r4_x, r3_w);
		r4_y = mix(r1_y,r4_y, r3_w);
		r4_z = mix(r1_z,r4_z, r3_w);
		r4_w = mix(r1_w,r4_w, r3_w);

		r2_x = select(r1_x,r4_x, isless(-r2_x, 0.0f));
		r2_y = select(r1_y,r4_y, isless(-r2_y, 0.0f));
		r2_z = select(r1_z,r4_z, isless(-r2_z, 0.0f));
		r2_w = select(r1_w,r4_w, isless(-r2_w, 0.0f));

		r0_x = mix(r3_x,r2_x, r1_w);
		r0_y = mix(r3_y,r2_y, r1_w);
		r0_z = mix(r3_z,r2_z, r1_w);

		r0_x = min(r0_x, r0_w);
		r0_y = min(r0_y, r0_w);
		r0_z = min(r0_z, r0_w);

		o0_x = r0_x;
		o0_y = r0_y;
		o0_z = r0_z;
		o0_w = r0_w;


		output_x[count] = o0_x;
		output_y[count] = o0_y;
		output_z[count] = o0_z;
		output_w[count] = o0_w;

		++count;
	} while (count < Many/4);

        int yaxis;
        int yaxisT, yaxisF;
	
        yaxisT = get_image_height(dest) - (loc_y + dim.w + 1);
        yaxisF = loc_y + dim.w;
        yaxis = select (yaxisF, yaxisT, flipped);

	__async_work_group_stream_to_image(dest, orig_loc_x + dim.z, yaxis, Many, output_x, output_y, output_z, output_w);
}





// stream samplers
__kernel void programMany_2(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, float4 l2, float4 l3, read_only image2d_t t0, sampler_t t_sampler0, read_only image2d_t t1, sampler_t t_sampler1)
{
	const int Many = 128;
	const float4 p0  = (float4)( 0x1.0c6f7ap-20, 0x1p+0, 0x1p-1, 0x1p+1 );
	const float4 p1  = (float4)( 0x1.8p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
	float4       f0_x, o0_x, r0_x, r1_x, r2_x, r3_x, r4_x, r5_x, r6_x;
	float4       f0_y, o0_y, r0_y, r1_y, r2_y, r3_y, r4_y, r5_y, r6_y;
	float4       f0_z, o0_z, r0_z, r1_z, r2_z, r3_z, r4_z, r5_z, r6_z;
	float4       f0_w, o0_w, r0_w, r1_w, r2_w, r3_w, r4_w, r5_w, r6_w;
	float4     input_x0[Many/4], input_y0[Many/4], input_z0[Many/4], input_w0[Many/4], output_x[Many/4], output_y[Many/4], output_z[Many/4], output_w[Many/4];
	float4     input_x1[Many/4], input_y1[Many/4], input_z1[Many/4], input_w1[Many/4];

	int orig_loc_x = get_global_id(0)*Many;
	int tmp = orig_loc_x;
	int loc_y = get_global_id(1);
	int count = 0;

	for (count = 0; count < Many/4; ++count)
	{
        	int4 loc_x = tmp + (int4)(0, 1, 2, 3);

		f0_x = st_origin.x + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.x + ((float)loc_y + 0.5f) * st_delta.z;
		f0_y = st_origin.y + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.y + ((float)loc_y + 0.5f) * st_delta.w;
		f0_z = 0.0f;
		f0_w = 0.0f;

		r5_x = f0_x;
		r5_y = f0_y;
		r5_z = f0_z;
		r5_w = f0_w;

		r1_x = r5_x * l0.x + r5_y * l0.y + l0.w;
        
		r1_y = r5_x * l1.x + r5_y * l1.y + l1.w;

		r4_x = r1_x;
		r4_y = r1_y;
		r4_z = r1_z;
		r4_w = r1_w;

		if (count == 0)
		{
			float2 stride;
			stride.x = r4_x.y - r4_x.x;
			stride.y = r4_y.y - r4_y.x;
			__async_work_group_stream_from_image(t0, t_sampler0, (float2)(r4_x.x, r4_y.x), stride, Many, input_x0, input_y0, input_z0, input_w0);
		}
		// extract the inputs you currently require
		r2_x = input_x0[count];
		r2_y = input_y0[count];
		r2_z = input_z0[count];
		r2_w = input_w0[count];
	
	        r6_x = r5_x * l2.x + r5_y * l2.y + l2.w;

	        r6_y = r5_x * l3.x + r5_y * l3.y + l3.w;

		r0_x = r6_x;
		r0_y = r6_y;
		r0_z = r6_z;
		r0_w = r6_w;

		if (count == 0)
		{
			float2 stride;
			stride.x = r0_x.y - r0_x.x;
			stride.y = r0_y.y - r0_y.x;
			__async_work_group_stream_from_image(t1, t_sampler1, (float2)(r0_x.x, r0_y.x), stride, Many, input_x1, input_y1, input_z1, input_w1);
		}
		// extract the inputs you currently require
		r3_x = input_x1[count];
		r3_y = input_y1[count];
		r3_z = input_z1[count];
		r3_w = input_w1[count];

		r1_w = max(r3_w, p0.x);
		r4_w = half_recip(r1_w);
		r5_w = r3_w;

		r5_x = r3_x*r4_w;
		r5_y = r3_y*r4_w;
		r5_z = r3_z*r4_w;

		r6_w = max(r2_w, p0.x);
		r0_w = half_recip(r6_w);
		r1_w = r2_w;

		r1_x = r2_x*r0_w;
		r1_y = r2_y*r0_w;
		r1_z = r2_z*r0_w;

		r4_x = p0.y-r1_w;
		r4_y = p0.y-r1_w;
		r4_z = p0.y-r1_w;
		r4_w = p0.y-r1_w;

		r6_x = r4_x*r3_w + r1_w;
		r6_y = r4_y*r3_w + r1_w;
		r6_z = r4_z*r3_w + r1_w;
		r6_w = r4_w*r3_w + r1_w;

		r0_w = r6_x;

		r2_x = r1_x-p0.z;
		r2_y = r1_y-p0.z;
		r2_z = r1_z-p0.z;
		r2_w = r1_w-p0.z;

		r4_x = p0.y-r1_x;
		r4_y = p0.y-r1_y;
		r4_z = p0.y-r1_z;
		r4_w = p0.y-r1_w;

		r6_x = r4_x*p0.w;
		r6_y = r4_y*p0.w;
		r6_z = r4_z*p0.w;
		r6_w = r4_w*p0.w;

		r4_x = half_powr(r5_x, r6_x);
		r4_y = half_powr(r5_y, r6_y);
		r4_z = half_powr(r5_z, r6_z);
		r4_w = half_powr(r5_w, r6_w);

		r6_x = p1.x-r1_x;
		r6_y = p1.x-r1_y;
		r6_z = p1.x-r1_z;
		r6_w = p1.x-r1_w;

		r5_x = half_powr(r5_x, r6_x);
		r5_y = half_powr(r5_y, r6_y);
		r5_z = half_powr(r5_z, r6_z);
		r5_w = half_powr(r5_w, r6_w);

		r4_x = clamp(select(r5_x,r4_x, isless(r2_x, 0.0f)), 0.0f, 1.0f);
		r4_y = clamp(select(r5_y,r4_y, isless(r2_y, 0.0f)), 0.0f, 1.0f);
		r4_z = clamp(select(r5_z,r4_z, isless(r2_z, 0.0f)), 0.0f, 1.0f);
		r4_w = clamp(select(r5_w,r4_w, isless(r2_w, 0.0f)), 0.0f, 1.0f);

		r2_x = fabs(r3_w);
		r2_y = fabs(r3_w);
		r2_z = fabs(r3_w);
		r2_w = fabs(r3_w);

		r4_x = mix(r1_x,r4_x, r3_w);
		r4_y = mix(r1_y,r4_y, r3_w);
		r4_z = mix(r1_z,r4_z, r3_w);
		r4_w = mix(r1_w,r4_w, r3_w);

		r2_x = select(r1_x,r4_x, isless(-r2_x, 0.0f));
		r2_y = select(r1_y,r4_y, isless(-r2_y, 0.0f));
		r2_z = select(r1_z,r4_z, isless(-r2_z, 0.0f));
		r2_w = select(r1_w,r4_w, isless(-r2_w, 0.0f));

		r0_x = mix(r3_x,r2_x, r1_w);
		r0_y = mix(r3_y,r2_y, r1_w);
		r0_z = mix(r3_z,r2_z, r1_w);

		r0_x = min(r0_x, r0_w);
		r0_y = min(r0_y, r0_w);
		r0_z = min(r0_z, r0_w);

		o0_x = r0_x;
		o0_y = r0_y;
		o0_z = r0_z;
		o0_w = r0_w;

	        int yaxis;
	        int yaxisT, yaxisF;
	
	        yaxisT = get_image_height(dest) - (loc_y + dim.w + 1);
	        yaxisF = loc_y + dim.w;
	        yaxis = select (yaxisF, yaxisT, flipped);

		output_x[count] = o0_x;
		output_y[count] = o0_y;
		output_z[count] = o0_z;
		output_w[count] = o0_w;
		if (count == Many/4-1)
		{
			__async_work_group_stream_to_image(dest, orig_loc_x + dim.z, yaxis, Many, output_x, output_y, output_z, output_w);
		}
		tmp += 4;
	}
}




// loop
__kernel void programMany(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, float4 l2, float4 l3, read_only image2d_t t0, sampler_t t_sampler0, read_only image2d_t t1, sampler_t t_sampler1)
{
	const int Many = 128;
	const float4 p0  = (float4)( 0x1.0c6f7ap-20, 0x1p+0, 0x1p-1, 0x1p+1 );
	const float4 p1  = (float4)( 0x1.8p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
	float4       f0_x, o0_x, r0_x, r1_x, r2_x, r3_x, r4_x, r5_x, r6_x;
	float4       f0_y, o0_y, r0_y, r1_y, r2_y, r3_y, r4_y, r5_y, r6_y;
	float4       f0_z, o0_z, r0_z, r1_z, r2_z, r3_z, r4_z, r5_z, r6_z;
	float4       f0_w, o0_w, r0_w, r1_w, r2_w, r3_w, r4_w, r5_w, r6_w;
	int orig_loc_x = get_global_id(0)*Many;
	int tmp = orig_loc_x;
	int loc_y = get_global_id(1);
	int count = 0;

	for (count = 0; count < Many/4; ++count)
	{
        	int4 loc_x = tmp + (int4)(0, 1, 2, 3);

		f0_x = st_origin.x + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.x + ((float)loc_y + 0.5f) * st_delta.z;
		f0_y = st_origin.y + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.y + ((float)loc_y + 0.5f) * st_delta.w;
		f0_z = 0.0f;
		f0_w = 0.0f;

		r5_x = f0_x;
		r5_y = f0_y;
		r5_z = f0_z;
		r5_w = f0_w;

		r1_x = r5_x * l0.x + r5_y * l0.y + l0.w;
        
		r1_y = r5_x * l1.x + r5_y * l1.y + l1.w;

		r4_x = r1_x;
		r4_y = r1_y;
		r4_z = r1_z;
		r4_w = r1_w;

	        read_transposed_imagef(t0, t_sampler0, r4_x, r4_y, &r2_x, &r2_y, &r2_z, &r2_w);
	
	        r6_x = r5_x * l2.x + r5_y * l2.y + l2.w;

	        r6_y = r5_x * l3.x + r5_y * l3.y + l3.w;

		r0_x = r6_x;
		r0_y = r6_y;
		r0_z = r6_z;
		r0_w = r6_w;

		read_transposed_imagef(t1, t_sampler1, r0_x, r0_y, &r3_x, &r3_y, &r3_z, &r3_w);

		r1_w = max(r3_w, p0.x);
		r4_w = half_recip(r1_w);
		r5_w = r3_w;

		r5_x = r3_x*r4_w;
		r5_y = r3_y*r4_w;
		r5_z = r3_z*r4_w;

		r6_w = max(r2_w, p0.x);
		r0_w = half_recip(r6_w);
		r1_w = r2_w;

		r1_x = r2_x*r0_w;
		r1_y = r2_y*r0_w;
		r1_z = r2_z*r0_w;

		r4_x = p0.y-r1_w;
		r4_y = p0.y-r1_w;
		r4_z = p0.y-r1_w;
		r4_w = p0.y-r1_w;

		r6_x = r4_x*r3_w + r1_w;
		r6_y = r4_y*r3_w + r1_w;
		r6_z = r4_z*r3_w + r1_w;
		r6_w = r4_w*r3_w + r1_w;

		r0_w = r6_x;

		r2_x = r1_x-p0.z;
		r2_y = r1_y-p0.z;
		r2_z = r1_z-p0.z;
		r2_w = r1_w-p0.z;

		r4_x = p0.y-r1_x;
		r4_y = p0.y-r1_y;
		r4_z = p0.y-r1_z;
		r4_w = p0.y-r1_w;

		r6_x = r4_x*p0.w;
		r6_y = r4_y*p0.w;
		r6_z = r4_z*p0.w;
		r6_w = r4_w*p0.w;

		r4_x = half_powr(r5_x, r6_x);
		r4_y = half_powr(r5_y, r6_y);
		r4_z = half_powr(r5_z, r6_z);
		r4_w = half_powr(r5_w, r6_w);

		r6_x = p1.x-r1_x;
		r6_y = p1.x-r1_y;
		r6_z = p1.x-r1_z;
		r6_w = p1.x-r1_w;

		r5_x = half_powr(r5_x, r6_x);
		r5_y = half_powr(r5_y, r6_y);
		r5_z = half_powr(r5_z, r6_z);
		r5_w = half_powr(r5_w, r6_w);

		r4_x = clamp(select(r5_x,r4_x, isless(r2_x, 0.0f)), 0.0f, 1.0f);
		r4_y = clamp(select(r5_y,r4_y, isless(r2_y, 0.0f)), 0.0f, 1.0f);
		r4_z = clamp(select(r5_z,r4_z, isless(r2_z, 0.0f)), 0.0f, 1.0f);
		r4_w = clamp(select(r5_w,r4_w, isless(r2_w, 0.0f)), 0.0f, 1.0f);

		r2_x = fabs(r3_w);
		r2_y = fabs(r3_w);
		r2_z = fabs(r3_w);
		r2_w = fabs(r3_w);

		r4_x = mix(r1_x,r4_x, r3_w);
		r4_y = mix(r1_y,r4_y, r3_w);
		r4_z = mix(r1_z,r4_z, r3_w);
		r4_w = mix(r1_w,r4_w, r3_w);

		r2_x = select(r1_x,r4_x, isless(-r2_x, 0.0f));
		r2_y = select(r1_y,r4_y, isless(-r2_y, 0.0f));
		r2_z = select(r1_z,r4_z, isless(-r2_z, 0.0f));
		r2_w = select(r1_w,r4_w, isless(-r2_w, 0.0f));

		r0_x = mix(r3_x,r2_x, r1_w);
		r0_y = mix(r3_y,r2_y, r1_w);
		r0_z = mix(r3_z,r2_z, r1_w);

		r0_x = min(r0_x, r0_w);
		r0_y = min(r0_y, r0_w);
		r0_z = min(r0_z, r0_w);

		o0_x = r0_x;
		o0_y = r0_y;
		o0_z = r0_z;
		o0_w = r0_w;

	        int yaxis;
	        int yaxisT, yaxisF;
	
	        yaxisT = get_image_height(dest) - (loc_y + dim.w + 1);
	        yaxisF = loc_y + dim.w;
	        yaxis = select (yaxisF, yaxisT, flipped);

	        write_transposed_imagef(dest, loc_x.x + dim.z, yaxis, o0_x, o0_y, o0_z, o0_w);

		tmp += 4;
	}
}



// vect samplers
__kernel void program4_3(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, float4 l2, float4 l3, read_only image2d_t t0, sampler_t t_sampler0, read_only image2d_t t1, sampler_t t_sampler1)
{
	const float4 p0  = (float4)( 0x1.0c6f7ap-20, 0x1p+0, 0x1p-1, 0x1p+1 );
	const float4 p1  = (float4)( 0x1.8p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
	float4       f0_x, o0_x, r0_x, r1_x, r2_x, r3_x, r4_x, r5_x, r6_x;
	float4       f0_y, o0_y, r0_y, r1_y, r2_y, r3_y, r4_y, r5_y, r6_y;
	float4       f0_z, o0_z, r0_z, r1_z, r2_z, r3_z, r4_z, r5_z, r6_z;
	float4       f0_w, o0_w, r0_w, r1_w, r2_w, r3_w, r4_w, r5_w, r6_w;
	int orig_loc_x = get_global_id(0)*4;
        int4 loc_x = orig_loc_x + (int4)(0, 1, 2, 3);
	int loc_y = get_global_id(1);

	f0_x = st_origin.x + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.x + ((float)loc_y + 0.5f) * st_delta.z;
	f0_y = st_origin.y + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.y + ((float)loc_y + 0.5f) * st_delta.w;
	f0_z = 0.0f;
	f0_w = 0.0f;

	r5_x = f0_x;
	r5_y = f0_y;
	r5_z = f0_z;
	r5_w = f0_w;

        r1_x = r5_x * l0.x + r5_y * l0.y + l0.w;
        
        r1_y = r5_x * l1.x + r5_y * l1.y + l1.w;

	r4_x = r1_x;
	r4_y = r1_y;
	r4_z = r1_z;
	r4_w = r1_w;

        read_transposed_imagef(t0, t_sampler0, r4_x, r4_y, &r2_x, &r2_y, &r2_z, &r2_w);

        r6_x = r5_x * l2.x + r5_y * l2.y + l2.w;

        r6_y = r5_x * l3.x + r5_y * l3.y + l3.w;

	r0_x = r6_x;
	r0_y = r6_y;
	r0_z = r6_z;
	r0_w = r6_w;

        read_transposed_imagef(t1, t_sampler1, r0_x, r0_y, &r3_x, &r3_y, &r3_z, &r3_w);

	r1_w = max(r3_w, p0.x);
	r4_w = half_recip(r1_w);
	r5_w = r3_w;

	r5_x = r3_x*r4_w;
	r5_y = r3_y*r4_w;
	r5_z = r3_z*r4_w;

	r6_w = max(r2_w, p0.x);
	r0_w = half_recip(r6_w);
	r1_w = r2_w;

	r1_x = r2_x*r0_w;
	r1_y = r2_y*r0_w;
	r1_z = r2_z*r0_w;

	r4_x = p0.y-r1_w;
	r4_y = p0.y-r1_w;
	r4_z = p0.y-r1_w;
	r4_w = p0.y-r1_w;

	r6_x = r4_x*r3_w + r1_w;
	r6_y = r4_y*r3_w + r1_w;
	r6_z = r4_z*r3_w + r1_w;
	r6_w = r4_w*r3_w + r1_w;

	r0_w = r6_x;

	r2_x = r1_x-p0.z;
	r2_y = r1_y-p0.z;
	r2_z = r1_z-p0.z;
	r2_w = r1_w-p0.z;

	r4_x = p0.y-r1_x;
	r4_y = p0.y-r1_y;
	r4_z = p0.y-r1_z;
	r4_w = p0.y-r1_w;

	r6_x = r4_x*p0.w;
	r6_y = r4_y*p0.w;
	r6_z = r4_z*p0.w;
	r6_w = r4_w*p0.w;

	r4_x = half_powr(r5_x, r6_x);
	r4_y = half_powr(r5_y, r6_y);
	r4_z = half_powr(r5_z, r6_z);
	r4_w = half_powr(r5_w, r6_w);

	r6_x = p1.x-r1_x;
	r6_y = p1.x-r1_y;
	r6_z = p1.x-r1_z;
	r6_w = p1.x-r1_w;

	r5_x = half_powr(r5_x, r6_x);
	r5_y = half_powr(r5_y, r6_y);
	r5_z = half_powr(r5_z, r6_z);
	r5_w = half_powr(r5_w, r6_w);

	r4_x = clamp(select(r5_x,r4_x, isless(r2_x, 0.0f)), 0.0f, 1.0f);
	r4_y = clamp(select(r5_y,r4_y, isless(r2_y, 0.0f)), 0.0f, 1.0f);
	r4_z = clamp(select(r5_z,r4_z, isless(r2_z, 0.0f)), 0.0f, 1.0f);
	r4_w = clamp(select(r5_w,r4_w, isless(r2_w, 0.0f)), 0.0f, 1.0f);

	r2_x = fabs(r3_w);
	r2_y = fabs(r3_w);
	r2_z = fabs(r3_w);
	r2_w = fabs(r3_w);

	r4_x = mix(r1_x,r4_x, r3_w);
	r4_y = mix(r1_y,r4_y, r3_w);
	r4_z = mix(r1_z,r4_z, r3_w);
	r4_w = mix(r1_w,r4_w, r3_w);

	r2_x = select(r1_x,r4_x, isless(-r2_x, 0.0f));
	r2_y = select(r1_y,r4_y, isless(-r2_y, 0.0f));
	r2_z = select(r1_z,r4_z, isless(-r2_z, 0.0f));
	r2_w = select(r1_w,r4_w, isless(-r2_w, 0.0f));

	r0_x = mix(r3_x,r2_x, r1_w);
	r0_y = mix(r3_y,r2_y, r1_w);
	r0_z = mix(r3_z,r2_z, r1_w);

	r0_x = min(r0_x, r0_w);
	r0_y = min(r0_y, r0_w);
	r0_z = min(r0_z, r0_w);

	o0_x = r0_x;
	o0_y = r0_y;
	o0_z = r0_z;
	o0_w = r0_w;

        int yaxis;
        int yaxisT, yaxisF;

        yaxisT = get_image_height(dest) - (loc_y + dim.w + 1);
        yaxisF = loc_y + dim.w;
        yaxis = select (yaxisF, yaxisT, flipped);

        write_transposed_imagef(dest, loc_x.x + dim.z, yaxis, o0_x, o0_y, o0_z, o0_w);
}



// vect dot product
__kernel void program4_2(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, float4 l2, float4 l3, read_only image2d_t t0, sampler_t t_sampler0, read_only image2d_t t1, sampler_t t_sampler1)
{
	const float4 p0  = (float4)( 0x1.0c6f7ap-20, 0x1p+0, 0x1p-1, 0x1p+1 );
	const float4 p1  = (float4)( 0x1.8p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
	float4       f0_x, o0_x, r0_x, r1_x, r2_x, r3_x, r4_x, r5_x, r6_x;
	float4       f0_y, o0_y, r0_y, r1_y, r2_y, r3_y, r4_y, r5_y, r6_y;
	float4       f0_z, o0_z, r0_z, r1_z, r2_z, r3_z, r4_z, r5_z, r6_z;
	float4       f0_w, o0_w, r0_w, r1_w, r2_w, r3_w, r4_w, r5_w, r6_w;
	int orig_loc_x = get_global_id(0)*4;
        int4 loc_x = orig_loc_x + (int4)(0, 1, 2, 3);
	int loc_y = get_global_id(1);

	f0_x = st_origin.x + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.x + ((float)loc_y + 0.5f) * st_delta.z;
	f0_y = st_origin.y + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.y + ((float)loc_y + 0.5f) * st_delta.w;
	f0_z = 0.0f;
	f0_w = 0.0f;

	r5_x = f0_x;
	r5_y = f0_y;
	r5_z = f0_z;
	r5_w = f0_w;

        r1_x = r5_x * l0.x + r5_y * l0.y + l0.w;
        
        r1_y = r5_x * l1.x + r5_y * l1.y + l1.w;

	r4_x = r1_x;
	r4_y = r1_y;
	r4_z = r1_z;
	r4_w = r1_w;

	float4 r2Tx = read_imagef(t0, t_sampler0, (float2)(r4_x.x, r4_y.x));
	float4 r2Ty = read_imagef(t0, t_sampler0, (float2)(r4_x.y, r4_y.y));
	float4 r2Tz = read_imagef(t0, t_sampler0, (float2)(r4_x.z, r4_y.z));
	float4 r2Tw = read_imagef(t0, t_sampler0, (float2)(r4_x.w, r4_y.w));
	r2_x = (float4)(r2Tx.x, r2Ty.x, r2Tz.x, r2Tw.x);
	r2_y = (float4)(r2Tx.y, r2Ty.y, r2Tz.y, r2Tw.y);
	r2_z = (float4)(r2Tx.z, r2Ty.z, r2Tz.z, r2Tw.z);
	r2_w = (float4)(r2Tx.w, r2Ty.w, r2Tz.w, r2Tw.w);


        r6_x = r5_x * l2.x + r5_y * l2.y + l2.w;

        r6_y = r5_x * l3.x + r5_y * l3.y + l3.w;

	r0_x = r6_x;
	r0_y = r6_y;
	r0_z = r6_z;
	r0_w = r6_w;

	float4 r3Tx = read_imagef(t1, t_sampler1, (float2)(r0_x.x, r0_y.x));
	float4 r3Ty = read_imagef(t1, t_sampler1, (float2)(r0_x.y, r0_y.y));
	float4 r3Tz = read_imagef(t1, t_sampler1, (float2)(r0_x.z, r0_y.z));
	float4 r3Tw = read_imagef(t1, t_sampler1, (float2)(r0_x.w, r0_y.w));
	r3_x = (float4)(r3Tx.x, r3Ty.x, r3Tz.x, r3Tw.x);
	r3_y = (float4)(r3Tx.y, r3Ty.y, r3Tz.y, r3Tw.y);
	r3_z = (float4)(r3Tx.z, r3Ty.z, r3Tz.z, r3Tw.z);
	r3_w = (float4)(r3Tx.w, r3Ty.w, r3Tz.w, r3Tw.w);

	r1_w = max(r3_w, p0.x);
	r4_w = half_recip(r1_w);
	r5_w = r3_w;

	r5_x = r3_x*r4_w;
	r5_y = r3_y*r4_w;
	r5_z = r3_z*r4_w;

	r6_w = max(r2_w, p0.x);
	r0_w = half_recip(r6_w);
	r1_w = r2_w;

	r1_x = r2_x*r0_w;
	r1_y = r2_y*r0_w;
	r1_z = r2_z*r0_w;

	r4_x = p0.y-r1_w;
	r4_y = p0.y-r1_w;
	r4_z = p0.y-r1_w;
	r4_w = p0.y-r1_w;

	r6_x = r4_x*r3_w + r1_w;
	r6_y = r4_y*r3_w + r1_w;
	r6_z = r4_z*r3_w + r1_w;
	r6_w = r4_w*r3_w + r1_w;

	r0_w = r6_x;

	r2_x = r1_x-p0.z;
	r2_y = r1_y-p0.z;
	r2_z = r1_z-p0.z;
	r2_w = r1_w-p0.z;

	r4_x = p0.y-r1_x;
	r4_y = p0.y-r1_y;
	r4_z = p0.y-r1_z;
	r4_w = p0.y-r1_w;

	r6_x = r4_x*p0.w;
	r6_y = r4_y*p0.w;
	r6_z = r4_z*p0.w;
	r6_w = r4_w*p0.w;

	r4_x = half_powr(r5_x, r6_x);
	r4_y = half_powr(r5_y, r6_y);
	r4_z = half_powr(r5_z, r6_z);
	r4_w = half_powr(r5_w, r6_w);

	r6_x = p1.x-r1_x;
	r6_y = p1.x-r1_y;
	r6_z = p1.x-r1_z;
	r6_w = p1.x-r1_w;

	r5_x = half_powr(r5_x, r6_x);
	r5_y = half_powr(r5_y, r6_y);
	r5_z = half_powr(r5_z, r6_z);
	r5_w = half_powr(r5_w, r6_w);

	r4_x = clamp(select(r5_x,r4_x, isless(r2_x, 0.0f)), 0.0f, 1.0f);
	r4_y = clamp(select(r5_y,r4_y, isless(r2_y, 0.0f)), 0.0f, 1.0f);
	r4_z = clamp(select(r5_z,r4_z, isless(r2_z, 0.0f)), 0.0f, 1.0f);
	r4_w = clamp(select(r5_w,r4_w, isless(r2_w, 0.0f)), 0.0f, 1.0f);

	r2_x = fabs(r3_w);
	r2_y = fabs(r3_w);
	r2_z = fabs(r3_w);
	r2_w = fabs(r3_w);

	r4_x = mix(r1_x,r4_x, r3_w);
	r4_y = mix(r1_y,r4_y, r3_w);
	r4_z = mix(r1_z,r4_z, r3_w);
	r4_w = mix(r1_w,r4_w, r3_w);

	r2_x = select(r1_x,r4_x, isless(-r2_x, 0.0f));
	r2_y = select(r1_y,r4_y, isless(-r2_y, 0.0f));
	r2_z = select(r1_z,r4_z, isless(-r2_z, 0.0f));
	r2_w = select(r1_w,r4_w, isless(-r2_w, 0.0f));

	r0_x = mix(r3_x,r2_x, r1_w);
	r0_y = mix(r3_y,r2_y, r1_w);
	r0_z = mix(r3_z,r2_z, r1_w);

	r0_x = min(r0_x, r0_w);
	r0_y = min(r0_y, r0_w);
	r0_z = min(r0_z, r0_w);

	o0_x = r0_x;
	o0_y = r0_y;
	o0_z = r0_z;
	o0_w = r0_w;

	write_imagef(dest, (int2)( loc_x.x + dim.z , flipped ? get_image_height(dest) - (loc_y + dim.w + 1) : loc_y + dim.w ), (float4)(o0_x.x, o0_y.x, o0_z.x, o0_w.x));
	write_imagef(dest, (int2)( loc_x.y + dim.z , flipped ? get_image_height(dest) - (loc_y + dim.w + 1) : loc_y + dim.w ), (float4)(o0_x.y, o0_y.y, o0_z.y, o0_w.y));
	write_imagef(dest, (int2)( loc_x.z + dim.z , flipped ? get_image_height(dest) - (loc_y + dim.w + 1) : loc_y + dim.w ), (float4)(o0_x.z, o0_y.z, o0_z.z, o0_w.z));
	write_imagef(dest, (int2)( loc_x.w + dim.z , flipped ? get_image_height(dest) - (loc_y + dim.w + 1) : loc_y + dim.w ), (float4)(o0_x.w, o0_y.w, o0_z.w, o0_w.w));
}



// vect 4
__kernel void program4(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, float4 l2, float4 l3, read_only image2d_t t0, sampler_t t_sampler0, read_only image2d_t t1, sampler_t t_sampler1)
{
	const float4 p0  = (float4)( 0x1.0c6f7ap-20, 0x1p+0, 0x1p-1, 0x1p+1 );
	const float4 p1  = (float4)( 0x1.8p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
	float4       f0_x, o0_x, r0_x, r1_x, r2_x, r3_x, r4_x, r5_x, r6_x;
	float4       f0_y, o0_y, r0_y, r1_y, r2_y, r3_y, r4_y, r5_y, r6_y;
	float4       f0_z, o0_z, r0_z, r1_z, r2_z, r3_z, r4_z, r5_z, r6_z;
	float4       f0_w, o0_w, r0_w, r1_w, r2_w, r3_w, r4_w, r5_w, r6_w;
	int orig_loc_x = get_global_id(0)*4;
        int4 loc_x = orig_loc_x + (int4)(0, 1, 2, 3);
	int loc_y = get_global_id(1);

	f0_x = st_origin.x + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.x + ((float)loc_y + 0.5f) * st_delta.z;
	f0_y = st_origin.y + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.y + ((float)loc_y + 0.5f) * st_delta.w;
	f0_z = 0.0f;
	f0_w = 0.0f;

	r5_x = f0_x;
	r5_y = f0_y;
	r5_z = f0_z;
	r5_w = f0_w;

	r1_x.x = dot((float2)(r5_x.x, r5_y.x),l0.xy) + l0.w;
	r1_x.y = dot((float2)(r5_x.y, r5_y.y),l0.xy) + l0.w;
	r1_x.z = dot((float2)(r5_x.z, r5_y.z),l0.xy) + l0.w;
	r1_x.w = dot((float2)(r5_x.w, r5_y.w),l0.xy) + l0.w;
        
	r1_y.x = dot((float2)(r5_x.x, r5_y.x),l1.xy) + l1.w;
	r1_y.y = dot((float2)(r5_x.y, r5_y.y),l1.xy) + l1.w;
	r1_y.z = dot((float2)(r5_x.z, r5_y.z),l1.xy) + l1.w;
	r1_y.w = dot((float2)(r5_x.w, r5_y.w),l1.xy) + l1.w;

	r4_x = r1_x;
	r4_y = r1_y;
	r4_z = r1_z;
	r4_w = r1_w;

	float4 r2Tx = read_imagef(t0, t_sampler0, (float2)(r4_x.x, r4_y.x));
	float4 r2Ty = read_imagef(t0, t_sampler0, (float2)(r4_x.y, r4_y.y));
	float4 r2Tz = read_imagef(t0, t_sampler0, (float2)(r4_x.z, r4_y.z));
	float4 r2Tw = read_imagef(t0, t_sampler0, (float2)(r4_x.w, r4_y.w));
	r2_x = (float4)(r2Tx.x, r2Ty.x, r2Tz.x, r2Tw.x);
	r2_y = (float4)(r2Tx.y, r2Ty.y, r2Tz.y, r2Tw.y);
	r2_z = (float4)(r2Tx.z, r2Ty.z, r2Tz.z, r2Tw.z);
	r2_w = (float4)(r2Tx.w, r2Ty.w, r2Tz.w, r2Tw.w);

	r6_x.x = dot((float2)(r5_x.x, r5_y.x),l2.xy) + l2.w;
	r6_x.y = dot((float2)(r5_x.y, r5_y.y),l2.xy) + l2.w;
	r6_x.z = dot((float2)(r5_x.z, r5_y.z),l2.xy) + l2.w;
	r6_x.w = dot((float2)(r5_x.w, r5_y.w),l2.xy) + l2.w;

	r6_y.x = dot((float2)(r5_x.x, r5_y.x),l3.xy) + l3.w;
	r6_y.y = dot((float2)(r5_x.y, r5_y.y),l3.xy) + l3.w;
	r6_y.z = dot((float2)(r5_x.z, r5_y.z),l3.xy) + l3.w;
	r6_y.w = dot((float2)(r5_x.w, r5_y.w),l3.xy) + l3.w;

	r0_x = r6_x;
	r0_y = r6_y;
	r0_z = r6_z;
	r0_w = r6_w;

	float4 r3Tx = read_imagef(t1, t_sampler1, (float2)(r0_x.x, r0_y.x));
	float4 r3Ty = read_imagef(t1, t_sampler1, (float2)(r0_x.y, r0_y.y));
	float4 r3Tz = read_imagef(t1, t_sampler1, (float2)(r0_x.z, r0_y.z));
	float4 r3Tw = read_imagef(t1, t_sampler1, (float2)(r0_x.w, r0_y.w));
	r3_x = (float4)(r3Tx.x, r3Ty.x, r3Tz.x, r3Tw.x);
	r3_y = (float4)(r3Tx.y, r3Ty.y, r3Tz.y, r3Tw.y);
	r3_z = (float4)(r3Tx.z, r3Ty.z, r3Tz.z, r3Tw.z);
	r3_w = (float4)(r3Tx.w, r3Ty.w, r3Tz.w, r3Tw.w);

	r1_w = max(r3_w, p0.x);
	r4_w = half_recip(r1_w);
	r5_w = r3_w;

	r5_x = r3_x*r4_w;
	r5_y = r3_y*r4_w;
	r5_z = r3_z*r4_w;

	r6_w = max(r2_w, p0.x);
	r0_w = half_recip(r6_w);
	r1_w = r2_w;

	r1_x = r2_x*r0_w;
	r1_y = r2_y*r0_w;
	r1_z = r2_z*r0_w;

	r4_x = p0.y-r1_w;
	r4_y = p0.y-r1_w;
	r4_z = p0.y-r1_w;
	r4_w = p0.y-r1_w;

	r6_x = r4_x*r3_w + r1_w;
	r6_y = r4_y*r3_w + r1_w;
	r6_z = r4_z*r3_w + r1_w;
	r6_w = r4_w*r3_w + r1_w;

	r0_w = r6_x;

	r2_x = r1_x-p0.z;
	r2_y = r1_y-p0.z;
	r2_z = r1_z-p0.z;
	r2_w = r1_w-p0.z;

	r4_x = p0.y-r1_x;
	r4_y = p0.y-r1_y;
	r4_z = p0.y-r1_z;
	r4_w = p0.y-r1_w;

	r6_x = r4_x*p0.w;
	r6_y = r4_y*p0.w;
	r6_z = r4_z*p0.w;
	r6_w = r4_w*p0.w;

	r4_x = half_powr(r5_x, r6_x);
	r4_y = half_powr(r5_y, r6_y);
	r4_z = half_powr(r5_z, r6_z);
	r4_w = half_powr(r5_w, r6_w);

	r6_x = p1.x-r1_x;
	r6_y = p1.x-r1_y;
	r6_z = p1.x-r1_z;
	r6_w = p1.x-r1_w;

	r5_x = half_powr(r5_x, r6_x);
	r5_y = half_powr(r5_y, r6_y);
	r5_z = half_powr(r5_z, r6_z);
	r5_w = half_powr(r5_w, r6_w);

	r4_x = clamp(select(r5_x,r4_x, isless(r2_x, 0.0f)), 0.0f, 1.0f);
	r4_y = clamp(select(r5_y,r4_y, isless(r2_y, 0.0f)), 0.0f, 1.0f);
	r4_z = clamp(select(r5_z,r4_z, isless(r2_z, 0.0f)), 0.0f, 1.0f);
	r4_w = clamp(select(r5_w,r4_w, isless(r2_w, 0.0f)), 0.0f, 1.0f);

	r2_x = fabs(r3_w);
	r2_y = fabs(r3_w);
	r2_z = fabs(r3_w);
	r2_w = fabs(r3_w);

	r4_x = mix(r1_x,r4_x, r3_w);
	r4_y = mix(r1_y,r4_y, r3_w);
	r4_z = mix(r1_z,r4_z, r3_w);
	r4_w = mix(r1_w,r4_w, r3_w);

	r2_x = select(r1_x,r4_x, isless(-r2_x, 0.0f));
	r2_y = select(r1_y,r4_y, isless(-r2_y, 0.0f));
	r2_z = select(r1_z,r4_z, isless(-r2_z, 0.0f));
	r2_w = select(r1_w,r4_w, isless(-r2_w, 0.0f));

	r0_x = mix(r3_x,r2_x, r1_w);
	r0_y = mix(r3_y,r2_y, r1_w);
	r0_z = mix(r3_z,r2_z, r1_w);

	r0_x = min(r0_x, r0_w);
	r0_y = min(r0_y, r0_w);
	r0_z = min(r0_z, r0_w);

	o0_x = r0_x;
	o0_y = r0_y;
	o0_z = r0_z;
	o0_w = r0_w;

	write_imagef(dest, (int2)( loc_x.x + dim.z , flipped ? get_image_height(dest) - (loc_y + dim.w + 1) : loc_y + dim.w ), (float4)(o0_x.x, o0_y.x, o0_z.x, o0_w.x));
	write_imagef(dest, (int2)( loc_x.y + dim.z , flipped ? get_image_height(dest) - (loc_y + dim.w + 1) : loc_y + dim.w ), (float4)(o0_x.y, o0_y.y, o0_z.y, o0_w.y));
	write_imagef(dest, (int2)( loc_x.z + dim.z , flipped ? get_image_height(dest) - (loc_y + dim.w + 1) : loc_y + dim.w ), (float4)(o0_x.z, o0_y.z, o0_z.z, o0_w.z));
	write_imagef(dest, (int2)( loc_x.w + dim.z , flipped ? get_image_height(dest) - (loc_y + dim.w + 1) : loc_y + dim.w ), (float4)(o0_x.w, o0_y.w, o0_z.w, o0_w.w));
}




__kernel void programScalar(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, float4 l2, float4 l3, read_only image2d_t t0, sampler_t t_sampler0, read_only image2d_t t1, sampler_t t_sampler1)
{
	const float4 p0  = (float4)( 0x1.0c6f7ap-20, 0x1p+0, 0x1p-1, 0x1p+1 );
	const float4 p1  = (float4)( 0x1.8p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
	float       f0_x, o0_x, r0_x, r1_x, r2_x, r3_x, r4_x, r5_x, r6_x;
	float       f0_y, o0_y, r0_y, r1_y, r2_y, r3_y, r4_y, r5_y, r6_y;
	float       f0_z, o0_z, r0_z, r1_z, r2_z, r3_z, r4_z, r5_z, r6_z;
	float       f0_w, o0_w, r0_w, r1_w, r2_w, r3_w, r4_w, r5_w, r6_w;
	int loc_x = get_global_id(0);
	int loc_y = get_global_id(1);

	f0_x = st_origin.x + ((float)loc_x + 0.5f) * st_delta.x + ((float)loc_y + 0.5f) * st_delta.z;
	f0_y = st_origin.y + ((float)loc_x + 0.5f) * st_delta.y + ((float)loc_y + 0.5f) * st_delta.w;
	f0_z = 0.0f;
	f0_w = 0.0f;

	r5_x = f0_x;
	r5_y = f0_y;
	r5_z = f0_z;
	r5_w = f0_w;

	r1_x = dot((float2)(r5_x, r5_y),l0.xy) + l0.w;
	r1_y = dot((float2)(r5_x, r5_y),l1.xy) + l1.w;

	r4_x = r1_x;
	r4_y = r1_y;
	r4_z = r1_z;
	r4_w = r1_w;

	float4 r2T = read_imagef(t0, t_sampler0, (float2)(r4_x, r4_y));
	r2_x = r2T.x;
	r2_y = r2T.y;
	r2_z = r2T.z;
	r2_w = r2T.w;

	r6_x = dot((float2)(r5_x, r5_y),l2.xy) + l2.w;
	r6_y = dot((float2)(r5_x, r5_y),l3.xy) + l3.w;

	r0_x = r6_x;
	r0_y = r6_y;
	r0_z = r6_z;
	r0_w = r6_w;

	float4 r3T = read_imagef(t1, t_sampler1, (float2)(r0_x, r0_y));
	r3_x = r3T.x;
	r3_y = r3T.y;
	r3_z = r3T.z;
	r3_w = r3T.w;

	r1_w = max(r3_w, p0.x);
	r4_w = half_recip(r1_w);
	r5_w = r3_w;

	r5_x = r3_x*r4_w;
	r5_y = r3_y*r4_w;
	r5_z = r3_z*r4_w;

	r6_w = max(r2_w, p0.x);
	r0_w = half_recip(r6_w);
	r1_w = r2_w;

	r1_x = r2_x*r0_w;
	r1_y = r2_y*r0_w;
	r1_z = r2_z*r0_w;

	r4_x = p0.y-r1_w;
	r4_y = p0.y-r1_w;
	r4_z = p0.y-r1_w;
	r4_w = p0.y-r1_w;

	r6_x = r4_x*r3_w + r1_w;
	r6_y = r4_y*r3_w + r1_w;
	r6_z = r4_z*r3_w + r1_w;
	r6_w = r4_w*r3_w + r1_w;

	r0_w = r6_x;

	r2_x = r1_x-p0.z;
	r2_y = r1_y-p0.z;
	r2_z = r1_z-p0.z;
	r2_w = r1_w-p0.z;

	r4_x = p0.y-r1_x;
	r4_y = p0.y-r1_y;
	r4_z = p0.y-r1_z;
	r4_w = p0.y-r1_w;

	r6_x = r4_x*p0.w;
	r6_y = r4_y*p0.w;
	r6_z = r4_z*p0.w;
	r6_w = r4_w*p0.w;

        float4 r4T = half_powr((float4)(r5_x, r5_y, r5_z, r5_w), (float4)(r6_x, r6_y, r6_z, r6_w));
        r4_x = r4T.x;
        r4_y = r4T.y;
        r4_z = r4T.z;
        r4_w = r4T.w;
//	r4_x = half_powr(r5_x, r6_x);
//	r4_y = half_powr(r5_y, r6_y);
//	r4_z = half_powr(r5_z, r6_z);
//	r4_w = half_powr(r5_w, r6_w);

	r6_x = p1.x-r1_x;
	r6_y = p1.x-r1_y;
	r6_z = p1.x-r1_z;
	r6_w = p1.x-r1_w;

        float4 r5T = half_powr((float4)(r5_x, r5_y, r5_z, r5_w), (float4)(r6_x, r6_y, r6_z, r6_w));
        r5_x = r5T.x;
        r5_y = r5T.y;
        r5_z = r5T.z;
        r5_w = r5T.w;
//	r5_x = half_powr(r5_x, r6_x);
//	r5_y = half_powr(r5_y, r6_y);
//	r5_z = half_powr(r5_z, r6_z);
//	r5_w = half_powr(r5_w, r6_w);

	r4_x = clamp(select(r5_x,r4_x, isless(r2_x, 0.0f)), 0.0f, 1.0f);
	r4_y = clamp(select(r5_y,r4_y, isless(r2_y, 0.0f)), 0.0f, 1.0f);
	r4_z = clamp(select(r5_z,r4_z, isless(r2_z, 0.0f)), 0.0f, 1.0f);
	r4_w = clamp(select(r5_w,r4_w, isless(r2_w, 0.0f)), 0.0f, 1.0f);

	r2_x = fabs(r3_w);
	r2_y = fabs(r3_w);
	r2_z = fabs(r3_w);
	r2_w = fabs(r3_w);

	r4_x = mix(r1_x,r4_x, r3_w);
	r4_y = mix(r1_y,r4_y, r3_w);
	r4_z = mix(r1_z,r4_z, r3_w);
	r4_w = mix(r1_w,r4_w, r3_w);

	r2_x = select(r1_x,r4_x, isless(-r2_x, 0.0f));
	r2_y = select(r1_y,r4_y, isless(-r2_y, 0.0f));
	r2_z = select(r1_z,r4_z, isless(-r2_z, 0.0f));
	r2_w = select(r1_w,r4_w, isless(-r2_w, 0.0f));

	r0_x = mix(r3_x,r2_x, r1_w);
	r0_y = mix(r3_y,r2_y, r1_w);
	r0_z = mix(r3_z,r2_z, r1_w);

	r0_x = min(r0_x, r0_w);
	r0_y = min(r0_y, r0_w);
	r0_z = min(r0_z, r0_w);

	o0_x = r0_x;
	o0_y = r0_y;
	o0_z = r0_z;
	o0_w = r0_w;

	write_imagef(dest, (int2)( loc_x + dim.z , flipped ? get_image_height(dest) - (loc_y + dim.w + 1) : loc_y + dim.w ), (float4)(o0_x, o0_y, o0_z, o0_w));
}




__kernel void program(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, float4 l2, float4 l3, read_only image2d_t t0, sampler_t t_sampler0, read_only image2d_t t1, sampler_t t_sampler1)
{
    const float4 p0  = (float4)( 0x1.0c6f7ap-20, 0x1p+0, 0x1p-1, 0x1p+1 );
    const float4 p1  = (float4)( 0x1.8p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    int          dest_width = dim.x;
    int          dest_height = dim.y;
    float4       o0, r0, r1, r2, r3, r4, r5, r6;
    float4       false_vector = (float4) 0.0f;
    float4       true_vector = (float4) 1.0f;
    float        unused_float1;
    float2       unused_float2;
    __float3_SPI       unused_float3;
    float4       unused_float4;
    int2         loc = (int2)( get_global_id(0), get_global_id(1) );
    float4 f0 = (float4)( st_origin.x + ((float)loc.x + 0.5f) * st_delta.x + ((float)loc.y + 0.5f) * st_delta.z, st_origin.y + ((float)loc.x + 0.5f) * st_delta.y + ((float)loc.y + 0.5f) * st_delta.w, 0.0f, 0.0f );
    r5 = f0;
    r1.x = dot(r5.xy,l0.xy) + l0.w;
    r1.y = dot(r5.xy,l1.xy) + l1.w;
    r4 = r1;
    r2 = read_imagef(t0, t_sampler0, r4.xy);
    r6.x = dot(r5.xy,l2.xy) + l2.w;
    r6.y = dot(r5.xy,l3.xy) + l3.w;
    r0 = r6;
    r3 = read_imagef(t1, t_sampler1, r0.xy);
    r1.w = max(r3.w, p0.x);
    r4.w = half_recip(r1.w);
    r5.w = r3.w;
    r5.xyz = r3.xyz*r4.www;
    r6.w = max(r2.w, p0.x);
    r0.w = half_recip(r6.w);
    r1.w = r2.w;
    r1.xyz = r2.xyz*r0.www;
    r4 = p0.yyyy-r1.wwww;
    r6 = r4*r3.wwww + r1.wwww;
    r0.w = r6.x;
    r2 = r1-p0.zzzz;
    r4 = p0.yyyy-r1;
    r6 = r4*p0.wwww;
    r4 = half_powr(r5, r6);
    r6 = p1.xxxx-r1;
    r5 = half_powr(r5, r6);
    r4 = clamp(select(r5,r4, isless(r2, 0.0f)), 0.0f, 1.0f);
    r2 = fabs(r3.wwww);
    r4 = mix(r1,r4, r3.wwww);
    r2 = select(r1,r4, isless(-r2, 0.0f));
    r0.xyz = mix(r3.xyz,r2.xyz, r1.www);
    r0.xyz = min(r0.xyz, r0.www);
    o0 = r0;
    write_imagef(dest, (int2)( loc.x + dim.z , flipped ? get_image_height(dest) - (loc.y + dim.w + 1) : loc.y + dim.w ), o0);
}



__kernel void program_trans(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, float4 l2, float4 l3, read_only image2d_t t0, sampler_t t_sampler0, read_only image2d_t t1, sampler_t t_sampler1)
{
    float4       f0_start, f0_end, f0_delta, tf0_start[4], tf0_delta[4];
    float4       f1_start, f1_end, f1_delta, tf1_start[4], tf1_delta[4];
    float4       f2_start, f2_end, f2_delta, tf2_start[4], tf2_delta[4];
    float4       gr0_1_0[64], gr0_1_1[64], gr0_1_2[64], gr0_1_3[64];
    float4       gr1_2_0[64], gr1_2_1[64], gr1_2_2[64], gr1_2_3[64];
    const float4 p0  = (float4)( 0x1.0c6f7ap-20, 0x1.0c6f7ap-20, 0x1.0c6f7ap-20, 0x1.0c6f7ap-20 );
    const float4 p1  = (float4)( 0x1p+0, 0x1p+0, 0x1p+0, 0x1p+0 );
    const float4 p2  = (float4)( 0x1p-1, 0x1p-1, 0x1p-1, 0x1p-1 );
    const float4 p3  = (float4)( 0x1p+1, 0x1p+1, 0x1p+1, 0x1p+1 );
    const float4 p4  = (float4)( 0x1.8p+0, 0x1.8p+0, 0x1.8p+0, 0x1.8p+0 );
    const float4 p5  = (float4)( -0x1p+0, -0x1p+0, -0x1p+0, -0x1p+0 );
    int          index = 0;
    int          total_index = 0;
    int          write_amount = 0;
    int          read_amount = 256;
    int          write_offset = 0;
    float4       o_r[64], o_g[64], o_b[64], o_a[64];
    int          dest_width = dim.x;
    int          dest_height = dim.y;
    float4       o0, r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, r16, r17, r18, r19, r20, r21, r22, r23, r24, r25, r26, r27, r28, r29, r30, r31, r32, r33, r34, r35, r36, r37, r38, r39, r40, r41, r42, r43, r44, r45, r46, r47, r48, r49, r50, r51, r52, r53, r54, r55, r56, r57, r58, r59, r60, r61, r62, r63, r64, r65, r66, r67, r68, r69, r70, r71, r72, r73, r74, r75, r76, r77, r78, r79, r80, r81, r82, r83, r84, r85, r86, r87, r88, r89, r90, r91, r92, r93, r94, r95, r96, r97, r98, r99, r100, r101, r102, r103, r104, r105, r106;
    float4       false_vector = (float4) 0.0f;
    float4       true_vector = (float4) 1.0f;
    float        unused_float1;
    float2       unused_float2;
    __float3_SPI       unused_float3;
    float4       unused_float4;
    float16      scratch16;
    int2         loc = (int2)( 0, get_global_id(0) );

    float4       loc_start = (float4)( st_origin.x + ((float)loc.x + 0.5f) * st_delta.x + ((float)loc.y + 0.5f) * st_delta.z, st_origin.y + ((float)loc.x + 0.5f) * st_delta.y + ((float)loc.y + 0.5f) * st_delta.w, 0.0f, 0.0f );
    float4       loc_end = loc_start + (float4)( (float)dest_width * st_delta.x, (float)dest_width * st_delta.y, 0.0f, 0.0f );
    r0 = (float4) 0.0f;
    r1 = (float4) 0.0f;
    r2 = (float4) 0.0f;

    // vertex start
    f0_start = loc_start;
    r1 = loc_start;
    r2.x = dot(r1.xy,l0.xy) + l0.w;
    r2.y = dot(r1.xy,l1.xy) + l1.w;
    f1_start = r2;
    r0.x = dot(r1.xy,l2.xy) + l2.w;
    r0.y = dot(r1.xy,l3.xy) + l3.w;
    f2_start = r0;

    // vertex end
    f0_end = loc_end;
    r1 = loc_end;
    r2.x = dot(r1.xy,l0.xy) + l0.w;
    r2.y = dot(r1.xy,l1.xy) + l1.w;
    f1_end = r2;
    r0.x = dot(r1.xy,l2.xy) + l2.w;
    r0.y = dot(r1.xy,l3.xy) + l3.w;
    f2_end = r0;

    f0_delta = (f0_end - f0_start) / (float)(dest_width);
    tf0_delta[0] = (float4) f0_delta.s0;
    tf0_start[0] = (float4) ( f0_start.s0, f0_start.s0 + f0_delta.s0, f0_start.s0 + 2.0*f0_delta.s0, f0_start.s0 + 3.0*f0_delta.s0 );
    tf0_delta[1] = (float4) f0_delta.s1;
    tf0_start[1] = (float4) ( f0_start.s1, f0_start.s1 + f0_delta.s1, f0_start.s1 + 2.0*f0_delta.s1, f0_start.s1 + 3.0*f0_delta.s1 );
    tf0_delta[2] = (float4) f0_delta.s2;
    tf0_start[2] = (float4) ( f0_start.s2, f0_start.s2 + f0_delta.s2, f0_start.s2 + 2.0*f0_delta.s2, f0_start.s2 + 3.0*f0_delta.s2 );
    tf0_delta[3] = (float4) f0_delta.s3;
    tf0_start[3] = (float4) ( f0_start.s3, f0_start.s3 + f0_delta.s3, f0_start.s3 + 2.0*f0_delta.s3, f0_start.s3 + 3.0*f0_delta.s3 );
    f1_delta = (f1_end - f1_start) / (float)(dest_width);
    tf1_delta[0] = (float4) f1_delta.s0;
    tf1_start[0] = (float4) ( f1_start.s0, f1_start.s0 + f1_delta.s0, f1_start.s0 + 2.0*f1_delta.s0, f1_start.s0 + 3.0*f1_delta.s0 );
    tf1_delta[1] = (float4) f1_delta.s1;
    tf1_start[1] = (float4) ( f1_start.s1, f1_start.s1 + f1_delta.s1, f1_start.s1 + 2.0*f1_delta.s1, f1_start.s1 + 3.0*f1_delta.s1 );
    tf1_delta[2] = (float4) f1_delta.s2;
    tf1_start[2] = (float4) ( f1_start.s2, f1_start.s2 + f1_delta.s2, f1_start.s2 + 2.0*f1_delta.s2, f1_start.s2 + 3.0*f1_delta.s2 );
    tf1_delta[3] = (float4) f1_delta.s3;
    tf1_start[3] = (float4) ( f1_start.s3, f1_start.s3 + f1_delta.s3, f1_start.s3 + 2.0*f1_delta.s3, f1_start.s3 + 3.0*f1_delta.s3 );
    f2_delta = (f2_end - f2_start) / (float)(dest_width);
    tf2_delta[0] = (float4) f2_delta.s0;
    tf2_start[0] = (float4) ( f2_start.s0, f2_start.s0 + f2_delta.s0, f2_start.s0 + 2.0*f2_delta.s0, f2_start.s0 + 3.0*f2_delta.s0 );
    tf2_delta[1] = (float4) f2_delta.s1;
    tf2_start[1] = (float4) ( f2_start.s1, f2_start.s1 + f2_delta.s1, f2_start.s1 + 2.0*f2_delta.s1, f2_start.s1 + 3.0*f2_delta.s1 );
    tf2_delta[2] = (float4) f2_delta.s2;
    tf2_start[2] = (float4) ( f2_start.s2, f2_start.s2 + f2_delta.s2, f2_start.s2 + 2.0*f2_delta.s2, f2_start.s2 + 3.0*f2_delta.s2 );
    tf2_delta[3] = (float4) f2_delta.s3;
    tf2_start[3] = (float4) ( f2_start.s3, f2_start.s3 + f2_delta.s3, f2_start.s3 + 2.0*f2_delta.s3, f2_start.s3 + 3.0*f2_delta.s3 );

    __async_work_group_stream_from_image(t0, t_sampler0, f1_start.xy, f1_delta.xy, read_amount, (float4 *)gr0_1_0, (float4 *)gr0_1_1, (float4 *)gr0_1_2, (float4 *)gr0_1_3);
    __async_work_group_stream_from_image(t1, t_sampler1, f2_start.xy, f2_delta.xy, read_amount, (float4 *)gr1_2_0, (float4 *)gr1_2_1, (float4 *)gr1_2_2, (float4 *)gr1_2_3);
    for(; loc.x<dest_width ; loc.x+=4)
    {
        r0 = gr0_1_0[index];
        r1 = gr0_1_1[index];
        r2 = gr0_1_2[index];
        r3 = gr0_1_3[index];
        r4 = gr1_2_0[index];
        r5 = gr1_2_1[index];
        r6 = gr1_2_2[index];
        r7 = gr1_2_3[index];
        r8 = max(r7, p0);
        r9 = half_recip(r8);
        r10 = r7;
        r11 = r4*r9;
        r12 = r5*r9;
        r13 = r6*r9;
        r14 = max(r3, p0);
        r15 = half_recip(r14);
        r16 = r3;
        r17 = r0*r15;
        r18 = r1*r15;
        r19 = r2*r15;
        r20 = p1-r16;
        r21 = p1-r16;
        r22 = p1-r16;
        r23 = p1-r16;
        r24 = r20*r7 + r16;
        r25 = r21*r7 + r16;
        r26 = r22*r7 + r16;
        r27 = r23*r7 + r16;
        r28 = r24;
        r29 = r17-p2;
        r30 = r18-p2;
        r31 = r19-p2;
        r32 = r16-p2;
        r33 = p1-r17;
        r34 = p1-r18;
        r35 = p1-r19;
        r36 = p1-r16;
        r37 = r33*p3;
        r38 = r34*p3;
        r39 = r35*p3;
        r40 = r36*p3;
        r41 = half_powr(r11, r37);
        r42 = half_powr(r12, r38);
        r43 = half_powr(r13, r39);
        r44 = half_powr(r10, r40);
        r45 = p4-r17;
        r46 = p4-r18;
        r47 = p4-r19;
        r48 = p4-r16;
        r49 = half_powr(r11, r45);
        r50 = half_powr(r12, r46);
        r51 = half_powr(r13, r47);
        r52 = half_powr(r10, r48);
        r53 = r49;
        r54 = r50;
        r55 = r51;
        r56 = r52;
        r57 = select(r53,r41, isless(r29, 0.0f));
        r58 = select(r54,r42, isless(r30, 0.0f));
        r59 = select(r55,r43, isless(r31, 0.0f));
        r60 = select(r56,r44, isless(r32, 0.0f));
        r61 = (float4) 0.0f;
        r62 = min(r57, p1);
        r63 = max(r62, r61);
        r64 = min(r58, p1);
        r65 = max(r64, r61);
        r66 = min(r59, p1);
        r67 = max(r66, r61);
        r68 = min(r60, p1);
        r69 = max(r68, r61);
        r70 = r63;
        r71 = r65;
        r72 = r67;
        r73 = r69;
        r74 = fabs(r7);
        r75 = fabs(r7);
        r76 = fabs(r7);
        r77 = fabs(r7);
        r78 = mix(r17,r70, r7);
        r79 = mix(r18,r71, r7);
        r80 = mix(r19,r72, r7);
        r81 = mix(r16,r73, r7);
        r82 = r78;
        r83 = r79;
        r84 = r80;
        r85 = r81;
        r86 = r74*p5;
        r87 = r75*p5;
        r88 = r76*p5;
        r89 = r77*p5;
        r90 = select(r17,r82, isless(r86, 0.0f));
        r91 = select(r18,r83, isless(r87, 0.0f));
        r92 = select(r19,r84, isless(r88, 0.0f));
        r93 = select(r16,r85, isless(r89, 0.0f));
        r94 = mix(r4,r90, r16);
        r95 = mix(r5,r91, r16);
        r96 = mix(r6,r92, r16);
        r97 = min(r94, r28);
        r98 = min(r95, r28);
        r99 = min(r96, r28);
        r100 = r97;
        r101 = r98;
        r102 = r99;
        r103 = r100;
        r104 = r101;
        r105 = r102;
        r106 = r28;
        o_r[index] = r103;
        o_g[index] = r104;
        o_b[index] = r105;
        o_a[index] = r106;
        index++;
        total_index++;
        if (index == 64)
        {
            write_amount = min(256, dest_width - write_offset);
            (void)__async_work_group_stream_to_image(dest, (size_t)(dim.z + write_offset), (size_t)(flipped ? get_image_height(dest) - (loc.y+dim.w+1): loc.y+dim.w), write_amount, (const float4 *)o_r, (const float4 *)o_g, (const float4 *)o_b, (const float4 *)o_a);
            index = 0;
            write_offset += write_amount;
            if (write_offset == dest_width) return;
            read_amount = min(256, dest_width - write_offset);
            __async_work_group_stream_from_image(t0, t_sampler0, f1_start.xy + ((float)4*total_index) * f1_delta.xy, f1_delta.xy, read_amount, (float4 *)gr0_1_0, (float4 *)gr0_1_1, (float4 *)gr0_1_2, (float4 *)gr0_1_3);
            __async_work_group_stream_from_image(t1, t_sampler1, f2_start.xy + ((float)4*total_index) * f2_delta.xy, f2_delta.xy, read_amount, (float4 *)gr1_2_0, (float4 *)gr1_2_1, (float4 *)gr1_2_2, (float4 *)gr1_2_3);
        }
;
    }
    if (index > 0)
    {
        (void)__async_work_group_stream_to_image(dest, (size_t)(dim.z + write_offset), (size_t)(flipped ? get_image_height(dest) - (loc.y+dim.w+1): loc.y+dim.w), (size_t)(dest_width - write_offset), (const float4 *)o_r, (const float4 *)o_g, (const float4 *)o_b, (const float4 *)o_a);
    }
}
