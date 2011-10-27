// improve loop

__kernel void program4_7(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, float4 l2, read_only image2d_t t0, sampler_t t_sampler0)
{

	//    const float4 p0  = (float4)( 0x1.0c6f7ap-20, 0x1p+0, -0x1p+1, 0x1.8p+1 );

	const float4 p0_x = (float4) (0x1.0c6f7ap-20, 0x1.0c6f7ap-20, 0x1.0c6f7ap-20, 0x1.0c6f7ap-20);
	const float4 p0_y = (float4) (0x1p+0, 0x1p+0, 0x1p+0, 0x1p+0);
	const float4 p0_z = (float4) (-0x1p+1, -0x1p+1, -0x1p+1, -0x1p+1);
	const float4 p0_w = (float4) (0x1.8p+1, 0x1.8p+1, 0x1.8p+1, 0x1.8p+1);

	//    float4       o0, r0, r1, r2, r3;
	float4 f0_x, f0_y, f0_z, f0_w;
	float4 o0_x, o0_y, o0_z, o0_w;
	float4 r0_x, r0_y, r0_z, r0_w;
	float4 r1_x, r1_y, r1_z, r1_w;
	float4 r2_x, r2_y, r2_z, r2_w;
	float4 r3_x, r3_y, r3_z, r3_w;

	//    int2         loc = (int2)( get_global_id(0), get_global_id(1) );
	int4 loc_x, loc_y;

	loc_y = get_global_id(1);
	int count=0;
	const int Many = 128;

	int tmp = get_global_id(0) * Many;
	int orig_loc_x = tmp;

	float4 output_x[Many/4], output_y[Many/4], output_z[Many/4], output_w[Many/4];

	//for (count = 0; count < Many/4; ++count) 
        do
        {

		loc_x = (int4)(tmp, tmp+1, tmp+2, tmp+3);

		//    float4 f0 = (float4)( st_origin.x + ((float)loc.x + 0.5f) * st_delta.x + ((float)loc.y + 0.5f) * st_delta.z, st_origin.y + ((float)loc.x + 0.5f) * st_delta.y + ((float)loc.y + 0.5f) * st_delta.w, 0.0f, 0.0f );
		// TODO : does vectorizer broadcast st_origin.y for example ?
		// TODO : maybe use function convert_float4 instead for loc_x conversion?
		f0_x = ((float4)st_origin.x) + (((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w)) + ((float4)0.5f)) * ((float4)st_delta.x) + (((float4)((float)loc_y.x, (float)loc_y.y, (float)loc_y.z, (float)loc_y.w)) + ((float4)0.5f)) * ((float4)st_delta.z);
		f0_y = ((float4)st_origin.y) + (((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w)) + ((float4)0.5f)) * ((float4)st_delta.y) + (((float4)((float)loc_y.x, (float)loc_y.y, (float)loc_y.z, (float)loc_y.w)) + ((float4)0.5f)) * ((float4)st_delta.w);
		f0_z = (float4)0.0f;
		f0_w = (float4)0.0f;

		//    r3 = f0;
		r3_x = f0_x;
		r3_y = f0_y;
		r3_z = f0_z;
		r3_w = f0_w;

		//    r1 = r3-l0;
		r1_x = r3_x - (float4)l0.x;
		r1_y = r3_y - (float4)l0.y;
		r1_z = r3_z - (float4)l0.z;
		r1_w = r3_w - (float4)l0.w;

		//    r2 = dot(r1.xy,r1.xy);
		r2_x = r1_x * r1_x + r1_y * r1_y;
		r2_y = r1_x * r1_x + r1_y * r1_y;
		r2_z = r1_x * r1_x + r1_y * r1_y;
		r2_w = r1_x * r1_x + r1_y * r1_y;

		//    r3 = max(r2, p0.xxxx);
		r3_x = max(r2_x, p0_x);
		r3_y = max(r2_y, p0_x);
		r3_z = max(r2_z, p0_x);
		r3_w = max(r2_w, p0_x);

		//    r3 = half_rsqrt(r3.xxxx);
		r3_x = half_rsqrt(r3_x);
		r3_y = r3_x;
		r3_z = r3_x;
		r3_w = r3_x;

		//    r2 = r3*r2;
		r2_x = r3_x * r2_x;
		r2_y = r3_y * r2_y;
		r2_z = r3_z * r2_z;
		r2_w = r3_w * r2_w;

		//    r3 = clamp(r2*-l0.zzzz + p0.yyyy, 0.0f, 1.0f);
		// TODO : can you broadcast lo.z into vector? maybe better l0.zzzz ?
		r3_x = clamp(r2_x*-(float4)l0.z + p0_y, (float4)0.0f, (float4)1.0f);
		r3_y = clamp(r2_y*-(float4)l0.z + p0_y, (float4)0.0f, (float4)1.0f);
		r3_z = clamp(r2_z*-(float4)l0.z + p0_y, (float4)0.0f, (float4)1.0f);
		r3_w = clamp(r2_w*-(float4)l0.z + p0_y, (float4)0.0f, (float4)1.0f);

		//    r2 = r3*p0.zzzz + p0.wwww;
		r2_x = r3_x * p0_z + p0_w;
		r2_y = r3_y * p0_z + p0_w;
		r2_z = r3_z * p0_z + p0_w;
		r2_w = r3_w * p0_z + p0_w;

		//    r2 = r2*r3;
		r2_x = r2_x * r3_x;
		r2_y = r2_y * r3_y;
		r2_z = r2_z * r3_z;
		r2_w = r2_w * r3_w;

		//    r2 = r2*r3;
		r2_x = r2_x * r3_x;
		r2_y = r2_y * r3_y;
		r2_z = r2_z * r3_z;
		r2_w = r2_w * r3_w;

		//    r2 = r2*l0.wwww + p0.yyyy;
		r2_x = r2_x * (float4)l0.w + p0_y;
		r2_y = r2_y * (float4)l0.w + p0_y;
		r2_z = r2_z * (float4)l0.w + p0_y;
		r2_w = r2_w * (float4)l0.w + p0_y;

		//    r1 = r1*r2.xxxx + l0;
		r1_x = r1_x * r2_x + (float4)l0.x;
		r1_y = r1_y * r2_x + (float4)l0.y;
		r1_z = r1_z * r2_x + (float4)l0.z;
		r1_w = r1_w * r2_x + (float4)l0.w;

		//    r0.x = dot(r1.xy,l1.xy) + l1.w;
		r0_x = (r1_x * (float4)l1.x + r1_y * (float4)l1.y) + l1.w;

		//    r0.y = dot(r1.xy,l2.xy) + l2.w;
		r0_y = (r1_x * (float4)l2.x + r1_y * (float4)l2.y) + l2.w;

		//    r0 = read_imagef(t0, t_sampler0, r0.xy);
		read_transposed_imagef(t0, t_sampler0, r0_x, r0_y, &r0_x, &r0_y, &r0_z, &r0_w);

		//    r0.xyz = min(r0.xyz, r0.www);
		r0_x = min(r0_x, r0_w);
		r0_y = min(r0_y, r0_w);
		r0_z = min(r0_z, r0_w);

		//    o0 = r0;
		o0_x = r0_x;
		o0_y = r0_y;
		o0_z = r0_z;
		o0_w = r0_w;

		//    write_imagef(dest, (int2)( loc.x + dim.z , flipped ? get_image_height(dest) - (loc.y + dim.w + 1) : loc.y + dim.w ), o0);

		output_x[count] = o0_x;
		output_y[count] = o0_y;
		output_z[count] = o0_z;
		output_w[count] = o0_w;

		tmp +=4;
                ++count;
	} while (count < Many/4);

	int yaxis;
	int yaxisT, yaxisF;

	yaxisT = get_image_height(dest) - (loc_y.x + dim.w + 1);
	yaxisF = loc_y.x + dim.w;
	yaxis = select (yaxisF, yaxisT, flipped);
	__async_work_group_stream_to_image(dest, orig_loc_x + dim.z, yaxis, Many, output_x, output_y, output_z, output_w);

}





// replace writes in program4_5 with a write that writes 64 items in a time
// cannot replace reads because the reads depend on differnt (non simple) calculations
// and we cannot calculate a single stride for them in this test, the write though,
// can be replaced because we write to a stream a series of pixels one by one in a
// given order
__kernel void program4_6(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, float4 l2, read_only image2d_t t0, sampler_t t_sampler0)
{

	//    const float4 p0  = (float4)( 0x1.0c6f7ap-20, 0x1p+0, -0x1p+1, 0x1.8p+1 );

	const float4 p0_x = (float4) (0x1.0c6f7ap-20, 0x1.0c6f7ap-20, 0x1.0c6f7ap-20, 0x1.0c6f7ap-20);
	const float4 p0_y = (float4) (0x1p+0, 0x1p+0, 0x1p+0, 0x1p+0);
	const float4 p0_z = (float4) (-0x1p+1, -0x1p+1, -0x1p+1, -0x1p+1);
	const float4 p0_w = (float4) (0x1.8p+1, 0x1.8p+1, 0x1.8p+1, 0x1.8p+1);

	//    float4       o0, r0, r1, r2, r3;
	float4 f0_x, f0_y, f0_z, f0_w;
	float4 o0_x, o0_y, o0_z, o0_w;
	float4 r0_x, r0_y, r0_z, r0_w;
	float4 r1_x, r1_y, r1_z, r1_w;
	float4 r2_x, r2_y, r2_z, r2_w;
	float4 r3_x, r3_y, r3_z, r3_w;

	//    int2         loc = (int2)( get_global_id(0), get_global_id(1) );
	int4 loc_x, loc_y;

	loc_y = get_global_id(1);
	int count;
	const int Many = 128;

	int tmp = get_global_id(0) * Many;
	int orig_loc_x = tmp;

	float4 output_x[Many/4], output_y[Many/4], output_z[Many/4], output_w[Many/4];

	for (count = 0; count < Many/4; ++count) {

		loc_x = (int4)(tmp, tmp+1, tmp+2, tmp+3);

		//    float4 f0 = (float4)( st_origin.x + ((float)loc.x + 0.5f) * st_delta.x + ((float)loc.y + 0.5f) * st_delta.z, st_origin.y + ((float)loc.x + 0.5f) * st_delta.y + ((float)loc.y + 0.5f) * st_delta.w, 0.0f, 0.0f );
		// TODO : does vectorizer broadcast st_origin.y for example ?
		// TODO : maybe use function convert_float4 instead for loc_x conversion?
		f0_x = ((float4)st_origin.x) + (((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w)) + ((float4)0.5f)) * ((float4)st_delta.x) + (((float4)((float)loc_y.x, (float)loc_y.y, (float)loc_y.z, (float)loc_y.w)) + ((float4)0.5f)) * ((float4)st_delta.z);
		f0_y = ((float4)st_origin.y) + (((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w)) + ((float4)0.5f)) * ((float4)st_delta.y) + (((float4)((float)loc_y.x, (float)loc_y.y, (float)loc_y.z, (float)loc_y.w)) + ((float4)0.5f)) * ((float4)st_delta.w);
		f0_z = (float4)0.0f;
		f0_w = (float4)0.0f;

		//    r3 = f0;
		r3_x = f0_x;
		r3_y = f0_y;
		r3_z = f0_z;
		r3_w = f0_w;

		//    r1 = r3-l0;
		r1_x = r3_x - (float4)l0.x;
		r1_y = r3_y - (float4)l0.y;
		r1_z = r3_z - (float4)l0.z;
		r1_w = r3_w - (float4)l0.w;

		//    r2 = dot(r1.xy,r1.xy);
		r2_x = r1_x * r1_x + r1_y * r1_y;
		r2_y = r1_x * r1_x + r1_y * r1_y;
		r2_z = r1_x * r1_x + r1_y * r1_y;
		r2_w = r1_x * r1_x + r1_y * r1_y;

		//    r3 = max(r2, p0.xxxx);
		r3_x = max(r2_x, p0_x);
		r3_y = max(r2_y, p0_x);
		r3_z = max(r2_z, p0_x);
		r3_w = max(r2_w, p0_x);

		//    r3 = half_rsqrt(r3.xxxx);
		r3_x = half_rsqrt(r3_x);
		r3_y = half_rsqrt(r3_x);
		r3_z = half_rsqrt(r3_x);
		r3_w = half_rsqrt(r3_x);

		//    r2 = r3*r2;
		r2_x = r3_x * r2_x;
		r2_y = r3_y * r2_y;
		r2_z = r3_z * r2_z;
		r2_w = r3_w * r2_w;

		//    r3 = clamp(r2*-l0.zzzz + p0.yyyy, 0.0f, 1.0f);
		// TODO : can you broadcast lo.z into vector? maybe better l0.zzzz ?
		r3_x = clamp(r2_x*-(float4)l0.z + p0_y, (float4)0.0f, (float4)1.0f);
		r3_y = clamp(r2_y*-(float4)l0.z + p0_y, (float4)0.0f, (float4)1.0f);
		r3_z = clamp(r2_z*-(float4)l0.z + p0_y, (float4)0.0f, (float4)1.0f);
		r3_w = clamp(r2_w*-(float4)l0.z + p0_y, (float4)0.0f, (float4)1.0f);

		//    r2 = r3*p0.zzzz + p0.wwww;
		r2_x = r3_x * p0_z + p0_w;
		r2_y = r3_y * p0_z + p0_w;
		r2_z = r3_z * p0_z + p0_w;
		r2_w = r3_w * p0_z + p0_w;

		//    r2 = r2*r3;
		r2_x = r2_x * r3_x;
		r2_y = r2_y * r3_y;
		r2_z = r2_z * r3_z;
		r2_w = r2_w * r3_w;

		//    r2 = r2*r3;
		r2_x = r2_x * r3_x;
		r2_y = r2_y * r3_y;
		r2_z = r2_z * r3_z;
		r2_w = r2_w * r3_w;

		//    r2 = r2*l0.wwww + p0.yyyy;
		r2_x = r2_x * (float4)l0.w + p0_y;
		r2_y = r2_y * (float4)l0.w + p0_y;
		r2_z = r2_z * (float4)l0.w + p0_y;
		r2_w = r2_w * (float4)l0.w + p0_y;

		//    r1 = r1*r2.xxxx + l0;
		r1_x = r1_x * r2_x + (float4)l0.x;
		r1_y = r1_y * r2_x + (float4)l0.y;
		r1_z = r1_z * r2_x + (float4)l0.z;
		r1_w = r1_w * r2_x + (float4)l0.w;

		//    r0.x = dot(r1.xy,l1.xy) + l1.w;
		r0_x = (r1_x * (float4)l1.x + r1_y * (float4)l1.y) + l1.w;

		//    r0.y = dot(r1.xy,l2.xy) + l2.w;
		r0_y = (r1_x * (float4)l2.x + r1_y * (float4)l2.y) + l2.w;

		//    r0 = read_imagef(t0, t_sampler0, r0.xy);
		read_transposed_imagef(t0, t_sampler0, r0_x, r0_y, &r0_x, &r0_y, &r0_z, &r0_w);

		//    r0.xyz = min(r0.xyz, r0.www);
		r0_x = min(r0_x, r0_w);
		r0_y = min(r0_y, r0_w);
		r0_z = min(r0_z, r0_w);

		//    o0 = r0;
		o0_x = r0_x;
		o0_y = r0_y;
		o0_z = r0_z;
		o0_w = r0_w;

		//    write_imagef(dest, (int2)( loc.x + dim.z , flipped ? get_image_height(dest) - (loc.y + dim.w + 1) : loc.y + dim.w ), o0);
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

		tmp +=4;
	}

}

// add loop to make each instance in program4_4 work on 64 items, when they are groupe in vectores of 4
__kernel void program4_5(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, float4 l2, read_only image2d_t t0, sampler_t t_sampler0)
{

	//    const float4 p0  = (float4)( 0x1.0c6f7ap-20, 0x1p+0, -0x1p+1, 0x1.8p+1 );

	const float4 p0_x = (float4) (0x1.0c6f7ap-20, 0x1.0c6f7ap-20, 0x1.0c6f7ap-20, 0x1.0c6f7ap-20);
	const float4 p0_y = (float4) (0x1p+0, 0x1p+0, 0x1p+0, 0x1p+0);
	const float4 p0_z = (float4) (-0x1p+1, -0x1p+1, -0x1p+1, -0x1p+1);
	const float4 p0_w = (float4) (0x1.8p+1, 0x1.8p+1, 0x1.8p+1, 0x1.8p+1);

	//    float4       o0, r0, r1, r2, r3;
	float4 f0_x, f0_y, f0_z, f0_w;
	float4 o0_x, o0_y, o0_z, o0_w;
	float4 r0_x, r0_y, r0_z, r0_w;
	float4 r1_x, r1_y, r1_z, r1_w;
	float4 r2_x, r2_y, r2_z, r2_w;
	float4 r3_x, r3_y, r3_z, r3_w;

	//    int2         loc = (int2)( get_global_id(0), get_global_id(1) );
	int4 loc_x, loc_y;

	loc_y = get_global_id(1);
	int count;
	const int Many = 128;

	int tmp = get_global_id(0) * Many;

	for (count = 0; count < Many/4; ++count) {

		loc_x = (int4)(tmp, tmp+1, tmp+2, tmp+3);

		//    float4 f0 = (float4)( st_origin.x + ((float)loc.x + 0.5f) * st_delta.x + ((float)loc.y + 0.5f) * st_delta.z, st_origin.y + ((float)loc.x + 0.5f) * st_delta.y + ((float)loc.y + 0.5f) * st_delta.w, 0.0f, 0.0f );
		f0_x = ((float4)st_origin.x) + (((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w)) + ((float4)0.5f)) * ((float4)st_delta.x) + (((float4)((float)loc_y.x, (float)loc_y.y, (float)loc_y.z, (float)loc_y.w)) + ((float4)0.5f)) * ((float4)st_delta.z);
		f0_y = ((float4)st_origin.y) + (((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w)) + ((float4)0.5f)) * ((float4)st_delta.y) + (((float4)((float)loc_y.x, (float)loc_y.y, (float)loc_y.z, (float)loc_y.w)) + ((float4)0.5f)) * ((float4)st_delta.w);
		f0_z = (float4)0.0f;
		f0_w = (float4)0.0f;

		//    r3 = f0;
		r3_x = f0_x;
		r3_y = f0_y;
		r3_z = f0_z;
		r3_w = f0_w;

		//    r1 = r3-l0;
		r1_x = r3_x - (float4)l0.x;
		r1_y = r3_y - (float4)l0.y;
		r1_z = r3_z - (float4)l0.z;
		r1_w = r3_w - (float4)l0.w;

		//    r2 = dot(r1.xy,r1.xy);
		r2_x = r1_x * r1_x + r1_y * r1_y;
		r2_y = r1_x * r1_x + r1_y * r1_y;
		r2_z = r1_x * r1_x + r1_y * r1_y;
		r2_w = r1_x * r1_x + r1_y * r1_y;

		//    r3 = max(r2, p0.xxxx);
		r3_x = max(r2_x, p0_x);
		r3_y = max(r2_y, p0_x);
		r3_z = max(r2_z, p0_x);
		r3_w = max(r2_w, p0_x);

		//    r3 = half_rsqrt(r3.xxxx);
		r3_x = half_rsqrt(r3_x);
		r3_y = half_rsqrt(r3_x);
		r3_z = half_rsqrt(r3_x);
		r3_w = half_rsqrt(r3_x);

		//    r2 = r3*r2;
		r2_x = r3_x * r2_x;
		r2_y = r3_y * r2_y;
		r2_z = r3_z * r2_z;
		r2_w = r3_w * r2_w;

		//    r3 = clamp(r2*-l0.zzzz + p0.yyyy, 0.0f, 1.0f);
		r3_x = clamp(r2_x*-(float4)l0.z + p0_y, (float4)0.0f, (float4)1.0f);
		r3_y = clamp(r2_y*-(float4)l0.z + p0_y, (float4)0.0f, (float4)1.0f);
		r3_z = clamp(r2_z*-(float4)l0.z + p0_y, (float4)0.0f, (float4)1.0f);
		r3_w = clamp(r2_w*-(float4)l0.z + p0_y, (float4)0.0f, (float4)1.0f);

		//    r2 = r3*p0.zzzz + p0.wwww;
		r2_x = r3_x * p0_z + p0_w;
		r2_y = r3_y * p0_z + p0_w;
		r2_z = r3_z * p0_z + p0_w;
		r2_w = r3_w * p0_z + p0_w;

		//    r2 = r2*r3;
		r2_x = r2_x * r3_x;
		r2_y = r2_y * r3_y;
		r2_z = r2_z * r3_z;
		r2_w = r2_w * r3_w;

		//    r2 = r2*r3;
		r2_x = r2_x * r3_x;
		r2_y = r2_y * r3_y;
		r2_z = r2_z * r3_z;
		r2_w = r2_w * r3_w;

		//    r2 = r2*l0.wwww + p0.yyyy;
		r2_x = r2_x * (float4)l0.w + p0_y;
		r2_y = r2_y * (float4)l0.w + p0_y;
		r2_z = r2_z * (float4)l0.w + p0_y;
		r2_w = r2_w * (float4)l0.w + p0_y;

		//    r1 = r1*r2.xxxx + l0;
		r1_x = r1_x * r2_x + (float4)l0.x;
		r1_y = r1_y * r2_x + (float4)l0.y;
		r1_z = r1_z * r2_x + (float4)l0.z;
		r1_w = r1_w * r2_x + (float4)l0.w;

		//    r0.x = dot(r1.xy,l1.xy) + l1.w;
		r0_x = (r1_x * (float4)l1.x + r1_y * (float4)l1.y) + l1.w;

		//    r0.y = dot(r1.xy,l2.xy) + l2.w;
		r0_y = (r1_x * (float4)l2.x + r1_y * (float4)l2.y) + l2.w;

		//    r0 = read_imagef(t0, t_sampler0, r0.xy);
		read_transposed_imagef(t0, t_sampler0, r0_x, r0_y, &r0_x, &r0_y, &r0_z, &r0_w);

		//    r0.xyz = min(r0.xyz, r0.www);
		r0_x = min(r0_x, r0_w);
		r0_y = min(r0_y, r0_w);
		r0_z = min(r0_z, r0_w);

		//    o0 = r0;
		o0_x = r0_x;
		o0_y = r0_y;
		o0_z = r0_z;
		o0_w = r0_w;

		//    write_imagef(dest, (int2)( loc.x + dim.z , flipped ? get_image_height(dest) - (loc.y + dim.w + 1) : loc.y + dim.w ), o0);
		int yaxis;
		int yaxisT, yaxisF;

		yaxisT = get_image_height(dest) - (loc_y.x + dim.w + 1);
		yaxisF = loc_y.x + dim.w;
		yaxis = select (yaxisF, yaxisT, flipped);

		write_transposed_imagef(dest, loc_x.x + dim.z, yaxis, o0_x, o0_y, o0_z, o0_w);

		tmp +=4;
	}

}

// replace reads and writes in program4_3 with read_transposed_imagef and write_transposed_imagef
__kernel void program4_4(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, float4 l2, read_only image2d_t t0, sampler_t t_sampler0)
{

	//    const float4 p0  = (float4)( 0x1.0c6f7ap-20, 0x1p+0, -0x1p+1, 0x1.8p+1 );

	const float4 p0_x = (float4) (0x1.0c6f7ap-20, 0x1.0c6f7ap-20, 0x1.0c6f7ap-20, 0x1.0c6f7ap-20);
	const float4 p0_y = (float4) (0x1p+0, 0x1p+0, 0x1p+0, 0x1p+0);
	const float4 p0_z = (float4) (-0x1p+1, -0x1p+1, -0x1p+1, -0x1p+1);
	const float4 p0_w = (float4) (0x1.8p+1, 0x1.8p+1, 0x1.8p+1, 0x1.8p+1);

	//    float4       o0, r0, r1, r2, r3;
	float4 o0_x, o0_y, o0_z, o0_w;
	float4 r0_x, r0_y, r0_z, r0_w;
	float4 r1_x, r1_y, r1_z, r1_w;
	float4 r2_x, r2_y, r2_z, r2_w;
	float4 r3_x, r3_y, r3_z, r3_w;

	//    int2         loc = (int2)( get_global_id(0), get_global_id(1) );
	int4 loc_x, loc_y;
	int tmp = get_global_id(0)*4;
	loc_x = (int4)(tmp, tmp+1, tmp+2, tmp+3);
	loc_y = get_global_id(1);

	//    float4 f0 = (float4)( st_origin.x + ((float)loc.x + 0.5f) * st_delta.x + ((float)loc.y + 0.5f) * st_delta.z, st_origin.y + ((float)loc.x + 0.5f) * st_delta.y + ((float)loc.y + 0.5f) * st_delta.w, 0.0f, 0.0f );
	float4 f0_x = ((float4)st_origin.x) + (((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w)) + ((float4)0.5f)) * ((float4)st_delta.x) + (((float4)((float)loc_y.x, (float)loc_y.y, (float)loc_y.z, (float)loc_y.w)) + ((float4)0.5f)) * ((float4)st_delta.z);
	float4 f0_y = ((float4)st_origin.y) + (((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w)) + ((float4)0.5f)) * ((float4)st_delta.y) + (((float4)((float)loc_y.x, (float)loc_y.y, (float)loc_y.z, (float)loc_y.w)) + ((float4)0.5f)) * ((float4)st_delta.w);
	float4 f0_z = (float4)0.0f;
	float4 f0_w = (float4)0.0f;

	//    r3 = f0;
	r3_x = f0_x;
	r3_y = f0_y;
	r3_z = f0_z;
	r3_w = f0_w;

	//    r1 = r3-l0;
	r1_x = r3_x - (float4)l0.x;
	r1_y = r3_y - (float4)l0.y;
	r1_z = r3_z - (float4)l0.z;
	r1_w = r3_w - (float4)l0.w;

	//    r2 = dot(r1.xy,r1.xy);
	r2_x = r1_x * r1_x + r1_y * r1_y;
	r2_y = r1_x * r1_x + r1_y * r1_y;
	r2_z = r1_x * r1_x + r1_y * r1_y;
	r2_w = r1_x * r1_x + r1_y * r1_y;

	//    r3 = max(r2, p0.xxxx);
	r3_x = max(r2_x, p0_x);
	r3_y = max(r2_y, p0_x);
	r3_z = max(r2_z, p0_x);
	r3_w = max(r2_w, p0_x);

	//    r3 = half_rsqrt(r3.xxxx);
	r3_x = half_rsqrt(r3_x);
	r3_y = half_rsqrt(r3_x);
	r3_z = half_rsqrt(r3_x);
	r3_w = half_rsqrt(r3_x);

	//    r2 = r3*r2;
	r2_x = r3_x * r2_x;
	r2_y = r3_y * r2_y;
	r2_z = r3_z * r2_z;
	r2_w = r3_w * r2_w;

	//    r3 = clamp(r2*-l0.zzzz + p0.yyyy, 0.0f, 1.0f);
	r3_x = clamp(r2_x*-(float4)l0.z + p0_y, (float4)0.0f, (float4)1.0f);
	r3_y = clamp(r2_y*-(float4)l0.z + p0_y, (float4)0.0f, (float4)1.0f);
	r3_z = clamp(r2_z*-(float4)l0.z + p0_y, (float4)0.0f, (float4)1.0f);
	r3_w = clamp(r2_w*-(float4)l0.z + p0_y, (float4)0.0f, (float4)1.0f);

	//    r2 = r3*p0.zzzz + p0.wwww;
	r2_x = r3_x * p0_z + p0_w;
	r2_y = r3_y * p0_z + p0_w;
	r2_z = r3_z * p0_z + p0_w;
	r2_w = r3_w * p0_z + p0_w;

	//    r2 = r2*r3;
	r2_x = r2_x * r3_x;
	r2_y = r2_y * r3_y;
	r2_z = r2_z * r3_z;
	r2_w = r2_w * r3_w;

	//    r2 = r2*r3;
	r2_x = r2_x * r3_x;
	r2_y = r2_y * r3_y;
	r2_z = r2_z * r3_z;
	r2_w = r2_w * r3_w;

	//    r2 = r2*l0.wwww + p0.yyyy;
	r2_x = r2_x * (float4)l0.w + p0_y;
	r2_y = r2_y * (float4)l0.w + p0_y;
	r2_z = r2_z * (float4)l0.w + p0_y;
	r2_w = r2_w * (float4)l0.w + p0_y;

	//    r1 = r1*r2.xxxx + l0;
	r1_x = r1_x * r2_x + (float4)l0.x;
	r1_y = r1_y * r2_x + (float4)l0.y;
	r1_z = r1_z * r2_x + (float4)l0.z;
	r1_w = r1_w * r2_x + (float4)l0.w;

	//    r0.x = dot(r1.xy,l1.xy) + l1.w;
	r0_x = (r1_x * (float4)l1.x + r1_y * (float4)l1.y) + l1.w;

	//    r0.y = dot(r1.xy,l2.xy) + l2.w;
	r0_y = (r1_x * (float4)l2.x + r1_y * (float4)l2.y) + l2.w;

	//    r0 = read_imagef(t0, t_sampler0, r0.xy);
	read_transposed_imagef(t0, t_sampler0, r0_x, r0_y, &r0_x, &r0_y, &r0_z, &r0_w);

	//    r0.xyz = min(r0.xyz, r0.www);
	r0_x = min(r0_x, r0_w);
	r0_y = min(r0_y, r0_w);
	r0_z = min(r0_z, r0_w);

	//    o0 = r0;
	o0_x = r0_x;
	o0_y = r0_y;
	o0_z = r0_z;
	o0_w = r0_w;

	//    write_imagef(dest, (int2)( loc.x + dim.z , flipped ? get_image_height(dest) - (loc.y + dim.w + 1) : loc.y + dim.w ), o0);
	int yaxis;
	int yaxisT, yaxisF;

	yaxisT = get_image_height(dest) - (loc_y.x + dim.w + 1);
	yaxisF = loc_y.x + dim.w;
	yaxis = select (yaxisF, yaxisT, flipped);

	write_transposed_imagef(dest, loc_x.x + dim.z, yaxis, o0_x, o0_y, o0_z, o0_w);

}

// replace dot function in program4_2 with the mathimatical calulation and vectorize it
__kernel void program4_3(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, float4 l2, read_only image2d_t t0, sampler_t t_sampler0)
{

	//    const float4 p0  = (float4)( 0x1.0c6f7ap-20, 0x1p+0, -0x1p+1, 0x1.8p+1 );

	const float4 p0_x = (float4) (0x1.0c6f7ap-20, 0x1.0c6f7ap-20, 0x1.0c6f7ap-20, 0x1.0c6f7ap-20);
	const float4 p0_y = (float4) (0x1p+0, 0x1p+0, 0x1p+0, 0x1p+0);
	const float4 p0_z = (float4) (-0x1p+1, -0x1p+1, -0x1p+1, -0x1p+1);
	const float4 p0_w = (float4) (0x1.8p+1, 0x1.8p+1, 0x1.8p+1, 0x1.8p+1);

	//    float4       o0, r0, r1, r2, r3;
	float4 o0_x, o0_y, o0_z, o0_w;
	float4 r0_x, r0_y, r0_z, r0_w;
	float4 r1_x, r1_y, r1_z, r1_w;
	float4 r2_x, r2_y, r2_z, r2_w;
	float4 r3_x, r3_y, r3_z, r3_w;

	//    int2         loc = (int2)( get_global_id(0), get_global_id(1) );
	int4 loc_x, loc_y;
	int tmp = get_global_id(0)*4;
	loc_x = (int4)(tmp, tmp+1, tmp+2, tmp+3);
	loc_y = get_global_id(1);

	//    float4 f0 = (float4)( st_origin.x + ((float)loc.x + 0.5f) * st_delta.x + ((float)loc.y + 0.5f) * st_delta.z, st_origin.y + ((float)loc.x + 0.5f) * st_delta.y + ((float)loc.y + 0.5f) * st_delta.w, 0.0f, 0.0f );
	float4 f0_x = ((float4)st_origin.x) + (((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w)) + ((float4)0.5f)) * ((float4)st_delta.x) + (((float4)((float)loc_y.x, (float)loc_y.y, (float)loc_y.z, (float)loc_y.w)) + ((float4)0.5f)) * ((float4)st_delta.z);
	float4 f0_y = ((float4)st_origin.y) + (((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w)) + ((float4)0.5f)) * ((float4)st_delta.y) + (((float4)((float)loc_y.x, (float)loc_y.y, (float)loc_y.z, (float)loc_y.w)) + ((float4)0.5f)) * ((float4)st_delta.w);
	float4 f0_z = (float4)0.0f;
	float4 f0_w = (float4)0.0f;

	//    r3 = f0;
	r3_x = f0_x;
	r3_y = f0_y;
	r3_z = f0_z;
	r3_w = f0_w;

	//    r1 = r3-l0;
	r1_x = r3_x - (float4)l0.x;
	r1_y = r3_y - (float4)l0.y;
	r1_z = r3_z - (float4)l0.z;
	r1_w = r3_w - (float4)l0.w;

	//    r2 = dot(r1.xy,r1.xy);
	r2_x = r1_x * r1_x + r1_y * r1_y;
	r2_y = r1_x * r1_x + r1_y * r1_y;
	r2_z = r1_x * r1_x + r1_y * r1_y;
	r2_w = r1_x * r1_x + r1_y * r1_y;

	//    r3 = max(r2, p0.xxxx);
	r3_x = max(r2_x, p0_x);
	r3_y = max(r2_y, p0_x);
	r3_z = max(r2_z, p0_x);
	r3_w = max(r2_w, p0_x);

	//    r3 = half_rsqrt(r3.xxxx);
	r3_x = half_rsqrt(r3_x);
	r3_y = half_rsqrt(r3_x);
	r3_z = half_rsqrt(r3_x);
	r3_w = half_rsqrt(r3_x);

	//    r2 = r3*r2;
	r2_x = r3_x * r2_x;
	r2_y = r3_y * r2_y;
	r2_z = r3_z * r2_z;
	r2_w = r3_w * r2_w;

	//    r3 = clamp(r2*-l0.zzzz + p0.yyyy, 0.0f, 1.0f);
	r3_x = clamp(r2_x*-(float4)l0.z + p0_y, (float4)0.0f, (float4)1.0f);
	r3_y = clamp(r2_y*-(float4)l0.z + p0_y, (float4)0.0f, (float4)1.0f);
	r3_z = clamp(r2_z*-(float4)l0.z + p0_y, (float4)0.0f, (float4)1.0f);
	r3_w = clamp(r2_w*-(float4)l0.z + p0_y, (float4)0.0f, (float4)1.0f);

	//    r2 = r3*p0.zzzz + p0.wwww;
	r2_x = r3_x * p0_z + p0_w;
	r2_y = r3_y * p0_z + p0_w;
	r2_z = r3_z * p0_z + p0_w;
	r2_w = r3_w * p0_z + p0_w;

	//    r2 = r2*r3;
	r2_x = r2_x * r3_x;
	r2_y = r2_y * r3_y;
	r2_z = r2_z * r3_z;
	r2_w = r2_w * r3_w;

	//    r2 = r2*r3;
	r2_x = r2_x * r3_x;
	r2_y = r2_y * r3_y;
	r2_z = r2_z * r3_z;
	r2_w = r2_w * r3_w;

	//    r2 = r2*l0.wwww + p0.yyyy;
	r2_x = r2_x * (float4)l0.w + p0_y;
	r2_y = r2_y * (float4)l0.w + p0_y;
	r2_z = r2_z * (float4)l0.w + p0_y;
	r2_w = r2_w * (float4)l0.w + p0_y;

	//    r1 = r1*r2.xxxx + l0;
	r1_x = r1_x * r2_x + (float4)l0.x;
	r1_y = r1_y * r2_x + (float4)l0.y;
	r1_z = r1_z * r2_x + (float4)l0.z;
	r1_w = r1_w * r2_x + (float4)l0.w;

	//    r0.x = dot(r1.xy,l1.xy) + l1.w;
	r0_x = (r1_x * (float4)l1.x + r1_y * (float4)l1.y) + l1.w;

	//    r0.y = dot(r1.xy,l2.xy) + l2.w;
	r0_y = (r1_x * (float4)l2.x + r1_y * (float4)l2.y) + l2.w;

	//    r0 = read_imagef(t0, t_sampler0, r0.xy);

	float4 r0T0, r0T1, r0T2, r0T3;
	r0T0 = read_imagef(t0, t_sampler0, (float2)(r0_x.x, r0_y.x));
	r0T1 = read_imagef(t0, t_sampler0, (float2)(r0_x.y, r0_y.y));
	r0T2 = read_imagef(t0, t_sampler0, (float2)(r0_x.z, r0_y.z));
	r0T3 = read_imagef(t0, t_sampler0, (float2)(r0_x.w, r0_y.w));
	// transpose back
	r0_x = (float4)(r0T0.x, r0T1.x, r0T2.x, r0T3.x);
	r0_y = (float4)(r0T0.y, r0T1.y, r0T2.y, r0T3.y);
	r0_z = (float4)(r0T0.z, r0T1.z, r0T2.z, r0T3.z);
	r0_w = (float4)(r0T0.w, r0T1.w, r0T2.w, r0T3.w);

	//    r0.xyz = min(r0.xyz, r0.www);
	r0_x = min(r0_x, r0_w);
	r0_y = min(r0_y, r0_w);
	r0_z = min(r0_z, r0_w);

	//    o0 = r0;
	o0_x = r0_x;
	o0_y = r0_y;
	o0_z = r0_z;
	o0_w = r0_w;

	//    write_imagef(dest, (int2)( loc.x + dim.z , flipped ? get_image_height(dest) - (loc.y + dim.w + 1) : loc.y + dim.w ), o0);
	write_imagef(dest, (int2)( loc_x.x + dim.z , flipped ? get_image_height(dest) - (loc_y.x + dim.w + 1) : loc_y.x + dim.w ), (float4)(o0_x.x, o0_y.x, o0_z.x, o0_w.x));
	write_imagef(dest, (int2)( loc_x.y + dim.z , flipped ? get_image_height(dest) - (loc_y.y + dim.w + 1) : loc_y.y + dim.w ), (float4)(o0_x.y, o0_y.y, o0_z.y, o0_w.y));
	write_imagef(dest, (int2)( loc_x.z + dim.z , flipped ? get_image_height(dest) - (loc_y.z + dim.w + 1) : loc_y.z + dim.w ), (float4)(o0_x.z, o0_y.z, o0_z.z, o0_w.z));
	write_imagef(dest, (int2)( loc_x.w + dim.z , flipped ? get_image_height(dest) - (loc_y.w + dim.w + 1) : loc_y.w + dim.w ), (float4)(o0_x.w, o0_y.w, o0_z.w, o0_w.w));

}

// vectorize program4_1
__kernel void program4_2(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, float4 l2, read_only image2d_t t0, sampler_t t_sampler0)
{

	//    const float4 p0  = (float4)( 0x1.0c6f7ap-20, 0x1p+0, -0x1p+1, 0x1.8p+1 );

	const float4 p0_x = (float4) (0x1.0c6f7ap-20, 0x1.0c6f7ap-20, 0x1.0c6f7ap-20, 0x1.0c6f7ap-20);
	const float4 p0_y = (float4) (0x1p+0, 0x1p+0, 0x1p+0, 0x1p+0);
	const float4 p0_z = (float4) (-0x1p+1, -0x1p+1, -0x1p+1, -0x1p+1);
	const float4 p0_w = (float4) (0x1.8p+1, 0x1.8p+1, 0x1.8p+1, 0x1.8p+1);

	//    float4       o0, r0, r1, r2, r3;
	float4 o0_x, o0_y, o0_z, o0_w;
	float4 r0_x, r0_y, r0_z, r0_w;
	float4 r1_x, r1_y, r1_z, r1_w;
	float4 r2_x, r2_y, r2_z, r2_w;
	float4 r3_x, r3_y, r3_z, r3_w;

	//    int2         loc = (int2)( get_global_id(0), get_global_id(1) );
	int4 loc_x, loc_y;
	int tmp = get_global_id(0)*4;
	loc_x = (int4)(tmp, tmp+1, tmp+2, tmp+3);
	loc_y = get_global_id(1);

	//    float4 f0 = (float4)( st_origin.x + ((float)loc.x + 0.5f) * st_delta.x + ((float)loc.y + 0.5f) * st_delta.z, st_origin.y + ((float)loc.x + 0.5f) * st_delta.y + ((float)loc.y + 0.5f) * st_delta.w, 0.0f, 0.0f );
	// TODO : could this pase be vectorized in this way? maybe use convert_float4 instead?
	float4 f0_x = ((float4)st_origin.x) + (((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w)) + ((float4)0.5f)) * ((float4)st_delta.x) + (((float4)((float)loc_y.x, (float)loc_y.y, (float)loc_y.z, (float)loc_y.w)) + ((float4)0.5f)) * ((float4)st_delta.z);
	float4 f0_y = ((float4)st_origin.y) + (((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w)) + ((float4)0.5f)) * ((float4)st_delta.y) + (((float4)((float)loc_y.x, (float)loc_y.y, (float)loc_y.z, (float)loc_y.w)) + ((float4)0.5f)) * ((float4)st_delta.w);
	float4 f0_z = (float4)0.0f;
	float4 f0_w = (float4)0.0f;

	//    r3 = f0;
	r3_x = f0_x;
	r3_y = f0_y;
	r3_z = f0_z;
	r3_w = f0_w;

	//    r1 = r3-l0;
	r1_x = r3_x - (float4)l0.x;
	r1_y = r3_y - (float4)l0.y;
	r1_z = r3_z - (float4)l0.z;
	r1_w = r3_w - (float4)l0.w;

	//    r2 = dot(r1.xy,r1.xy);
	float4 r2T0 = dot((float2)(r1_x.x, r1_y.x), (float2)(r1_x.x, r1_y.x));
	float4 r2T1 = dot((float2)(r1_x.y, r1_y.y), (float2)(r1_x.y, r1_y.y));
	float4 r2T2 = dot((float2)(r1_x.z, r1_y.z), (float2)(r1_x.z, r1_y.z));
	float4 r2T3 = dot((float2)(r1_x.w, r1_y.w), (float2)(r1_x.w, r1_y.w));
	// transpose back
	r2_x = (float4)(r2T0.x, r2T1.x, r2T2.x, r2T3.x);
	r2_y = (float4)(r2T0.y, r2T1.y, r2T2.y, r2T3.y);
	r2_z = (float4)(r2T0.z, r2T1.z, r2T2.z, r2T3.z);
	r2_w = (float4)(r2T0.w, r2T1.w, r2T2.w, r2T3.w);

	//    r3 = max(r2, p0.xxxx);
	r3_x = max(r2_x, p0_x);
	r3_y = max(r2_y, p0_x);
	r3_z = max(r2_z, p0_x);
	r3_w = max(r2_w, p0_x);

	//    r3 = half_rsqrt(r3.xxxx);
	r3_x = half_rsqrt(r3_x);
	r3_y = half_rsqrt(r3_x);
	r3_z = half_rsqrt(r3_x);
	r3_w = half_rsqrt(r3_x);

	//    r2 = r3*r2;
	r2_x = r3_x * r2_x;
	r2_y = r3_y * r2_y;
	r2_z = r3_z * r2_z;
	r2_w = r3_w * r2_w;

	//    r3 = clamp(r2*-l0.zzzz + p0.yyyy, 0.0f, 1.0f);
	// TODO : can you broadcast lo.z into vector? maybe better l0.zzzz ?
	r3_x = clamp(r2_x*-(float4)l0.z + p0_y, (float4)0.0f, (float4)1.0f);
	r3_y = clamp(r2_y*-(float4)l0.z + p0_y, (float4)0.0f, (float4)1.0f);
	r3_z = clamp(r2_z*-(float4)l0.z + p0_y, (float4)0.0f, (float4)1.0f);
	r3_w = clamp(r2_w*-(float4)l0.z + p0_y, (float4)0.0f, (float4)1.0f);

	//    r2 = r3*p0.zzzz + p0.wwww;
	r2_x = r3_x * p0_z + p0_w;
	r2_y = r3_y * p0_z + p0_w;
	r2_z = r3_z * p0_z + p0_w;
	r2_w = r3_w * p0_z + p0_w;

	//    r2 = r2*r3;
	r2_x = r2_x * r3_x;
	r2_y = r2_y * r3_y;
	r2_z = r2_z * r3_z;
	r2_w = r2_w * r3_w;

	//    r2 = r2*r3;
	r2_x = r2_x * r3_x;
	r2_y = r2_y * r3_y;
	r2_z = r2_z * r3_z;
	r2_w = r2_w * r3_w;

	//    r2 = r2*l0.wwww + p0.yyyy;
	r2_x = r2_x * (float4)l0.w + p0_y;
	r2_y = r2_y * (float4)l0.w + p0_y;
	r2_z = r2_z * (float4)l0.w + p0_y;
	r2_w = r2_w * (float4)l0.w + p0_y;

	//    r1 = r1*r2.xxxx + l0;
	r1_x = r1_x * r2_x + (float4)l0.x;
	r1_y = r1_y * r2_x + (float4)l0.y;
	r1_z = r1_z * r2_x + (float4)l0.z;
	r1_w = r1_w * r2_x + (float4)l0.w;

	//    r0.x = dot(r1.xy,l1.xy) + l1.w;
	r0_x.x = dot((float2)(r1_x.x, r1_y.x), l1.xy) + l1.w;
	r0_x.y = dot((float2)(r1_x.y, r1_y.y), l1.xy) + l1.w;
	r0_x.z = dot((float2)(r1_x.z, r1_y.z), l1.xy) + l1.w;
	r0_x.w = dot((float2)(r1_x.w, r1_y.w), l1.xy) + l1.w;

	//    r0.y = dot(r1.xy,l2.xy) + l2.w;
	r0_y.x = dot((float2)(r1_x.x, r1_y.x), l2.xy) + l2.w;
	r0_y.y = dot((float2)(r1_x.y, r1_y.y), l2.xy) + l2.w;
	r0_y.z = dot((float2)(r1_x.z, r1_y.z), l2.xy) + l2.w;
	r0_y.w = dot((float2)(r1_x.w, r1_y.w), l2.xy) + l2.w;

	//    r0 = read_imagef(t0, t_sampler0, r0.xy);

	float4 r0T0, r0T1, r0T2, r0T3;
	r0T0 = read_imagef(t0, t_sampler0, (float2)(r0_x.x, r0_y.x));
	r0T1 = read_imagef(t0, t_sampler0, (float2)(r0_x.y, r0_y.y));
	r0T2 = read_imagef(t0, t_sampler0, (float2)(r0_x.z, r0_y.z));
	r0T3 = read_imagef(t0, t_sampler0, (float2)(r0_x.w, r0_y.w));
	// transpose back
	r0_x = (float4)(r0T0.x, r0T1.x, r0T2.x, r0T3.x);
	r0_y = (float4)(r0T0.y, r0T1.y, r0T2.y, r0T3.y);
	r0_z = (float4)(r0T0.z, r0T1.z, r0T2.z, r0T3.z);
	r0_w = (float4)(r0T0.w, r0T1.w, r0T2.w, r0T3.w);

	//    r0.xyz = min(r0.xyz, r0.www);
	r0_x = min(r0_x, r0_w);
	r0_y = min(r0_y, r0_w);
	r0_z = min(r0_z, r0_w);

	//    o0 = r0;
	o0_x = r0_x;
	o0_y = r0_y;
	o0_z = r0_z;
	o0_w = r0_w;

	//    write_imagef(dest, (int2)( loc.x + dim.z , flipped ? get_image_height(dest) - (loc.y + dim.w + 1) : loc.y + dim.w ), o0);
	write_imagef(dest, (int2)( loc_x.x + dim.z , flipped ? get_image_height(dest) - (loc_y.x + dim.w + 1) : loc_y.x + dim.w ), (float4)(o0_x.x, o0_y.x, o0_z.x, o0_w.x));
	write_imagef(dest, (int2)( loc_x.y + dim.z , flipped ? get_image_height(dest) - (loc_y.y + dim.w + 1) : loc_y.y + dim.w ), (float4)(o0_x.y, o0_y.y, o0_z.y, o0_w.y));
	write_imagef(dest, (int2)( loc_x.z + dim.z , flipped ? get_image_height(dest) - (loc_y.z + dim.w + 1) : loc_y.z + dim.w ), (float4)(o0_x.z, o0_y.z, o0_z.z, o0_w.z));
	write_imagef(dest, (int2)( loc_x.w + dim.z , flipped ? get_image_height(dest) - (loc_y.w + dim.w + 1) : loc_y.w + dim.w ), (float4)(o0_x.w, o0_y.w, o0_z.w, o0_w.w));

}

// scalarize program
__kernel void program4_1(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, float4 l2, read_only image2d_t t0, sampler_t t_sampler0)
{
	//    const float4 p0  = (float4)( 0x1.0c6f7ap-20, 0x1p+0, -0x1p+1, 0x1.8p+1 );

	const float p0_x = 0x1.0c6f7ap-20;
	const float p0_y = 0x1p+0;
	const float p0_z = -0x1p+1;
	const float p0_w = 0x1.8p+1;

	//    float4       o0, r0, r1, r2, r3;
	float o0_x, o0_y, o0_z, o0_w;
	float r0_x, r0_y, r0_z, r0_w;
	float r1_x, r1_y, r1_z, r1_w;
	float r2_x, r2_y, r2_z, r2_w;
	float r3_x, r3_y, r3_z, r3_w;

	//    int2         loc = (int2)( get_global_id(0), get_global_id(1) );
	int loc_x = get_global_id(0);
	int loc_y = get_global_id(1);

	//    float4 f0 = (float4)( st_origin.x + ((float)loc.x + 0.5f) * st_delta.x + ((float)loc.y + 0.5f) * st_delta.z, st_origin.y + ((float)loc.x + 0.5f) * st_delta.y + ((float)loc.y + 0.5f) * st_delta.w, 0.0f, 0.0f );
	float f0_x = st_origin.x + ((float)loc_x + 0.5f) * st_delta.x + ((float)loc_y + 0.5f) * st_delta.z;
	float f0_y = st_origin.y + ((float)loc_x + 0.5f) * st_delta.y + ((float)loc_y + 0.5f) * st_delta.w;
	float f0_z = 0.0f;
	float f0_w = 0.0f;

	//    r3 = f0;
	r3_x = f0_x;
	r3_y = f0_y;
	r3_z = f0_z;
	r3_w = f0_w;

	//    r1 = r3-l0;
	r1_x = r3_x - l0.x;
	r1_y = r3_y - l0.y;
	r1_z = r3_z - l0.z;
	r1_w = r3_w - l0.w;

	//    r2 = dot(r1.xy,r1.xy);
	float4 r2T = dot((float2)(r1_x, r1_y), (float2)(r1_x, r1_y));
	r2_x = r2T.x;
	r2_y = r2T.y;
	r2_z = r2T.z;
	r2_w = r2T.w;

	//    r3 = max(r2, p0.xxxx);
	r3_x = max(r2_x, p0_x);
	r3_y = max(r2_y, p0_x);
	r3_z = max(r2_z, p0_x);
	r3_w = max(r2_w, p0_x);

	//    r3 = half_rsqrt(r3.xxxx);
	r3_x = half_rsqrt(r3_x);
	r3_y = half_rsqrt(r3_x);
	r3_z = half_rsqrt(r3_x);
	r3_w = half_rsqrt(r3_x);

	//    r2 = r3*r2;
	r2_x = r3_x * r2_x;
	r2_y = r3_y * r2_y;
	r2_z = r3_z * r2_z;
	r2_w = r3_w * r2_w;

	//    r3 = clamp(r2*-l0.zzzz + p0.yyyy, 0.0f, 1.0f);
	r3_x = clamp(r2_x*-l0.z + p0_y, 0.0f, 1.0f);
	r3_y = clamp(r2_y*-l0.z + p0_y, 0.0f, 1.0f);
	r3_z = clamp(r2_z*-l0.z + p0_y, 0.0f, 1.0f);
	r3_w = clamp(r2_w*-l0.z + p0_y, 0.0f, 1.0f);

	//    r2 = r3*p0.zzzz + p0.wwww;
	r2_x = r3_x * p0_z + p0_w;
	r2_y = r3_y * p0_z + p0_w;
	r2_z = r3_z * p0_z + p0_w;
	r2_w = r3_w * p0_z + p0_w;

	//    r2 = r2*r3;
	r2_x = r2_x * r3_x;
	r2_y = r2_y * r3_y;
	r2_z = r2_z * r3_z;
	r2_w = r2_w * r3_w;

	//    r2 = r2*r3;
	r2_x = r2_x * r3_x;
	r2_y = r2_y * r3_y;
	r2_z = r2_z * r3_z;
	r2_w = r2_w * r3_w;

	//    r2 = r2*l0.wwww + p0.yyyy;
	r2_x = r2_x * l0.w + p0_y;
	r2_y = r2_y * l0.w + p0_y;
	r2_z = r2_z * l0.w + p0_y;
	r2_w = r2_w * l0.w + p0_y;

	//    r1 = r1*r2.xxxx + l0;
	r1_x = r1_x * r2_x + l0.x;
	r1_y = r1_y * r2_x + l0.y;
	r1_z = r1_z * r2_x + l0.z;
	r1_w = r1_w * r2_x + l0.w;

	//    r0.x = dot(r1.xy,l1.xy) + l1.w;
	r0_x = dot((float2)(r1_x, r1_y), l1.xy) + l1.w;

	//    r0.y = dot(r1.xy,l2.xy) + l2.w;
	r0_y = dot((float2)(r1_x, r1_y), l2.xy) + l2.w;

	//    r0 = read_imagef(t0, t_sampler0, r0.xy);
	float4 r0T;
	r0T.x = r0_x;
	r0T.y = r0_y;
	r0T.z = r0_z;
	r0T.w = r0_w;
	r0T = read_imagef(t0, t_sampler0, r0T.xy);
	r0_x = r0T.x;
	r0_y = r0T.y;
	r0_z = r0T.z;
	r0_w = r0T.w;

	//    r0.xyz = min(r0.xyz, r0.www);
	r0_x = min(r0_x, r0_w);
	r0_y = min(r0_y, r0_w);
	r0_z = min(r0_z, r0_w);

	//    o0 = r0;
	o0_x = r0_x;
	o0_y = r0_y;
	o0_z = r0_z;
	o0_w = r0_w;

	float4 o0T;
	o0T.x = o0_x;
	o0T.y = o0_y;
	o0T.z = o0_z;
	o0T.w = o0_w;

	//    write_imagef(dest, (int2)( loc.x + dim.z , flipped ? get_image_height(dest) - (loc.y + dim.w + 1) : loc.y + dim.w ), o0);
	write_imagef(dest, (int2)( loc_x + dim.z , flipped ? get_image_height(dest) - (loc_y + dim.w + 1) : loc_y + dim.w ), o0T);
}

// ======================================================
//                        original
// ======================================================

__kernel void program(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, float4 l2, read_only image2d_t t0, sampler_t t_sampler0)
{
	const float4 p0 = (float4)( 0x1.0c6f7ap-20, 0x1p+0, -0x1p+1, 0x1.8p+1 );
	int dest_width = dim.x;
	int dest_height = dim.y;
	float4 o0, r0, r1, r2, r3;
	float4 false_vector = (float4) 0.0f;
	float4 true_vector = (float4) 1.0f;
	float unused_float1;
	float2 unused_float2;
	__float3_SPI unused_float3;
	float4 unused_float4;
	int2 loc = (int2)( get_global_id(0), get_global_id(1) );
	float4 f0 = (float4)( st_origin.x + ((float)loc.x + 0.5f) * st_delta.x + ((float)loc.y + 0.5f) * st_delta.z, st_origin.y + ((float)loc.x + 0.5f) * st_delta.y + ((float)loc.y + 0.5f) * st_delta.w, 0.0f, 0.0f );
	r3 = f0;
	r1 = r3-l0;
	r2 = dot(r1.xy,r1.xy);
	r3 = max(r2, p0.xxxx);
	r3 = half_rsqrt(r3.xxxx);
	r2 = r3*r2;
	r3 = clamp(r2*-l0.zzzz + p0.yyyy, 0.0f, 1.0f);
	r2 = r3*p0.zzzz + p0.wwww;
	r2 = r2*r3;
	r2 = r2*r3;
	r2 = r2*l0.wwww + p0.yyyy;
	r1 = r1*r2.xxxx + l0;
	r0.x = dot(r1.xy,l1.xy) + l1.w;
	r0.y = dot(r1.xy,l2.xy) + l2.w;
	r0 = read_imagef(t0, t_sampler0, r0.xy);
	r0.xyz = min(r0.xyz, r0.www);
	o0 = r0;
	write_imagef(dest, (int2)( loc.x + dim.z , flipped ? get_image_height(dest) - (loc.y + dim.w + 1) : loc.y + dim.w ), o0);
}

__kernel void program_trans(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, float4 l2, read_only image2d_t t0, sampler_t t_sampler0)
{
	float4 f0_start, f0_end, f0_delta, tf0_start[4], tf0_delta[4];
	float4 f1_start, f1_end, f1_delta, tf1_start[4], tf1_delta[4];
	float4 f2_start, f2_end, f2_delta, tf2_start[4], tf2_delta[4];
	const float4 p0 = (float4)( 0x1.0c6f7ap-20, 0x1.0c6f7ap-20, 0x1.0c6f7ap-20, 0x1.0c6f7ap-20 );
	const float4 p1 = (float4)( 0x1p+0, 0x1p+0, 0x1p+0, 0x1p+0 );
	const float4 p2 = (float4)( 0x1p-1, 0x1p-1, 0x1p-1, 0x1p-1 );
	const float4 p3 = (float4)( -0x1p+0, -0x1p+0, -0x1p+0, -0x1p+0 );
	const float4 p4 = (float4)( -0x1p+1, -0x1p+1, -0x1p+1, -0x1p+1 );
	const float4 p5 = (float4)( 0x1.8p+1, 0x1.8p+1, 0x1.8p+1, 0x1.8p+1 );
	int index = 0;
	int total_index = 0;
	int write_amount = 0;
	int read_amount = 256;
	int write_offset = 0;
	float4 o_r[64], o_g[64], o_b[64], o_a[64];
	int dest_width = dim.x;
	int dest_height = dim.y;
	float4 o0, r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, r16, r17, r18, r19, r20, r21, r22, r23, r24, r25, r26, r27, r28, r29, r30, r31, r32, r33, r34, r35, r36, r37, r38, r39, r40, r41, r42, r43, r44, r45, r46, r47, r48, r49, r50, r51, r52, r53, r54, r55, r56, r57, r58, r59, r60, r61, r62, r63, r64, r65, r66, r67, r68, r69, r70, r71, r72, r73, r74, r75, r76, r77, r78, r79, r80, r81, r82, r83, r84, r85, r86, r87, r88, r89, r90, r91, r92, r93, r94, r95, r96, r97, r98, r99, r100, r101, r102, r103, r104, r105, r106, r107, r108, r109, r110, r111, r112, r113, r114, r115, r116, r117, r118, r119, r120, r121, r122, r123, r124, r125, r126, r127, r128, r129, r130, r131, r132, r133, r134, r135, r136, r137, r138, r139, r140, r141, r142, r143, r144, r145;
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

	// vertex start
	f0_start = loc_start;
	r0 = loc_start;
	r0 = r0-l0;
	f2_start = r0;
	f1_start = r0;

	// vertex end
	f0_end = loc_end;
	r0 = loc_end;
	r0 = r0-l0;
	f2_end = r0;
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
	f2_delta = (f2_end - f2_start) / (float)(dest_width);
	tf2_delta[0] = (float4) f2_delta.s0;
	tf2_start[0] = (float4) ( f2_start.s0, f2_start.s0 + f2_delta.s0, f2_start.s0 + 2.0*f2_delta.s0, f2_start.s0 + 3.0*f2_delta.s0 );
	tf2_delta[1] = (float4) f2_delta.s1;
	tf2_start[1] = (float4) ( f2_start.s1, f2_start.s1 + f2_delta.s1, f2_start.s1 + 2.0*f2_delta.s1, f2_start.s1 + 3.0*f2_delta.s1 );
	tf2_delta[2] = (float4) f2_delta.s2;
	tf2_start[2] = (float4) ( f2_start.s2, f2_start.s2 + f2_delta.s2, f2_start.s2 + 2.0*f2_delta.s2, f2_start.s2 + 3.0*f2_delta.s2 );
	tf2_delta[3] = (float4) f2_delta.s3;
	tf2_start[3] = (float4) ( f2_start.s3, f2_start.s3 + f2_delta.s3, f2_start.s3 + 2.0*f2_delta.s3, f2_start.s3 + 3.0*f2_delta.s3 );

	for(; loc.x<dest_width; loc.x+=4)
	{
		r0 = tf2_start[0] + (float)(loc.x) * tf2_delta[0];
		r1 = tf2_start[1] + (float)(loc.x) * tf2_delta[1];
		r2 = tf2_start[2] + (float)(loc.x) * tf2_delta[2];
		r3 = tf2_start[3] + (float)(loc.x) * tf2_delta[3];
		r4 = tf2_start[0] + (float)(loc.x) * tf2_delta[0];
		r5 = tf2_start[1] + (float)(loc.x) * tf2_delta[1];
		r6 = tf2_start[2] + (float)(loc.x) * tf2_delta[2];
		r7 = tf2_start[3] + (float)(loc.x) * tf2_delta[3];
		r8 = r0*r4;
		r9 = r1*r5;
		r10 = r8+r9;
		r11 = r10;
		r12 = r10;
		r13 = r10;
		r14 = r10;
		r15 = max(r11, p0);
		r16 = max(r12, p0);
		r17 = max(r13, p0);
		r18 = max(r14, p0);
		r19 = fabs(r15);
		r20 = half_rsqrt(r19);
		r21 = r20*r20;
		r22 = r21*r19;
		r23 = r22-p1;
		r24 = r23*r20;
		r25 = r24*p2;
		r26 = r20-r25;
		r27 = fabs(r15);
		r28 = half_rsqrt(r27);
		r29 = r28*r28;
		r30 = r29*r27;
		r31 = r30-p1;
		r32 = r31*r28;
		r33 = r32*p2;
		r34 = r28-r33;
		r35 = fabs(r15);
		r36 = half_rsqrt(r35);
		r37 = r36*r36;
		r38 = r37*r35;
		r39 = r38-p1;
		r40 = r39*r36;
		r41 = r40*p2;
		r42 = r36-r41;
		r43 = fabs(r15);
		r44 = half_rsqrt(r43);
		r45 = r44*r44;
		r46 = r45*r43;
		r47 = r46-p1;
		r48 = r47*r44;
		r49 = r48*p2;
		r50 = r44-r49;
		r51 = r26;
		r52 = r51*r11;
		r53 = r34*r12;
		r54 = r42*r13;
		r55 = r50*r14;
		r56 = r52;
		r57 = r53;
		r58 = r54;
		r59 = r55;
		r60 = l0.zzzz*p3;
		r61 = l0.zzzz*p3;
		r62 = l0.zzzz*p3;
		r63 = l0.zzzz*p3;
		r64 = r56*r60 + p1;
		r65 = r57*r61 + p1;
		r66 = r58*r62 + p1;
		r67 = r59*r63 + p1;
		r68 = (float4) 0.0f;
		r69 = min(r64, p1);
		r70 = max(r69, r68);
		r71 = min(r65, p1);
		r72 = max(r71, r68);
		r73 = min(r66, p1);
		r74 = max(r73, r68);
		r75 = min(r67, p1);
		r76 = max(r75, r68);
		r77 = r70;
		r78 = r72;
		r79 = r74;
		r80 = r76;
		r81 = r77*p4 + p5;
		r82 = r78*p4 + p5;
		r83 = r79*p4 + p5;
		r84 = r80*p4 + p5;
		r85 = r81*r77;
		r86 = r82*r78;
		r87 = r83*r79;
		r88 = r84*r80;
		r89 = r85;
		r90 = r86;
		r91 = r87;
		r92 = r88;
		r93 = r89*r77;
		r94 = r90*r78;
		r95 = r91*r79;
		r96 = r92*r80;
		r97 = r93;
		r98 = r94;
		r99 = r95;
		r100 = r96;
		r101 = r97*l0.wwww + p1;
		r102 = r98*l0.wwww + p1;
		r103 = r99*l0.wwww + p1;
		r104 = r100*l0.wwww + p1;
		r105 = r101;
		r106 = r102;
		r107 = r103;
		r108 = r104;
		r109 = tf1_start[0] + (float)(loc.x) * tf1_delta[0];
		r110 = tf1_start[1] + (float)(loc.x) * tf1_delta[1];
		r111 = tf1_start[2] + (float)(loc.x) * tf1_delta[2];
		r112 = tf1_start[3] + (float)(loc.x) * tf1_delta[3];
		r113 = r109*r105 + l0.xxxx;
		r114 = r110*r105 + l0.yyyy;
		r115 = r111*r105 + l0.zzzz;
		r116 = r112*r105 + l0.wwww;
		r117 = r113;
		r118 = r117*l1.xxxx;
		r119 = r118+l1.wwww;
		r120 = r114*l1.yyyy;
		r121 = r119+r120;
		r122 = r121;
		r123 = r117*l2.xxxx;
		r124 = r123+l2.wwww;
		r125 = r114*l2.yyyy;
		r126 = r124+r125;
		r127 = r126;
		read_transposed_imagef(t0, t_sampler0, r122, r127, &r128, &r129, &r130, &r131);
		r132 = r128;
		r133 = r129;
		r134 = r130;
		r135 = r131;
		r136 = min(r132, r135);
		r137 = min(r133, r135);
		r138 = min(r134, r135);
		r139 = r136;
		r140 = r137;
		r141 = r138;
		r142 = r139;
		r143 = r140;
		r144 = r141;
		r145 = r135;
		o_r[index] = r142;
		o_g[index] = r143;
		o_b[index] = r144;
		o_a[index] = r145;
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
		}
		;
	}
	if (index> 0)
	{
		(void)__async_work_group_stream_to_image(dest, (size_t)(dim.z + write_offset), (size_t)(flipped ? get_image_height(dest) - (loc.y+dim.w+1): loc.y+dim.w), (size_t)(dest_width - write_offset), (const float4 *)o_r, (const float4 *)o_g, (const float4 *)o_b, (const float4 *)o_a);
	}
}
