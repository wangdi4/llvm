//improve loop

__kernel void program4_7(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, read_only image2d_t t0, sampler_t t_sampler0)
{

	//	const float4 p0 = (float4)( 0x1p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
	const float4 p0_x = (float4)0x1p+0;
	const float4 p0_y = (float4)0x0p+0;
	const float4 p0_z = (float4)0x0p+0;
	const float4 p0_w = (float4)0x0p+0;

	//	const float4 p1 = (float4)( 0x1p+0, 0x1p+0, 0x0p+0, 0x0p+0 );
	const float4 p1_x = (float4)0x1p+0;
	const float4 p1_y = (float4)0x1p+0;
	const float4 p1_z = (float4)0x0p+0;
	const float4 p1_w = (float4)0x0p+0;

	//	const float4 p2 = (float4)( 0x0p+0, 0x1p+0, 0x0p+0, 0x0p+0 );
	const float4 p2_x = (float4)0x0p+0;
	const float4 p2_y = (float4)0x1p+0;
	const float4 p2_z = (float4)0x0p+0;
	const float4 p2_w = (float4)0x0p+0;

	//	const float4 p3 = (float4)( -0x1p+0, 0x1p+0, 0x0p+0, 0x0p+0 );
	const float4 p3_x = (float4)-0x1p+0;
	const float4 p3_y = (float4)0x1p+0;
	const float4 p3_z = (float4)0x0p+0;
	const float4 p3_w = (float4)0x0p+0;

	//	const float4 p4 = (float4)( -0x1p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
	const float4 p4_x = (float4)-0x1p+0;
	const float4 p4_y = (float4)0x0p+0;
	const float4 p4_z = (float4)0x0p+0;
	const float4 p4_w = (float4)0x0p+0;

	//	const float4 p5 = (float4)( -0x1p+0, -0x1p+0, 0x0p+0, 0x0p+0 );
	const float4 p5_x = (float4)-0x1p+0;
	const float4 p5_y = (float4)-0x1p+0;
	const float4 p5_z = (float4)0x0p+0;
	const float4 p5_w = (float4)0x0p+0;

	//	const float4 p6 = (float4)( 0x0p+0, -0x1p+0, 0x0p+0, 0x0p+0 );
	const float4 p6_x = (float4)0x0p+0;
	const float4 p6_y = (float4)-0x1p+0;
	const float4 p6_z = (float4)0x0p+0;
	const float4 p6_w = (float4)0x0p+0;

	//	const float4 p7 = (float4)( 0x1p+0, -0x1p+0, 0x0p+0, 0x0p+0 );
	const float4 p7_x = (float4)0x1p+0;
	const float4 p7_y = (float4)-0x1p+0;
	const float4 p7_z = (float4)0x0p+0;
	const float4 p7_w = (float4)0x0p+0;

	int4 dest_width = (int4)dim.x;
	int4 dest_height = (int4)dim.y;

	//	float4 o0, r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10;
	float4 o0_x, o0_y, o0_z, o0_w;
	float4 r0_x, r0_y, r0_z, r0_w;
	float4 r1_x, r1_y, r1_z, r1_w;
	float4 r2_x, r2_y, r2_z, r2_w;
	float4 r3_x, r3_y, r3_z, r3_w;
	float4 r4_x, r4_y, r4_z, r4_w;
	float4 r5_x, r5_y, r5_z, r5_w;
	float4 r6_x, r6_y, r6_z, r6_w;
	float4 r7_x, r7_y, r7_z, r7_w;
	float4 r8_x, r8_y, r8_z, r8_w;
	float4 r9_x, r9_y, r9_z, r9_w;
	float4 r10_x, r10_y, r10_z, r10_w;

	float4 f0_x, f0_y, f0_z, f0_w;

	//	int2 loc = (int2)( get_global_id(0), get_global_id(1) );
	int4 loc_x, loc_y;

	loc_y = get_global_id(1);
	int count;
	const int Many = 128;

	int tmp = get_global_id(0) * Many;
	int orig_loc_x = tmp;

	float4     output_x[Many/4], output_y[Many/4], output_z[Many/4], output_w[Many/4];
	float4     input0_x[Many/4], input0_y[Many/4], input0_z[Many/4], input0_w[Many/4];
	float4     input1_x[Many/4], input1_y[Many/4], input1_z[Many/4], input1_w[Many/4];
	float4     input3_x[Many/4], input3_y[Many/4], input3_z[Many/4], input3_w[Many/4];
	float4     input4_x[Many/4], input4_y[Many/4], input4_z[Many/4], input4_w[Many/4];
	float4     input5_x[Many/4], input5_y[Many/4], input5_z[Many/4], input5_w[Many/4];
	float4     input6_x[Many/4], input6_y[Many/4], input6_z[Many/4], input6_w[Many/4];
	float4     input7_x[Many/4], input7_y[Many/4], input7_z[Many/4], input7_w[Many/4];
	float4     input8_x[Many/4], input8_y[Many/4], input8_z[Many/4], input8_w[Many/4];
	float4     input9_x[Many/4], input9_y[Many/4], input9_z[Many/4], input9_w[Many/4];


	loc_x = (int4)(tmp, tmp+1, tmp+2, tmp+3);

	f0_x = (float4)(st_origin.x + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.x + ((float4)((float)loc_y.x, (float)loc_y.y, (float)loc_y.z, (float)loc_y.w) + 0.5f) * st_delta.z);
	f0_y = (float4)(st_origin.y + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.y + ((float4)((float)loc_y.x, (float)loc_y.y, (float)loc_y.z, (float)loc_y.w) + 0.5f) * st_delta.w);
	f0_z = (float4)0.0f;
	f0_w = (float4)0.0f;

	r2_x = f0_x;
	r2_y = f0_y;
	r2_z = f0_z;
	r2_w = f0_w;

	r10_x = r2_x + p0_x;
	r10_y = r2_y + p0_y;
	r10_z = r2_z + p0_z;
	r10_w = r2_w + p0_w;

	r8_x = (r10_x * (float4)l0.x + r10_y * (float4)l0.y) + l0.w;
	r8_y = (r10_x * (float4)l1.x + r10_y * (float4)l1.y) + l1.w;

	r10_x = r8_x;
	r10_y = r8_y;
	r10_z = r8_z;
	r10_w = r8_w;

	{
		float2 stride;
		stride.x = r10_x.y - r10_x.x;
		stride.y = r10_y.y - r10_y.x;
		__async_work_group_stream_from_image(t0, t_sampler0, (float2)(r10_x.x, r10_y.x), stride, Many, input8_x, input8_y, input8_z, input8_w);
	}

	r10_x = r2_x + p1_x;
	r10_y = r2_y + p1_y;
	r10_z = r2_z + p1_z;
	r10_w = r2_w + p1_w;

	r5_x = (r10_x * (float4)l0.x + r10_y * (float4)l0.y) + l0.w;
	r5_y = (r10_x * (float4)l1.x + r10_y * (float4)l1.y) + l1.w;

	r10_x = r5_x;
	r10_y = r5_y;
	r10_z = r5_z;
	r10_w = r5_w;

	{
		float2 stride;
		stride.x = r10_x.y - r10_x.x;
		stride.y = r10_y.y - r10_y.x;
		__async_work_group_stream_from_image(t0, t_sampler0, (float2)(r10_x.x, r10_y.x), stride, Many, input5_x, input5_y, input5_z, input5_w);
	}

	r10_x = r2_x + p2_x;
	r10_y = r2_y + p2_y;
	r10_z = r2_z + p2_z;
	r10_w = r2_w + p2_w;

	r4_x = (r10_x * (float4)l0.x + r10_y * (float4)l0.y) + l0.w;
	r4_y = (r10_x * (float4)l1.x + r10_y * (float4)l1.y) + l1.w;

	r10_x = r4_x;
	r10_y = r4_y;
	r10_z = r4_z;
	r10_w = r4_w;

	{
		float2 stride;
		stride.x = r10_x.y - r10_x.x;
		stride.y = r10_y.y - r10_y.x;
		__async_work_group_stream_from_image(t0, t_sampler0, (float2)(r10_x.x, r10_y.x), stride, Many, input4_x, input4_y, input4_z, input4_w);
	}

	r10_x = r2_x + p3_x;
	r10_y = r2_y + p3_y;
	r10_z = r2_z + p3_z;
	r10_w = r2_w + p3_w;

	r6_x = (r10_x * (float4)l0.x + r10_y * (float4)l0.y) + l0.w;
	r6_y = (r10_x * (float4)l1.x + r10_y * (float4)l1.y) + l1.w;

	r10_x = r6_x;
	r10_y = r6_y;
	r10_z = r6_z;
	r10_w = r6_w;

	{
		float2 stride;
		stride.x = r10_x.y - r10_x.x;
		stride.y = r10_y.y - r10_y.x;
		__async_work_group_stream_from_image(t0, t_sampler0, (float2)(r10_x.x, r10_y.x), stride, Many, input6_x, input6_y, input6_z, input6_w);
	}

	r10_x = r2_x + p4_x;
	r10_y = r2_y + p4_y;
	r10_z = r2_z + p4_z;
	r10_w = r2_w + p4_w;

	r7_x = (r10_x * (float4)l0.x + r10_y * (float4)l0.y) + l0.w;
	r7_y = (r10_x * (float4)l1.x + r10_y * (float4)l1.y) + l1.w;

	r10_x = r7_x;
	r10_y = r7_y;
	r10_z = r7_z;
	r10_w = r7_w;

	if (count == 0)
	{
		float2 stride;
		stride.x = r10_x.y - r10_x.x;
		stride.y = r10_y.y - r10_y.x;
		__async_work_group_stream_from_image(t0, t_sampler0, (float2)(r10_x.x, r10_y.x), stride, Many, input7_x, input7_y, input7_z, input7_w);
	}

	r10_x = r2_x + p5_x;
	r10_y = r2_y + p5_y;
	r10_z = r2_z + p5_z;
	r10_w = r2_w + p5_w;

	r3_x = (r10_x * (float4)l0.x + r10_y * (float4)l0.y) + l0.w;
	r3_y = (r10_x * (float4)l1.x + r10_y * (float4)l1.y) + l1.w;

	r10_x = r3_x;
	r10_y = r3_y;
	r10_z = r3_z;
	r10_w = r3_w;

	{
		float2 stride;
		stride.x = r10_x.y - r10_x.x;
		stride.y = r10_y.y - r10_y.x;
		__async_work_group_stream_from_image(t0, t_sampler0, (float2)(r10_x.x, r10_y.x), stride, Many, input3_x, input3_y, input3_z, input3_w);
	}

	r10_x = r2_x + p6_x;
	r10_y = r2_y + p6_y;
	r10_z = r2_z + p6_z;
	r10_w = r2_w + p6_w;

	r1_x = (r10_x * (float4)l0.x + r10_y * (float4)l0.y) + l0.w;
	r1_y = (r10_x * (float4)l1.x + r10_y * (float4)l1.y) + l1.w;

	r10_x = r1_x;
	r10_y = r1_y;
	r10_z = r1_z;
	r10_w = r1_w;

	{
		float2 stride;
		stride.x = r10_x.y - r10_x.x;
		stride.y = r10_y.y - r10_y.x;
		__async_work_group_stream_from_image(t0, t_sampler0, (float2)(r10_x.x, r10_y.x), stride, Many, input1_x, input1_y, input1_z, input1_w);
	}

	r10_x = r2_x + p7_x;
	r10_y = r2_y + p7_y;
	r10_z = r2_z + p7_z;
	r10_w = r2_w + p7_w;

	r0_x = (r10_x * (float4)l0.x + r10_y * (float4)l0.y) + l0.w;
	r0_y = (r10_x * (float4)l1.x + r10_y * (float4)l1.y) + l1.w;

	{
		float2 stride;
		stride.x = r0_x.y - r0_x.x;
		stride.y = r0_y.y - r0_y.x;
		__async_work_group_stream_from_image(t0, t_sampler0, (float2)(r0_x.x, r0_y.x), stride, Many, input0_x, input0_y, input0_z, input0_w);
	}

	r9_x = (r2_x * (float4)l0.x + r2_y * (float4)l0.y) + l0.w;
	r9_y = (r2_x * (float4)l1.x + r2_y * (float4)l1.y) + l1.w;

	r2_x = r9_x;
	r2_y = r9_y;
	r2_z = r9_z;
	r2_w = r9_w;

	{
		float2 stride;
		stride.x = r2_x.y - r2_x.x;
		stride.y = r2_y.y - r2_y.x;
		__async_work_group_stream_from_image(t0, t_sampler0, (float2)(r2_x.x, r2_y.x), stride, Many, input9_x, input9_y, input9_z, input9_w);
	}

	//for (count = 0; count < Many/4; ++count) 
	do
	{


		r8_x = input8_x[count];
		r8_y = input8_y[count];
		r8_z = input8_z[count];
		r8_w = input8_w[count];

		r5_x = input5_x[count];
		r5_y = input5_y[count];
		r5_z = input5_z[count];
		r5_w = input5_w[count];

		r4_x = input4_x[count];
		r4_y = input4_y[count];
		r4_z = input4_z[count];
		r4_w = input4_w[count];

		r6_x = input6_x[count];
		r6_y = input6_y[count];
		r6_z = input6_z[count];
		r6_w = input6_w[count];

		r7_x = input7_x[count];
		r7_y = input7_y[count];
		r7_z = input7_z[count];
		r7_w = input7_w[count];

		r3_x = input3_x[count];
		r3_y = input3_y[count];
		r3_z = input3_z[count];
		r3_w = input3_w[count];

		r1_x = input1_x[count];
		r1_y = input1_y[count];
		r1_z = input1_z[count];
		r1_w = input1_w[count];

		r0_x = input0_x[count];
		r0_y = input0_y[count];
		r0_z = input0_z[count];
		r0_w = input0_w[count];

		r9_x = input9_x[count];
		r9_y = input9_y[count];
		r9_z = input9_z[count];
		r9_w = input9_w[count];

		//	r2 = min(r5, r4);
		r2_x = min(r5_x, r4_x);
		r2_y = min(r5_y, r4_y);
		r2_z = min(r5_z, r4_z);
		r2_w = min(r5_w, r4_w);

		//	r4 = max(r5, r4);
		r4_x = max(r5_x, r4_x);
		r4_y = max(r5_y, r4_y);
		r4_z = max(r5_z, r4_z);
		r4_w = max(r5_w, r4_w);

		//	r5 = min(r7, r3);
		r5_x = min(r7_x, r3_x);
		r5_y = min(r7_y, r3_y);
		r5_z = min(r7_z, r3_z);
		r5_w = min(r7_w, r3_w);

		//	r3 = max(r7, r3);
		r3_x = max(r7_x, r3_x);
		r3_y = max(r7_y, r3_y);
		r3_z = max(r7_z, r3_z);
		r3_w = max(r7_w, r3_w);

		//	r7 = min(r0, r9);
		r7_x = min(r0_x, r9_x);
		r7_y = min(r0_y, r9_y);
		r7_z = min(r0_z, r9_z);
		r7_w = min(r0_w, r9_w);

		//	r9 = max(r0, r9);
		r9_x = max(r0_x, r9_x);
		r9_y = max(r0_y, r9_y);
		r9_z = max(r0_z, r9_z);
		r9_w = max(r0_w, r9_w);

		//	r0 = min(r8, r2);
		r0_x = min(r8_x, r2_x);
		r0_y = min(r8_y, r2_y);
		r0_z = min(r8_z, r2_z);
		r0_w = min(r8_w, r2_w);

		//	r2 = max(r8, r2);
		r2_x = max(r8_x, r2_x);
		r2_y = max(r8_y, r2_y);
		r2_z = max(r8_z, r2_z);
		r2_w = max(r8_w, r2_w);

		//	r8 = min(r6, r5);
		r8_x = min(r6_x, r5_x);
		r8_y = min(r6_y, r5_y);
		r8_z = min(r6_z, r5_z);
		r8_w = min(r6_w, r5_w);

		//	r5 = max(r6, r5);
		r5_x = max(r6_x, r5_x);
		r5_y = max(r6_y, r5_y);
		r5_z = max(r6_z, r5_z);
		r5_w = max(r6_w, r5_w);

		//	r6 = min(r1, r7);
		r6_x = min(r1_x, r7_x);
		r6_y = min(r1_y, r7_y);
		r6_z = min(r1_z, r7_z);
		r6_w = min(r1_w, r7_w);

		//	r7 = max(r1, r7);
		r7_x = max(r1_x, r7_x);
		r7_y = max(r1_y, r7_y);
		r7_z = max(r1_z, r7_z);
		r7_w = max(r1_w, r7_w);

		//	r1 = min(r2, r4);
		r1_x = min(r2_x, r4_x);
		r1_y = min(r2_y, r4_y);
		r1_z = min(r2_z, r4_z);
		r1_w = min(r2_w, r4_w);

		//	r4 = max(r2, r4);
		r4_x = max(r2_x, r4_x);
		r4_y = max(r2_y, r4_y);
		r4_z = max(r2_z, r4_z);
		r4_w = max(r2_w, r4_w);

		//	r2 = min(r5, r3);
		r2_x = min(r5_x, r3_x);
		r2_y = min(r5_y, r3_y);
		r2_z = min(r5_z, r3_z);
		r2_w = min(r5_w, r3_w);

		//	r3 = max(r5, r3);
		r3_x = max(r5_x, r3_x);
		r3_y = max(r5_y, r3_y);
		r3_z = max(r5_z, r3_z);
		r3_w = max(r5_w, r3_w);

		//	r5 = min(r7, r9);
		r5_x = min(r7_x, r9_x);
		r5_y = min(r7_y, r9_y);
		r5_z = min(r7_z, r9_z);
		r5_w = min(r7_w, r9_w);

		//	r7 = max(r7, r9);
		r7_x = max(r7_x, r9_x);
		r7_y = max(r7_y, r9_y);
		r7_z = max(r7_z, r9_z);
		r7_w = max(r7_w, r9_w);

		//	r0 = max(r0, r8);
		r0_x = max(r0_x, r8_x);
		r0_y = max(r0_y, r8_y);
		r0_z = max(r0_z, r8_z);
		r0_w = max(r0_w, r8_w);

		//	r3 = min(r3, r7);
		r3_x = min(r3_x, r7_x);
		r3_y = min(r3_y, r7_y);
		r3_z = min(r3_z, r7_z);
		r3_w = min(r3_w, r7_w);

		//	r0 = max(r0, r6);
		r0_x = max(r0_x, r6_x);
		r0_y = max(r0_y, r6_y);
		r0_z = max(r0_z, r6_z);
		r0_w = max(r0_w, r6_w);

		//	r3 = min(r4, r3);
		r3_x = min(r4_x, r3_x);
		r3_y = min(r4_y, r3_y);
		r3_z = min(r4_z, r3_z);
		r3_w = min(r4_w, r3_w);

		//	r4 = min(r2, r5);
		r4_x = min(r2_x, r5_x);
		r4_y = min(r2_y, r5_y);
		r4_z = min(r2_z, r5_z);
		r4_w = min(r2_w, r5_w);

		//	r2 = max(r2, r5);
		r2_x = max(r2_x, r5_x);
		r2_y = max(r2_y, r5_y);
		r2_z = max(r2_z, r5_z);
		r2_w = max(r2_w, r5_w);

		//	r1 = max(r1, r4);
		r1_x = max(r1_x, r4_x);
		r1_y = max(r1_y, r4_y);
		r1_z = max(r1_z, r4_z);
		r1_w = max(r1_w, r4_w);

		//	r1 = min(r2, r1);
		r1_x = min(r2_x, r1_x);
		r1_y = min(r2_y, r1_y);
		r1_z = min(r2_z, r1_z);
		r1_w = min(r2_w, r1_w);

		//	r2 = min(r1, r3);
		r2_x = min(r1_x, r3_x);
		r2_y = min(r1_y, r3_y);
		r2_z = min(r1_z, r3_z);
		r2_w = min(r1_w, r3_w);

		//	r1 = max(r1, r3);
		r1_x = max(r1_x, r3_x);
		r1_y = max(r1_y, r3_y);
		r1_z = max(r1_z, r3_z);
		r1_w = max(r1_w, r3_w);

		//	r0 = max(r0, r2);
		r0_x = max(r0_x, r2_x);
		r0_y = max(r0_y, r2_y);
		r0_z = max(r0_z, r2_z);
		r0_w = max(r0_w, r2_w);

		//	r0 = min(r0, r1);
		r0_x = min(r0_x, r1_x);
		r0_y = min(r0_y, r1_y);
		r0_z = min(r0_z, r1_z);
		r0_w = min(r0_w, r1_w);

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
__kernel void program4_6(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, read_only image2d_t t0, sampler_t t_sampler0)
{

	//	const float4 p0 = (float4)( 0x1p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
	const float4 p0_x = (float4)0x1p+0;
	const float4 p0_y = (float4)0x0p+0;
	const float4 p0_z = (float4)0x0p+0;
	const float4 p0_w = (float4)0x0p+0;

	//	const float4 p1 = (float4)( 0x1p+0, 0x1p+0, 0x0p+0, 0x0p+0 );
	const float4 p1_x = (float4)0x1p+0;
	const float4 p1_y = (float4)0x1p+0;
	const float4 p1_z = (float4)0x0p+0;
	const float4 p1_w = (float4)0x0p+0;

	//	const float4 p2 = (float4)( 0x0p+0, 0x1p+0, 0x0p+0, 0x0p+0 );
	const float4 p2_x = (float4)0x0p+0;
	const float4 p2_y = (float4)0x1p+0;
	const float4 p2_z = (float4)0x0p+0;
	const float4 p2_w = (float4)0x0p+0;

	//	const float4 p3 = (float4)( -0x1p+0, 0x1p+0, 0x0p+0, 0x0p+0 );
	const float4 p3_x = (float4)-0x1p+0;
	const float4 p3_y = (float4)0x1p+0;
	const float4 p3_z = (float4)0x0p+0;
	const float4 p3_w = (float4)0x0p+0;

	//	const float4 p4 = (float4)( -0x1p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
	const float4 p4_x = (float4)-0x1p+0;
	const float4 p4_y = (float4)0x0p+0;
	const float4 p4_z = (float4)0x0p+0;
	const float4 p4_w = (float4)0x0p+0;

	//	const float4 p5 = (float4)( -0x1p+0, -0x1p+0, 0x0p+0, 0x0p+0 );
	const float4 p5_x = (float4)-0x1p+0;
	const float4 p5_y = (float4)-0x1p+0;
	const float4 p5_z = (float4)0x0p+0;
	const float4 p5_w = (float4)0x0p+0;

	//	const float4 p6 = (float4)( 0x0p+0, -0x1p+0, 0x0p+0, 0x0p+0 );
	const float4 p6_x = (float4)0x0p+0;
	const float4 p6_y = (float4)-0x1p+0;
	const float4 p6_z = (float4)0x0p+0;
	const float4 p6_w = (float4)0x0p+0;

	//	const float4 p7 = (float4)( 0x1p+0, -0x1p+0, 0x0p+0, 0x0p+0 );
	const float4 p7_x = (float4)0x1p+0;
	const float4 p7_y = (float4)-0x1p+0;
	const float4 p7_z = (float4)0x0p+0;
	const float4 p7_w = (float4)0x0p+0;

	int4 dest_width = (int4)dim.x;
	int4 dest_height = (int4)dim.y;

	//	float4 o0, r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10;
	float4 o0_x, o0_y, o0_z, o0_w;
	float4 r0_x, r0_y, r0_z, r0_w;
	float4 r1_x, r1_y, r1_z, r1_w;
	float4 r2_x, r2_y, r2_z, r2_w;
	float4 r3_x, r3_y, r3_z, r3_w;
	float4 r4_x, r4_y, r4_z, r4_w;
	float4 r5_x, r5_y, r5_z, r5_w;
	float4 r6_x, r6_y, r6_z, r6_w;
	float4 r7_x, r7_y, r7_z, r7_w;
	float4 r8_x, r8_y, r8_z, r8_w;
	float4 r9_x, r9_y, r9_z, r9_w;
	float4 r10_x, r10_y, r10_z, r10_w;

	float4 f0_x, f0_y, f0_z, f0_w;

	//	int2 loc = (int2)( get_global_id(0), get_global_id(1) );
	int4 loc_x, loc_y;

	loc_y = get_global_id(1);
	int count;
	const int Many = 128;

	int tmp = get_global_id(0) * Many;
	int orig_loc_x = tmp;

	float4     output_x[Many/4], output_y[Many/4], output_z[Many/4], output_w[Many/4];
	float4     input0_x[Many/4], input0_y[Many/4], input0_z[Many/4], input0_w[Many/4];
	float4     input1_x[Many/4], input1_y[Many/4], input1_z[Many/4], input1_w[Many/4];
	float4     input3_x[Many/4], input3_y[Many/4], input3_z[Many/4], input3_w[Many/4];
	float4     input4_x[Many/4], input4_y[Many/4], input4_z[Many/4], input4_w[Many/4];
	float4     input5_x[Many/4], input5_y[Many/4], input5_z[Many/4], input5_w[Many/4];
	float4     input6_x[Many/4], input6_y[Many/4], input6_z[Many/4], input6_w[Many/4];
	float4     input7_x[Many/4], input7_y[Many/4], input7_z[Many/4], input7_w[Many/4];
	float4     input8_x[Many/4], input8_y[Many/4], input8_z[Many/4], input8_w[Many/4];
	float4     input9_x[Many/4], input9_y[Many/4], input9_z[Many/4], input9_w[Many/4];

	for (count = 0; count < Many/4; ++count) {

		loc_x = (int4)(tmp, tmp+1, tmp+2, tmp+3);

		// float4 f0 = (float4)( st_origin.x + ((float)loc.x + 0.5f) * st_delta.x + ((float)loc.y + 0.5f) * st_delta.z, st_origin.y + ((float)loc.x + 0.5f) * st_delta.y + ((float)loc.y + 0.5f) * st_delta.w, 0.0f, 0.0f );
		f0_x = (float4)(st_origin.x + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.x + ((float4)((float)loc_y.x, (float)loc_y.y, (float)loc_y.z, (float)loc_y.w) + 0.5f) * st_delta.z);
		f0_y = (float4)(st_origin.y + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.y + ((float4)((float)loc_y.x, (float)loc_y.y, (float)loc_y.z, (float)loc_y.w) + 0.5f) * st_delta.w);
		f0_z = (float4)0.0f;
		f0_w = (float4)0.0f;

		//	r2 = f0;
		r2_x = f0_x;
		r2_y = f0_y;
		r2_z = f0_z;
		r2_w = f0_w;

		//	r10 = r2+p0;
		r10_x = r2_x + p0_x;
		r10_y = r2_y + p0_y;
		r10_z = r2_z + p0_z;
		r10_w = r2_w + p0_w;

		//	r8.x = dot(r10.xy,l0.xy) + l0.w;
		r8_x = (r10_x * (float4)l0.x + r10_y * (float4)l0.y) + l0.w;

		//	r8.y = dot(r10.xy,l1.xy) + l1.w;
		r8_y = (r10_x * (float4)l1.x + r10_y * (float4)l1.y) + l1.w;

		//	r10 = r8;
		r10_x = r8_x;
		r10_y = r8_y;
		r10_z = r8_z;
		r10_w = r8_w;

		if (count == 0)
		{
			float2 stride;
			stride.x = r10_x.y - r10_x.x;
			stride.y = r10_y.y - r10_y.x;
			__async_work_group_stream_from_image(t0, t_sampler0, (float2)(r10_x.x, r10_y.x), stride, Many, input8_x, input8_y, input8_z, input8_w);
		}
		r8_x = input8_x[count];
		r8_y = input8_y[count];
		r8_z = input8_z[count];
		r8_w = input8_w[count];
		//	r8 = read_imagef(t0, t_sampler0, r10.xy);
		//	read_transposed_imagef(t0, t_sampler0, r10_x, r10_y, &r8_x, &r8_y, &r8_z, &r8_w);

		//	r10 = r2+p1;
		r10_x = r2_x + p1_x;
		r10_y = r2_y + p1_y;
		r10_z = r2_z + p1_z;
		r10_w = r2_w + p1_w;

		//	r5.x = dot(r10.xy,l0.xy) + l0.w;
		r5_x = (r10_x * (float4)l0.x + r10_y * (float4)l0.y) + l0.w;

		//	r5.y = dot(r10.xy,l1.xy) + l1.w;
		r5_y = (r10_x * (float4)l1.x + r10_y * (float4)l1.y) + l1.w;

		//	r10 = r5;
		r10_x = r5_x;
		r10_y = r5_y;
		r10_z = r5_z;
		r10_w = r5_w;

		if (count == 0)
		{
			float2 stride;
			stride.x = r10_x.y - r10_x.x;
			stride.y = r10_y.y - r10_y.x;
			__async_work_group_stream_from_image(t0, t_sampler0, (float2)(r10_x.x, r10_y.x), stride, Many, input5_x, input5_y, input5_z, input5_w);
		}
		r5_x = input5_x[count];
		r5_y = input5_y[count];
		r5_z = input5_z[count];
		r5_w = input5_w[count];
		//	r5 = read_imagef(t0, t_sampler0, r10.xy);
		//	read_transposed_imagef(t0, t_sampler0, r10_x, r10_y, &r5_x, &r5_y, &r5_z, &r5_w);

		//	r10 = r2+p2;
		r10_x = r2_x + p2_x;
		r10_y = r2_y + p2_y;
		r10_z = r2_z + p2_z;
		r10_w = r2_w + p2_w;

		//	r4.x = dot(r10.xy,l0.xy) + l0.w;
		r4_x = (r10_x * (float4)l0.x + r10_y * (float4)l0.y) + l0.w;

		//	r4.y = dot(r10.xy,l1.xy) + l1.w;
		r4_y = (r10_x * (float4)l1.x + r10_y * (float4)l1.y) + l1.w;

		//	r10 = r4;
		r10_x = r4_x;
		r10_y = r4_y;
		r10_z = r4_z;
		r10_w = r4_w;

		if (count == 0)
		{
			float2 stride;
			stride.x = r10_x.y - r10_x.x;
			stride.y = r10_y.y - r10_y.x;
			__async_work_group_stream_from_image(t0, t_sampler0, (float2)(r10_x.x, r10_y.x), stride, Many, input4_x, input4_y, input4_z, input4_w);
		}
		r4_x = input4_x[count];
		r4_y = input4_y[count];
		r4_z = input4_z[count];
		r4_w = input4_w[count];
		// 	r4 = read_imagef(t0, t_sampler0, r10.xy);
		//	read_transposed_imagef(t0, t_sampler0, r10_x, r10_y, &r4_x, &r4_y, &r4_z, &r4_w);

		//	r10 = r2+p3;
		r10_x = r2_x + p3_x;
		r10_y = r2_y + p3_y;
		r10_z = r2_z + p3_z;
		r10_w = r2_w + p3_w;

		//	r6.x = dot(r10.xy,l0.xy) + l0.w;
		r6_x = (r10_x * (float4)l0.x + r10_y * (float4)l0.y) + l0.w;

		//	r6.y = dot(r10.xy,l1.xy) + l1.w;
		r6_y = (r10_x * (float4)l1.x + r10_y * (float4)l1.y) + l1.w;

		//	r10 = r6;
		r10_x = r6_x;
		r10_y = r6_y;
		r10_z = r6_z;
		r10_w = r6_w;

		if (count == 0)
		{
			float2 stride;
			stride.x = r10_x.y - r10_x.x;
			stride.y = r10_y.y - r10_y.x;
			__async_work_group_stream_from_image(t0, t_sampler0, (float2)(r10_x.x, r10_y.x), stride, Many, input6_x, input6_y, input6_z, input6_w);
		}
		r6_x = input6_x[count];
		r6_y = input6_y[count];
		r6_z = input6_z[count];
		r6_w = input6_w[count];
		//	r6 = read_imagef(t0, t_sampler0, r10.xy);
		//	read_transposed_imagef(t0, t_sampler0, r10_x, r10_y, &r6_x, &r6_y, &r6_z, &r6_w);

		//	r10 = r2+p4;
		r10_x = r2_x + p4_x;
		r10_y = r2_y + p4_y;
		r10_z = r2_z + p4_z;
		r10_w = r2_w + p4_w;

		//	r7.x = dot(r10.xy,l0.xy) + l0.w;
		r7_x = (r10_x * (float4)l0.x + r10_y * (float4)l0.y) + l0.w;

		//	r7.y = dot(r10.xy,l1.xy) + l1.w;
		r7_y = (r10_x * (float4)l1.x + r10_y * (float4)l1.y) + l1.w;

		//	r10 = r7;
		r10_x = r7_x;
		r10_y = r7_y;
		r10_z = r7_z;
		r10_w = r7_w;

		if (count == 0)
		{
			float2 stride;
			stride.x = r10_x.y - r10_x.x;
			stride.y = r10_y.y - r10_y.x;
			__async_work_group_stream_from_image(t0, t_sampler0, (float2)(r10_x.x, r10_y.x), stride, Many, input7_x, input7_y, input7_z, input7_w);
		}
		r7_x = input7_x[count];
		r7_y = input7_y[count];
		r7_z = input7_z[count];
		r7_w = input7_w[count];
		//	r7 = read_imagef(t0, t_sampler0, r10.xy);
		//	read_transposed_imagef(t0, t_sampler0, r10_x, r10_y, &r7_x, &r7_y, &r7_z, &r7_w);

		//	r10 = r2+p5;
		r10_x = r2_x + p5_x;
		r10_y = r2_y + p5_y;
		r10_z = r2_z + p5_z;
		r10_w = r2_w + p5_w;

		//	r3.x = dot(r10.xy,l0.xy) + l0.w;
		r3_x = (r10_x * (float4)l0.x + r10_y * (float4)l0.y) + l0.w;

		//	r3.y = dot(r10.xy,l1.xy) + l1.w;
		r3_y = (r10_x * (float4)l1.x + r10_y * (float4)l1.y) + l1.w;

		//	r10 = r3;
		r10_x = r3_x;
		r10_y = r3_y;
		r10_z = r3_z;
		r10_w = r3_w;

		if (count == 0)
		{
			float2 stride;
			stride.x = r10_x.y - r10_x.x;
			stride.y = r10_y.y - r10_y.x;
			__async_work_group_stream_from_image(t0, t_sampler0, (float2)(r10_x.x, r10_y.x), stride, Many, input3_x, input3_y, input3_z, input3_w);
		}
		r3_x = input3_x[count];
		r3_y = input3_y[count];
		r3_z = input3_z[count];
		r3_w = input3_w[count];
		//	r3 = read_imagef(t0, t_sampler0, r10.xy);
		//	read_transposed_imagef(t0, t_sampler0, r10_x, r10_y, &r3_x, &r3_y, &r3_z, &r3_w);

		//	r10 = r2+p6;
		r10_x = r2_x + p6_x;
		r10_y = r2_y + p6_y;
		r10_z = r2_z + p6_z;
		r10_w = r2_w + p6_w;

		//	r1.x = dot(r10.xy,l0.xy) + l0.w;
		r1_x = (r10_x * (float4)l0.x + r10_y * (float4)l0.y) + l0.w;

		//	r1.y = dot(r10.xy,l1.xy) + l1.w;
		r1_y = (r10_x * (float4)l1.x + r10_y * (float4)l1.y) + l1.w;

		//	r10 = r1;
		r10_x = r1_x;
		r10_y = r1_y;
		r10_z = r1_z;
		r10_w = r1_w;

		if (count == 0)
		{
			float2 stride;
			stride.x = r10_x.y - r10_x.x;
			stride.y = r10_y.y - r10_y.x;
			__async_work_group_stream_from_image(t0, t_sampler0, (float2)(r10_x.x, r10_y.x), stride, Many, input1_x, input1_y, input1_z, input1_w);
		}
		r1_x = input1_x[count];
		r1_y = input1_y[count];
		r1_z = input1_z[count];
		r1_w = input1_w[count];
		//	r1 = read_imagef(t0, t_sampler0, r10.xy);
		//	read_transposed_imagef(t0, t_sampler0, r10_x, r10_y, &r1_x, &r1_y, &r1_z, &r1_w);

		//	r10 = r2+p7;
		r10_x = r2_x + p7_x;
		r10_y = r2_y + p7_y;
		r10_z = r2_z + p7_z;
		r10_w = r2_w + p7_w;

		//	r0.x = dot(r10.xy,l0.xy) + l0.w;
		r0_x = (r10_x * (float4)l0.x + r10_y * (float4)l0.y) + l0.w;

		//	r0.y = dot(r10.xy,l1.xy) + l1.w;
		r0_y = (r10_x * (float4)l1.x + r10_y * (float4)l1.y) + l1.w;

		if (count == 0)
		{
			float2 stride;
			stride.x = r0_x.y - r0_x.x;
			stride.y = r0_y.y - r0_y.x;
			__async_work_group_stream_from_image(t0, t_sampler0, (float2)(r0_x.x, r0_y.x), stride, Many, input0_x, input0_y, input0_z, input0_w);
		}
		r0_x = input0_x[count];
		r0_y = input0_y[count];
		r0_z = input0_z[count];
		r0_w = input0_w[count];
		//	r0 = read_imagef(t0, t_sampler0, r0.xy);
		//	read_transposed_imagef(t0, t_sampler0, r0_x, r0_y, &r0_x, &r0_y, &r0_z, &r0_w);

		//	r9.x = dot(r2.xy,l0.xy) + l0.w;
		r9_x = (r2_x * (float4)l0.x + r2_y * (float4)l0.y) + l0.w;

		//	r9.y = dot(r2.xy,l1.xy) + l1.w;
		r9_y = (r2_x * (float4)l1.x + r2_y * (float4)l1.y) + l1.w;

		//	r2 = r9;
		r2_x = r9_x;
		r2_y = r9_y;
		r2_z = r9_z;
		r2_w = r9_w;

		if (count == 0)
		{
			float2 stride;
			stride.x = r2_x.y - r2_x.x;
			stride.y = r2_y.y - r2_y.x;
			__async_work_group_stream_from_image(t0, t_sampler0, (float2)(r2_x.x, r2_y.x), stride, Many, input9_x, input9_y, input9_z, input9_w);
		}
		r9_x = input9_x[count];
		r9_y = input9_y[count];
		r9_z = input9_z[count];
		r9_w = input9_w[count];
		//	r9 = read_imagef(t0, t_sampler0, r2.xy);
		//	read_transposed_imagef(t0, t_sampler0, r2_x, r2_y, &r9_x, &r9_y, &r9_z, &r9_w);

		//	r2 = min(r5, r4);
		r2_x = min(r5_x, r4_x);
		r2_y = min(r5_y, r4_y);
		r2_z = min(r5_z, r4_z);
		r2_w = min(r5_w, r4_w);

		//	r4 = max(r5, r4);
		r4_x = max(r5_x, r4_x);
		r4_y = max(r5_y, r4_y);
		r4_z = max(r5_z, r4_z);
		r4_w = max(r5_w, r4_w);

		//	r5 = min(r7, r3);
		r5_x = min(r7_x, r3_x);
		r5_y = min(r7_y, r3_y);
		r5_z = min(r7_z, r3_z);
		r5_w = min(r7_w, r3_w);

		//	r3 = max(r7, r3);
		r3_x = max(r7_x, r3_x);
		r3_y = max(r7_y, r3_y);
		r3_z = max(r7_z, r3_z);
		r3_w = max(r7_w, r3_w);

		//	r7 = min(r0, r9);
		r7_x = min(r0_x, r9_x);
		r7_y = min(r0_y, r9_y);
		r7_z = min(r0_z, r9_z);
		r7_w = min(r0_w, r9_w);

		//	r9 = max(r0, r9);
		r9_x = max(r0_x, r9_x);
		r9_y = max(r0_y, r9_y);
		r9_z = max(r0_z, r9_z);
		r9_w = max(r0_w, r9_w);

		//	r0 = min(r8, r2);
		r0_x = min(r8_x, r2_x);
		r0_y = min(r8_y, r2_y);
		r0_z = min(r8_z, r2_z);
		r0_w = min(r8_w, r2_w);

		//	r2 = max(r8, r2);
		r2_x = max(r8_x, r2_x);
		r2_y = max(r8_y, r2_y);
		r2_z = max(r8_z, r2_z);
		r2_w = max(r8_w, r2_w);

		//	r8 = min(r6, r5);
		r8_x = min(r6_x, r5_x);
		r8_y = min(r6_y, r5_y);
		r8_z = min(r6_z, r5_z);
		r8_w = min(r6_w, r5_w);

		//	r5 = max(r6, r5);
		r5_x = max(r6_x, r5_x);
		r5_y = max(r6_y, r5_y);
		r5_z = max(r6_z, r5_z);
		r5_w = max(r6_w, r5_w);

		//	r6 = min(r1, r7);
		r6_x = min(r1_x, r7_x);
		r6_y = min(r1_y, r7_y);
		r6_z = min(r1_z, r7_z);
		r6_w = min(r1_w, r7_w);

		//	r7 = max(r1, r7);
		r7_x = max(r1_x, r7_x);
		r7_y = max(r1_y, r7_y);
		r7_z = max(r1_z, r7_z);
		r7_w = max(r1_w, r7_w);

		//	r1 = min(r2, r4);
		r1_x = min(r2_x, r4_x);
		r1_y = min(r2_y, r4_y);
		r1_z = min(r2_z, r4_z);
		r1_w = min(r2_w, r4_w);

		//	r4 = max(r2, r4);
		r4_x = max(r2_x, r4_x);
		r4_y = max(r2_y, r4_y);
		r4_z = max(r2_z, r4_z);
		r4_w = max(r2_w, r4_w);

		//	r2 = min(r5, r3);
		r2_x = min(r5_x, r3_x);
		r2_y = min(r5_y, r3_y);
		r2_z = min(r5_z, r3_z);
		r2_w = min(r5_w, r3_w);

		//	r3 = max(r5, r3);
		r3_x = max(r5_x, r3_x);
		r3_y = max(r5_y, r3_y);
		r3_z = max(r5_z, r3_z);
		r3_w = max(r5_w, r3_w);

		//	r5 = min(r7, r9);
		r5_x = min(r7_x, r9_x);
		r5_y = min(r7_y, r9_y);
		r5_z = min(r7_z, r9_z);
		r5_w = min(r7_w, r9_w);

		//	r7 = max(r7, r9);
		r7_x = max(r7_x, r9_x);
		r7_y = max(r7_y, r9_y);
		r7_z = max(r7_z, r9_z);
		r7_w = max(r7_w, r9_w);

		//	r0 = max(r0, r8);
		r0_x = max(r0_x, r8_x);
		r0_y = max(r0_y, r8_y);
		r0_z = max(r0_z, r8_z);
		r0_w = max(r0_w, r8_w);

		//	r3 = min(r3, r7);
		r3_x = min(r3_x, r7_x);
		r3_y = min(r3_y, r7_y);
		r3_z = min(r3_z, r7_z);
		r3_w = min(r3_w, r7_w);

		//	r0 = max(r0, r6);
		r0_x = max(r0_x, r6_x);
		r0_y = max(r0_y, r6_y);
		r0_z = max(r0_z, r6_z);
		r0_w = max(r0_w, r6_w);

		//	r3 = min(r4, r3);
		r3_x = min(r4_x, r3_x);
		r3_y = min(r4_y, r3_y);
		r3_z = min(r4_z, r3_z);
		r3_w = min(r4_w, r3_w);

		//	r4 = min(r2, r5);
		r4_x = min(r2_x, r5_x);
		r4_y = min(r2_y, r5_y);
		r4_z = min(r2_z, r5_z);
		r4_w = min(r2_w, r5_w);

		//	r2 = max(r2, r5);
		r2_x = max(r2_x, r5_x);
		r2_y = max(r2_y, r5_y);
		r2_z = max(r2_z, r5_z);
		r2_w = max(r2_w, r5_w);

		//	r1 = max(r1, r4);
		r1_x = max(r1_x, r4_x);
		r1_y = max(r1_y, r4_y);
		r1_z = max(r1_z, r4_z);
		r1_w = max(r1_w, r4_w);

		//	r1 = min(r2, r1);
		r1_x = min(r2_x, r1_x);
		r1_y = min(r2_y, r1_y);
		r1_z = min(r2_z, r1_z);
		r1_w = min(r2_w, r1_w);

		//	r2 = min(r1, r3);
		r2_x = min(r1_x, r3_x);
		r2_y = min(r1_y, r3_y);
		r2_z = min(r1_z, r3_z);
		r2_w = min(r1_w, r3_w);

		//	r1 = max(r1, r3);
		r1_x = max(r1_x, r3_x);
		r1_y = max(r1_y, r3_y);
		r1_z = max(r1_z, r3_z);
		r1_w = max(r1_w, r3_w);

		//	r0 = max(r0, r2);
		r0_x = max(r0_x, r2_x);
		r0_y = max(r0_y, r2_y);
		r0_z = max(r0_z, r2_z);
		r0_w = max(r0_w, r2_w);

		//	r0 = min(r0, r1);
		r0_x = min(r0_x, r1_x);
		r0_y = min(r0_y, r1_y);
		r0_z = min(r0_z, r1_z);
		r0_w = min(r0_w, r1_w);

		//	r0.xyz = min(r0.xyz, r0.www);
		r0_x = min(r0_x, r0_w);
		r0_y = min(r0_y, r0_w);
		r0_z = min(r0_z, r0_w);

		//	o0 = r0;
		o0_x = r0_x;
		o0_y = r0_y;
		o0_z = r0_z;
		o0_w = r0_w;

		//	write_imagef(dest, (int2)( loc.x + dim.z , flipped ? get_image_height(dest) - (loc.y + dim.w + 1) : loc.y + dim.w ), o0);
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

	//	const float4 p0 = (float4)( 0x1p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
	const float4 p0_x = (float4)0x1p+0;
	const float4 p0_y = (float4)0x0p+0;
	const float4 p0_z = (float4)0x0p+0;
	const float4 p0_w = (float4)0x0p+0;

	//	const float4 p1 = (float4)( 0x1p+0, 0x1p+0, 0x0p+0, 0x0p+0 );
	const float4 p1_x = (float4)0x1p+0;
	const float4 p1_y = (float4)0x1p+0;
	const float4 p1_z = (float4)0x0p+0;
	const float4 p1_w = (float4)0x0p+0;

	//	const float4 p2 = (float4)( 0x0p+0, 0x1p+0, 0x0p+0, 0x0p+0 );
	const float4 p2_x = (float4)0x0p+0;
	const float4 p2_y = (float4)0x1p+0;
	const float4 p2_z = (float4)0x0p+0;
	const float4 p2_w = (float4)0x0p+0;

	//	const float4 p3 = (float4)( -0x1p+0, 0x1p+0, 0x0p+0, 0x0p+0 );
	const float4 p3_x = (float4)-0x1p+0;
	const float4 p3_y = (float4)0x1p+0;
	const float4 p3_z = (float4)0x0p+0;
	const float4 p3_w = (float4)0x0p+0;

	//	const float4 p4 = (float4)( -0x1p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
	const float4 p4_x = (float4)-0x1p+0;
	const float4 p4_y = (float4)0x0p+0;
	const float4 p4_z = (float4)0x0p+0;
	const float4 p4_w = (float4)0x0p+0;

	//	const float4 p5 = (float4)( -0x1p+0, -0x1p+0, 0x0p+0, 0x0p+0 );
	const float4 p5_x = (float4)-0x1p+0;
	const float4 p5_y = (float4)-0x1p+0;
	const float4 p5_z = (float4)0x0p+0;
	const float4 p5_w = (float4)0x0p+0;

	//	const float4 p6 = (float4)( 0x0p+0, -0x1p+0, 0x0p+0, 0x0p+0 );
	const float4 p6_x = (float4)0x0p+0;
	const float4 p6_y = (float4)-0x1p+0;
	const float4 p6_z = (float4)0x0p+0;
	const float4 p6_w = (float4)0x0p+0;

	//	const float4 p7 = (float4)( 0x1p+0, -0x1p+0, 0x0p+0, 0x0p+0 );
	const float4 p7_x = (float4)0x1p+0;
	const float4 p7_y = (float4)-0x1p+0;
	const float4 p7_z = (float4)0x0p+0;
	const float4 p7_w = (float4)0x0p+0;

	int4 dest_width = (int4)dim.x;
	int4 dest_height = (int4)dim.y;

	//	float4 o0, r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10;
	float4 o0_x, o0_y, o0_z, o0_w;
	float4 r0_x, r0_y, r0_z, r0_w;
	float4 r1_x, r1_y, r1_z, r1_w;
	float4 r2_x, r2_y, r2_z, r2_w;
	float4 r3_x, r3_y, r3_z, r3_w;
	float4 r4_x, r4_y, r4_z, r4_w;
	float4 r5_x, r5_y, r5_z, r5_w;
	float4 r6_x, r6_y, r6_z, r6_w;
	float4 r7_x, r7_y, r7_z, r7_w;
	float4 r8_x, r8_y, r8_z, r8_w;
	float4 r9_x, r9_y, r9_z, r9_w;
	float4 r10_x, r10_y, r10_z, r10_w;

	float4 f0_x, f0_y, f0_z, f0_w;

	//	int2 loc = (int2)( get_global_id(0), get_global_id(1) );
	int4 loc_x, loc_y;

	loc_y = get_global_id(1);
	int count;
	const int Many = 128;

	int tmp = get_global_id(0) * Many;

	for (count = 0; count < Many/4; ++count) {

		loc_x = (int4)(tmp, tmp+1, tmp+2, tmp+3);

		// float4 f0 = (float4)( st_origin.x + ((float)loc.x + 0.5f) * st_delta.x + ((float)loc.y + 0.5f) * st_delta.z, st_origin.y + ((float)loc.x + 0.5f) * st_delta.y + ((float)loc.y + 0.5f) * st_delta.w, 0.0f, 0.0f );
		f0_x = (float4)(st_origin.x + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.x + ((float4)((float)loc_y.x, (float)loc_y.y, (float)loc_y.z, (float)loc_y.w) + 0.5f) * st_delta.z);
		f0_y = (float4)(st_origin.y + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.y + ((float4)((float)loc_y.x, (float)loc_y.y, (float)loc_y.z, (float)loc_y.w) + 0.5f) * st_delta.w);
		f0_z = (float4)0.0f;
		f0_w = (float4)0.0f;

		//	r2 = f0;
		r2_x = f0_x;
		r2_y = f0_y;
		r2_z = f0_z;
		r2_w = f0_w;

		//	r10 = r2+p0;
		r10_x = r2_x + p0_x;
		r10_y = r2_y + p0_y;
		r10_z = r2_z + p0_z;
		r10_w = r2_w + p0_w;

		//	r8.x = dot(r10.xy,l0.xy) + l0.w;
		r8_x = (r10_x * (float4)l0.x + r10_y * (float4)l0.y) + l0.w;

		//	r8.y = dot(r10.xy,l1.xy) + l1.w;
		r8_y = (r10_x * (float4)l1.x + r10_y * (float4)l1.y) + l1.w;

		//	r10 = r8;
		r10_x = r8_x;
		r10_y = r8_y;
		r10_z = r8_z;
		r10_w = r8_w;

		//	r8 = read_imagef(t0, t_sampler0, r10.xy);
		read_transposed_imagef(t0, t_sampler0, r10_x, r10_y, &r8_x, &r8_y, &r8_z, &r8_w);

		//	r10 = r2+p1;
		r10_x = r2_x + p1_x;
		r10_y = r2_y + p1_y;
		r10_z = r2_z + p1_z;
		r10_w = r2_w + p1_w;

		//	r5.x = dot(r10.xy,l0.xy) + l0.w;
		r5_x = (r10_x * (float4)l0.x + r10_y * (float4)l0.y) + l0.w;

		//	r5.y = dot(r10.xy,l1.xy) + l1.w;
		r5_y = (r10_x * (float4)l1.x + r10_y * (float4)l1.y) + l1.w;

		//	r10 = r5;
		r10_x = r5_x;
		r10_y = r5_y;
		r10_z = r5_z;
		r10_w = r5_w;

		//	r5 = read_imagef(t0, t_sampler0, r10.xy);
		read_transposed_imagef(t0, t_sampler0, r10_x, r10_y, &r5_x, &r5_y, &r5_z, &r5_w);

		//	r10 = r2+p2;
		r10_x = r2_x + p2_x;
		r10_y = r2_y + p2_y;
		r10_z = r2_z + p2_z;
		r10_w = r2_w + p2_w;

		//	r4.x = dot(r10.xy,l0.xy) + l0.w;
		r4_x = (r10_x * (float4)l0.x + r10_y * (float4)l0.y) + l0.w;

		//	r4.y = dot(r10.xy,l1.xy) + l1.w;
		r4_y = (r10_x * (float4)l1.x + r10_y * (float4)l1.y) + l1.w;

		//	r10 = r4;
		r10_x = r4_x;
		r10_y = r4_y;
		r10_z = r4_z;
		r10_w = r4_w;

		// r4 = read_imagef(t0, t_sampler0, r10.xy);
		read_transposed_imagef(t0, t_sampler0, r10_x, r10_y, &r4_x, &r4_y, &r4_z, &r4_w);

		//	r10 = r2+p3;
		r10_x = r2_x + p3_x;
		r10_y = r2_y + p3_y;
		r10_z = r2_z + p3_z;
		r10_w = r2_w + p3_w;

		//	r6.x = dot(r10.xy,l0.xy) + l0.w;
		r6_x = (r10_x * (float4)l0.x + r10_y * (float4)l0.y) + l0.w;

		//	r6.y = dot(r10.xy,l1.xy) + l1.w;
		r6_y = (r10_x * (float4)l1.x + r10_y * (float4)l1.y) + l1.w;

		//	r10 = r6;
		r10_x = r6_x;
		r10_y = r6_y;
		r10_z = r6_z;
		r10_w = r6_w;

		//	r6 = read_imagef(t0, t_sampler0, r10.xy);
		read_transposed_imagef(t0, t_sampler0, r10_x, r10_y, &r6_x, &r6_y, &r6_z, &r6_w);

		//	r10 = r2+p4;
		r10_x = r2_x + p4_x;
		r10_y = r2_y + p4_y;
		r10_z = r2_z + p4_z;
		r10_w = r2_w + p4_w;

		//	r7.x = dot(r10.xy,l0.xy) + l0.w;
		r7_x = (r10_x * (float4)l0.x + r10_y * (float4)l0.y) + l0.w;

		//	r7.y = dot(r10.xy,l1.xy) + l1.w;
		r7_y = (r10_x * (float4)l1.x + r10_y * (float4)l1.y) + l1.w;

		//	r10 = r7;
		r10_x = r7_x;
		r10_y = r7_y;
		r10_z = r7_z;
		r10_w = r7_w;

		//	r7 = read_imagef(t0, t_sampler0, r10.xy);
		read_transposed_imagef(t0, t_sampler0, r10_x, r10_y, &r7_x, &r7_y, &r7_z, &r7_w);

		//	r10 = r2+p5;
		r10_x = r2_x + p5_x;
		r10_y = r2_y + p5_y;
		r10_z = r2_z + p5_z;
		r10_w = r2_w + p5_w;

		//	r3.x = dot(r10.xy,l0.xy) + l0.w;
		r3_x = (r10_x * (float4)l0.x + r10_y * (float4)l0.y) + l0.w;

		//	r3.y = dot(r10.xy,l1.xy) + l1.w;
		r3_y = (r10_x * (float4)l1.x + r10_y * (float4)l1.y) + l1.w;

		//	r10 = r3;
		r10_x = r3_x;
		r10_y = r3_y;
		r10_z = r3_z;
		r10_w = r3_w;

		//	r3 = read_imagef(t0, t_sampler0, r10.xy);
		read_transposed_imagef(t0, t_sampler0, r10_x, r10_y, &r3_x, &r3_y, &r3_z, &r3_w);

		//	r10 = r2+p6;
		r10_x = r2_x + p6_x;
		r10_y = r2_y + p6_y;
		r10_z = r2_z + p6_z;
		r10_w = r2_w + p6_w;

		//	r1.x = dot(r10.xy,l0.xy) + l0.w;
		r1_x = (r10_x * (float4)l0.x + r10_y * (float4)l0.y) + l0.w;

		//	r1.y = dot(r10.xy,l1.xy) + l1.w;
		r1_y = (r10_x * (float4)l1.x + r10_y * (float4)l1.y) + l1.w;

		//	r10 = r1;
		r10_x = r1_x;
		r10_y = r1_y;
		r10_z = r1_z;
		r10_w = r1_w;

		//	r1 = read_imagef(t0, t_sampler0, r10.xy);
		read_transposed_imagef(t0, t_sampler0, r10_x, r10_y, &r1_x, &r1_y, &r1_z, &r1_w);

		//	r10 = r2+p7;
		r10_x = r2_x + p7_x;
		r10_y = r2_y + p7_y;
		r10_z = r2_z + p7_z;
		r10_w = r2_w + p7_w;

		//	r0.x = dot(r10.xy,l0.xy) + l0.w;
		r0_x = (r10_x * (float4)l0.x + r10_y * (float4)l0.y) + l0.w;

		//	r0.y = dot(r10.xy,l1.xy) + l1.w;
		r0_y = (r10_x * (float4)l1.x + r10_y * (float4)l1.y) + l1.w;

		//	r0 = read_imagef(t0, t_sampler0, r0.xy);
		read_transposed_imagef(t0, t_sampler0, r0_x, r0_y, &r0_x, &r0_y, &r0_z, &r0_w);

		//	r9.x = dot(r2.xy,l0.xy) + l0.w;
		r9_x = (r2_x * (float4)l0.x + r2_y * (float4)l0.y) + l0.w;

		//	r9.y = dot(r2.xy,l1.xy) + l1.w;
		r9_y = (r2_x * (float4)l1.x + r2_y * (float4)l1.y) + l1.w;

		//	r2 = r9;
		r2_x = r9_x;
		r2_y = r9_y;
		r2_z = r9_z;
		r2_w = r9_w;

		//	r9 = read_imagef(t0, t_sampler0, r2.xy);
		read_transposed_imagef(t0, t_sampler0, r2_x, r2_y, &r9_x, &r9_y, &r9_z, &r9_w);

		//	r2 = min(r5, r4);
		r2_x = min(r5_x, r4_x);
		r2_y = min(r5_y, r4_y);
		r2_z = min(r5_z, r4_z);
		r2_w = min(r5_w, r4_w);

		//	r4 = max(r5, r4);
		r4_x = max(r5_x, r4_x);
		r4_y = max(r5_y, r4_y);
		r4_z = max(r5_z, r4_z);
		r4_w = max(r5_w, r4_w);

		//	r5 = min(r7, r3);
		r5_x = min(r7_x, r3_x);
		r5_y = min(r7_y, r3_y);
		r5_z = min(r7_z, r3_z);
		r5_w = min(r7_w, r3_w);

		//	r3 = max(r7, r3);
		r3_x = max(r7_x, r3_x);
		r3_y = max(r7_y, r3_y);
		r3_z = max(r7_z, r3_z);
		r3_w = max(r7_w, r3_w);

		//	r7 = min(r0, r9);
		r7_x = min(r0_x, r9_x);
		r7_y = min(r0_y, r9_y);
		r7_z = min(r0_z, r9_z);
		r7_w = min(r0_w, r9_w);

		//	r9 = max(r0, r9);
		r9_x = max(r0_x, r9_x);
		r9_y = max(r0_y, r9_y);
		r9_z = max(r0_z, r9_z);
		r9_w = max(r0_w, r9_w);

		//	r0 = min(r8, r2);
		r0_x = min(r8_x, r2_x);
		r0_y = min(r8_y, r2_y);
		r0_z = min(r8_z, r2_z);
		r0_w = min(r8_w, r2_w);

		//	r2 = max(r8, r2);
		r2_x = max(r8_x, r2_x);
		r2_y = max(r8_y, r2_y);
		r2_z = max(r8_z, r2_z);
		r2_w = max(r8_w, r2_w);

		//	r8 = min(r6, r5);
		r8_x = min(r6_x, r5_x);
		r8_y = min(r6_y, r5_y);
		r8_z = min(r6_z, r5_z);
		r8_w = min(r6_w, r5_w);

		//	r5 = max(r6, r5);
		r5_x = max(r6_x, r5_x);
		r5_y = max(r6_y, r5_y);
		r5_z = max(r6_z, r5_z);
		r5_w = max(r6_w, r5_w);

		//	r6 = min(r1, r7);
		r6_x = min(r1_x, r7_x);
		r6_y = min(r1_y, r7_y);
		r6_z = min(r1_z, r7_z);
		r6_w = min(r1_w, r7_w);

		//	r7 = max(r1, r7);
		r7_x = max(r1_x, r7_x);
		r7_y = max(r1_y, r7_y);
		r7_z = max(r1_z, r7_z);
		r7_w = max(r1_w, r7_w);

		//	r1 = min(r2, r4);
		r1_x = min(r2_x, r4_x);
		r1_y = min(r2_y, r4_y);
		r1_z = min(r2_z, r4_z);
		r1_w = min(r2_w, r4_w);

		//	r4 = max(r2, r4);
		r4_x = max(r2_x, r4_x);
		r4_y = max(r2_y, r4_y);
		r4_z = max(r2_z, r4_z);
		r4_w = max(r2_w, r4_w);

		//	r2 = min(r5, r3);
		r2_x = min(r5_x, r3_x);
		r2_y = min(r5_y, r3_y);
		r2_z = min(r5_z, r3_z);
		r2_w = min(r5_w, r3_w);

		//	r3 = max(r5, r3);
		r3_x = max(r5_x, r3_x);
		r3_y = max(r5_y, r3_y);
		r3_z = max(r5_z, r3_z);
		r3_w = max(r5_w, r3_w);

		//	r5 = min(r7, r9);
		r5_x = min(r7_x, r9_x);
		r5_y = min(r7_y, r9_y);
		r5_z = min(r7_z, r9_z);
		r5_w = min(r7_w, r9_w);

		//	r7 = max(r7, r9);
		r7_x = max(r7_x, r9_x);
		r7_y = max(r7_y, r9_y);
		r7_z = max(r7_z, r9_z);
		r7_w = max(r7_w, r9_w);

		//	r0 = max(r0, r8);
		r0_x = max(r0_x, r8_x);
		r0_y = max(r0_y, r8_y);
		r0_z = max(r0_z, r8_z);
		r0_w = max(r0_w, r8_w);

		//	r3 = min(r3, r7);
		r3_x = min(r3_x, r7_x);
		r3_y = min(r3_y, r7_y);
		r3_z = min(r3_z, r7_z);
		r3_w = min(r3_w, r7_w);

		//	r0 = max(r0, r6);
		r0_x = max(r0_x, r6_x);
		r0_y = max(r0_y, r6_y);
		r0_z = max(r0_z, r6_z);
		r0_w = max(r0_w, r6_w);

		//	r3 = min(r4, r3);
		r3_x = min(r4_x, r3_x);
		r3_y = min(r4_y, r3_y);
		r3_z = min(r4_z, r3_z);
		r3_w = min(r4_w, r3_w);

		//	r4 = min(r2, r5);
		r4_x = min(r2_x, r5_x);
		r4_y = min(r2_y, r5_y);
		r4_z = min(r2_z, r5_z);
		r4_w = min(r2_w, r5_w);

		//	r2 = max(r2, r5);
		r2_x = max(r2_x, r5_x);
		r2_y = max(r2_y, r5_y);
		r2_z = max(r2_z, r5_z);
		r2_w = max(r2_w, r5_w);

		//	r1 = max(r1, r4);
		r1_x = max(r1_x, r4_x);
		r1_y = max(r1_y, r4_y);
		r1_z = max(r1_z, r4_z);
		r1_w = max(r1_w, r4_w);

		//	r1 = min(r2, r1);
		r1_x = min(r2_x, r1_x);
		r1_y = min(r2_y, r1_y);
		r1_z = min(r2_z, r1_z);
		r1_w = min(r2_w, r1_w);

		//	r2 = min(r1, r3);
		r2_x = min(r1_x, r3_x);
		r2_y = min(r1_y, r3_y);
		r2_z = min(r1_z, r3_z);
		r2_w = min(r1_w, r3_w);

		//	r1 = max(r1, r3);
		r1_x = max(r1_x, r3_x);
		r1_y = max(r1_y, r3_y);
		r1_z = max(r1_z, r3_z);
		r1_w = max(r1_w, r3_w);

		//	r0 = max(r0, r2);
		r0_x = max(r0_x, r2_x);
		r0_y = max(r0_y, r2_y);
		r0_z = max(r0_z, r2_z);
		r0_w = max(r0_w, r2_w);

		//	r0 = min(r0, r1);
		r0_x = min(r0_x, r1_x);
		r0_y = min(r0_y, r1_y);
		r0_z = min(r0_z, r1_z);
		r0_w = min(r0_w, r1_w);

		//	r0.xyz = min(r0.xyz, r0.www);
		r0_x = min(r0_x, r0_w);
		r0_y = min(r0_y, r0_w);
		r0_z = min(r0_z, r0_w);

		//	o0 = r0;
		o0_x = r0_x;
		o0_y = r0_y;
		o0_z = r0_z;
		o0_w = r0_w;

		//	write_imagef(dest, (int2)( loc.x + dim.z , flipped ? get_image_height(dest) - (loc.y + dim.w + 1) : loc.y + dim.w ), o0);
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

	//	const float4 p0 = (float4)( 0x1p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
	const float4 p0_x = (float4)0x1p+0;
	const float4 p0_y = (float4)0x0p+0;
	const float4 p0_z = (float4)0x0p+0;
	const float4 p0_w = (float4)0x0p+0;

	//	const float4 p1 = (float4)( 0x1p+0, 0x1p+0, 0x0p+0, 0x0p+0 );
	const float4 p1_x = (float4)0x1p+0;
	const float4 p1_y = (float4)0x1p+0;
	const float4 p1_z = (float4)0x0p+0;
	const float4 p1_w = (float4)0x0p+0;

	//	const float4 p2 = (float4)( 0x0p+0, 0x1p+0, 0x0p+0, 0x0p+0 );
	const float4 p2_x = (float4)0x0p+0;
	const float4 p2_y = (float4)0x1p+0;
	const float4 p2_z = (float4)0x0p+0;
	const float4 p2_w = (float4)0x0p+0;

	//	const float4 p3 = (float4)( -0x1p+0, 0x1p+0, 0x0p+0, 0x0p+0 );
	const float4 p3_x = (float4)-0x1p+0;
	const float4 p3_y = (float4)0x1p+0;
	const float4 p3_z = (float4)0x0p+0;
	const float4 p3_w = (float4)0x0p+0;

	//	const float4 p4 = (float4)( -0x1p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
	const float4 p4_x = (float4)-0x1p+0;
	const float4 p4_y = (float4)0x0p+0;
	const float4 p4_z = (float4)0x0p+0;
	const float4 p4_w = (float4)0x0p+0;

	//	const float4 p5 = (float4)( -0x1p+0, -0x1p+0, 0x0p+0, 0x0p+0 );
	const float4 p5_x = (float4)-0x1p+0;
	const float4 p5_y = (float4)-0x1p+0;
	const float4 p5_z = (float4)0x0p+0;
	const float4 p5_w = (float4)0x0p+0;

	//	const float4 p6 = (float4)( 0x0p+0, -0x1p+0, 0x0p+0, 0x0p+0 );
	const float4 p6_x = (float4)0x0p+0;
	const float4 p6_y = (float4)-0x1p+0;
	const float4 p6_z = (float4)0x0p+0;
	const float4 p6_w = (float4)0x0p+0;

	//	const float4 p7 = (float4)( 0x1p+0, -0x1p+0, 0x0p+0, 0x0p+0 );
	const float4 p7_x = (float4)0x1p+0;
	const float4 p7_y = (float4)-0x1p+0;
	const float4 p7_z = (float4)0x0p+0;
	const float4 p7_w = (float4)0x0p+0;

	int4 dest_width = (int4)dim.x;
	int4 dest_height = (int4)dim.y;

	//	float4 o0, r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10;
	float4 o0_x, o0_y, o0_z, o0_w;
	float4 r0_x, r0_y, r0_z, r0_w;
	float4 r1_x, r1_y, r1_z, r1_w;
	float4 r2_x, r2_y, r2_z, r2_w;
	float4 r3_x, r3_y, r3_z, r3_w;
	float4 r4_x, r4_y, r4_z, r4_w;
	float4 r5_x, r5_y, r5_z, r5_w;
	float4 r6_x, r6_y, r6_z, r6_w;
	float4 r7_x, r7_y, r7_z, r7_w;
	float4 r8_x, r8_y, r8_z, r8_w;
	float4 r9_x, r9_y, r9_z, r9_w;
	float4 r10_x, r10_y, r10_z, r10_w;

	//	int2 loc = (int2)( get_global_id(0), get_global_id(1) );
	int4 tmp_loc_x = (int4)get_global_id(0) * 4;
	int4 loc_y = (int4)get_global_id(1);
	int4 loc_x = tmp_loc_x + (int4)(0, 1, 2, 3);

	// float4 f0 = (float4)( st_origin.x + ((float)loc.x + 0.5f) * st_delta.x + ((float)loc.y + 0.5f) * st_delta.z, st_origin.y + ((float)loc.x + 0.5f) * st_delta.y + ((float)loc.y + 0.5f) * st_delta.w, 0.0f, 0.0f );
	float4 f0_x = (float4)(st_origin.x + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.x + ((float4)((float)loc_y.x, (float)loc_y.y, (float)loc_y.z, (float)loc_y.w) + 0.5f) * st_delta.z);
	float4 f0_y = (float4)(st_origin.y + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.y + ((float4)((float)loc_y.x, (float)loc_y.y, (float)loc_y.z, (float)loc_y.w) + 0.5f) * st_delta.w);
	float4 f0_z = (float4)0.0f;
	float4 f0_w = (float4)0.0f;

	//	r2 = f0;
	r2_x = f0_x;
	r2_y = f0_y;
	r2_z = f0_z;
	r2_w = f0_w;

	//	r10 = r2+p0;
	r10_x = r2_x + p0_x;
	r10_y = r2_y + p0_y;
	r10_z = r2_z + p0_z;
	r10_w = r2_w + p0_w;

	//	r8.x = dot(r10.xy,l0.xy) + l0.w;
	r8_x = (r10_x * (float4)l0.x + r10_y * (float4)l0.y) + l0.w;

	//	r8.y = dot(r10.xy,l1.xy) + l1.w;
	r8_y = (r10_x * (float4)l1.x + r10_y * (float4)l1.y) + l1.w;

	//	r10 = r8;
	r10_x = r8_x;
	r10_y = r8_y;
	r10_z = r8_z;
	r10_w = r8_w;

	//	r8 = read_imagef(t0, t_sampler0, r10.xy);
	read_transposed_imagef(t0, t_sampler0, r10_x, r10_y, &r8_x, &r8_y, &r8_z, &r8_w);

	//	r10 = r2+p1;
	r10_x = r2_x + p1_x;
	r10_y = r2_y + p1_y;
	r10_z = r2_z + p1_z;
	r10_w = r2_w + p1_w;

	//	r5.x = dot(r10.xy,l0.xy) + l0.w;
	r5_x = (r10_x * (float4)l0.x + r10_y * (float4)l0.y) + l0.w;

	//	r5.y = dot(r10.xy,l1.xy) + l1.w;
	r5_y = (r10_x * (float4)l1.x + r10_y * (float4)l1.y) + l1.w;

	//	r10 = r5;
	r10_x = r5_x;
	r10_y = r5_y;
	r10_z = r5_z;
	r10_w = r5_w;

	//	r5 = read_imagef(t0, t_sampler0, r10.xy);
	read_transposed_imagef(t0, t_sampler0, r10_x, r10_y, &r5_x, &r5_y, &r5_z, &r5_w);

	//	r10 = r2+p2;
	r10_x = r2_x + p2_x;
	r10_y = r2_y + p2_y;
	r10_z = r2_z + p2_z;
	r10_w = r2_w + p2_w;

	//	r4.x = dot(r10.xy,l0.xy) + l0.w;
	r4_x = (r10_x * (float4)l0.x + r10_y * (float4)l0.y) + l0.w;

	//	r4.y = dot(r10.xy,l1.xy) + l1.w;
	r4_y = (r10_x * (float4)l1.x + r10_y * (float4)l1.y) + l1.w;

	//	r10 = r4;
	r10_x = r4_x;
	r10_y = r4_y;
	r10_z = r4_z;
	r10_w = r4_w;

	// r4 = read_imagef(t0, t_sampler0, r10.xy);
	read_transposed_imagef(t0, t_sampler0, r10_x, r10_y, &r4_x, &r4_y, &r4_z, &r4_w);

	//	r10 = r2+p3;
	r10_x = r2_x + p3_x;
	r10_y = r2_y + p3_y;
	r10_z = r2_z + p3_z;
	r10_w = r2_w + p3_w;

	//	r6.x = dot(r10.xy,l0.xy) + l0.w;
	r6_x = (r10_x * (float4)l0.x + r10_y * (float4)l0.y) + l0.w;

	//	r6.y = dot(r10.xy,l1.xy) + l1.w;
	r6_y = (r10_x * (float4)l1.x + r10_y * (float4)l1.y) + l1.w;

	//	r10 = r6;
	r10_x = r6_x;
	r10_y = r6_y;
	r10_z = r6_z;
	r10_w = r6_w;

	//	r6 = read_imagef(t0, t_sampler0, r10.xy);
	read_transposed_imagef(t0, t_sampler0, r10_x, r10_y, &r6_x, &r6_y, &r6_z, &r6_w);

	//	r10 = r2+p4;
	r10_x = r2_x + p4_x;
	r10_y = r2_y + p4_y;
	r10_z = r2_z + p4_z;
	r10_w = r2_w + p4_w;

	//	r7.x = dot(r10.xy,l0.xy) + l0.w;
	r7_x = (r10_x * (float4)l0.x + r10_y * (float4)l0.y) + l0.w;

	//	r7.y = dot(r10.xy,l1.xy) + l1.w;
	r7_y = (r10_x * (float4)l1.x + r10_y * (float4)l1.y) + l1.w;

	//	r10 = r7;
	r10_x = r7_x;
	r10_y = r7_y;
	r10_z = r7_z;
	r10_w = r7_w;

	//	r7 = read_imagef(t0, t_sampler0, r10.xy);
	read_transposed_imagef(t0, t_sampler0, r10_x, r10_y, &r7_x, &r7_y, &r7_z, &r7_w);

	//	r10 = r2+p5;
	r10_x = r2_x + p5_x;
	r10_y = r2_y + p5_y;
	r10_z = r2_z + p5_z;
	r10_w = r2_w + p5_w;

	//	r3.x = dot(r10.xy,l0.xy) + l0.w;
	r3_x = (r10_x * (float4)l0.x + r10_y * (float4)l0.y) + l0.w;

	//	r3.y = dot(r10.xy,l1.xy) + l1.w;
	r3_y = (r10_x * (float4)l1.x + r10_y * (float4)l1.y) + l1.w;

	//	r10 = r3;
	r10_x = r3_x;
	r10_y = r3_y;
	r10_z = r3_z;
	r10_w = r3_w;

	//	r3 = read_imagef(t0, t_sampler0, r10.xy);
	read_transposed_imagef(t0, t_sampler0, r10_x, r10_y, &r3_x, &r3_y, &r3_z, &r3_w);

	//	r10 = r2+p6;
	r10_x = r2_x + p6_x;
	r10_y = r2_y + p6_y;
	r10_z = r2_z + p6_z;
	r10_w = r2_w + p6_w;

	//	r1.x = dot(r10.xy,l0.xy) + l0.w;
	r1_x = (r10_x * (float4)l0.x + r10_y * (float4)l0.y) + l0.w;

	//	r1.y = dot(r10.xy,l1.xy) + l1.w;
	r1_y = (r10_x * (float4)l1.x + r10_y * (float4)l1.y) + l1.w;

	//	r10 = r1;
	r10_x = r1_x;
	r10_y = r1_y;
	r10_z = r1_z;
	r10_w = r1_w;

	//	r1 = read_imagef(t0, t_sampler0, r10.xy);
	read_transposed_imagef(t0, t_sampler0, r10_x, r10_y, &r1_x, &r1_y, &r1_z, &r1_w);

	//	r10 = r2+p7;
	r10_x = r2_x + p7_x;
	r10_y = r2_y + p7_y;
	r10_z = r2_z + p7_z;
	r10_w = r2_w + p7_w;

	//	r0.x = dot(r10.xy,l0.xy) + l0.w;
	r0_x = (r10_x * (float4)l0.x + r10_y * (float4)l0.y) + l0.w;

	//	r0.y = dot(r10.xy,l1.xy) + l1.w;
	r0_y = (r10_x * (float4)l1.x + r10_y * (float4)l1.y) + l1.w;

	//	r0 = read_imagef(t0, t_sampler0, r0.xy);
	read_transposed_imagef(t0, t_sampler0, r0_x, r0_y, &r0_x, &r0_y, &r0_z, &r0_w);

	//	r9.x = dot(r2.xy,l0.xy) + l0.w;
	r9_x = (r2_x * (float4)l0.x + r2_y * (float4)l0.y) + l0.w;

	//	r9.y = dot(r2.xy,l1.xy) + l1.w;
	r9_y = (r2_x * (float4)l1.x + r2_y * (float4)l1.y) + l1.w;

	//	r2 = r9;
	r2_x = r9_x;
	r2_y = r9_y;
	r2_z = r9_z;
	r2_w = r9_w;

	//	r9 = read_imagef(t0, t_sampler0, r2.xy);
	read_transposed_imagef(t0, t_sampler0, r2_x, r2_y, &r9_x, &r9_y, &r9_z, &r9_w);

	//	r2 = min(r5, r4);
	r2_x = min(r5_x, r4_x);
	r2_y = min(r5_y, r4_y);
	r2_z = min(r5_z, r4_z);
	r2_w = min(r5_w, r4_w);

	//	r4 = max(r5, r4);
	r4_x = max(r5_x, r4_x);
	r4_y = max(r5_y, r4_y);
	r4_z = max(r5_z, r4_z);
	r4_w = max(r5_w, r4_w);

	//	r5 = min(r7, r3);
	r5_x = min(r7_x, r3_x);
	r5_y = min(r7_y, r3_y);
	r5_z = min(r7_z, r3_z);
	r5_w = min(r7_w, r3_w);

	//	r3 = max(r7, r3);
	r3_x = max(r7_x, r3_x);
	r3_y = max(r7_y, r3_y);
	r3_z = max(r7_z, r3_z);
	r3_w = max(r7_w, r3_w);

	//	r7 = min(r0, r9);
	r7_x = min(r0_x, r9_x);
	r7_y = min(r0_y, r9_y);
	r7_z = min(r0_z, r9_z);
	r7_w = min(r0_w, r9_w);

	//	r9 = max(r0, r9);
	r9_x = max(r0_x, r9_x);
	r9_y = max(r0_y, r9_y);
	r9_z = max(r0_z, r9_z);
	r9_w = max(r0_w, r9_w);

	//	r0 = min(r8, r2);
	r0_x = min(r8_x, r2_x);
	r0_y = min(r8_y, r2_y);
	r0_z = min(r8_z, r2_z);
	r0_w = min(r8_w, r2_w);

	//	r2 = max(r8, r2);
	r2_x = max(r8_x, r2_x);
	r2_y = max(r8_y, r2_y);
	r2_z = max(r8_z, r2_z);
	r2_w = max(r8_w, r2_w);

	//	r8 = min(r6, r5);
	r8_x = min(r6_x, r5_x);
	r8_y = min(r6_y, r5_y);
	r8_z = min(r6_z, r5_z);
	r8_w = min(r6_w, r5_w);

	//	r5 = max(r6, r5);
	r5_x = max(r6_x, r5_x);
	r5_y = max(r6_y, r5_y);
	r5_z = max(r6_z, r5_z);
	r5_w = max(r6_w, r5_w);

	//	r6 = min(r1, r7);
	r6_x = min(r1_x, r7_x);
	r6_y = min(r1_y, r7_y);
	r6_z = min(r1_z, r7_z);
	r6_w = min(r1_w, r7_w);

	//	r7 = max(r1, r7);
	r7_x = max(r1_x, r7_x);
	r7_y = max(r1_y, r7_y);
	r7_z = max(r1_z, r7_z);
	r7_w = max(r1_w, r7_w);

	//	r1 = min(r2, r4);
	r1_x = min(r2_x, r4_x);
	r1_y = min(r2_y, r4_y);
	r1_z = min(r2_z, r4_z);
	r1_w = min(r2_w, r4_w);

	//	r4 = max(r2, r4);
	r4_x = max(r2_x, r4_x);
	r4_y = max(r2_y, r4_y);
	r4_z = max(r2_z, r4_z);
	r4_w = max(r2_w, r4_w);

	//	r2 = min(r5, r3);
	r2_x = min(r5_x, r3_x);
	r2_y = min(r5_y, r3_y);
	r2_z = min(r5_z, r3_z);
	r2_w = min(r5_w, r3_w);

	//	r3 = max(r5, r3);
	r3_x = max(r5_x, r3_x);
	r3_y = max(r5_y, r3_y);
	r3_z = max(r5_z, r3_z);
	r3_w = max(r5_w, r3_w);

	//	r5 = min(r7, r9);
	r5_x = min(r7_x, r9_x);
	r5_y = min(r7_y, r9_y);
	r5_z = min(r7_z, r9_z);
	r5_w = min(r7_w, r9_w);

	//	r7 = max(r7, r9);
	r7_x = max(r7_x, r9_x);
	r7_y = max(r7_y, r9_y);
	r7_z = max(r7_z, r9_z);
	r7_w = max(r7_w, r9_w);

	//	r0 = max(r0, r8);
	r0_x = max(r0_x, r8_x);
	r0_y = max(r0_y, r8_y);
	r0_z = max(r0_z, r8_z);
	r0_w = max(r0_w, r8_w);

	//	r3 = min(r3, r7);
	r3_x = min(r3_x, r7_x);
	r3_y = min(r3_y, r7_y);
	r3_z = min(r3_z, r7_z);
	r3_w = min(r3_w, r7_w);

	//	r0 = max(r0, r6);
	r0_x = max(r0_x, r6_x);
	r0_y = max(r0_y, r6_y);
	r0_z = max(r0_z, r6_z);
	r0_w = max(r0_w, r6_w);

	//	r3 = min(r4, r3);
	r3_x = min(r4_x, r3_x);
	r3_y = min(r4_y, r3_y);
	r3_z = min(r4_z, r3_z);
	r3_w = min(r4_w, r3_w);

	//	r4 = min(r2, r5);
	r4_x = min(r2_x, r5_x);
	r4_y = min(r2_y, r5_y);
	r4_z = min(r2_z, r5_z);
	r4_w = min(r2_w, r5_w);

	//	r2 = max(r2, r5);
	r2_x = max(r2_x, r5_x);
	r2_y = max(r2_y, r5_y);
	r2_z = max(r2_z, r5_z);
	r2_w = max(r2_w, r5_w);

	//	r1 = max(r1, r4);
	r1_x = max(r1_x, r4_x);
	r1_y = max(r1_y, r4_y);
	r1_z = max(r1_z, r4_z);
	r1_w = max(r1_w, r4_w);

	//	r1 = min(r2, r1);
	r1_x = min(r2_x, r1_x);
	r1_y = min(r2_y, r1_y);
	r1_z = min(r2_z, r1_z);
	r1_w = min(r2_w, r1_w);

	//	r2 = min(r1, r3);
	r2_x = min(r1_x, r3_x);
	r2_y = min(r1_y, r3_y);
	r2_z = min(r1_z, r3_z);
	r2_w = min(r1_w, r3_w);

	//	r1 = max(r1, r3);
	r1_x = max(r1_x, r3_x);
	r1_y = max(r1_y, r3_y);
	r1_z = max(r1_z, r3_z);
	r1_w = max(r1_w, r3_w);

	//	r0 = max(r0, r2);
	r0_x = max(r0_x, r2_x);
	r0_y = max(r0_y, r2_y);
	r0_z = max(r0_z, r2_z);
	r0_w = max(r0_w, r2_w);

	//	r0 = min(r0, r1);
	r0_x = min(r0_x, r1_x);
	r0_y = min(r0_y, r1_y);
	r0_z = min(r0_z, r1_z);
	r0_w = min(r0_w, r1_w);

	//	r0.xyz = min(r0.xyz, r0.www);
	r0_x = min(r0_x, r0_w);
	r0_y = min(r0_y, r0_w);
	r0_z = min(r0_z, r0_w);

	//	o0 = r0;
	o0_x = r0_x;
	o0_y = r0_y;
	o0_z = r0_z;
	o0_w = r0_w;

	//	write_imagef(dest, (int2)( loc.x + dim.z , flipped ? get_image_height(dest) - (loc.y + dim.w + 1) : loc.y + dim.w ), o0);
	int yaxis;
	int yaxisT, yaxisF;

	yaxisT = get_image_height(dest) - (loc_y.x + dim.w + 1);
	yaxisF = loc_y.x + dim.w;
	yaxis = select (yaxisF, yaxisT, flipped);

	write_transposed_imagef(dest, loc_x.x + dim.z, yaxis, o0_x, o0_y, o0_z, o0_w);
}

// replace dot fumction in program4_2 with the mathimatical calulation and vectorize it
__kernel void program4_3(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, read_only image2d_t t0, sampler_t t_sampler0)
{

	//	const float4 p0 = (float4)( 0x1p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
	const float4 p0_x = (float4)0x1p+0;
	const float4 p0_y = (float4)0x0p+0;
	const float4 p0_z = (float4)0x0p+0;
	const float4 p0_w = (float4)0x0p+0;

	//	const float4 p1 = (float4)( 0x1p+0, 0x1p+0, 0x0p+0, 0x0p+0 );
	const float4 p1_x = (float4)0x1p+0;
	const float4 p1_y = (float4)0x1p+0;
	const float4 p1_z = (float4)0x0p+0;
	const float4 p1_w = (float4)0x0p+0;

	//	const float4 p2 = (float4)( 0x0p+0, 0x1p+0, 0x0p+0, 0x0p+0 );
	const float4 p2_x = (float4)0x0p+0;
	const float4 p2_y = (float4)0x1p+0;
	const float4 p2_z = (float4)0x0p+0;
	const float4 p2_w = (float4)0x0p+0;

	//	const float4 p3 = (float4)( -0x1p+0, 0x1p+0, 0x0p+0, 0x0p+0 );
	const float4 p3_x = (float4)-0x1p+0;
	const float4 p3_y = (float4)0x1p+0;
	const float4 p3_z = (float4)0x0p+0;
	const float4 p3_w = (float4)0x0p+0;

	//	const float4 p4 = (float4)( -0x1p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
	const float4 p4_x = (float4)-0x1p+0;
	const float4 p4_y = (float4)0x0p+0;
	const float4 p4_z = (float4)0x0p+0;
	const float4 p4_w = (float4)0x0p+0;

	//	const float4 p5 = (float4)( -0x1p+0, -0x1p+0, 0x0p+0, 0x0p+0 );
	const float4 p5_x = (float4)-0x1p+0;
	const float4 p5_y = (float4)-0x1p+0;
	const float4 p5_z = (float4)0x0p+0;
	const float4 p5_w = (float4)0x0p+0;

	//	const float4 p6 = (float4)( 0x0p+0, -0x1p+0, 0x0p+0, 0x0p+0 );
	const float4 p6_x = (float4)0x0p+0;
	const float4 p6_y = (float4)-0x1p+0;
	const float4 p6_z = (float4)0x0p+0;
	const float4 p6_w = (float4)0x0p+0;

	//	const float4 p7 = (float4)( 0x1p+0, -0x1p+0, 0x0p+0, 0x0p+0 );
	const float4 p7_x = (float4)0x1p+0;
	const float4 p7_y = (float4)-0x1p+0;
	const float4 p7_z = (float4)0x0p+0;
	const float4 p7_w = (float4)0x0p+0;

	int4 dest_width = (int4)dim.x;
	int4 dest_height = (int4)dim.y;

	//	float4 o0, r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10;
	float4 o0_x, o0_y, o0_z, o0_w;
	float4 r0_x, r0_y, r0_z, r0_w;
	float4 r1_x, r1_y, r1_z, r1_w;
	float4 r2_x, r2_y, r2_z, r2_w;
	float4 r3_x, r3_y, r3_z, r3_w;
	float4 r4_x, r4_y, r4_z, r4_w;
	float4 r5_x, r5_y, r5_z, r5_w;
	float4 r6_x, r6_y, r6_z, r6_w;
	float4 r7_x, r7_y, r7_z, r7_w;
	float4 r8_x, r8_y, r8_z, r8_w;
	float4 r9_x, r9_y, r9_z, r9_w;
	float4 r10_x, r10_y, r10_z, r10_w;

	//	int2 loc = (int2)( get_global_id(0), get_global_id(1) );
	int4 tmp_loc_x = (int4)get_global_id(0) * 4;
	int4 loc_y = (int4)get_global_id(1);
	int4 loc_x = tmp_loc_x + (int4)(0, 1, 2, 3);

	// float4 f0 = (float4)( st_origin.x + ((float)loc.x + 0.5f) * st_delta.x + ((float)loc.y + 0.5f) * st_delta.z, st_origin.y + ((float)loc.x + 0.5f) * st_delta.y + ((float)loc.y + 0.5f) * st_delta.w, 0.0f, 0.0f );
	float4 f0_x = (float4)(st_origin.x + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.x + ((float4)((float)loc_y.x, (float)loc_y.y, (float)loc_y.z, (float)loc_y.w) + 0.5f) * st_delta.z);
	float4 f0_y = (float4)(st_origin.y + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.y + ((float4)((float)loc_y.x, (float)loc_y.y, (float)loc_y.z, (float)loc_y.w) + 0.5f) * st_delta.w);
	float4 f0_z = (float4)0.0f;
	float4 f0_w = (float4)0.0f;

	//	r2 = f0;
	r2_x = f0_x;
	r2_y = f0_y;
	r2_z = f0_z;
	r2_w = f0_w;

	//	r10 = r2+p0;
	r10_x = r2_x + p0_x;
	r10_y = r2_y + p0_y;
	r10_z = r2_z + p0_z;
	r10_w = r2_w + p0_w;

	//	r8.x = dot(r10.xy,l0.xy) + l0.w;
	r8_x = (r10_x * (float4)l0.x + r10_y * (float4)l0.y) + l0.w;

	//	r8.y = dot(r10.xy,l1.xy) + l1.w;
	r8_y = (r10_x * (float4)l1.x + r10_y * (float4)l1.y) + l1.w;

	//	r10 = r8;
	r10_x = r8_x;
	r10_y = r8_y;
	r10_z = r8_z;
	r10_w = r8_w;

	//	r8 = read_imagef(t0, t_sampler0, r10.xy);
	float4 r8T0, r8T1, r8T2, r8T3;
	r8T0 = read_imagef(t0, t_sampler0, (float2)(r10_x.x, r10_y.x));
	r8T1 = read_imagef(t0, t_sampler0, (float2)(r10_x.y, r10_y.y));
	r8T2 = read_imagef(t0, t_sampler0, (float2)(r10_x.z, r10_y.z));
	r8T3 = read_imagef(t0, t_sampler0, (float2)(r10_x.w, r10_y.w));
	r8_x = (float4)(r8T0.x, r8T1.x, r8T2.x, r8T3.x);
	r8_y = (float4)(r8T0.y, r8T1.y, r8T2.y, r8T3.y);
	r8_z = (float4)(r8T0.z, r8T1.z, r8T2.z, r8T3.z);
	r8_w = (float4)(r8T0.w, r8T1.w, r8T2.w, r8T3.w);

	//	r10 = r2+p1;
	r10_x = r2_x + p1_x;
	r10_y = r2_y + p1_y;
	r10_z = r2_z + p1_z;
	r10_w = r2_w + p1_w;

	//	r5.x = dot(r10.xy,l0.xy) + l0.w;
	r5_x = (r10_x * (float4)l0.x + r10_y * (float4)l0.y) + l0.w;

	//	r5.y = dot(r10.xy,l1.xy) + l1.w;
	r5_y = (r10_x * (float4)l1.x + r10_y * (float4)l1.y) + l1.w;

	//	r10 = r5;
	r10_x = r5_x;
	r10_y = r5_y;
	r10_z = r5_z;
	r10_w = r5_w;

	//	r5 = read_imagef(t0, t_sampler0, r10.xy);
	float4 r5T0, r5T1, r5T2, r5T3;
	r5T0 = read_imagef(t0, t_sampler0, (float2)(r10_x.x, r10_y.x));
	r5T1 = read_imagef(t0, t_sampler0, (float2)(r10_x.y, r10_y.y));
	r5T2 = read_imagef(t0, t_sampler0, (float2)(r10_x.z, r10_y.z));
	r5T3 = read_imagef(t0, t_sampler0, (float2)(r10_x.w, r10_y.w));
	r5_x = (float4)(r5T0.x, r5T1.x, r5T2.x, r5T3.x);
	r5_y = (float4)(r5T0.y, r5T1.y, r5T2.y, r5T3.y);
	r5_z = (float4)(r5T0.z, r5T1.z, r5T2.z, r5T3.z);
	r5_w = (float4)(r5T0.w, r5T1.w, r5T2.w, r5T3.w);

	//	r10 = r2+p2;
	r10_x = r2_x + p2_x;
	r10_y = r2_y + p2_y;
	r10_z = r2_z + p2_z;
	r10_w = r2_w + p2_w;

	//	r4.x = dot(r10.xy,l0.xy) + l0.w;
	r4_x = (r10_x * (float4)l0.x + r10_y * (float4)l0.y) + l0.w;

	//	r4.y = dot(r10.xy,l1.xy) + l1.w;
	r4_y = (r10_x * (float4)l1.x + r10_y * (float4)l1.y) + l1.w;

	//	r10 = r4;
	r10_x = r4_x;
	r10_y = r4_y;
	r10_z = r4_z;
	r10_w = r4_w;

	// r4 = read_imagef(t0, t_sampler0, r10.xy);
	float4 r4T0, r4T1, r4T2, r4T3;
	r4T0 = read_imagef(t0, t_sampler0, (float2)(r10_x.x, r10_y.x));
	r4T1 = read_imagef(t0, t_sampler0, (float2)(r10_x.y, r10_y.y));
	r4T2 = read_imagef(t0, t_sampler0, (float2)(r10_x.z, r10_y.z));
	r4T3 = read_imagef(t0, t_sampler0, (float2)(r10_x.w, r10_y.w));
	r4_x = (float4)(r4T0.x, r4T1.x, r4T2.x, r4T3.x);
	r4_y = (float4)(r4T0.y, r4T1.y, r4T2.y, r4T3.y);
	r4_z = (float4)(r4T0.z, r4T1.z, r4T2.z, r4T3.z);
	r4_w = (float4)(r4T0.w, r4T1.w, r4T2.w, r4T3.w);

	//	r10 = r2+p3;
	r10_x = r2_x + p3_x;
	r10_y = r2_y + p3_y;
	r10_z = r2_z + p3_z;
	r10_w = r2_w + p3_w;

	//	r6.x = dot(r10.xy,l0.xy) + l0.w;
	r6_x = (r10_x * (float4)l0.x + r10_y * (float4)l0.y) + l0.w;

	//	r6.y = dot(r10.xy,l1.xy) + l1.w;
	r6_y = (r10_x * (float4)l1.x + r10_y * (float4)l1.y) + l1.w;

	//	r10 = r6;
	r10_x = r6_x;
	r10_y = r6_y;
	r10_z = r6_z;
	r10_w = r6_w;

	//	r6 = read_imagef(t0, t_sampler0, r10.xy);
	float4 r6T0, r6T1, r6T2, r6T3;
	r6T0 = read_imagef(t0, t_sampler0, (float2)(r10_x.x, r10_y.x));
	r6T1 = read_imagef(t0, t_sampler0, (float2)(r10_x.y, r10_y.y));
	r6T2 = read_imagef(t0, t_sampler0, (float2)(r10_x.z, r10_y.z));
	r6T3 = read_imagef(t0, t_sampler0, (float2)(r10_x.w, r10_y.w));
	r6_x = (float4)(r6T0.x, r6T1.x, r6T2.x, r6T3.x);
	r6_y = (float4)(r6T0.y, r6T1.y, r6T2.y, r6T3.y);
	r6_z = (float4)(r6T0.z, r6T1.z, r6T2.z, r6T3.z);
	r6_w = (float4)(r6T0.w, r6T1.w, r6T2.w, r6T3.w);

	//	r10 = r2+p4;
	r10_x = r2_x + p4_x;
	r10_y = r2_y + p4_y;
	r10_z = r2_z + p4_z;
	r10_w = r2_w + p4_w;

	//	r7.x = dot(r10.xy,l0.xy) + l0.w;
	r7_x = (r10_x * (float4)l0.x + r10_y * (float4)l0.y) + l0.w;

	//	r7.y = dot(r10.xy,l1.xy) + l1.w;
	r7_y = (r10_x * (float4)l1.x + r10_y * (float4)l1.y) + l1.w;

	//	r10 = r7;
	r10_x = r7_x;
	r10_y = r7_y;
	r10_z = r7_z;
	r10_w = r7_w;

	//	r7 = read_imagef(t0, t_sampler0, r10.xy);
	float4 r7T0, r7T1, r7T2, r7T3;
	r7T0 = read_imagef(t0, t_sampler0, (float2)(r10_x.x, r10_y.x));
	r7T1 = read_imagef(t0, t_sampler0, (float2)(r10_x.y, r10_y.y));
	r7T2 = read_imagef(t0, t_sampler0, (float2)(r10_x.z, r10_y.z));
	r7T3 = read_imagef(t0, t_sampler0, (float2)(r10_x.w, r10_y.w));
	r7_x = (float4)(r7T0.x, r7T1.x, r7T2.x, r7T3.x);
	r7_y = (float4)(r7T0.y, r7T1.y, r7T2.y, r7T3.y);
	r7_z = (float4)(r7T0.z, r7T1.z, r7T2.z, r7T3.z);
	r7_w = (float4)(r7T0.w, r7T1.w, r7T2.w, r7T3.w);

	//	r10 = r2+p5;
	r10_x = r2_x + p5_x;
	r10_y = r2_y + p5_y;
	r10_z = r2_z + p5_z;
	r10_w = r2_w + p5_w;

	//	r3.x = dot(r10.xy,l0.xy) + l0.w;
	r3_x = (r10_x * (float4)l0.x + r10_y * (float4)l0.y) + l0.w;

	//	r3.y = dot(r10.xy,l1.xy) + l1.w;
	r3_y = (r10_x * (float4)l1.x + r10_y * (float4)l1.y) + l1.w;

	//	r10 = r3;
	r10_x = r3_x;
	r10_y = r3_y;
	r10_z = r3_z;
	r10_w = r3_w;

	//	r3 = read_imagef(t0, t_sampler0, r10.xy);
	float4 r3T0, r3T1, r3T2, r3T3;
	r3T0 = read_imagef(t0, t_sampler0, (float2)(r10_x.x, r10_y.x));
	r3T1 = read_imagef(t0, t_sampler0, (float2)(r10_x.y, r10_y.y));
	r3T2 = read_imagef(t0, t_sampler0, (float2)(r10_x.z, r10_y.z));
	r3T3 = read_imagef(t0, t_sampler0, (float2)(r10_x.w, r10_y.w));
	r3_x = (float4)(r3T0.x, r3T1.x, r3T2.x, r3T3.x);
	r3_y = (float4)(r3T0.y, r3T1.y, r3T2.y, r3T3.y);
	r3_z = (float4)(r3T0.z, r3T1.z, r3T2.z, r3T3.z);
	r3_w = (float4)(r3T0.w, r3T1.w, r3T2.w, r3T3.w);

	//	r10 = r2+p6;
	r10_x = r2_x + p6_x;
	r10_y = r2_y + p6_y;
	r10_z = r2_z + p6_z;
	r10_w = r2_w + p6_w;

	//	r1.x = dot(r10.xy,l0.xy) + l0.w;
	r1_x = (r10_x * (float4)l0.x + r10_y * (float4)l0.y) + l0.w;

	//	r1.y = dot(r10.xy,l1.xy) + l1.w;
	r1_y = (r10_x * (float4)l1.x + r10_y * (float4)l1.y) + l1.w;

	//	r10 = r1;
	r10_x = r1_x;
	r10_y = r1_y;
	r10_z = r1_z;
	r10_w = r1_w;

	//	r1 = read_imagef(t0, t_sampler0, r10.xy);
	float4 r1T0, r1T1, r1T2, r1T3;
	r1T0 = read_imagef(t0, t_sampler0, (float2)(r10_x.x, r10_y.x));
	r1T1 = read_imagef(t0, t_sampler0, (float2)(r10_x.y, r10_y.y));
	r1T2 = read_imagef(t0, t_sampler0, (float2)(r10_x.z, r10_y.z));
	r1T3 = read_imagef(t0, t_sampler0, (float2)(r10_x.w, r10_y.w));
	r1_x = (float4)(r1T0.x, r1T1.x, r1T2.x, r1T3.x);
	r1_y = (float4)(r1T0.y, r1T1.y, r1T2.y, r1T3.y);
	r1_z = (float4)(r1T0.z, r1T1.z, r1T2.z, r1T3.z);
	r1_w = (float4)(r1T0.w, r1T1.w, r1T2.w, r1T3.w);

	//	r10 = r2+p7;
	r10_x = r2_x + p7_x;
	r10_y = r2_y + p7_y;
	r10_z = r2_z + p7_z;
	r10_w = r2_w + p7_w;

	//	r0.x = dot(r10.xy,l0.xy) + l0.w;
	r0_x = (r10_x * (float4)l0.x + r10_y * (float4)l0.y) + l0.w;

	//	r0.y = dot(r10.xy,l1.xy) + l1.w;
	r0_y = (r10_x * (float4)l1.x + r10_y * (float4)l1.y) + l1.w;

	//	r0 = read_imagef(t0, t_sampler0, r0.xy);
	float4 r0T0, r0T1, r0T2, r0T3;
	r0T0 = read_imagef(t0, t_sampler0, (float2)(r0_x.x, r0_y.x));
	r0T1 = read_imagef(t0, t_sampler0, (float2)(r0_x.y, r0_y.y));
	r0T2 = read_imagef(t0, t_sampler0, (float2)(r0_x.z, r0_y.z));
	r0T3 = read_imagef(t0, t_sampler0, (float2)(r0_x.w, r0_y.w));
	r0_x = (float4) (r0T0.x, r0T1.x, r0T2.x, r0T3.x);
	r0_y = (float4) (r0T0.y, r0T1.y, r0T2.y, r0T3.y);
	r0_z = (float4) (r0T0.z, r0T1.z, r0T2.z, r0T3.z);
	r0_w = (float4) (r0T0.w, r0T1.w, r0T2.w, r0T3.w);

	//	r9.x = dot(r2.xy,l0.xy) + l0.w;
	r9_x = (r2_x * (float4)l0.x + r2_y * (float4)l0.y) + l0.w;

	//	r9.y = dot(r2.xy,l1.xy) + l1.w;
	r9_y = (r2_x * (float4)l1.x + r2_y * (float4)l1.y) + l1.w;

	//	r2 = r9;
	r2_x = r9_x;
	r2_y = r9_y;
	r2_z = r9_z;
	r2_w = r9_w;

	//	r9 = read_imagef(t0, t_sampler0, r2.xy);
	float4 r9T0, r9T1, r9T2, r9T3;
	r9T0 = read_imagef(t0, t_sampler0, (float2)(r2_x.x, r2_y.x));
	r9T1 = read_imagef(t0, t_sampler0, (float2)(r2_x.y, r2_y.y));
	r9T2 = read_imagef(t0, t_sampler0, (float2)(r2_x.z, r2_y.z));
	r9T3 = read_imagef(t0, t_sampler0, (float2)(r2_x.w, r2_y.w));
	r9_x = (float4)(r9T0.x, r9T1.x, r9T2.x, r9T3.x);
	r9_y = (float4)(r9T0.y, r9T1.y, r9T2.y, r9T3.y);
	r9_z = (float4)(r9T0.z, r9T1.z, r9T2.z, r9T3.z);
	r9_w = (float4)(r9T0.w, r9T1.w, r9T2.w, r9T3.w);

	//	r2 = min(r5, r4);
	r2_x = min(r5_x, r4_x);
	r2_y = min(r5_y, r4_y);
	r2_z = min(r5_z, r4_z);
	r2_w = min(r5_w, r4_w);

	//	r4 = max(r5, r4);
	r4_x = max(r5_x, r4_x);
	r4_y = max(r5_y, r4_y);
	r4_z = max(r5_z, r4_z);
	r4_w = max(r5_w, r4_w);

	//	r5 = min(r7, r3);
	r5_x = min(r7_x, r3_x);
	r5_y = min(r7_y, r3_y);
	r5_z = min(r7_z, r3_z);
	r5_w = min(r7_w, r3_w);

	//	r3 = max(r7, r3);
	r3_x = max(r7_x, r3_x);
	r3_y = max(r7_y, r3_y);
	r3_z = max(r7_z, r3_z);
	r3_w = max(r7_w, r3_w);

	//	r7 = min(r0, r9);
	r7_x = min(r0_x, r9_x);
	r7_y = min(r0_y, r9_y);
	r7_z = min(r0_z, r9_z);
	r7_w = min(r0_w, r9_w);

	//	r9 = max(r0, r9);
	r9_x = max(r0_x, r9_x);
	r9_y = max(r0_y, r9_y);
	r9_z = max(r0_z, r9_z);
	r9_w = max(r0_w, r9_w);

	//	r0 = min(r8, r2);
	r0_x = min(r8_x, r2_x);
	r0_y = min(r8_y, r2_y);
	r0_z = min(r8_z, r2_z);
	r0_w = min(r8_w, r2_w);

	//	r2 = max(r8, r2);
	r2_x = max(r8_x, r2_x);
	r2_y = max(r8_y, r2_y);
	r2_z = max(r8_z, r2_z);
	r2_w = max(r8_w, r2_w);

	//	r8 = min(r6, r5);
	r8_x = min(r6_x, r5_x);
	r8_y = min(r6_y, r5_y);
	r8_z = min(r6_z, r5_z);
	r8_w = min(r6_w, r5_w);

	//	r5 = max(r6, r5);
	r5_x = max(r6_x, r5_x);
	r5_y = max(r6_y, r5_y);
	r5_z = max(r6_z, r5_z);
	r5_w = max(r6_w, r5_w);

	//	r6 = min(r1, r7);
	r6_x = min(r1_x, r7_x);
	r6_y = min(r1_y, r7_y);
	r6_z = min(r1_z, r7_z);
	r6_w = min(r1_w, r7_w);

	//	r7 = max(r1, r7);
	r7_x = max(r1_x, r7_x);
	r7_y = max(r1_y, r7_y);
	r7_z = max(r1_z, r7_z);
	r7_w = max(r1_w, r7_w);

	//	r1 = min(r2, r4);
	r1_x = min(r2_x, r4_x);
	r1_y = min(r2_y, r4_y);
	r1_z = min(r2_z, r4_z);
	r1_w = min(r2_w, r4_w);

	//	r4 = max(r2, r4);
	r4_x = max(r2_x, r4_x);
	r4_y = max(r2_y, r4_y);
	r4_z = max(r2_z, r4_z);
	r4_w = max(r2_w, r4_w);

	//	r2 = min(r5, r3);
	r2_x = min(r5_x, r3_x);
	r2_y = min(r5_y, r3_y);
	r2_z = min(r5_z, r3_z);
	r2_w = min(r5_w, r3_w);

	//	r3 = max(r5, r3);
	r3_x = max(r5_x, r3_x);
	r3_y = max(r5_y, r3_y);
	r3_z = max(r5_z, r3_z);
	r3_w = max(r5_w, r3_w);

	//	r5 = min(r7, r9);
	r5_x = min(r7_x, r9_x);
	r5_y = min(r7_y, r9_y);
	r5_z = min(r7_z, r9_z);
	r5_w = min(r7_w, r9_w);

	//	r7 = max(r7, r9);
	r7_x = max(r7_x, r9_x);
	r7_y = max(r7_y, r9_y);
	r7_z = max(r7_z, r9_z);
	r7_w = max(r7_w, r9_w);

	//	r0 = max(r0, r8);
	r0_x = max(r0_x, r8_x);
	r0_y = max(r0_y, r8_y);
	r0_z = max(r0_z, r8_z);
	r0_w = max(r0_w, r8_w);

	//	r3 = min(r3, r7);
	r3_x = min(r3_x, r7_x);
	r3_y = min(r3_y, r7_y);
	r3_z = min(r3_z, r7_z);
	r3_w = min(r3_w, r7_w);

	//	r0 = max(r0, r6);
	r0_x = max(r0_x, r6_x);
	r0_y = max(r0_y, r6_y);
	r0_z = max(r0_z, r6_z);
	r0_w = max(r0_w, r6_w);

	//	r3 = min(r4, r3);
	r3_x = min(r4_x, r3_x);
	r3_y = min(r4_y, r3_y);
	r3_z = min(r4_z, r3_z);
	r3_w = min(r4_w, r3_w);

	//	r4 = min(r2, r5);
	r4_x = min(r2_x, r5_x);
	r4_y = min(r2_y, r5_y);
	r4_z = min(r2_z, r5_z);
	r4_w = min(r2_w, r5_w);

	//	r2 = max(r2, r5);
	r2_x = max(r2_x, r5_x);
	r2_y = max(r2_y, r5_y);
	r2_z = max(r2_z, r5_z);
	r2_w = max(r2_w, r5_w);

	//	r1 = max(r1, r4);
	r1_x = max(r1_x, r4_x);
	r1_y = max(r1_y, r4_y);
	r1_z = max(r1_z, r4_z);
	r1_w = max(r1_w, r4_w);

	//	r1 = min(r2, r1);
	r1_x = min(r2_x, r1_x);
	r1_y = min(r2_y, r1_y);
	r1_z = min(r2_z, r1_z);
	r1_w = min(r2_w, r1_w);

	//	r2 = min(r1, r3);
	r2_x = min(r1_x, r3_x);
	r2_y = min(r1_y, r3_y);
	r2_z = min(r1_z, r3_z);
	r2_w = min(r1_w, r3_w);

	//	r1 = max(r1, r3);
	r1_x = max(r1_x, r3_x);
	r1_y = max(r1_y, r3_y);
	r1_z = max(r1_z, r3_z);
	r1_w = max(r1_w, r3_w);

	//	r0 = max(r0, r2);
	r0_x = max(r0_x, r2_x);
	r0_y = max(r0_y, r2_y);
	r0_z = max(r0_z, r2_z);
	r0_w = max(r0_w, r2_w);

	//	r0 = min(r0, r1);
	r0_x = min(r0_x, r1_x);
	r0_y = min(r0_y, r1_y);
	r0_z = min(r0_z, r1_z);
	r0_w = min(r0_w, r1_w);

	//	r0.xyz = min(r0.xyz, r0.www);
	r0_x = min(r0_x, r0_w);
	r0_y = min(r0_y, r0_w);
	r0_z = min(r0_z, r0_w);

	//	o0 = r0;
	o0_x = r0_x;
	o0_y = r0_y;
	o0_z = r0_z;
	o0_w = r0_w;

	//	write_imagef(dest, (int2)( loc.x + dim.z , flipped ? get_image_height(dest) - (loc.y + dim.w + 1) : loc.y + dim.w ), o0);
	write_imagef(dest, (int2)( loc_x.x + dim.z , flipped ? get_image_height(dest) - (loc_y.x + dim.w + 1) : loc_y.x + dim.w ), (float4)(o0_x.x, o0_y.x, o0_z.x, o0_w.x));
	write_imagef(dest, (int2)( loc_x.y + dim.z , flipped ? get_image_height(dest) - (loc_y.y + dim.w + 1) : loc_y.y + dim.w ), (float4)(o0_x.y, o0_y.y, o0_z.y, o0_w.y));
	write_imagef(dest, (int2)( loc_x.z + dim.z , flipped ? get_image_height(dest) - (loc_y.z + dim.w + 1) : loc_y.z + dim.w ), (float4)(o0_x.z, o0_y.z, o0_z.z, o0_w.z));
	write_imagef(dest, (int2)( loc_x.w + dim.z , flipped ? get_image_height(dest) - (loc_y.w + dim.w + 1) : loc_y.w + dim.w ), (float4)(o0_x.w, o0_y.w, o0_z.w, o0_w.w));

}

// vectorize program4_1
__kernel void program4_2(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, read_only image2d_t t0, sampler_t t_sampler0)
{

	//	const float4 p0 = (float4)( 0x1p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
	const float4 p0_x = (float4)0x1p+0;
	const float4 p0_y = (float4)0x0p+0;
	const float4 p0_z = (float4)0x0p+0;
	const float4 p0_w = (float4)0x0p+0;

	//	const float4 p1 = (float4)( 0x1p+0, 0x1p+0, 0x0p+0, 0x0p+0 );
	const float4 p1_x = (float4)0x1p+0;
	const float4 p1_y = (float4)0x1p+0;
	const float4 p1_z = (float4)0x0p+0;
	const float4 p1_w = (float4)0x0p+0;

	//	const float4 p2 = (float4)( 0x0p+0, 0x1p+0, 0x0p+0, 0x0p+0 );
	const float4 p2_x = (float4)0x0p+0;
	const float4 p2_y = (float4)0x1p+0;
	const float4 p2_z = (float4)0x0p+0;
	const float4 p2_w = (float4)0x0p+0;

	//	const float4 p3 = (float4)( -0x1p+0, 0x1p+0, 0x0p+0, 0x0p+0 );
	const float4 p3_x = (float4)-0x1p+0;
	const float4 p3_y = (float4)0x1p+0;
	const float4 p3_z = (float4)0x0p+0;
	const float4 p3_w = (float4)0x0p+0;

	//	const float4 p4 = (float4)( -0x1p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
	const float4 p4_x = (float4)-0x1p+0;
	const float4 p4_y = (float4)0x0p+0;
	const float4 p4_z = (float4)0x0p+0;
	const float4 p4_w = (float4)0x0p+0;

	//	const float4 p5 = (float4)( -0x1p+0, -0x1p+0, 0x0p+0, 0x0p+0 );
	const float4 p5_x = (float4)-0x1p+0;
	const float4 p5_y = (float4)-0x1p+0;
	const float4 p5_z = (float4)0x0p+0;
	const float4 p5_w = (float4)0x0p+0;

	//	const float4 p6 = (float4)( 0x0p+0, -0x1p+0, 0x0p+0, 0x0p+0 );
	const float4 p6_x = (float4)0x0p+0;
	const float4 p6_y = (float4)-0x1p+0;
	const float4 p6_z = (float4)0x0p+0;
	const float4 p6_w = (float4)0x0p+0;

	//	const float4 p7 = (float4)( 0x1p+0, -0x1p+0, 0x0p+0, 0x0p+0 );
	const float4 p7_x = (float4)0x1p+0;
	const float4 p7_y = (float4)-0x1p+0;
	const float4 p7_z = (float4)0x0p+0;
	const float4 p7_w = (float4)0x0p+0;

	int4 dest_width = (int4)dim.x;
	int4 dest_height = (int4)dim.y;

	//	float4 o0, r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10;
	float4 o0_x, o0_y, o0_z, o0_w;
	float4 r0_x, r0_y, r0_z, r0_w;
	float4 r1_x, r1_y, r1_z, r1_w;
	float4 r2_x, r2_y, r2_z, r2_w;
	float4 r3_x, r3_y, r3_z, r3_w;
	float4 r4_x, r4_y, r4_z, r4_w;
	float4 r5_x, r5_y, r5_z, r5_w;
	float4 r6_x, r6_y, r6_z, r6_w;
	float4 r7_x, r7_y, r7_z, r7_w;
	float4 r8_x, r8_y, r8_z, r8_w;
	float4 r9_x, r9_y, r9_z, r9_w;
	float4 r10_x, r10_y, r10_z, r10_w;

	//	int2 loc = (int2)( get_global_id(0), get_global_id(1) );
	int4 tmp_loc_x = (int4)get_global_id(0) * 4;
	int4 loc_y = (int4)get_global_id(1);
	int4 loc_x = tmp_loc_x + (int4)(0, 1, 2, 3);

	// float4 f0 = (float4)( st_origin.x + ((float)loc.x + 0.5f) * st_delta.x + ((float)loc.y + 0.5f) * st_delta.z, st_origin.y + ((float)loc.x + 0.5f) * st_delta.y + ((float)loc.y + 0.5f) * st_delta.w, 0.0f, 0.0f );
	float4 f0_x = (float4)(st_origin.x + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.x + ((float4)((float)loc_y.x, (float)loc_y.y, (float)loc_y.z, (float)loc_y.w) + 0.5f) * st_delta.z);
	float4 f0_y = (float4)(st_origin.y + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.y + ((float4)((float)loc_y.x, (float)loc_y.y, (float)loc_y.z, (float)loc_y.w) + 0.5f) * st_delta.w);
	float4 f0_z = (float4)0.0f;
	float4 f0_w = (float4)0.0f;

	//	r2 = f0;
	r2_x = f0_x;
	r2_y = f0_y;
	r2_z = f0_z;
	r2_w = f0_w;

	//	r10 = r2+p0;
	r10_x = r2_x + p0_x;
	r10_y = r2_y + p0_y;
	r10_z = r2_z + p0_z;
	r10_w = r2_w + p0_w;

	//	r8.x = dot(r10.xy,l0.xy) + l0.w;
	r8_x.x = dot((float2)(r10_x.x, r10_y.x),l0.xy) + l0.w;
	r8_x.y = dot((float2)(r10_x.y, r10_y.y),l0.xy) + l0.w;
	r8_x.z = dot((float2)(r10_x.z, r10_y.z),l0.xy) + l0.w;
	r8_x.w = dot((float2)(r10_x.w, r10_y.w),l0.xy) + l0.w;

	//	r8.y = dot(r10.xy,l1.xy) + l1.w;
	r8_y.x = dot((float2)(r10_x.x, r10_y.x),l1.xy) + l1.w;
	r8_y.y = dot((float2)(r10_x.y, r10_y.y),l1.xy) + l1.w;
	r8_y.z = dot((float2)(r10_x.z, r10_y.z),l1.xy) + l1.w;
	r8_y.w = dot((float2)(r10_x.w, r10_y.w),l1.xy) + l1.w;

	//	r10 = r8;
	r10_x = r8_x;
	r10_y = r8_y;
	r10_z = r8_z;
	r10_w = r8_w;

	//	r8 = read_imagef(t0, t_sampler0, r10.xy);
	float4 r8T0, r8T1, r8T2, r8T3;
	r8T0 = read_imagef(t0, t_sampler0, (float2)(r10_x.x, r10_y.x));
	r8T1 = read_imagef(t0, t_sampler0, (float2)(r10_x.y, r10_y.y));
	r8T2 = read_imagef(t0, t_sampler0, (float2)(r10_x.z, r10_y.z));
	r8T3 = read_imagef(t0, t_sampler0, (float2)(r10_x.w, r10_y.w));
	r8_x = (float4)(r8T0.x, r8T1.x, r8T2.x, r8T3.x);
	r8_y = (float4)(r8T0.y, r8T1.y, r8T2.y, r8T3.y);
	r8_z = (float4)(r8T0.z, r8T1.z, r8T2.z, r8T3.z);
	r8_w = (float4)(r8T0.w, r8T1.w, r8T2.w, r8T3.w);

	//	r10 = r2+p1;
	r10_x = r2_x + p1_x;
	r10_y = r2_y + p1_y;
	r10_z = r2_z + p1_z;
	r10_w = r2_w + p1_w;

	//	r5.x = dot(r10.xy,l0.xy) + l0.w;
	r5_x.x = dot((float2)(r10_x.x, r10_y.x),l0.xy) + l0.w;
	r5_x.y = dot((float2)(r10_x.y, r10_y.y),l0.xy) + l0.w;
	r5_x.z = dot((float2)(r10_x.z, r10_y.z),l0.xy) + l0.w;
	r5_x.w = dot((float2)(r10_x.w, r10_y.w),l0.xy) + l0.w;

	//	r5.y = dot(r10.xy,l1.xy) + l1.w;
	r5_y.x = dot((float2)(r10_x.x, r10_y.x),l1.xy) + l1.w;
	r5_y.y = dot((float2)(r10_x.y, r10_y.y),l1.xy) + l1.w;
	r5_y.z = dot((float2)(r10_x.z, r10_y.z),l1.xy) + l1.w;
	r5_y.w = dot((float2)(r10_x.w, r10_y.w),l1.xy) + l1.w;

	//	r10 = r5;
	r10_x = r5_x;
	r10_y = r5_y;
	r10_z = r5_z;
	r10_w = r5_w;

	//	r5 = read_imagef(t0, t_sampler0, r10.xy);
	float4 r5T0, r5T1, r5T2, r5T3;
	r5T0 = read_imagef(t0, t_sampler0, (float2)(r10_x.x, r10_y.x));
	r5T1 = read_imagef(t0, t_sampler0, (float2)(r10_x.y, r10_y.y));
	r5T2 = read_imagef(t0, t_sampler0, (float2)(r10_x.z, r10_y.z));
	r5T3 = read_imagef(t0, t_sampler0, (float2)(r10_x.w, r10_y.w));
	r5_x = (float4)(r5T0.x, r5T1.x, r5T2.x, r5T3.x);
	r5_y = (float4)(r5T0.y, r5T1.y, r5T2.y, r5T3.y);
	r5_z = (float4)(r5T0.z, r5T1.z, r5T2.z, r5T3.z);
	r5_w = (float4)(r5T0.w, r5T1.w, r5T2.w, r5T3.w);

	//	r10 = r2+p2;
	r10_x = r2_x + p2_x;
	r10_y = r2_y + p2_y;
	r10_z = r2_z + p2_z;
	r10_w = r2_w + p2_w;

	//	r4.x = dot(r10.xy,l0.xy) + l0.w;
	r4_x.x = dot((float2)(r10_x.x, r10_y.x),l0.xy) + l0.w;
	r4_x.y = dot((float2)(r10_x.y, r10_y.y),l0.xy) + l0.w;
	r4_x.z = dot((float2)(r10_x.z, r10_y.z),l0.xy) + l0.w;
	r4_x.w = dot((float2)(r10_x.w, r10_y.w),l0.xy) + l0.w;

	//	r4.y = dot(r10.xy,l1.xy) + l1.w;
	r4_y.x = dot((float2)(r10_x.x, r10_y.x),l1.xy) + l1.w;
	r4_y.y = dot((float2)(r10_x.y, r10_y.y),l1.xy) + l1.w;
	r4_y.z = dot((float2)(r10_x.z, r10_y.z),l1.xy) + l1.w;
	r4_y.w = dot((float2)(r10_x.w, r10_y.w),l1.xy) + l1.w;

	//	r10 = r4;
	r10_x = r4_x;
	r10_y = r4_y;
	r10_z = r4_z;
	r10_w = r4_w;

	// r4 = read_imagef(t0, t_sampler0, r10.xy);
	float4 r4T0, r4T1, r4T2, r4T3;
	r4T0 = read_imagef(t0, t_sampler0, (float2)(r10_x.x, r10_y.x));
	r4T1 = read_imagef(t0, t_sampler0, (float2)(r10_x.y, r10_y.y));
	r4T2 = read_imagef(t0, t_sampler0, (float2)(r10_x.z, r10_y.z));
	r4T3 = read_imagef(t0, t_sampler0, (float2)(r10_x.w, r10_y.w));
	r4_x = (float4)(r4T0.x, r4T1.x, r4T2.x, r4T3.x);
	r4_y = (float4)(r4T0.y, r4T1.y, r4T2.y, r4T3.y);
	r4_z = (float4)(r4T0.z, r4T1.z, r4T2.z, r4T3.z);
	r4_w = (float4)(r4T0.w, r4T1.w, r4T2.w, r4T3.w);

	//	r10 = r2+p3;
	r10_x = r2_x + p3_x;
	r10_y = r2_y + p3_y;
	r10_z = r2_z + p3_z;
	r10_w = r2_w + p3_w;

	//	r6.x = dot(r10.xy,l0.xy) + l0.w;
	r6_x.x = dot((float2)(r10_x.x, r10_y.x),l0.xy) + l0.w;
	r6_x.y = dot((float2)(r10_x.y, r10_y.y),l0.xy) + l0.w;
	r6_x.z = dot((float2)(r10_x.z, r10_y.z),l0.xy) + l0.w;
	r6_x.w = dot((float2)(r10_x.w, r10_y.w),l0.xy) + l0.w;

	//	r6.y = dot(r10.xy,l1.xy) + l1.w;
	r6_y.x = dot((float2)(r10_x.x, r10_y.x),l1.xy) + l1.w;
	r6_y.y = dot((float2)(r10_x.y, r10_y.y),l1.xy) + l1.w;
	r6_y.z = dot((float2)(r10_x.z, r10_y.z),l1.xy) + l1.w;
	r6_y.w = dot((float2)(r10_x.w, r10_y.w),l1.xy) + l1.w;

	//	r10 = r6;
	r10_x = r6_x;
	r10_y = r6_y;
	r10_z = r6_z;
	r10_w = r6_w;

	//	r6 = read_imagef(t0, t_sampler0, r10.xy);
	float4 r6T0, r6T1, r6T2, r6T3;
	r6T0 = read_imagef(t0, t_sampler0, (float2)(r10_x.x, r10_y.x));
	r6T1 = read_imagef(t0, t_sampler0, (float2)(r10_x.y, r10_y.y));
	r6T2 = read_imagef(t0, t_sampler0, (float2)(r10_x.z, r10_y.z));
	r6T3 = read_imagef(t0, t_sampler0, (float2)(r10_x.w, r10_y.w));
	r6_x = (float4)(r6T0.x, r6T1.x, r6T2.x, r6T3.x);
	r6_y = (float4)(r6T0.y, r6T1.y, r6T2.y, r6T3.y);
	r6_z = (float4)(r6T0.z, r6T1.z, r6T2.z, r6T3.z);
	r6_w = (float4)(r6T0.w, r6T1.w, r6T2.w, r6T3.w);

	//	r10 = r2+p4;
	r10_x = r2_x + p4_x;
	r10_y = r2_y + p4_y;
	r10_z = r2_z + p4_z;
	r10_w = r2_w + p4_w;

	//	r7.x = dot(r10.xy,l0.xy) + l0.w;
	r7_x.x = dot((float2)(r10_x.x, r10_y.x),l0.xy) + l0.w;
	r7_x.y = dot((float2)(r10_x.y, r10_y.y),l0.xy) + l0.w;
	r7_x.z = dot((float2)(r10_x.z, r10_y.z),l0.xy) + l0.w;
	r7_x.w = dot((float2)(r10_x.w, r10_y.w),l0.xy) + l0.w;

	//	r7.y = dot(r10.xy,l1.xy) + l1.w;
	r7_y.x = dot((float2)(r10_x.x, r10_y.x),l1.xy) + l1.w;
	r7_y.y = dot((float2)(r10_x.y, r10_y.y),l1.xy) + l1.w;
	r7_y.z = dot((float2)(r10_x.z, r10_y.z),l1.xy) + l1.w;
	r7_y.w = dot((float2)(r10_x.w, r10_y.w),l1.xy) + l1.w;

	//	r10 = r7;
	r10_x = r7_x;
	r10_y = r7_y;
	r10_z = r7_z;
	r10_w = r7_w;

	//	r7 = read_imagef(t0, t_sampler0, r10.xy);
	float4 r7T0, r7T1, r7T2, r7T3;
	r7T0 = read_imagef(t0, t_sampler0, (float2)(r10_x.x, r10_y.x));
	r7T1 = read_imagef(t0, t_sampler0, (float2)(r10_x.y, r10_y.y));
	r7T2 = read_imagef(t0, t_sampler0, (float2)(r10_x.z, r10_y.z));
	r7T3 = read_imagef(t0, t_sampler0, (float2)(r10_x.w, r10_y.w));
	r7_x = (float4)(r7T0.x, r7T1.x, r7T2.x, r7T3.x);
	r7_y = (float4)(r7T0.y, r7T1.y, r7T2.y, r7T3.y);
	r7_z = (float4)(r7T0.z, r7T1.z, r7T2.z, r7T3.z);
	r7_w = (float4)(r7T0.w, r7T1.w, r7T2.w, r7T3.w);

	//	r10 = r2+p5;
	r10_x = r2_x + p5_x;
	r10_y = r2_y + p5_y;
	r10_z = r2_z + p5_z;
	r10_w = r2_w + p5_w;

	//	r3.x = dot(r10.xy,l0.xy) + l0.w;
	r3_x.x = dot((float2)(r10_x.x, r10_y.x),l0.xy) + l0.w;
	r3_x.y = dot((float2)(r10_x.y, r10_y.y),l0.xy) + l0.w;
	r3_x.z = dot((float2)(r10_x.z, r10_y.z),l0.xy) + l0.w;
	r3_x.w = dot((float2)(r10_x.w, r10_y.w),l0.xy) + l0.w;

	//	r3.y = dot(r10.xy,l1.xy) + l1.w;
	r3_y.x = dot((float2)(r10_x.x, r10_y.x),l1.xy) + l1.w;
	r3_y.y = dot((float2)(r10_x.y, r10_y.y),l1.xy) + l1.w;
	r3_y.z = dot((float2)(r10_x.z, r10_y.z),l1.xy) + l1.w;
	r3_y.w = dot((float2)(r10_x.w, r10_y.w),l1.xy) + l1.w;

	//	r10 = r3;
	r10_x = r3_x;
	r10_y = r3_y;
	r10_z = r3_z;
	r10_w = r3_w;

	//	r3 = read_imagef(t0, t_sampler0, r10.xy);
	float4 r3T0, r3T1, r3T2, r3T3;
	r3T0 = read_imagef(t0, t_sampler0, (float2)(r10_x.x, r10_y.x));
	r3T1 = read_imagef(t0, t_sampler0, (float2)(r10_x.y, r10_y.y));
	r3T2 = read_imagef(t0, t_sampler0, (float2)(r10_x.z, r10_y.z));
	r3T3 = read_imagef(t0, t_sampler0, (float2)(r10_x.w, r10_y.w));
	r3_x = (float4)(r3T0.x, r3T1.x, r3T2.x, r3T3.x);
	r3_y = (float4)(r3T0.y, r3T1.y, r3T2.y, r3T3.y);
	r3_z = (float4)(r3T0.z, r3T1.z, r3T2.z, r3T3.z);
	r3_w = (float4)(r3T0.w, r3T1.w, r3T2.w, r3T3.w);

	//	r10 = r2+p6;
	r10_x = r2_x + p6_x;
	r10_y = r2_y + p6_y;
	r10_z = r2_z + p6_z;
	r10_w = r2_w + p6_w;

	//	r1.x = dot(r10.xy,l0.xy) + l0.w;
	r1_x.x = dot((float2)(r10_x.x, r10_y.x),l0.xy) + l0.w;
	r1_x.y = dot((float2)(r10_x.y, r10_y.y),l0.xy) + l0.w;
	r1_x.z = dot((float2)(r10_x.z, r10_y.z),l0.xy) + l0.w;
	r1_x.w = dot((float2)(r10_x.w, r10_y.w),l0.xy) + l0.w;

	//	r1.y = dot(r10.xy,l1.xy) + l1.w;
	r1_y.x = dot((float2)(r10_x.x, r10_y.x),l1.xy) + l1.w;
	r1_y.y = dot((float2)(r10_x.y, r10_y.y),l1.xy) + l1.w;
	r1_y.z = dot((float2)(r10_x.z, r10_y.z),l1.xy) + l1.w;
	r1_y.w = dot((float2)(r10_x.w, r10_y.w),l1.xy) + l1.w;

	//	r10 = r1;
	r10_x = r1_x;
	r10_y = r1_y;
	r10_z = r1_z;
	r10_w = r1_w;

	//	r1 = read_imagef(t0, t_sampler0, r10.xy);
	float4 r1T0, r1T1, r1T2, r1T3;
	r1T0 = read_imagef(t0, t_sampler0, (float2)(r10_x.x, r10_y.x));
	r1T1 = read_imagef(t0, t_sampler0, (float2)(r10_x.y, r10_y.y));
	r1T2 = read_imagef(t0, t_sampler0, (float2)(r10_x.z, r10_y.z));
	r1T3 = read_imagef(t0, t_sampler0, (float2)(r10_x.w, r10_y.w));
	r1_x = (float4)(r1T0.x, r1T1.x, r1T2.x, r1T3.x);
	r1_y = (float4)(r1T0.y, r1T1.y, r1T2.y, r1T3.y);
	r1_z = (float4)(r1T0.z, r1T1.z, r1T2.z, r1T3.z);
	r1_w = (float4)(r1T0.w, r1T1.w, r1T2.w, r1T3.w);

	//	r10 = r2+p7;
	r10_x = r2_x + p7_x;
	r10_y = r2_y + p7_y;
	r10_z = r2_z + p7_z;
	r10_w = r2_w + p7_w;

	//	r0.x = dot(r10.xy,l0.xy) + l0.w;
	r0_x.x = dot((float2)(r10_x.x, r10_y.x),l0.xy) + l0.w;
	r0_x.y = dot((float2)(r10_x.y, r10_y.y),l0.xy) + l0.w;
	r0_x.z = dot((float2)(r10_x.z, r10_y.z),l0.xy) + l0.w;
	r0_x.w = dot((float2)(r10_x.w, r10_y.w),l0.xy) + l0.w;

	//	r0.y = dot(r10.xy,l1.xy) + l1.w;
	r0_y.x = dot((float2)(r10_x.x, r10_y.x),l1.xy) + l1.w;
	r0_y.y = dot((float2)(r10_x.y, r10_y.y),l1.xy) + l1.w;
	r0_y.z = dot((float2)(r10_x.z, r10_y.z),l1.xy) + l1.w;
	r0_y.w = dot((float2)(r10_x.w, r10_y.w),l1.xy) + l1.w;

	//	r0 = read_imagef(t0, t_sampler0, r0.xy);
	float4 r0T0, r0T1, r0T2, r0T3;
	r0T0 = read_imagef(t0, t_sampler0, (float2)(r0_x.x, r0_y.x));
	r0T1 = read_imagef(t0, t_sampler0, (float2)(r0_x.y, r0_y.y));
	r0T2 = read_imagef(t0, t_sampler0, (float2)(r0_x.z, r0_y.z));
	r0T3 = read_imagef(t0, t_sampler0, (float2)(r0_x.w, r0_y.w));
	r0_x = (float4) (r0T0.x, r0T1.x, r0T2.x, r0T3.x);
	r0_y = (float4) (r0T0.y, r0T1.y, r0T2.y, r0T3.y);
	r0_z = (float4) (r0T0.z, r0T1.z, r0T2.z, r0T3.z);
	r0_w = (float4) (r0T0.w, r0T1.w, r0T2.w, r0T3.w);

	//	r9.x = dot(r2.xy,l0.xy) + l0.w;
	r9_x.x = dot((float2)(r2_x.x, r2_y.x),l0.xy) + l0.w;
	r9_x.y = dot((float2)(r2_x.y, r2_y.y),l0.xy) + l0.w;
	r9_x.z = dot((float2)(r2_x.z, r2_y.z),l0.xy) + l0.w;
	r9_x.w = dot((float2)(r2_x.w, r2_y.w),l0.xy) + l0.w;

	//	r9.y = dot(r2.xy,l1.xy) + l1.w;
	r9_y.x = dot((float2)(r2_x.x, r2_y.x),l1.xy) + l1.w;
	r9_y.y = dot((float2)(r2_x.y, r2_y.y),l1.xy) + l1.w;
	r9_y.z = dot((float2)(r2_x.z, r2_y.z),l1.xy) + l1.w;
	r9_y.w = dot((float2)(r2_x.w, r2_y.w),l1.xy) + l1.w;

	//	r2 = r9;
	r2_x = r9_x;
	r2_y = r9_y;
	r2_z = r9_z;
	r2_w = r9_w;

	//	r9 = read_imagef(t0, t_sampler0, r2.xy);
	float4 r9T0, r9T1, r9T2, r9T3;
	r9T0 = read_imagef(t0, t_sampler0, (float2)(r2_x.x, r2_y.x));
	r9T1 = read_imagef(t0, t_sampler0, (float2)(r2_x.y, r2_y.y));
	r9T2 = read_imagef(t0, t_sampler0, (float2)(r2_x.z, r2_y.z));
	r9T3 = read_imagef(t0, t_sampler0, (float2)(r2_x.w, r2_y.w));
	r9_x = (float4)(r9T0.x, r9T1.x, r9T2.x, r9T3.x);
	r9_y = (float4)(r9T0.y, r9T1.y, r9T2.y, r9T3.y);
	r9_z = (float4)(r9T0.z, r9T1.z, r9T2.z, r9T3.z);
	r9_w = (float4)(r9T0.w, r9T1.w, r9T2.w, r9T3.w);

	//	r2 = min(r5, r4);
	r2_x = min(r5_x, r4_x);
	r2_y = min(r5_y, r4_y);
	r2_z = min(r5_z, r4_z);
	r2_w = min(r5_w, r4_w);

	//	r4 = max(r5, r4);
	r4_x = max(r5_x, r4_x);
	r4_y = max(r5_y, r4_y);
	r4_z = max(r5_z, r4_z);
	r4_w = max(r5_w, r4_w);

	//	r5 = min(r7, r3);
	r5_x = min(r7_x, r3_x);
	r5_y = min(r7_y, r3_y);
	r5_z = min(r7_z, r3_z);
	r5_w = min(r7_w, r3_w);

	//	r3 = max(r7, r3);
	r3_x = max(r7_x, r3_x);
	r3_y = max(r7_y, r3_y);
	r3_z = max(r7_z, r3_z);
	r3_w = max(r7_w, r3_w);

	//	r7 = min(r0, r9);
	r7_x = min(r0_x, r9_x);
	r7_y = min(r0_y, r9_y);
	r7_z = min(r0_z, r9_z);
	r7_w = min(r0_w, r9_w);

	//	r9 = max(r0, r9);
	r9_x = max(r0_x, r9_x);
	r9_y = max(r0_y, r9_y);
	r9_z = max(r0_z, r9_z);
	r9_w = max(r0_w, r9_w);

	//	r0 = min(r8, r2);
	r0_x = min(r8_x, r2_x);
	r0_y = min(r8_y, r2_y);
	r0_z = min(r8_z, r2_z);
	r0_w = min(r8_w, r2_w);

	//	r2 = max(r8, r2);
	r2_x = max(r8_x, r2_x);
	r2_y = max(r8_y, r2_y);
	r2_z = max(r8_z, r2_z);
	r2_w = max(r8_w, r2_w);

	//	r8 = min(r6, r5);
	r8_x = min(r6_x, r5_x);
	r8_y = min(r6_y, r5_y);
	r8_z = min(r6_z, r5_z);
	r8_w = min(r6_w, r5_w);

	//	r5 = max(r6, r5);
	r5_x = max(r6_x, r5_x);
	r5_y = max(r6_y, r5_y);
	r5_z = max(r6_z, r5_z);
	r5_w = max(r6_w, r5_w);

	//	r6 = min(r1, r7);
	r6_x = min(r1_x, r7_x);
	r6_y = min(r1_y, r7_y);
	r6_z = min(r1_z, r7_z);
	r6_w = min(r1_w, r7_w);

	//	r7 = max(r1, r7);
	r7_x = max(r1_x, r7_x);
	r7_y = max(r1_y, r7_y);
	r7_z = max(r1_z, r7_z);
	r7_w = max(r1_w, r7_w);

	//	r1 = min(r2, r4);
	r1_x = min(r2_x, r4_x);
	r1_y = min(r2_y, r4_y);
	r1_z = min(r2_z, r4_z);
	r1_w = min(r2_w, r4_w);

	//	r4 = max(r2, r4);
	r4_x = max(r2_x, r4_x);
	r4_y = max(r2_y, r4_y);
	r4_z = max(r2_z, r4_z);
	r4_w = max(r2_w, r4_w);

	//	r2 = min(r5, r3);
	r2_x = min(r5_x, r3_x);
	r2_y = min(r5_y, r3_y);
	r2_z = min(r5_z, r3_z);
	r2_w = min(r5_w, r3_w);

	//	r3 = max(r5, r3);
	r3_x = max(r5_x, r3_x);
	r3_y = max(r5_y, r3_y);
	r3_z = max(r5_z, r3_z);
	r3_w = max(r5_w, r3_w);

	//	r5 = min(r7, r9);
	r5_x = min(r7_x, r9_x);
	r5_y = min(r7_y, r9_y);
	r5_z = min(r7_z, r9_z);
	r5_w = min(r7_w, r9_w);

	//	r7 = max(r7, r9);
	r7_x = max(r7_x, r9_x);
	r7_y = max(r7_y, r9_y);
	r7_z = max(r7_z, r9_z);
	r7_w = max(r7_w, r9_w);

	//	r0 = max(r0, r8);
	r0_x = max(r0_x, r8_x);
	r0_y = max(r0_y, r8_y);
	r0_z = max(r0_z, r8_z);
	r0_w = max(r0_w, r8_w);

	//	r3 = min(r3, r7);
	r3_x = min(r3_x, r7_x);
	r3_y = min(r3_y, r7_y);
	r3_z = min(r3_z, r7_z);
	r3_w = min(r3_w, r7_w);

	//	r0 = max(r0, r6);
	r0_x = max(r0_x, r6_x);
	r0_y = max(r0_y, r6_y);
	r0_z = max(r0_z, r6_z);
	r0_w = max(r0_w, r6_w);

	//	r3 = min(r4, r3);
	r3_x = min(r4_x, r3_x);
	r3_y = min(r4_y, r3_y);
	r3_z = min(r4_z, r3_z);
	r3_w = min(r4_w, r3_w);

	//	r4 = min(r2, r5);
	r4_x = min(r2_x, r5_x);
	r4_y = min(r2_y, r5_y);
	r4_z = min(r2_z, r5_z);
	r4_w = min(r2_w, r5_w);

	//	r2 = max(r2, r5);
	r2_x = max(r2_x, r5_x);
	r2_y = max(r2_y, r5_y);
	r2_z = max(r2_z, r5_z);
	r2_w = max(r2_w, r5_w);

	//	r1 = max(r1, r4);
	r1_x = max(r1_x, r4_x);
	r1_y = max(r1_y, r4_y);
	r1_z = max(r1_z, r4_z);
	r1_w = max(r1_w, r4_w);

	//	r1 = min(r2, r1);
	r1_x = min(r2_x, r1_x);
	r1_y = min(r2_y, r1_y);
	r1_z = min(r2_z, r1_z);
	r1_w = min(r2_w, r1_w);

	//	r2 = min(r1, r3);
	r2_x = min(r1_x, r3_x);
	r2_y = min(r1_y, r3_y);
	r2_z = min(r1_z, r3_z);
	r2_w = min(r1_w, r3_w);

	//	r1 = max(r1, r3);
	r1_x = max(r1_x, r3_x);
	r1_y = max(r1_y, r3_y);
	r1_z = max(r1_z, r3_z);
	r1_w = max(r1_w, r3_w);

	//	r0 = max(r0, r2);
	r0_x = max(r0_x, r2_x);
	r0_y = max(r0_y, r2_y);
	r0_z = max(r0_z, r2_z);
	r0_w = max(r0_w, r2_w);

	//	r0 = min(r0, r1);
	r0_x = min(r0_x, r1_x);
	r0_y = min(r0_y, r1_y);
	r0_z = min(r0_z, r1_z);
	r0_w = min(r0_w, r1_w);

	//	r0.xyz = min(r0.xyz, r0.www);
	r0_x = min(r0_x, r0_w);
	r0_y = min(r0_y, r0_w);
	r0_z = min(r0_z, r0_w);

	//	o0 = r0;
	o0_x = r0_x;
	o0_y = r0_y;
	o0_z = r0_z;
	o0_w = r0_w;

	//	write_imagef(dest, (int2)( loc.x + dim.z , flipped ? get_image_height(dest) - (loc.y + dim.w + 1) : loc.y + dim.w ), o0);
	write_imagef(dest, (int2)( loc_x.x + dim.z , flipped ? get_image_height(dest) - (loc_y.x + dim.w + 1) : loc_y.x + dim.w ), (float4)(o0_x.x, o0_y.x, o0_z.x, o0_w.x));
	write_imagef(dest, (int2)( loc_x.y + dim.z , flipped ? get_image_height(dest) - (loc_y.y + dim.w + 1) : loc_y.y + dim.w ), (float4)(o0_x.y, o0_y.y, o0_z.y, o0_w.y));
	write_imagef(dest, (int2)( loc_x.z + dim.z , flipped ? get_image_height(dest) - (loc_y.z + dim.w + 1) : loc_y.z + dim.w ), (float4)(o0_x.z, o0_y.z, o0_z.z, o0_w.z));
	write_imagef(dest, (int2)( loc_x.w + dim.z , flipped ? get_image_height(dest) - (loc_y.w + dim.w + 1) : loc_y.w + dim.w ), (float4)(o0_x.w, o0_y.w, o0_z.w, o0_w.w));

}

// scalarize program
__kernel void program4_1(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, read_only image2d_t t0, sampler_t t_sampler0)
{

	//	const float4 p0 = (float4)( 0x1p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
	const float p0_x = 0x1p+0;
	const float p0_y = 0x0p+0;
	const float p0_z = 0x0p+0;
	const float p0_w = 0x0p+0;

	//	const float4 p1 = (float4)( 0x1p+0, 0x1p+0, 0x0p+0, 0x0p+0 );
	const float p1_x = 0x1p+0;
	const float p1_y = 0x1p+0;
	const float p1_z = 0x0p+0;
	const float p1_w = 0x0p+0;

	//	const float4 p2 = (float4)( 0x0p+0, 0x1p+0, 0x0p+0, 0x0p+0 );
	const float p2_x = 0x0p+0;
	const float p2_y = 0x1p+0;
	const float p2_z = 0x0p+0;
	const float p2_w = 0x0p+0;

	//	const float4 p3 = (float4)( -0x1p+0, 0x1p+0, 0x0p+0, 0x0p+0 );
	const float p3_x = -0x1p+0;
	const float p3_y = 0x1p+0;
	const float p3_z = 0x0p+0;
	const float p3_w = 0x0p+0;

	//	const float4 p4 = (float4)( -0x1p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
	const float p4_x = -0x1p+0;
	const float p4_y = 0x0p+0;
	const float p4_z = 0x0p+0;
	const float p4_w = 0x0p+0;

	//	const float4 p5 = (float4)( -0x1p+0, -0x1p+0, 0x0p+0, 0x0p+0 );
	const float p5_x = -0x1p+0;
	const float p5_y = -0x1p+0;
	const float p5_z = 0x0p+0;
	const float p5_w = 0x0p+0;

	//	const float4 p6 = (float4)( 0x0p+0, -0x1p+0, 0x0p+0, 0x0p+0 );
	const float p6_x = 0x0p+0;
	const float p6_y = -0x1p+0;
	const float p6_z = 0x0p+0;
	const float p6_w = 0x0p+0;

	//	const float4 p7 = (float4)( 0x1p+0, -0x1p+0, 0x0p+0, 0x0p+0 );
	const float p7_x = 0x1p+0;
	const float p7_y = -0x1p+0;
	const float p7_z = 0x0p+0;
	const float p7_w = 0x0p+0;

	//	float4 o0, r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10;
	float o0_x, o0_y, o0_z, o0_w;
	float r0_x, r0_y, r0_z, r0_w;
	float r1_x, r1_y, r1_z, r1_w;
	float r2_x, r2_y, r2_z, r2_w;
	float r3_x, r3_y, r3_z, r3_w;
	float r4_x, r4_y, r4_z, r4_w;
	float r5_x, r5_y, r5_z, r5_w;
	float r6_x, r6_y, r6_z, r6_w;
	float r7_x, r7_y, r7_z, r7_w;
	float r8_x, r8_y, r8_z, r8_w;
	float r9_x, r9_y, r9_z, r9_w;
	float r10_x, r10_y, r10_z, r10_w;

	//	int2 loc = (int2)( get_global_id(0), get_global_id(1) );
	int loc_x = get_global_id(0);
	int loc_y = get_global_id(1);

	// float4 f0 = (float4)( st_origin.x + ((float)loc.x + 0.5f) * st_delta.x + ((float)loc.y + 0.5f) * st_delta.z, st_origin.y + ((float)loc.x + 0.5f) * st_delta.y + ((float)loc.y + 0.5f) * st_delta.w, 0.0f, 0.0f );
	float f0_x = st_origin.x + ((float)loc_x + 0.5f) * st_delta.x + ((float)loc_y + 0.5f) * st_delta.z;
	float f0_y = st_origin.y + ((float)loc_x + 0.5f) * st_delta.y + ((float)loc_y + 0.5f) * st_delta.w;
	float f0_z = 0.0f;
	float f0_w = 0.0f;

	//	r2 = f0;
	r2_x = f0_x;
	r2_y = f0_y;
	r2_z = f0_z;
	r2_w = f0_w;

	//	r10 = r2+p0;
	r10_x = r2_x + p0_x;
	r10_y = r2_y + p0_y;
	r10_z = r2_z + p0_z;
	r10_w = r2_w + p0_w;

	//	r8.x = dot(r10.xy,l0.xy) + l0.w;
	r8_x = dot((float2)(r10_x, r10_y),l0.xy) + l0.w;

	//	r8.y = dot(r10.xy,l1.xy) + l1.w;
	r8_y = dot((float2)(r10_x, r10_y),l1.xy) + l1.w;

	//	r10 = r8;
	r10_x = r8_x;
	r10_y = r8_y;
	r10_z = r8_z;
	r10_w = r8_w;

	//	r8 = read_imagef(t0, t_sampler0, r10.xy);
	float4 r8T;
	r8T = read_imagef(t0, t_sampler0, (float2)(r10_x, r10_y));
	r8_x = r8T.x;
	r8_y = r8T.y;
	r8_z = r8T.z;
	r8_w = r8T.w;

	//	r10 = r2+p1;
	r10_x = r2_x + p1_x;
	r10_y = r2_y + p1_y;
	r10_z = r2_z + p1_z;
	r10_w = r2_w + p1_w;

	//	r5.x = dot(r10.xy,l0.xy) + l0.w;
	r5_x = dot((float2)(r10_x, r10_y),l0.xy) + l0.w;

	//	r5.y = dot(r10.xy,l1.xy) + l1.w;
	r5_y = dot((float2)(r10_x, r10_y),l1.xy) + l1.w;

	//	r10 = r5;
	r10_x = r5_x;
	r10_y = r5_y;
	r10_z = r5_z;
	r10_w = r5_w;

	//	r5 = read_imagef(t0, t_sampler0, r10.xy);
	float4 r5T;
	r5T = read_imagef(t0, t_sampler0, (float2)(r10_x, r10_y));
	r5_x = r5T.x;
	r5_y = r5T.y;
	r5_z = r5T.z;
	r5_w = r5T.w;

	//	r10 = r2+p2;
	r10_x = r2_x + p2_x;
	r10_y = r2_y + p2_y;
	r10_z = r2_z + p2_z;
	r10_w = r2_w + p2_w;

	//	r4.x = dot(r10.xy,l0.xy) + l0.w;
	r4_x = dot((float2)(r10_x, r10_y),l0.xy) + l0.w;

	//	r4.y = dot(r10.xy,l1.xy) + l1.w;
	r4_y = dot((float2)(r10_x, r10_y),l1.xy) + l1.w;

	//	r10 = r4;
	r10_x = r4_x;
	r10_y = r4_y;
	r10_z = r4_z;
	r10_w = r4_w;

	// r4 = read_imagef(t0, t_sampler0, r10.xy);
	float4 r4T;
	r4T = read_imagef(t0, t_sampler0, (float2)(r10_x, r10_y));
	r4_x = r4T.x;
	r4_y = r4T.y;
	r4_z = r4T.z;
	r4_w = r4T.w;

	//	r10 = r2+p3;
	r10_x = r2_x + p3_x;
	r10_y = r2_y + p3_y;
	r10_z = r2_z + p3_z;
	r10_w = r2_w + p3_w;

	//	r6.x = dot(r10.xy,l0.xy) + l0.w;
	r6_x = dot((float2)(r10_x, r10_y),l0.xy) + l0.w;

	//	r6.y = dot(r10.xy,l1.xy) + l1.w;
	r6_y = dot((float2)(r10_x, r10_y),l1.xy) + l1.w;

	//	r10 = r6;
	r10_x = r6_x;
	r10_y = r6_y;
	r10_z = r6_z;
	r10_w = r6_w;

	//	r6 = read_imagef(t0, t_sampler0, r10.xy);
	float4 r6T;
	r6T = read_imagef(t0, t_sampler0, (float2)(r10_x, r10_y));
	r6_x = r6T.x;
	r6_y = r6T.y;
	r6_z = r6T.z;
	r6_w = r6T.w;

	//	r10 = r2+p4;
	r10_x = r2_x + p4_x;
	r10_y = r2_y + p4_y;
	r10_z = r2_z + p4_z;
	r10_w = r2_w + p4_w;

	//	r7.x = dot(r10.xy,l0.xy) + l0.w;
	r7_x = dot((float2)(r10_x, r10_y),l0.xy) + l0.w;

	//	r7.y = dot(r10.xy,l1.xy) + l1.w;
	r7_y = dot((float2)(r10_x, r10_y),l1.xy) + l1.w;

	//	r10 = r7;
	r10_x = r7_x;
	r10_y = r7_y;
	r10_z = r7_z;
	r10_w = r7_w;

	//	r7 = read_imagef(t0, t_sampler0, r10.xy);
	float4 r7T;
	r7T = read_imagef(t0, t_sampler0, (float2)(r10_x, r10_y));
	r7_x = r7T.x;
	r7_y = r7T.y;
	r7_z = r7T.z;
	r7_w = r7T.w;

	//	r10 = r2+p5;
	r10_x = r2_x + p5_x;
	r10_y = r2_y + p5_y;
	r10_z = r2_z + p5_z;
	r10_w = r2_w + p5_w;

	//	r3.x = dot(r10.xy,l0.xy) + l0.w;
	r3_x = dot((float2)(r10_x, r10_y),l0.xy) + l0.w;

	//	r3.y = dot(r10.xy,l1.xy) + l1.w;
	r3_y = dot((float2)(r10_x, r10_y),l1.xy) + l1.w;

	//	r10 = r3;
	r10_x = r3_x;
	r10_y = r3_y;
	r10_z = r3_z;
	r10_w = r3_w;

	//	r3 = read_imagef(t0, t_sampler0, r10.xy);
	float4 r3T;
	r3T = read_imagef(t0, t_sampler0, (float2)(r10_x, r10_y));
	r3_x = r3T.x;
	r3_y = r3T.y;
	r3_z = r3T.z;
	r3_w = r3T.w;

	//	r10 = r2+p6;
	r10_x = r2_x + p6_x;
	r10_y = r2_y + p6_y;
	r10_z = r2_z + p6_z;
	r10_w = r2_w + p6_w;

	//	r1.x = dot(r10.xy,l0.xy) + l0.w;
	r1_x = dot((float2)(r10_x, r10_y),l0.xy) + l0.w;

	//	r1.y = dot(r10.xy,l1.xy) + l1.w;
	r1_y = dot((float2)(r10_x, r10_y),l1.xy) + l1.w;

	//	r10 = r1;
	r10_x = r1_x;
	r10_y = r1_y;
	r10_z = r1_z;
	r10_w = r1_w;

	//	r1 = read_imagef(t0, t_sampler0, r10.xy);
	float4 r1T;
	r1T = read_imagef(t0, t_sampler0, (float2)(r10_x, r10_y));
	r1_x = r1T.x;
	r1_y = r1T.y;
	r1_z = r1T.z;
	r1_w = r1T.w;

	//	r10 = r2+p7;
	r10_x = r2_x + p7_x;
	r10_y = r2_y + p7_y;
	r10_z = r2_z + p7_z;
	r10_w = r2_w + p7_w;

	//	r0.x = dot(r10.xy,l0.xy) + l0.w;
	r0_x = dot((float2)(r10_x, r10_y),l0.xy) + l0.w;

	//	r0.y = dot(r10.xy,l1.xy) + l1.w;
	r0_y = dot((float2)(r10_x, r10_y),l1.xy) + l1.w;

	//	r0 = read_imagef(t0, t_sampler0, r0.xy);
	float4 r0T;
	r0T = read_imagef(t0, t_sampler0, (float2)(r0_x, r0_y));
	r0_x = r0T.x;
	r0_y = r0T.y;
	r0_z = r0T.z;
	r0_w = r0T.w;

	//	r9.x = dot(r2.xy,l0.xy) + l0.w;
	r9_x = dot((float2)(r2_x, r2_y),l0.xy) + l0.w;

	//	r9.y = dot(r2.xy,l1.xy) + l1.w;
	r9_y = dot((float2)(r2_x, r2_y),l1.xy) + l1.w;

	//	r2 = r9;
	r2_x = r9_x;
	r2_y = r9_y;
	r2_z = r9_z;
	r2_w = r9_w;

	//	r9 = read_imagef(t0, t_sampler0, r2.xy);
	float4 r9T;
	r9T = read_imagef(t0, t_sampler0, (float2)(r2_x, r2_y));
	r9_x = r9T.x;
	r9_y = r9T.y;
	r9_z = r9T.z;
	r9_w = r9T.w;

	//	r2 = min(r5, r4);
	r2_x = min(r5_x, r4_x);
	r2_y = min(r5_y, r4_y);
	r2_z = min(r5_z, r4_z);
	r2_w = min(r5_w, r4_w);

	//	r4 = max(r5, r4);
	r4_x = max(r5_x, r4_x);
	r4_y = max(r5_y, r4_y);
	r4_z = max(r5_z, r4_z);
	r4_w = max(r5_w, r4_w);

	//	r5 = min(r7, r3);
	r5_x = min(r7_x, r3_x);
	r5_y = min(r7_y, r3_y);
	r5_z = min(r7_z, r3_z);
	r5_w = min(r7_w, r3_w);

	//	r3 = max(r7, r3);
	r3_x = max(r7_x, r3_x);
	r3_y = max(r7_y, r3_y);
	r3_z = max(r7_z, r3_z);
	r3_w = max(r7_w, r3_w);

	//	r7 = min(r0, r9);
	r7_x = min(r0_x, r9_x);
	r7_y = min(r0_y, r9_y);
	r7_z = min(r0_z, r9_z);
	r7_w = min(r0_w, r9_w);

	//	r9 = max(r0, r9);
	r9_x = max(r0_x, r9_x);
	r9_y = max(r0_y, r9_y);
	r9_z = max(r0_z, r9_z);
	r9_w = max(r0_w, r9_w);

	//	r0 = min(r8, r2);
	r0_x = min(r8_x, r2_x);
	r0_y = min(r8_y, r2_y);
	r0_z = min(r8_z, r2_z);
	r0_w = min(r8_w, r2_w);

	//	r2 = max(r8, r2);
	r2_x = max(r8_x, r2_x);
	r2_y = max(r8_y, r2_y);
	r2_z = max(r8_z, r2_z);
	r2_w = max(r8_w, r2_w);

	//	r8 = min(r6, r5);
	r8_x = min(r6_x, r5_x);
	r8_y = min(r6_y, r5_y);
	r8_z = min(r6_z, r5_z);
	r8_w = min(r6_w, r5_w);

	//	r5 = max(r6, r5);
	r5_x = max(r6_x, r5_x);
	r5_y = max(r6_y, r5_y);
	r5_z = max(r6_z, r5_z);
	r5_w = max(r6_w, r5_w);

	//	r6 = min(r1, r7);
	r6_x = min(r1_x, r7_x);
	r6_y = min(r1_y, r7_y);
	r6_z = min(r1_z, r7_z);
	r6_w = min(r1_w, r7_w);

	//	r7 = max(r1, r7);
	r7_x = max(r1_x, r7_x);
	r7_y = max(r1_y, r7_y);
	r7_z = max(r1_z, r7_z);
	r7_w = max(r1_w, r7_w);

	//	r1 = min(r2, r4);
	r1_x = min(r2_x, r4_x);
	r1_y = min(r2_y, r4_y);
	r1_z = min(r2_z, r4_z);
	r1_w = min(r2_w, r4_w);

	//	r4 = max(r2, r4);
	r4_x = max(r2_x, r4_x);
	r4_y = max(r2_y, r4_y);
	r4_z = max(r2_z, r4_z);
	r4_w = max(r2_w, r4_w);

	//	r2 = min(r5, r3);
	r2_x = min(r5_x, r3_x);
	r2_y = min(r5_y, r3_y);
	r2_z = min(r5_z, r3_z);
	r2_w = min(r5_w, r3_w);

	//	r3 = max(r5, r3);
	r3_x = max(r5_x, r3_x);
	r3_y = max(r5_y, r3_y);
	r3_z = max(r5_z, r3_z);
	r3_w = max(r5_w, r3_w);

	//	r5 = min(r7, r9);
	r5_x = min(r7_x, r9_x);
	r5_y = min(r7_y, r9_y);
	r5_z = min(r7_z, r9_z);
	r5_w = min(r7_w, r9_w);

	//	r7 = max(r7, r9);
	r7_x = max(r7_x, r9_x);
	r7_y = max(r7_y, r9_y);
	r7_z = max(r7_z, r9_z);
	r7_w = max(r7_w, r9_w);

	//	r0 = max(r0, r8);
	r0_x = max(r0_x, r8_x);
	r0_y = max(r0_y, r8_y);
	r0_z = max(r0_z, r8_z);
	r0_w = max(r0_w, r8_w);

	//	r3 = min(r3, r7);
	r3_x = min(r3_x, r7_x);
	r3_y = min(r3_y, r7_y);
	r3_z = min(r3_z, r7_z);
	r3_w = min(r3_w, r7_w);

	//	r0 = max(r0, r6);
	r0_x = max(r0_x, r6_x);
	r0_y = max(r0_y, r6_y);
	r0_z = max(r0_z, r6_z);
	r0_w = max(r0_w, r6_w);

	//	r3 = min(r4, r3);
	r3_x = min(r4_x, r3_x);
	r3_y = min(r4_y, r3_y);
	r3_z = min(r4_z, r3_z);
	r3_w = min(r4_w, r3_w);

	//	r4 = min(r2, r5);
	r4_x = min(r2_x, r5_x);
	r4_y = min(r2_y, r5_y);
	r4_z = min(r2_z, r5_z);
	r4_w = min(r2_w, r5_w);

	//	r2 = max(r2, r5);
	r2_x = max(r2_x, r5_x);
	r2_y = max(r2_y, r5_y);
	r2_z = max(r2_z, r5_z);
	r2_w = max(r2_w, r5_w);

	//	r1 = max(r1, r4);
	r1_x = max(r1_x, r4_x);
	r1_y = max(r1_y, r4_y);
	r1_z = max(r1_z, r4_z);
	r1_w = max(r1_w, r4_w);

	//	r1 = min(r2, r1);
	r1_x = min(r2_x, r1_x);
	r1_y = min(r2_y, r1_y);
	r1_z = min(r2_z, r1_z);
	r1_w = min(r2_w, r1_w);

	//	r2 = min(r1, r3);
	r2_x = min(r1_x, r3_x);
	r2_y = min(r1_y, r3_y);
	r2_z = min(r1_z, r3_z);
	r2_w = min(r1_w, r3_w);

	//	r1 = max(r1, r3);
	r1_x = max(r1_x, r3_x);
	r1_y = max(r1_y, r3_y);
	r1_z = max(r1_z, r3_z);
	r1_w = max(r1_w, r3_w);

	//	r0 = max(r0, r2);
	r0_x = max(r0_x, r2_x);
	r0_y = max(r0_y, r2_y);
	r0_z = max(r0_z, r2_z);
	r0_w = max(r0_w, r2_w);

	//	r0 = min(r0, r1);
	r0_x = min(r0_x, r1_x);
	r0_y = min(r0_y, r1_y);
	r0_z = min(r0_z, r1_z);
	r0_w = min(r0_w, r1_w);

	//	r0.xyz = min(r0.xyz, r0.www);
	r0_x = min(r0_x, r0_w);
	r0_y = min(r0_y, r0_w);
	r0_z = min(r0_z, r0_w);

	//	o0 = r0;
	o0_x = r0_x;
	o0_y = r0_y;
	o0_z = r0_z;
	o0_w = r0_w;

	//	write_imagef(dest, (int2)( loc.x + dim.z , flipped ? get_image_height(dest) - (loc.y + dim.w + 1) : loc.y + dim.w ), o0);
	write_imagef(dest, (int2)( loc_x + dim.z , flipped ? get_image_height(dest) - (loc_y + dim.w + 1) : loc_y + dim.w ), (float4)(o0_x, o0_y, o0_z, o0_w));

}

// =================================================================================================================
//                                                     original
// =================================================================================================================

__kernel void program(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, read_only image2d_t t0, sampler_t t_sampler0)
{
	const float4 p0 = (float4)( 0x1p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
	const float4 p1 = (float4)( 0x1p+0, 0x1p+0, 0x0p+0, 0x0p+0 );
	const float4 p2 = (float4)( 0x0p+0, 0x1p+0, 0x0p+0, 0x0p+0 );
	const float4 p3 = (float4)( -0x1p+0, 0x1p+0, 0x0p+0, 0x0p+0 );
	const float4 p4 = (float4)( -0x1p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
	const float4 p5 = (float4)( -0x1p+0, -0x1p+0, 0x0p+0, 0x0p+0 );
	const float4 p6 = (float4)( 0x0p+0, -0x1p+0, 0x0p+0, 0x0p+0 );
	const float4 p7 = (float4)( 0x1p+0, -0x1p+0, 0x0p+0, 0x0p+0 );
	int dest_width = dim.x;
	int dest_height = dim.y;
	float4 o0, r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10;
	float4 false_vector = (float4) 0.0f;
	float4 true_vector = (float4) 1.0f;
	float unused_float1;
	float2 unused_float2;
	__float3_SPI unused_float3;
	float4 unused_float4;
	int2 loc = (int2)( get_global_id(0), get_global_id(1) );
	float4 f0 = (float4)( st_origin.x + ((float)loc.x + 0.5f) * st_delta.x + ((float)loc.y + 0.5f) * st_delta.z, st_origin.y + ((float)loc.x + 0.5f) * st_delta.y + ((float)loc.y + 0.5f) * st_delta.w, 0.0f, 0.0f );
	r2 = f0;
	r10 = r2+p0;
	r8.x = dot(r10.xy,l0.xy) + l0.w;
	r8.y = dot(r10.xy,l1.xy) + l1.w;
	r10 = r8;
	r8 = read_imagef(t0, t_sampler0, r10.xy);
	r10 = r2+p1;
	r5.x = dot(r10.xy,l0.xy) + l0.w;
	r5.y = dot(r10.xy,l1.xy) + l1.w;
	r10 = r5;
	r5 = read_imagef(t0, t_sampler0, r10.xy);
	r10 = r2+p2;
	r4.x = dot(r10.xy,l0.xy) + l0.w;
	r4.y = dot(r10.xy,l1.xy) + l1.w;
	r10 = r4;
	r4 = read_imagef(t0, t_sampler0, r10.xy);
	r10 = r2+p3;
	r6.x = dot(r10.xy,l0.xy) + l0.w;
	r6.y = dot(r10.xy,l1.xy) + l1.w;
	r10 = r6;
	r6 = read_imagef(t0, t_sampler0, r10.xy);
	r10 = r2+p4;
	r7.x = dot(r10.xy,l0.xy) + l0.w;
	r7.y = dot(r10.xy,l1.xy) + l1.w;
	r10 = r7;
	r7 = read_imagef(t0, t_sampler0, r10.xy);
	r10 = r2+p5;
	r3.x = dot(r10.xy,l0.xy) + l0.w;
	r3.y = dot(r10.xy,l1.xy) + l1.w;
	r10 = r3;
	r3 = read_imagef(t0, t_sampler0, r10.xy);
	r10 = r2+p6;
	r1.x = dot(r10.xy,l0.xy) + l0.w;
	r1.y = dot(r10.xy,l1.xy) + l1.w;
	r10 = r1;
	r1 = read_imagef(t0, t_sampler0, r10.xy);
	r10 = r2+p7;
	r0.x = dot(r10.xy,l0.xy) + l0.w;
	r0.y = dot(r10.xy,l1.xy) + l1.w;
	r0 = read_imagef(t0, t_sampler0, r0.xy);
	r9.x = dot(r2.xy,l0.xy) + l0.w;
	r9.y = dot(r2.xy,l1.xy) + l1.w;
	r2 = r9;
	r9 = read_imagef(t0, t_sampler0, r2.xy);
	r2 = min(r5, r4);
	r4 = max(r5, r4);
	r5 = min(r7, r3);
	r3 = max(r7, r3);
	r7 = min(r0, r9);
	r9 = max(r0, r9);
	r0 = min(r8, r2);
	r2 = max(r8, r2);
	r8 = min(r6, r5);
	r5 = max(r6, r5);
	r6 = min(r1, r7);
	r7 = max(r1, r7);
	r1 = min(r2, r4);
	r4 = max(r2, r4);
	r2 = min(r5, r3);
	r3 = max(r5, r3);
	r5 = min(r7, r9);
	r7 = max(r7, r9);
	r0 = max(r0, r8);
	r3 = min(r3, r7);
	r0 = max(r0, r6);
	r3 = min(r4, r3);
	r4 = min(r2, r5);
	r2 = max(r2, r5);
	r1 = max(r1, r4);
	r1 = min(r2, r1);
	r2 = min(r1, r3);
	r1 = max(r1, r3);
	r0 = max(r0, r2);
	r0 = min(r0, r1);
	r0.xyz = min(r0.xyz, r0.www);
	o0 = r0;
	write_imagef(dest, (int2)( loc.x + dim.z , flipped ? get_image_height(dest) - (loc.y + dim.w + 1) : loc.y + dim.w ), o0);
}

__kernel void program_trans(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, read_only image2d_t t0, sampler_t t_sampler0)
{
	float4 f0_start, f0_end, f0_delta, tf0_start[4], tf0_delta[4];
	float4 f1_start, f1_end, f1_delta, tf1_start[4], tf1_delta[4];
	float4 f2_start, f2_end, f2_delta, tf2_start[4], tf2_delta[4];
	float4 f3_start, f3_end, f3_delta, tf3_start[4], tf3_delta[4];
	float4 f4_start, f4_end, f4_delta, tf4_start[4], tf4_delta[4];
	float4 f5_start, f5_end, f5_delta, tf5_start[4], tf5_delta[4];
	float4 f6_start, f6_end, f6_delta, tf6_start[4], tf6_delta[4];
	float4 f7_start, f7_end, f7_delta, tf7_start[4], tf7_delta[4];
	float4 f8_start, f8_end, f8_delta, tf8_start[4], tf8_delta[4];
	float4 f9_start, f9_end, f9_delta, tf9_start[4], tf9_delta[4];
	float4 gr0_3_0[64], gr0_3_1[64], gr0_3_2[64], gr0_3_3[64];
	float4 gr0_1_0[64], gr0_1_1[64], gr0_1_2[64], gr0_1_3[64];
	float4 gr0_5_0[64], gr0_5_1[64], gr0_5_2[64], gr0_5_3[64];
	float4 gr0_7_0[64], gr0_7_1[64], gr0_7_2[64], gr0_7_3[64];
	float4 gr0_8_0[64], gr0_8_1[64], gr0_8_2[64], gr0_8_3[64];
	float4 gr0_4_0[64], gr0_4_1[64], gr0_4_2[64], gr0_4_3[64];
	float4 gr0_6_0[64], gr0_6_1[64], gr0_6_2[64], gr0_6_3[64];
	float4 gr0_9_0[64], gr0_9_1[64], gr0_9_2[64], gr0_9_3[64];
	float4 gr0_2_0[64], gr0_2_1[64], gr0_2_2[64], gr0_2_3[64];
	int index = 0;
	int total_index = 0;
	int write_amount = 0;
	int read_amount = 256;
	int write_offset = 0;
	float4 o_r[64], o_g[64], o_b[64], o_a[64];
	int dest_width = dim.x;
	int dest_height = dim.y;
	float4 o0, r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, r16, r17, r18, r19, r20, r21, r22, r23, r24, r25, r26, r27, r28, r29, r30, r31, r32, r33, r34, r35, r36, r37, r38, r39, r40, r41, r42, r43, r44, r45, r46, r47, r48, r49, r50, r51, r52, r53, r54, r55, r56, r57, r58, r59, r60, r61, r62, r63, r64, r65, r66, r67, r68, r69, r70, r71, r72, r73, r74, r75, r76, r77, r78, r79, r80, r81, r82, r83, r84, r85, r86, r87, r88, r89, r90, r91, r92, r93, r94, r95, r96, r97, r98, r99, r100, r101, r102, r103, r104, r105, r106, r107, r108, r109, r110, r111, r112, r113, r114, r115, r116, r117, r118, r119, r120, r121, r122, r123, r124, r125, r126, r127, r128, r129, r130, r131, r132, r133, r134, r135, r136, r137, r138, r139, r140, r141, r142, r143, r144, r145, r146, r147, r148, r149, r150, r151, r152, r153, r154, r155, r156, r157, r158, r159, r160, r161, r162, r163, r164, r165, r166, r167, r168, r169, r170, r171, r172, r173, r174, r175, r176, r177, r178, r179, r180, r181, r182, r183, r184, r185, r186, r187, r188, r189, r190, r191, r192, r193, r194, r195, r196, r197, r198, r199, r200, r201, r202, r203, r204, r205, r206, r207, r208, r209, r210, r211, r212, r213, r214, r215, r216, r217, r218, r219, r220, r221, r222, r223, r224, r225, r226, r227, r228, r229, r230, r231, r232, r233, r234, r235, r236, r237, r238, r239, r240, r241;
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
	r2 = (float4) 0.0f;
	r3 = (float4) 0.0f;
	r4 = (float4) 0.0f;
	r5 = (float4) 0.0f;
	r6 = (float4) 0.0f;
	r7 = (float4) 0.0f;
	r8 = (float4) 0.0f;
	r9 = (float4) 0.0f;
	r10 = (float4) 0.0f;

	// vertex start
	f0_start = loc_start;
	r1 = loc_start;
	r10 = r1+(float4)( 0x1p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
	r9.x = dot(r10.xy,l0.xy) + l0.w;
	r9.y = dot(r10.xy,l1.xy) + l1.w;
	f1_start = r9;
	r9 = r1+(float4)( 0x1p+0, 0x1p+0, 0x0p+0, 0x0p+0 );
	r8.x = dot(r9.xy,l0.xy) + l0.w;
	r8.y = dot(r9.xy,l1.xy) + l1.w;
	f2_start = r8;
	r8 = r1+(float4)( 0x0p+0, 0x1p+0, 0x0p+0, 0x0p+0 );
	r7.x = dot(r8.xy,l0.xy) + l0.w;
	r7.y = dot(r8.xy,l1.xy) + l1.w;
	f3_start = r7;
	r7 = r1+(float4)( -0x1p+0, 0x1p+0, 0x0p+0, 0x0p+0 );
	r6.x = dot(r7.xy,l0.xy) + l0.w;
	r6.y = dot(r7.xy,l1.xy) + l1.w;
	f4_start = r6;
	r6 = r1+(float4)( -0x1p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
	r5.x = dot(r6.xy,l0.xy) + l0.w;
	r5.y = dot(r6.xy,l1.xy) + l1.w;
	f5_start = r5;
	r5 = r1+(float4)( -0x1p+0, -0x1p+0, 0x0p+0, 0x0p+0 );
	r4.x = dot(r5.xy,l0.xy) + l0.w;
	r4.y = dot(r5.xy,l1.xy) + l1.w;
	f6_start = r4;
	r4 = r1+(float4)( 0x0p+0, -0x1p+0, 0x0p+0, 0x0p+0 );
	r3.x = dot(r4.xy,l0.xy) + l0.w;
	r3.y = dot(r4.xy,l1.xy) + l1.w;
	f7_start = r3;
	r3 = r1+(float4)( 0x1p+0, -0x1p+0, 0x0p+0, 0x0p+0 );
	r2.x = dot(r3.xy,l0.xy) + l0.w;
	r2.y = dot(r3.xy,l1.xy) + l1.w;
	f8_start = r2;
	r0.x = dot(r1.xy,l0.xy) + l0.w;
	r0.y = dot(r1.xy,l1.xy) + l1.w;
	f9_start = r0;

	// vertex end
	f0_end = loc_end;
	r1 = loc_end;
	r10 = r1+(float4)( 0x1p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
	r9.x = dot(r10.xy,l0.xy) + l0.w;
	r9.y = dot(r10.xy,l1.xy) + l1.w;
	f1_end = r9;
	r9 = r1+(float4)( 0x1p+0, 0x1p+0, 0x0p+0, 0x0p+0 );
	r8.x = dot(r9.xy,l0.xy) + l0.w;
	r8.y = dot(r9.xy,l1.xy) + l1.w;
	f2_end = r8;
	r8 = r1+(float4)( 0x0p+0, 0x1p+0, 0x0p+0, 0x0p+0 );
	r7.x = dot(r8.xy,l0.xy) + l0.w;
	r7.y = dot(r8.xy,l1.xy) + l1.w;
	f3_end = r7;
	r7 = r1+(float4)( -0x1p+0, 0x1p+0, 0x0p+0, 0x0p+0 );
	r6.x = dot(r7.xy,l0.xy) + l0.w;
	r6.y = dot(r7.xy,l1.xy) + l1.w;
	f4_end = r6;
	r6 = r1+(float4)( -0x1p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
	r5.x = dot(r6.xy,l0.xy) + l0.w;
	r5.y = dot(r6.xy,l1.xy) + l1.w;
	f5_end = r5;
	r5 = r1+(float4)( -0x1p+0, -0x1p+0, 0x0p+0, 0x0p+0 );
	r4.x = dot(r5.xy,l0.xy) + l0.w;
	r4.y = dot(r5.xy,l1.xy) + l1.w;
	f6_end = r4;
	r4 = r1+(float4)( 0x0p+0, -0x1p+0, 0x0p+0, 0x0p+0 );
	r3.x = dot(r4.xy,l0.xy) + l0.w;
	r3.y = dot(r4.xy,l1.xy) + l1.w;
	f7_end = r3;
	r3 = r1+(float4)( 0x1p+0, -0x1p+0, 0x0p+0, 0x0p+0 );
	r2.x = dot(r3.xy,l0.xy) + l0.w;
	r2.y = dot(r3.xy,l1.xy) + l1.w;
	f8_end = r2;
	r0.x = dot(r1.xy,l0.xy) + l0.w;
	r0.y = dot(r1.xy,l1.xy) + l1.w;
	f9_end = r0;

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
	f3_delta = (f3_end - f3_start) / (float)(dest_width);
	tf3_delta[0] = (float4) f3_delta.s0;
	tf3_start[0] = (float4) ( f3_start.s0, f3_start.s0 + f3_delta.s0, f3_start.s0 + 2.0*f3_delta.s0, f3_start.s0 + 3.0*f3_delta.s0 );
	tf3_delta[1] = (float4) f3_delta.s1;
	tf3_start[1] = (float4) ( f3_start.s1, f3_start.s1 + f3_delta.s1, f3_start.s1 + 2.0*f3_delta.s1, f3_start.s1 + 3.0*f3_delta.s1 );
	tf3_delta[2] = (float4) f3_delta.s2;
	tf3_start[2] = (float4) ( f3_start.s2, f3_start.s2 + f3_delta.s2, f3_start.s2 + 2.0*f3_delta.s2, f3_start.s2 + 3.0*f3_delta.s2 );
	tf3_delta[3] = (float4) f3_delta.s3;
	tf3_start[3] = (float4) ( f3_start.s3, f3_start.s3 + f3_delta.s3, f3_start.s3 + 2.0*f3_delta.s3, f3_start.s3 + 3.0*f3_delta.s3 );
	f4_delta = (f4_end - f4_start) / (float)(dest_width);
	tf4_delta[0] = (float4) f4_delta.s0;
	tf4_start[0] = (float4) ( f4_start.s0, f4_start.s0 + f4_delta.s0, f4_start.s0 + 2.0*f4_delta.s0, f4_start.s0 + 3.0*f4_delta.s0 );
	tf4_delta[1] = (float4) f4_delta.s1;
	tf4_start[1] = (float4) ( f4_start.s1, f4_start.s1 + f4_delta.s1, f4_start.s1 + 2.0*f4_delta.s1, f4_start.s1 + 3.0*f4_delta.s1 );
	tf4_delta[2] = (float4) f4_delta.s2;
	tf4_start[2] = (float4) ( f4_start.s2, f4_start.s2 + f4_delta.s2, f4_start.s2 + 2.0*f4_delta.s2, f4_start.s2 + 3.0*f4_delta.s2 );
	tf4_delta[3] = (float4) f4_delta.s3;
	tf4_start[3] = (float4) ( f4_start.s3, f4_start.s3 + f4_delta.s3, f4_start.s3 + 2.0*f4_delta.s3, f4_start.s3 + 3.0*f4_delta.s3 );
	f5_delta = (f5_end - f5_start) / (float)(dest_width);
	tf5_delta[0] = (float4) f5_delta.s0;
	tf5_start[0] = (float4) ( f5_start.s0, f5_start.s0 + f5_delta.s0, f5_start.s0 + 2.0*f5_delta.s0, f5_start.s0 + 3.0*f5_delta.s0 );
	tf5_delta[1] = (float4) f5_delta.s1;
	tf5_start[1] = (float4) ( f5_start.s1, f5_start.s1 + f5_delta.s1, f5_start.s1 + 2.0*f5_delta.s1, f5_start.s1 + 3.0*f5_delta.s1 );
	tf5_delta[2] = (float4) f5_delta.s2;
	tf5_start[2] = (float4) ( f5_start.s2, f5_start.s2 + f5_delta.s2, f5_start.s2 + 2.0*f5_delta.s2, f5_start.s2 + 3.0*f5_delta.s2 );
	tf5_delta[3] = (float4) f5_delta.s3;
	tf5_start[3] = (float4) ( f5_start.s3, f5_start.s3 + f5_delta.s3, f5_start.s3 + 2.0*f5_delta.s3, f5_start.s3 + 3.0*f5_delta.s3 );
	f6_delta = (f6_end - f6_start) / (float)(dest_width);
	tf6_delta[0] = (float4) f6_delta.s0;
	tf6_start[0] = (float4) ( f6_start.s0, f6_start.s0 + f6_delta.s0, f6_start.s0 + 2.0*f6_delta.s0, f6_start.s0 + 3.0*f6_delta.s0 );
	tf6_delta[1] = (float4) f6_delta.s1;
	tf6_start[1] = (float4) ( f6_start.s1, f6_start.s1 + f6_delta.s1, f6_start.s1 + 2.0*f6_delta.s1, f6_start.s1 + 3.0*f6_delta.s1 );
	tf6_delta[2] = (float4) f6_delta.s2;
	tf6_start[2] = (float4) ( f6_start.s2, f6_start.s2 + f6_delta.s2, f6_start.s2 + 2.0*f6_delta.s2, f6_start.s2 + 3.0*f6_delta.s2 );
	tf6_delta[3] = (float4) f6_delta.s3;
	tf6_start[3] = (float4) ( f6_start.s3, f6_start.s3 + f6_delta.s3, f6_start.s3 + 2.0*f6_delta.s3, f6_start.s3 + 3.0*f6_delta.s3 );
	f7_delta = (f7_end - f7_start) / (float)(dest_width);
	tf7_delta[0] = (float4) f7_delta.s0;
	tf7_start[0] = (float4) ( f7_start.s0, f7_start.s0 + f7_delta.s0, f7_start.s0 + 2.0*f7_delta.s0, f7_start.s0 + 3.0*f7_delta.s0 );
	tf7_delta[1] = (float4) f7_delta.s1;
	tf7_start[1] = (float4) ( f7_start.s1, f7_start.s1 + f7_delta.s1, f7_start.s1 + 2.0*f7_delta.s1, f7_start.s1 + 3.0*f7_delta.s1 );
	tf7_delta[2] = (float4) f7_delta.s2;
	tf7_start[2] = (float4) ( f7_start.s2, f7_start.s2 + f7_delta.s2, f7_start.s2 + 2.0*f7_delta.s2, f7_start.s2 + 3.0*f7_delta.s2 );
	tf7_delta[3] = (float4) f7_delta.s3;
	tf7_start[3] = (float4) ( f7_start.s3, f7_start.s3 + f7_delta.s3, f7_start.s3 + 2.0*f7_delta.s3, f7_start.s3 + 3.0*f7_delta.s3 );
	f8_delta = (f8_end - f8_start) / (float)(dest_width);
	tf8_delta[0] = (float4) f8_delta.s0;
	tf8_start[0] = (float4) ( f8_start.s0, f8_start.s0 + f8_delta.s0, f8_start.s0 + 2.0*f8_delta.s0, f8_start.s0 + 3.0*f8_delta.s0 );
	tf8_delta[1] = (float4) f8_delta.s1;
	tf8_start[1] = (float4) ( f8_start.s1, f8_start.s1 + f8_delta.s1, f8_start.s1 + 2.0*f8_delta.s1, f8_start.s1 + 3.0*f8_delta.s1 );
	tf8_delta[2] = (float4) f8_delta.s2;
	tf8_start[2] = (float4) ( f8_start.s2, f8_start.s2 + f8_delta.s2, f8_start.s2 + 2.0*f8_delta.s2, f8_start.s2 + 3.0*f8_delta.s2 );
	tf8_delta[3] = (float4) f8_delta.s3;
	tf8_start[3] = (float4) ( f8_start.s3, f8_start.s3 + f8_delta.s3, f8_start.s3 + 2.0*f8_delta.s3, f8_start.s3 + 3.0*f8_delta.s3 );
	f9_delta = (f9_end - f9_start) / (float)(dest_width);
	tf9_delta[0] = (float4) f9_delta.s0;
	tf9_start[0] = (float4) ( f9_start.s0, f9_start.s0 + f9_delta.s0, f9_start.s0 + 2.0*f9_delta.s0, f9_start.s0 + 3.0*f9_delta.s0 );
	tf9_delta[1] = (float4) f9_delta.s1;
	tf9_start[1] = (float4) ( f9_start.s1, f9_start.s1 + f9_delta.s1, f9_start.s1 + 2.0*f9_delta.s1, f9_start.s1 + 3.0*f9_delta.s1 );
	tf9_delta[2] = (float4) f9_delta.s2;
	tf9_start[2] = (float4) ( f9_start.s2, f9_start.s2 + f9_delta.s2, f9_start.s2 + 2.0*f9_delta.s2, f9_start.s2 + 3.0*f9_delta.s2 );
	tf9_delta[3] = (float4) f9_delta.s3;
	tf9_start[3] = (float4) ( f9_start.s3, f9_start.s3 + f9_delta.s3, f9_start.s3 + 2.0*f9_delta.s3, f9_start.s3 + 3.0*f9_delta.s3 );

	__async_work_group_stream_from_image(t0, t_sampler0, f3_start.xy, f3_delta.xy, read_amount, (float4 *)gr0_3_0, (float4 *)gr0_3_1, (float4 *)gr0_3_2, (float4 *)gr0_3_3);
	__async_work_group_stream_from_image(t0, t_sampler0, f1_start.xy, f1_delta.xy, read_amount, (float4 *)gr0_1_0, (float4 *)gr0_1_1, (float4 *)gr0_1_2, (float4 *)gr0_1_3);
	__async_work_group_stream_from_image(t0, t_sampler0, f5_start.xy, f5_delta.xy, read_amount, (float4 *)gr0_5_0, (float4 *)gr0_5_1, (float4 *)gr0_5_2, (float4 *)gr0_5_3);
	__async_work_group_stream_from_image(t0, t_sampler0, f7_start.xy, f7_delta.xy, read_amount, (float4 *)gr0_7_0, (float4 *)gr0_7_1, (float4 *)gr0_7_2, (float4 *)gr0_7_3);
	__async_work_group_stream_from_image(t0, t_sampler0, f8_start.xy, f8_delta.xy, read_amount, (float4 *)gr0_8_0, (float4 *)gr0_8_1, (float4 *)gr0_8_2, (float4 *)gr0_8_3);
	__async_work_group_stream_from_image(t0, t_sampler0, f4_start.xy, f4_delta.xy, read_amount, (float4 *)gr0_4_0, (float4 *)gr0_4_1, (float4 *)gr0_4_2, (float4 *)gr0_4_3);
	__async_work_group_stream_from_image(t0, t_sampler0, f6_start.xy, f6_delta.xy, read_amount, (float4 *)gr0_6_0, (float4 *)gr0_6_1, (float4 *)gr0_6_2, (float4 *)gr0_6_3);
	__async_work_group_stream_from_image(t0, t_sampler0, f9_start.xy, f9_delta.xy, read_amount, (float4 *)gr0_9_0, (float4 *)gr0_9_1, (float4 *)gr0_9_2, (float4 *)gr0_9_3);
	__async_work_group_stream_from_image(t0, t_sampler0, f2_start.xy, f2_delta.xy, read_amount, (float4 *)gr0_2_0, (float4 *)gr0_2_1, (float4 *)gr0_2_2, (float4 *)gr0_2_3);
	for(; loc.x<dest_width; loc.x+=4)
	{
		r0 = gr0_1_0[index];
		r1 = gr0_1_1[index];
		r2 = gr0_1_2[index];
		r3 = gr0_1_3[index];
		r4 = gr0_2_0[index];
		r5 = gr0_2_1[index];
		r6 = gr0_2_2[index];
		r7 = gr0_2_3[index];
		r8 = gr0_3_0[index];
		r9 = gr0_3_1[index];
		r10 = gr0_3_2[index];
		r11 = gr0_3_3[index];
		r12 = gr0_4_0[index];
		r13 = gr0_4_1[index];
		r14 = gr0_4_2[index];
		r15 = gr0_4_3[index];
		r16 = gr0_5_0[index];
		r17 = gr0_5_1[index];
		r18 = gr0_5_2[index];
		r19 = gr0_5_3[index];
		r20 = gr0_6_0[index];
		r21 = gr0_6_1[index];
		r22 = gr0_6_2[index];
		r23 = gr0_6_3[index];
		r24 = gr0_7_0[index];
		r25 = gr0_7_1[index];
		r26 = gr0_7_2[index];
		r27 = gr0_7_3[index];
		r28 = gr0_8_0[index];
		r29 = gr0_8_1[index];
		r30 = gr0_8_2[index];
		r31 = gr0_8_3[index];
		r32 = gr0_9_0[index];
		r33 = gr0_9_1[index];
		r34 = gr0_9_2[index];
		r35 = gr0_9_3[index];
		r36 = min(r4, r8);
		r37 = min(r5, r9);
		r38 = min(r6, r10);
		r39 = min(r7, r11);
		r40 = max(r4, r8);
		r41 = max(r5, r9);
		r42 = max(r6, r10);
		r43 = max(r7, r11);
		r44 = r40;
		r45 = r41;
		r46 = r42;
		r47 = r43;
		r48 = min(r16, r20);
		r49 = min(r17, r21);
		r50 = min(r18, r22);
		r51 = min(r19, r23);
		r52 = max(r16, r20);
		r53 = max(r17, r21);
		r54 = max(r18, r22);
		r55 = max(r19, r23);
		r56 = r52;
		r57 = r53;
		r58 = r54;
		r59 = r55;
		r60 = min(r28, r32);
		r61 = min(r29, r33);
		r62 = min(r30, r34);
		r63 = min(r31, r35);
		r64 = max(r28, r32);
		r65 = max(r29, r33);
		r66 = max(r30, r34);
		r67 = max(r31, r35);
		r68 = r64;
		r69 = r65;
		r70 = r66;
		r71 = r67;
		r72 = min(r0, r36);
		r73 = min(r1, r37);
		r74 = min(r2, r38);
		r75 = min(r3, r39);
		r76 = max(r0, r36);
		r77 = max(r1, r37);
		r78 = max(r2, r38);
		r79 = max(r3, r39);
		r80 = r76;
		r81 = r77;
		r82 = r78;
		r83 = r79;
		r84 = min(r12, r48);
		r85 = min(r13, r49);
		r86 = min(r14, r50);
		r87 = min(r15, r51);
		r88 = max(r12, r48);
		r89 = max(r13, r49);
		r90 = max(r14, r50);
		r91 = max(r15, r51);
		r92 = r88;
		r93 = r89;
		r94 = r90;
		r95 = r91;
		r96 = min(r24, r60);
		r97 = min(r25, r61);
		r98 = min(r26, r62);
		r99 = min(r27, r63);
		r100 = max(r24, r60);
		r101 = max(r25, r61);
		r102 = max(r26, r62);
		r103 = max(r27, r63);
		r104 = r100;
		r105 = r101;
		r106 = r102;
		r107 = r103;
		r108 = min(r80, r44);
		r109 = min(r81, r45);
		r110 = min(r82, r46);
		r111 = min(r83, r47);
		r112 = max(r80, r44);
		r113 = max(r81, r45);
		r114 = max(r82, r46);
		r115 = max(r83, r47);
		r116 = r112;
		r117 = r113;
		r118 = r114;
		r119 = r115;
		r120 = min(r92, r56);
		r121 = min(r93, r57);
		r122 = min(r94, r58);
		r123 = min(r95, r59);
		r124 = max(r92, r56);
		r125 = max(r93, r57);
		r126 = max(r94, r58);
		r127 = max(r95, r59);
		r128 = r124;
		r129 = r125;
		r130 = r126;
		r131 = r127;
		r132 = min(r104, r68);
		r133 = min(r105, r69);
		r134 = min(r106, r70);
		r135 = min(r107, r71);
		r136 = max(r104, r68);
		r137 = max(r105, r69);
		r138 = max(r106, r70);
		r139 = max(r107, r71);
		r140 = r136;
		r141 = r137;
		r142 = r138;
		r143 = r139;
		r144 = max(r72, r84);
		r145 = max(r73, r85);
		r146 = max(r74, r86);
		r147 = max(r75, r87);
		r148 = r144;
		r149 = r145;
		r150 = r146;
		r151 = r147;
		r152 = min(r128, r140);
		r153 = min(r129, r141);
		r154 = min(r130, r142);
		r155 = min(r131, r143);
		r156 = r152;
		r157 = r153;
		r158 = r154;
		r159 = r155;
		r160 = max(r148, r96);
		r161 = max(r149, r97);
		r162 = max(r150, r98);
		r163 = max(r151, r99);
		r164 = r160;
		r165 = r161;
		r166 = r162;
		r167 = r163;
		r168 = min(r116, r156);
		r169 = min(r117, r157);
		r170 = min(r118, r158);
		r171 = min(r119, r159);
		r172 = r168;
		r173 = r169;
		r174 = r170;
		r175 = r171;
		r176 = min(r120, r132);
		r177 = min(r121, r133);
		r178 = min(r122, r134);
		r179 = min(r123, r135);
		r180 = max(r120, r132);
		r181 = max(r121, r133);
		r182 = max(r122, r134);
		r183 = max(r123, r135);
		r184 = r180;
		r185 = r181;
		r186 = r182;
		r187 = r183;
		r188 = max(r108, r176);
		r189 = max(r109, r177);
		r190 = max(r110, r178);
		r191 = max(r111, r179);
		r192 = r188;
		r193 = r189;
		r194 = r190;
		r195 = r191;
		r196 = min(r184, r192);
		r197 = min(r185, r193);
		r198 = min(r186, r194);
		r199 = min(r187, r195);
		r200 = r196;
		r201 = r197;
		r202 = r198;
		r203 = r199;
		r204 = min(r200, r172);
		r205 = min(r201, r173);
		r206 = min(r202, r174);
		r207 = min(r203, r175);
		r208 = max(r200, r172);
		r209 = max(r201, r173);
		r210 = max(r202, r174);
		r211 = max(r203, r175);
		r212 = r208;
		r213 = r209;
		r214 = r210;
		r215 = r211;
		r216 = max(r164, r204);
		r217 = max(r165, r205);
		r218 = max(r166, r206);
		r219 = max(r167, r207);
		r220 = r216;
		r221 = r217;
		r222 = r218;
		r223 = r219;
		r224 = min(r220, r212);
		r225 = min(r221, r213);
		r226 = min(r222, r214);
		r227 = min(r223, r215);
		r228 = r224;
		r229 = r225;
		r230 = r226;
		r231 = r227;
		r232 = min(r228, r231);
		r233 = min(r229, r231);
		r234 = min(r230, r231);
		r235 = r232;
		r236 = r233;
		r237 = r234;
		r238 = r235;
		r239 = r236;
		r240 = r237;
		r241 = r231;
		o_r[index] = r238;
		o_g[index] = r239;
		o_b[index] = r240;
		o_a[index] = r241;
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
			__async_work_group_stream_from_image(t0, t_sampler0, f3_start.xy + ((float)4*total_index) * f3_delta.xy, f3_delta.xy, read_amount, (float4 *)gr0_3_0, (float4 *)gr0_3_1, (float4 *)gr0_3_2, (float4 *)gr0_3_3);
			__async_work_group_stream_from_image(t0, t_sampler0, f1_start.xy + ((float)4*total_index) * f1_delta.xy, f1_delta.xy, read_amount, (float4 *)gr0_1_0, (float4 *)gr0_1_1, (float4 *)gr0_1_2, (float4 *)gr0_1_3);
			__async_work_group_stream_from_image(t0, t_sampler0, f5_start.xy + ((float)4*total_index) * f5_delta.xy, f5_delta.xy, read_amount, (float4 *)gr0_5_0, (float4 *)gr0_5_1, (float4 *)gr0_5_2, (float4 *)gr0_5_3);
			__async_work_group_stream_from_image(t0, t_sampler0, f7_start.xy + ((float)4*total_index) * f7_delta.xy, f7_delta.xy, read_amount, (float4 *)gr0_7_0, (float4 *)gr0_7_1, (float4 *)gr0_7_2, (float4 *)gr0_7_3);
			__async_work_group_stream_from_image(t0, t_sampler0, f8_start.xy + ((float)4*total_index) * f8_delta.xy, f8_delta.xy, read_amount, (float4 *)gr0_8_0, (float4 *)gr0_8_1, (float4 *)gr0_8_2, (float4 *)gr0_8_3);
			__async_work_group_stream_from_image(t0, t_sampler0, f4_start.xy + ((float)4*total_index) * f4_delta.xy, f4_delta.xy, read_amount, (float4 *)gr0_4_0, (float4 *)gr0_4_1, (float4 *)gr0_4_2, (float4 *)gr0_4_3);
			__async_work_group_stream_from_image(t0, t_sampler0, f6_start.xy + ((float)4*total_index) * f6_delta.xy, f6_delta.xy, read_amount, (float4 *)gr0_6_0, (float4 *)gr0_6_1, (float4 *)gr0_6_2, (float4 *)gr0_6_3);
			__async_work_group_stream_from_image(t0, t_sampler0, f9_start.xy + ((float)4*total_index) * f9_delta.xy, f9_delta.xy, read_amount, (float4 *)gr0_9_0, (float4 *)gr0_9_1, (float4 *)gr0_9_2, (float4 *)gr0_9_3);
			__async_work_group_stream_from_image(t0, t_sampler0, f2_start.xy + ((float)4*total_index) * f2_delta.xy, f2_delta.xy, read_amount, (float4 *)gr0_2_0, (float4 *)gr0_2_1, (float4 *)gr0_2_2, (float4 *)gr0_2_3);
		}
		;
	}
	if (index> 0)
	{
		(void)__async_work_group_stream_to_image(dest, (size_t)(dim.z + write_offset), (size_t)(flipped ? get_image_height(dest) - (loc.y+dim.w+1): loc.y+dim.w), (size_t)(dest_width - write_offset), (const float4 *)o_r, (const float4 *)o_g, (const float4 *)o_b, (const float4 *)o_a);
	}
}
