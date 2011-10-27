__kernel void program8(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, read_only image2d_t t0, sampler_t t_sampler0)
{
	//  float4       o0, r0, r1;
	float4 o0_x, o0_y, o0_z, o0_w;
	float4 r0_x, r0_y, r0_z, r0_w;
	float4 r1_x, r1_y, r1_z, r1_w;

	float4 f0_x, f0_y, f0_z, f0_w;

	// int2         loc = (int2)( get_global_id(0), get_global_id(1) );
	int4 loc_x, loc_y;

	loc_y = get_global_id(1);
	int count;
	const int Many = 512;

	int tmp = get_global_id(0) * Many;
	int orig_loc_x = tmp;

	float4 input_x[Many/4], input_y[Many/4], input_z[Many/4], input_w[Many/4];
	float4 output_x[Many/4], output_y[Many/4], output_z[Many/4], output_w[Many/4];

	loc_x = (int4)(tmp, tmp+1, tmp+2, tmp+3);

	// float4 f0 = (float4)( st_origin.x + ((float)loc.x + 0.5f) * st_delta.x + ((float)loc.y + 0.5f) * st_delta.z, st_origin.y + ((float)loc.x + 0.5f) * st_delta.y + ((float)loc.y + 0.5f) * st_delta.w, 0.0f, 0.0f );
	f0_x = st_origin.x + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.x + ((float4)((float)loc_y.x, (float)loc_y.y, (float)loc_y.z, (float)loc_y.w) + 0.5f) * st_delta.z;
	f0_y = st_origin.y + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.y + ((float4)((float)loc_y.x, (float)loc_y.y, (float)loc_y.z, (float)loc_y.w) + 0.5f) * st_delta.w;
	f0_z = (float4)0.0f;
	f0_w = (float4)0.0f;

	//	r1 = f0;
	r1_x = f0_x;
	r1_y = f0_y;
	r1_z = f0_z;
	r1_w = f0_w;

	//	r0.x = dot(r1.xy,l0.xy) + l0.w;
	r0_x = (r1_x * (float4)l0.x + r1_y * (float4) l0.y) + l0.w;

	//	r0.y = dot(r1.xy,l1.xy) + l1.w;
	r0_y = (r1_x * (float4)l1.x + r1_y * (float4) l1.y) + l1.w;

	//	r0 = read_imagef(t0, t_sampler0, r0.xy);
	// read inputs only once!
	float2 stride;
	stride.x = r0_x.y - r0_x.x;
	stride.y = r0_y.y - r0_y.x;
	__async_work_group_stream_from_image(t0, t_sampler0, (float2)(r0_x.x, r0_y.x), stride, Many, input_x, input_y, input_z, input_w);

	for (count = 0; count < Many/4; count+=2) {

		// extract the inputs you currently require
		r0_x = input_x[count];
		r0_y = input_y[count];
		r0_z = input_z[count];
		r0_w = input_w[count];
		r1_x = input_x[count+1];
		r1_y = input_y[count+1];
		r1_z = input_z[count+1];
		r1_w = input_w[count+1];

		//	r0.xyz = r0.www-r0.xyz;
		r0_x = r0_w - r0_x;
		r0_y = r0_w - r0_y;
		r0_z = r0_w - r0_z;
		r1_x = r1_w - r1_x;
		r1_y = r1_w - r1_y;
		r1_z = r1_w - r1_z;

		//	r0.xyz = min(r0.xyz, r0.www);
		r0_x = min(r0_x, r0_w);
		r0_y = min(r0_y, r0_w);
		r0_z = min(r0_z, r0_w);
		r1_x = min(r1_x, r1_w);
		r1_y = min(r1_y, r1_w);
		r1_z = min(r1_z, r1_w);

		output_x[count] = r0_x;
		output_y[count] = r0_y;
		output_z[count] = r0_z;
		output_w[count] = r0_w;
		output_x[count+1] = r1_x;
		output_y[count+1] = r1_y;
		output_z[count+1] = r1_z;
		output_w[count+1] = r1_w;
	}

	int yaxis;
	int yaxisT, yaxisF;

	yaxisT = get_image_height(dest) - (loc_y.x + dim.w + 1);
	yaxisF = loc_y.x + dim.w;
	yaxis = select (yaxisF, yaxisT, flipped);

	// write outputs only once, at last loop!
	__async_work_group_stream_to_image(dest, orig_loc_x + dim.z, yaxis, Many, output_x, output_y, output_z, output_w);
}



// move calculation used only once outside the loop
__kernel void program4_7(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, read_only image2d_t t0, sampler_t t_sampler0)
{
	//  float4       o0, r0, r1;
	float4 o0_x, o0_y, o0_z, o0_w;
	float4 r0_x, r0_y, r0_z, r0_w;
	float4 r1_x, r1_y, r1_z, r1_w;

	float4 f0_x, f0_y, f0_z, f0_w;

	// int2         loc = (int2)( get_global_id(0), get_global_id(1) );
	int4 loc_x, loc_y;

	loc_y = get_global_id(1);
	int count;
	const int Many = 128;

	int tmp = get_global_id(0) * Many;
	int orig_loc_x = tmp;

	float4 input_x[Many/4], input_y[Many/4], input_z[Many/4], input_w[Many/4];
	float4 output_x[Many/4], output_y[Many/4], output_z[Many/4], output_w[Many/4];

	loc_x = (int4)(tmp, tmp+1, tmp+2, tmp+3);

	// float4 f0 = (float4)( st_origin.x + ((float)loc.x + 0.5f) * st_delta.x + ((float)loc.y + 0.5f) * st_delta.z, st_origin.y + ((float)loc.x + 0.5f) * st_delta.y + ((float)loc.y + 0.5f) * st_delta.w, 0.0f, 0.0f );
	f0_x = st_origin.x + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.x + ((float4)((float)loc_y.x, (float)loc_y.y, (float)loc_y.z, (float)loc_y.w) + 0.5f) * st_delta.z;
	f0_y = st_origin.y + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.y + ((float4)((float)loc_y.x, (float)loc_y.y, (float)loc_y.z, (float)loc_y.w) + 0.5f) * st_delta.w;
	f0_z = (float4)0.0f;
	f0_w = (float4)0.0f;

	//	r1 = f0;
	r1_x = f0_x;
	r1_y = f0_y;
	r1_z = f0_z;
	r1_w = f0_w;

	//	r0.x = dot(r1.xy,l0.xy) + l0.w;
	r0_x = (r1_x * (float4)l0.x + r1_y * (float4) l0.y) + l0.w;

	//	r0.y = dot(r1.xy,l1.xy) + l1.w;
	r0_y = (r1_x * (float4)l1.x + r1_y * (float4) l1.y) + l1.w;

	//	r0 = read_imagef(t0, t_sampler0, r0.xy);
	// read inputs only once!
	float2 stride;
	stride.x = r0_x.y - r0_x.x;
	stride.y = r0_y.y - r0_y.x;
	__async_work_group_stream_from_image(t0, t_sampler0, (float2)(r0_x.x, r0_y.x), stride, Many, input_x, input_y, input_z, input_w);

	for (count = 0; count < Many/4; ++count) {

		// extract the inputs you currently require
		r0_x = input_x[count];
		r0_y = input_y[count];
		r0_z = input_z[count];
		r0_w = input_w[count];

		//	r0.xyz = r0.www-r0.xyz;
		r0_x = r0_w - r0_x;
		r0_y = r0_w - r0_y;
		r0_z = r0_w - r0_z;

		//	r0.xyz = min(r0.xyz, r0.www);
		r0_x = min(r0_x, r0_w);
		r0_y = min(r0_y, r0_w);
		r0_z = min(r0_z, r0_w);

		//	o0 = r0;
		o0_x = r0_x;
		o0_y = r0_y;
		o0_z = r0_z;
		o0_w = r0_w;

		output_x[count] = o0_x;
		output_y[count] = o0_y;
		output_z[count] = o0_z;
		output_w[count] = o0_w;
	}

	int yaxis;
	int yaxisT, yaxisF;

	yaxisT = get_image_height(dest) - (loc_y.x + dim.w + 1);
	yaxisF = loc_y.x + dim.w;
	yaxis = select (yaxisF, yaxisT, flipped);

	// write outputs only once, at last loop!
	__async_work_group_stream_to_image(dest, orig_loc_x + dim.z, yaxis, Many, output_x, output_y, output_z, output_w);
}

// replace reads/writes in program4_5 with a read/write that reads/writes 64 items in a time
// the read can be replace because the coordinates calculation consists of simple operations
// the write  can be replaced because we write to a stream a series of pixels one by one in a
// given order

// TODO : the read is originated by dot function that translates to * and +, does it still
// considered as produced by simple operations
__kernel void program4_6(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, read_only image2d_t t0, sampler_t t_sampler0)
{
	//  float4       o0, r0, r1;
	float4 o0_x, o0_y, o0_z, o0_w;
	float4 r0_x, r0_y, r0_z, r0_w;
	float4 r1_x, r1_y, r1_z, r1_w;

	float4 f0_x, f0_y, f0_z, f0_w;

	// int2         loc = (int2)( get_global_id(0), get_global_id(1) );
	int4 loc_x, loc_y;

	loc_y = get_global_id(1);
	int count;
	const int Many = 128;

	int tmp = get_global_id(0) * Many;
	int orig_loc_x = tmp;

	float4 input_x[Many/4], input_y[Many/4], input_z[Many/4], input_w[Many/4];
	float4 output_x[Many/4], output_y[Many/4], output_z[Many/4], output_w[Many/4];

	for (count = 0; count < Many/4; ++count) {

		loc_x = (int4)(tmp, tmp+1, tmp+2, tmp+3);

		// float4 f0 = (float4)( st_origin.x + ((float)loc.x + 0.5f) * st_delta.x + ((float)loc.y + 0.5f) * st_delta.z, st_origin.y + ((float)loc.x + 0.5f) * st_delta.y + ((float)loc.y + 0.5f) * st_delta.w, 0.0f, 0.0f );
		f0_x = st_origin.x + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.x + ((float4)((float)loc_y.x, (float)loc_y.y, (float)loc_y.z, (float)loc_y.w) + 0.5f) * st_delta.z;
		f0_y = st_origin.y + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.y + ((float4)((float)loc_y.x, (float)loc_y.y, (float)loc_y.z, (float)loc_y.w) + 0.5f) * st_delta.w;
		f0_z = (float4)0.0f;
		f0_w = (float4)0.0f;

		//	r1 = f0;
		r1_x = f0_x;
		r1_y = f0_y;
		r1_z = f0_z;
		r1_w = f0_w;

		//	r0.x = dot(r1.xy,l0.xy) + l0.w;
		r0_x = (r1_x * (float4)l0.x + r1_y * (float4) l0.y) + l0.w;

		//	r0.y = dot(r1.xy,l1.xy) + l1.w;
		r0_y = (r1_x * (float4)l1.x + r1_y * (float4) l1.y) + l1.w;

		//	r0 = read_imagef(t0, t_sampler0, r0.xy);
		// read inputs only once!
		if (count == 0)
		{
			float2 stride;
			stride.x = r0_x.y - r0_x.x;
			stride.y = r0_y.y - r0_y.x;
			__async_work_group_stream_from_image(t0, t_sampler0, (float2)(r0_x.x, r0_y.x), stride, Many, input_x, input_y, input_z, input_w);
		}
		// extract the inputs you currently require
		r0_x = input_x[count];
		r0_y = input_y[count];
		r0_z = input_z[count];
		r0_w = input_w[count];

		//	r0.xyz = r0.www-r0.xyz;
		r0_x = r0_w - r0_x;
		r0_y = r0_w - r0_y;
		r0_z = r0_w - r0_z;

		//	r0.xyz = min(r0.xyz, r0.www);
		r0_x = min(r0_x, r0_w);
		r0_y = min(r0_y, r0_w);
		r0_z = min(r0_z, r0_w);

		//	o0 = r0;
		o0_x = r0_x;
		o0_y = r0_y;
		o0_z = r0_z;
		o0_w = r0_w;

		int yaxis;
		int yaxisT, yaxisF;

		yaxisT = get_image_height(dest) - (loc_y.x + dim.w + 1);
		yaxisF = loc_y.x + dim.w;
		yaxis = select (yaxisF, yaxisT, flipped);

		output_x[count] = o0_x;
		output_y[count] = o0_y;
		output_z[count] = o0_z;
		output_w[count] = o0_w;
		// write outputs only once, at last loop!
		if (count == Many/4-1)
		{
			__async_work_group_stream_to_image(dest, orig_loc_x + dim.z, yaxis, Many, output_x, output_y, output_z, output_w);
		}

		tmp += 4;
	}
}

// add loop to make each instance in program4_4 work on 64 items, when they are groupe in vectores of 4
__kernel void program4_5(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, read_only image2d_t t0, sampler_t t_sampler0)
{
	//  float4       o0, r0, r1;
	float4 o0_x, o0_y, o0_z, o0_w;
	float4 r0_x, r0_y, r0_z, r0_w;
	float4 r1_x, r1_y, r1_z, r1_w;

	float4 f0_x, f0_y, f0_z, f0_w;

	// int2         loc = (int2)( get_global_id(0), get_global_id(1) );
	int4 loc_x, loc_y;

	loc_y = get_global_id(1);
	int count;
	const int Many = 128;

	int tmp = get_global_id(0) * Many;

	for (count = 0; count < Many/4; ++count) {

		loc_x = (int4)(tmp, tmp+1, tmp+2, tmp+3);

		// float4 f0 = (float4)( st_origin.x + ((float)loc.x + 0.5f) * st_delta.x + ((float)loc.y + 0.5f) * st_delta.z, st_origin.y + ((float)loc.x + 0.5f) * st_delta.y + ((float)loc.y + 0.5f) * st_delta.w, 0.0f, 0.0f );
		f0_x = st_origin.x + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.x + ((float4)((float)loc_y.x, (float)loc_y.y, (float)loc_y.z, (float)loc_y.w) + 0.5f) * st_delta.z;
		f0_y = st_origin.y + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.y + ((float4)((float)loc_y.x, (float)loc_y.y, (float)loc_y.z, (float)loc_y.w) + 0.5f) * st_delta.w;
		f0_z = (float4)0.0f;
		f0_w = (float4)0.0f;

		//	r1 = f0;
		r1_x = f0_x;
		r1_y = f0_y;
		r1_z = f0_z;
		r1_w = f0_w;

		//	r0.x = dot(r1.xy,l0.xy) + l0.w;
		r0_x = (r1_x * (float4)l0.x + r1_y * (float4) l0.y) + l0.w;

		//	r0.y = dot(r1.xy,l1.xy) + l1.w;
		r0_y = (r1_x * (float4)l1.x + r1_y * (float4) l1.y) + l1.w;

		//	r0 = read_imagef(t0, t_sampler0, r0.xy);
		read_transposed_imagef(t0, t_sampler0, r0_x, r0_y, &r0_x, &r0_y, &r0_z, &r0_w);

		//	r0.xyz = r0.www-r0.xyz;
		r0_x = r0_w - r0_x;
		r0_y = r0_w - r0_y;
		r0_z = r0_w - r0_z;

		//	r0.xyz = min(r0.xyz, r0.www);
		r0_x = min(r0_x, r0_w);
		r0_y = min(r0_y, r0_w);
		r0_z = min(r0_z, r0_w);

		//	o0 = r0;
		o0_x = r0_x;
		o0_y = r0_y;
		o0_z = r0_z;
		o0_w = r0_w;

		int yaxis;
		int yaxisT, yaxisF;

		yaxisT = get_image_height(dest) - (loc_y.x + dim.w + 1);
		yaxisF = loc_y.x + dim.w;
		yaxis = select (yaxisF, yaxisT, flipped);

		write_transposed_imagef(dest, loc_x.x + dim.z, yaxis, o0_x, o0_y, o0_z, o0_w);

		tmp += 4;
	}
}

// replace reads and writes in program4_3 with read_transposed_imagef and write_transposed_imagef
__kernel void program4_4(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, read_only image2d_t t0, sampler_t t_sampler0)
{
	//  float4       o0, r0, r1;
	float4 o0_x, o0_y, o0_z, o0_w;
	float4 r0_x, r0_y, r0_z, r0_w;
	float4 r1_x, r1_y, r1_z, r1_w;

	// int2         loc = (int2)( get_global_id(0), get_global_id(1) );
	int4 loc_x, loc_y;
	int tmp = get_global_id(0)*4;
	loc_x = (int4)(tmp, tmp+1, tmp+2, tmp+3);
	loc_y = get_global_id(1);

	// float4 f0 = (float4)( st_origin.x + ((float)loc.x + 0.5f) * st_delta.x + ((float)loc.y + 0.5f) * st_delta.z, st_origin.y + ((float)loc.x + 0.5f) * st_delta.y + ((float)loc.y + 0.5f) * st_delta.w, 0.0f, 0.0f );
	float4 f0_x = st_origin.x + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.x + ((float4)((float)loc_y.x, (float)loc_y.y, (float)loc_y.z, (float)loc_y.w) + 0.5f) * st_delta.z;
	float4 f0_y = st_origin.y + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.y + ((float4)((float)loc_y.x, (float)loc_y.y, (float)loc_y.z, (float)loc_y.w) + 0.5f) * st_delta.w;
	float4 f0_z = (float4)0.0f;
	float4 f0_w = (float4)0.0f;

	//	r1 = f0;
	r1_x = f0_x;
	r1_y = f0_y;
	r1_z = f0_z;
	r1_w = f0_w;

	//	r0.x = dot(r1.xy,l0.xy) + l0.w;
	r0_x = (r1_x * (float4)l0.x + r1_y * (float4) l0.y) + l0.w;

	//	r0.y = dot(r1.xy,l1.xy) + l1.w;
	r0_y = (r1_x * (float4)l1.x + r1_y * (float4) l1.y) + l1.w;

	//	r0 = read_imagef(t0, t_sampler0, r0.xy);
	read_transposed_imagef(t0, t_sampler0, r0_x, r0_y, &r0_x, &r0_y, &r0_z, &r0_w);

	//	r0.xyz = r0.www-r0.xyz;
	r0_x = r0_w - r0_x;
	r0_y = r0_w - r0_y;
	r0_z = r0_w - r0_z;

	//	r0.xyz = min(r0.xyz, r0.www);
	r0_x = min(r0_x, r0_w);
	r0_y = min(r0_y, r0_w);
	r0_z = min(r0_z, r0_w);

	//	o0 = r0;
	o0_x = r0_x;
	o0_y = r0_y;
	o0_z = r0_z;
	o0_w = r0_w;

	int yaxis;
	int yaxisT, yaxisF;

	yaxisT = get_image_height(dest) - (loc_y.x + dim.w + 1);
	yaxisF = loc_y.x + dim.w;
	yaxis = select (yaxisF, yaxisT, flipped);

	write_transposed_imagef(dest, loc_x.x + dim.z, yaxis, o0_x, o0_y, o0_z, o0_w);
}

// replace dot function in program4_2 with the mathimatical calulation and vectorize it
__kernel void program4_3(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, read_only image2d_t t0, sampler_t t_sampler0)
{
	//  float4       o0, r0, r1;
	float4 o0_x, o0_y, o0_z, o0_w;
	float4 r0_x, r0_y, r0_z, r0_w;
	float4 r1_x, r1_y, r1_z, r1_w;

	// int2         loc = (int2)( get_global_id(0), get_global_id(1) );
	int4 loc_x, loc_y;
	int tmp = get_global_id(0)*4;
	loc_x = (int4)(tmp, tmp+1, tmp+2, tmp+3);
	loc_y = get_global_id(1);

	// float4 f0 = (float4)( st_origin.x + ((float)loc.x + 0.5f) * st_delta.x + ((float)loc.y + 0.5f) * st_delta.z, st_origin.y + ((float)loc.x + 0.5f) * st_delta.y + ((float)loc.y + 0.5f) * st_delta.w, 0.0f, 0.0f );
	float4 f0_x = st_origin.x + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.x + ((float4)((float)loc_y.x, (float)loc_y.y, (float)loc_y.z, (float)loc_y.w) + 0.5f) * st_delta.z;
	float4 f0_y = st_origin.y + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.y + ((float4)((float)loc_y.x, (float)loc_y.y, (float)loc_y.z, (float)loc_y.w) + 0.5f) * st_delta.w;
	float4 f0_z = (float4)0.0f;
	float4 f0_w = (float4)0.0f;

	//	r1 = f0;
	r1_x = f0_x;
	r1_y = f0_y;
	r1_z = f0_z;
	r1_w = f0_w;

	//	r0.x = dot(r1.xy,l0.xy) + l0.w;
	r0_x = (r1_x * (float4)l0.x + r1_y * (float4) l0.y) + l0.w;

	//	r0.y = dot(r1.xy,l1.xy) + l1.w;
	r0_y = (r1_x * (float4)l1.x + r1_y * (float4) l1.y) + l1.w;

	//	r0 = read_imagef(t0, t_sampler0, r0.xy);
	float4 r0T0, r0T1, r0T2, r0T3;
	r0T0 = read_imagef(t0, t_sampler0, (float2)(r0_x.x, r0_y.x));
	r0T1 = read_imagef(t0, t_sampler0, (float2)(r0_x.y, r0_y.y));
	r0T2 = read_imagef(t0, t_sampler0, (float2)(r0_x.z, r0_y.z));
	r0T3 = read_imagef(t0, t_sampler0, (float2)(r0_x.w, r0_y.w));
	r0_x = (float4)(r0T0.x, r0T1.x, r0T2.x, r0T3.x);
	r0_y = (float4)(r0T0.y, r0T1.y, r0T2.y, r0T3.y);
	r0_z = (float4)(r0T0.z, r0T1.z, r0T2.z, r0T3.z);
	r0_w = (float4)(r0T0.w, r0T1.w, r0T2.w, r0T3.w);

	//	r0.xyz = r0.www-r0.xyz;
	r0_x = r0_w - r0_x;
	r0_y = r0_w - r0_y;
	r0_z = r0_w - r0_z;

	//	r0.xyz = min(r0.xyz, r0.www);
	r0_x = min(r0_x, r0_w);
	r0_y = min(r0_y, r0_w);
	r0_z = min(r0_z, r0_w);

	//	o0 = r0;
	o0_x = r0_x;
	o0_y = r0_y;
	o0_z = r0_z;
	o0_w = r0_w;

	write_imagef(dest, (int2)( loc_x.x + dim.z , flipped ? get_image_height(dest) - (loc_y.x + dim.w + 1) : loc_y.x + dim.w ), (float4)(o0_x.x, o0_y.x, o0_z.x, o0_w.x));
	write_imagef(dest, (int2)( loc_x.y + dim.z , flipped ? get_image_height(dest) - (loc_y.y + dim.w + 1) : loc_y.y + dim.w ), (float4)(o0_x.y, o0_y.y, o0_z.y, o0_w.y));
	write_imagef(dest, (int2)( loc_x.z + dim.z , flipped ? get_image_height(dest) - (loc_y.z + dim.w + 1) : loc_y.z + dim.w ), (float4)(o0_x.z, o0_y.z, o0_z.z, o0_w.z));
	write_imagef(dest, (int2)( loc_x.w + dim.z , flipped ? get_image_height(dest) - (loc_y.w + dim.w + 1) : loc_y.w + dim.w ), (float4)(o0_x.w, o0_y.w, o0_z.w, o0_w.w));
}

// vectorize program4_1
__kernel void program4_2(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, read_only image2d_t t0, sampler_t t_sampler0)
{
	//  float4       o0, r0, r1;
	float4 o0_x, o0_y, o0_z, o0_w;
	float4 r0_x, r0_y, r0_z, r0_w;
	float4 r1_x, r1_y, r1_z, r1_w;

	// int2         loc = (int2)( get_global_id(0), get_global_id(1) );
	int4 loc_x, loc_y;
	int tmp = get_global_id(0)*4;
	loc_x = (int4)(tmp, tmp+1, tmp+2, tmp+3);
	loc_y = get_global_id(1);

	// float4 f0 = (float4)( st_origin.x + ((float)loc.x + 0.5f) * st_delta.x + ((float)loc.y + 0.5f) * st_delta.z, st_origin.y + ((float)loc.x + 0.5f) * st_delta.y + ((float)loc.y + 0.5f) * st_delta.w, 0.0f, 0.0f );
	float4 f0_x = st_origin.x + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.x + ((float4)((float)loc_y.x, (float)loc_y.y, (float)loc_y.z, (float)loc_y.w) + 0.5f) * st_delta.z;
	float4 f0_y = st_origin.y + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.y + ((float4)((float)loc_y.x, (float)loc_y.y, (float)loc_y.z, (float)loc_y.w) + 0.5f) * st_delta.w;
	float4 f0_z = (float4)0.0f;
	float4 f0_w = (float4)0.0f;

	//	r1 = f0;
	r1_x = f0_x;
	r1_y = f0_y;
	r1_z = f0_z;
	r1_w = f0_w;

	//	r0.x = dot(r1.xy,l0.xy) + l0.w;
	r0_x.x = dot((float2)(r1_x.x, r1_y.x),l0.xy) + l0.w;
	r0_x.y = dot((float2)(r1_x.y, r1_y.y),l0.xy) + l0.w;
	r0_x.z = dot((float2)(r1_x.z, r1_y.z),l0.xy) + l0.w;
	r0_x.w = dot((float2)(r1_x.w, r1_y.w),l0.xy) + l0.w;

	//	r0.y = dot(r1.xy,l1.xy) + l1.w;
	r0_y.x = dot((float2)(r1_x.x, r1_y.x),l1.xy) + l1.w;
	r0_y.y = dot((float2)(r1_x.y, r1_y.y),l1.xy) + l1.w;
	r0_y.z = dot((float2)(r1_x.z, r1_y.z),l1.xy) + l1.w;
	r0_y.w = dot((float2)(r1_x.w, r1_y.w),l1.xy) + l1.w;

	//	r0 = read_imagef(t0, t_sampler0, r0.xy);
	float4 r0T0, r0T1, r0T2, r0T3;
	r0T0 = read_imagef(t0, t_sampler0, (float2)(r0_x.x, r0_y.x));
	r0T1 = read_imagef(t0, t_sampler0, (float2)(r0_x.y, r0_y.y));
	r0T2 = read_imagef(t0, t_sampler0, (float2)(r0_x.z, r0_y.z));
	r0T3 = read_imagef(t0, t_sampler0, (float2)(r0_x.w, r0_y.w));
	r0_x = (float4)(r0T0.x, r0T1.x, r0T2.x, r0T3.x);
	r0_y = (float4)(r0T0.y, r0T1.y, r0T2.y, r0T3.y);
	r0_z = (float4)(r0T0.z, r0T1.z, r0T2.z, r0T3.z);
	r0_w = (float4)(r0T0.w, r0T1.w, r0T2.w, r0T3.w);

	//	r0.xyz = r0.www-r0.xyz;
	r0_x = r0_w - r0_x;
	r0_y = r0_w - r0_y;
	r0_z = r0_w - r0_z;

	//	r0.xyz = min(r0.xyz, r0.www);
	r0_x = min(r0_x, r0_w);
	r0_y = min(r0_y, r0_w);
	r0_z = min(r0_z, r0_w);

	//	o0 = r0;
	o0_x = r0_x;
	o0_y = r0_y;
	o0_z = r0_z;
	o0_w = r0_w;

	write_imagef(dest, (int2)( loc_x.x + dim.z , flipped ? get_image_height(dest) - (loc_y.x + dim.w + 1) : loc_y.x + dim.w ), (float4)(o0_x.x, o0_y.x, o0_z.x, o0_w.x));
	write_imagef(dest, (int2)( loc_x.y + dim.z , flipped ? get_image_height(dest) - (loc_y.y + dim.w + 1) : loc_y.y + dim.w ), (float4)(o0_x.y, o0_y.y, o0_z.y, o0_w.y));
	write_imagef(dest, (int2)( loc_x.z + dim.z , flipped ? get_image_height(dest) - (loc_y.z + dim.w + 1) : loc_y.z + dim.w ), (float4)(o0_x.z, o0_y.z, o0_z.z, o0_w.z));
	write_imagef(dest, (int2)( loc_x.w + dim.z , flipped ? get_image_height(dest) - (loc_y.w + dim.w + 1) : loc_y.w + dim.w ), (float4)(o0_x.w, o0_y.w, o0_z.w, o0_w.w));
}

// scalarize program
__kernel void program4_1(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, read_only image2d_t t0, sampler_t t_sampler0)
{
	//  float4       o0, r0, r1;
	float o0_x, o0_y, o0_z, o0_w;
	float r0_x, r0_y, r0_z, r0_w;
	float r1_x, r1_y, r1_z, r1_w;

	// int2         loc = (int2)( get_global_id(0), get_global_id(1) );
	int loc_x = get_global_id(0);
	int loc_y = get_global_id(1);

	// float4 f0 = (float4)( st_origin.x + ((float)loc.x + 0.5f) * st_delta.x + ((float)loc.y + 0.5f) * st_delta.z, st_origin.y + ((float)loc.x + 0.5f) * st_delta.y + ((float)loc.y + 0.5f) * st_delta.w, 0.0f, 0.0f );
	float f0_x = st_origin.x + ((float)loc_x + 0.5f) * st_delta.x + ((float)loc_y + 0.5f) * st_delta.z;
	float f0_y = st_origin.y + ((float)loc_x + 0.5f) * st_delta.y + ((float)loc_y + 0.5f) * st_delta.w;
	float f0_z = 0.0f;
	float f0_w = 0.0f;

	//	r1 = f0;
	r1_x = f0_x;
	r1_y = f0_y;
	r1_z = f0_z;
	r1_w = f0_w;

	//	r0.x = dot(r1.xy,l0.xy) + l0.w;
	r0_x = dot((float2)(r1_x, r1_y),l0.xy) + l0.w;

	//	r0.y = dot(r1.xy,l1.xy) + l1.w;
	r0_y = dot((float2)(r1_x, r1_y),l1.xy) + l1.w;

	//	r0 = read_imagef(t0, t_sampler0, r0.xy);
	float4 r0T;
	r0T = read_imagef(t0, t_sampler0, (float2)(r0_x, r0_y));
	r0_x = r0T.x;
	r0_y = r0T.y;
	r0_z = r0T.z;
	r0_w = r0T.w;

	//	r0.xyz = r0.www-r0.xyz;
	r0_x = r0_w - r0_x;
	r0_y = r0_w - r0_y;
	r0_z = r0_w - r0_z;

	//	r0.xyz = min(r0.xyz, r0.www);
	r0_x = min(r0_x, r0_w);
	r0_y = min(r0_y, r0_w);
	r0_z = min(r0_z, r0_w);

	//	o0 = r0;
	o0_x = r0_x;
	o0_y = r0_y;
	o0_z = r0_z;
	o0_w = r0_w;

	write_imagef(dest, (int2)( loc_x + dim.z , flipped ? get_image_height(dest) - (loc_y + dim.w + 1) : loc_y + dim.w ), (float4)(o0_x, o0_y, o0_z, o0_w));
}

// =================================================================================================================
//                                                     original
// =================================================================================================================

__kernel void program(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, read_only image2d_t t0, sampler_t t_sampler0)
{
	int dest_width = dim.x;
	int dest_height = dim.y;
	float4 o0, r0, r1;
	float4 false_vector = (float4) 0.0f;
	float4 true_vector = (float4) 1.0f;
	float unused_float1;
	float2 unused_float2;
	__float3_SPI unused_float3;
	float4 unused_float4;
	int2 loc = (int2)( get_global_id(0), get_global_id(1) );
	float4 f0 = (float4)( st_origin.x + ((float)loc.x + 0.5f) * st_delta.x + ((float)loc.y + 0.5f) * st_delta.z, st_origin.y + ((float)loc.x + 0.5f) * st_delta.y + ((float)loc.y + 0.5f) * st_delta.w, 0.0f, 0.0f );
	r1 = f0;
	r0.x = dot(r1.xy,l0.xy) + l0.w;
	r0.y = dot(r1.xy,l1.xy) + l1.w;
	r0 = read_imagef(t0, t_sampler0, r0.xy);
	r0.xyz = r0.www-r0.xyz;
	r0.xyz = min(r0.xyz, r0.www);
	o0 = r0;
	write_imagef(dest, (int2)( loc.x + dim.z , flipped ? get_image_height(dest) - (loc.y + dim.w + 1) : loc.y + dim.w ), o0);
}

__kernel void program_trans(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, read_only image2d_t t0, sampler_t t_sampler0)
{
	float4 f0_start, f0_end, f0_delta, tf0_start[4], tf0_delta[4];
	float4 f1_start, f1_end, f1_delta, tf1_start[4], tf1_delta[4];
	float4 gr0_1_0[64], gr0_1_1[64], gr0_1_2[64], gr0_1_3[64];
	int index = 0;
	int total_index = 0;
	int write_amount = 0;
	int read_amount = 256;
	int write_offset = 0;
	float4 o_r[64], o_g[64], o_b[64], o_a[64];
	int dest_width = dim.x;
	int dest_height = dim.y;
	float4 o0, r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, r16, r17, r18, r19;
	float4 false_vector = (float4) 0.0f;
	float4 true_vector = (float4) 1.0f;
	float unused_float1;
	float2 unused_float2;
	__float3_SPI unused_float3;
	float4 unused_float4;
	float16 scratch16;
	int2 loc = (int2)( 0, get_global_id(0) );

	float4 loc_start = (float4)( st_origin.x + ((float)loc.x + 0.5f) * st_delta.x + ((float)loc.y + 0.5f) * st_delta.z, st_origin.y + ((float)loc.x + 0.5f) * st_delta.y + ((float)loc.y + 0.5f) * st_delta.w, 0.0f, 0.0f );
	float4 loc_end = loc_start + (float4)( (float)dest_width * st_delta.x, (float)dest_width * st_delta.y, 0.0f, 0.0f );
	r0 = (float4) 0.0f;
	r1 = (float4) 0.0f;

	// vertex start
	f0_start = loc_start;
	r1 = loc_start;
	r0.x = dot(r1.xy,l0.xy) + l0.w;
	r0.y = dot(r1.xy,l1.xy) + l1.w;
	f1_start = r0;

	// vertex end
	f0_end = loc_end;
	r1 = loc_end;
	r0.x = dot(r1.xy,l0.xy) + l0.w;
	r0.y = dot(r1.xy,l1.xy) + l1.w;
	f1_end = r0;

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

	__async_work_group_stream_from_image(t0, t_sampler0, f1_start.xy, f1_delta.xy, read_amount, (float4 *)gr0_1_0, (float4 *)gr0_1_1, (float4 *)gr0_1_2, (float4 *)gr0_1_3);
	for(; loc.x<dest_width; loc.x+=4)
	{
		r0 = gr0_1_0[index];
		r1 = gr0_1_1[index];
		r2 = gr0_1_2[index];
		r3 = gr0_1_3[index];
		r4 = r3-r0;
		r5 = r3-r1;
		r6 = r3-r2;
		r7 = r4;
		r8 = r5;
		r9 = r6;
		r10 = min(r7, r3);
		r11 = min(r8, r3);
		r12 = min(r9, r3);
		r13 = r10;
		r14 = r11;
		r15 = r12;
		r16 = r13;
		r17 = r14;
		r18 = r15;
		r19 = r3;
		o_r[index] = r16;
		o_g[index] = r17;
		o_b[index] = r18;
		o_a[index] = r19;
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
		}
		;
	}
	if (index> 0)
	{
		(void)__async_work_group_stream_to_image(dest, (size_t)(dim.z + write_offset), (size_t)(flipped ? get_image_height(dest) - (loc.y+dim.w+1): loc.y+dim.w), (size_t)(dest_width - write_offset), (const float4 *)o_r, (const float4 *)o_g, (const float4 *)o_b, (const float4 *)o_a);
	}
}

