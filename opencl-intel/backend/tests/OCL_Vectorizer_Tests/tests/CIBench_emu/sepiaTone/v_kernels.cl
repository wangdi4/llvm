// Reduced: back to "transposed4" samplers
__kernel void programManyReduce(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, float4 l2, float4 l3, read_only image2d_t t0, sampler_t t_sampler0)
{
    const float4 p0_x  = (float4)( 0x1.b33334p-3, 0x1.b33334p-3, 0x1.b33334p-3, 0x1.b33334p-3 );
    const float4 p0_y  = (float4)( 0x1.6e48e8p-1, 0x1.6e48e8p-1, 0x1.6e48e8p-1, 0x1.6e48e8p-1 );
    const float4 p0_z  = (float4)( 0x1.275254p-4, 0x1.275254p-4, 0x1.275254p-4, 0x1.275254p-4 );
    const float4 p0_w  = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p1_x  = (float4)( 0x1.a36e2ep-14, 0x1.a36e2ep-14, 0x1.a36e2ep-14, 0x1.a36e2ep-14 );
    const float4 p1_y  = (float4)( 0x1.5c28f6p-4, 0x1.5c28f6p-4, 0x1.5c28f6p-4, 0x1.5c28f6p-4 );
    const float4 p1_z  = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p1_w  = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p2_x  = (float4)( 0x1.d595dap-11, 0x1.d595dap-11, 0x1.d595dap-11, 0x1.d595dap-11 );
    const float4 p2_y  = (float4)( -0x1.218e3cp-10, -0x1.218e3cp-10, -0x1.218e3cp-10, -0x1.218e3cp-10 );
    const float4 p2_z  = (float4)( -0x1.3ee89ep-11, -0x1.3ee89ep-11, -0x1.3ee89ep-11, -0x1.3ee89ep-11 );
    const float4 p2_w  = (float4)( 0x1.0c7ca6p-5, 0x1.0c7ca6p-5, 0x1.0c7ca6p-5, 0x1.0c7ca6p-5 );
    const float4 p3_x  = (float4)( -0x1.c2bf7cp+11, -0x1.c2bf7cp+11, -0x1.c2bf7cp+11, -0x1.c2bf7cp+11 );
    const float4 p3_y  = (float4)( 0x1.0efb7cp-3, 0x1.0efb7cp-3, 0x1.0efb7cp-3, 0x1.0efb7cp-3 );
    const float4 p3_z  = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p3_w  = (float4)( -0x1.97e1fcp-3, -0x1.97e1fcp-3, -0x1.97e1fcp-3, -0x1.97e1fcp-3 );
    const float4 p4_x  = (float4)( 0x1.62659ep+9, 0x1.62659ep+9, 0x1.62659ep+9, 0x1.62659ep+9 );
    const float4 p4_y  = (float4)( -0x1.8fad94p-2, -0x1.8fad94p-2, -0x1.8fad94p-2, -0x1.8fad94p-2 );
    const float4 p4_z  = (float4)( -0x1.df8f8cp-5, -0x1.df8f8cp-5, -0x1.df8f8cp-5, -0x1.df8f8cp-5 );
    const float4 p4_w  = (float4)( 0x1.52ff12p-1, 0x1.52ff12p-1, 0x1.52ff12p-1, 0x1.52ff12p-1 );
    const float4 p5_x  = (float4)( -0x1.9777ap+5, -0x1.9777ap+5, -0x1.9777ap+5, -0x1.9777ap+5 );
    const float4 p5_y  = (float4)( 0x1.dca79ap-2, 0x1.dca79ap-2, 0x1.dca79ap-2, 0x1.dca79ap-2 );
    const float4 p5_z  = (float4)( 0x1.070dd8p+0, 0x1.070dd8p+0, 0x1.070dd8p+0, 0x1.070dd8p+0 );
    const float4 p5_w  = (float4)( -0x1.d0565ap-1, -0x1.d0565ap-1, -0x1.d0565ap-1, -0x1.d0565ap-1 );
    const float4 p6_x  = (float4)( 0x1.8eef1cp+1, 0x1.8eef1cp+1, 0x1.8eef1cp+1, 0x1.8eef1cp+1 );
    const float4 p6_y  = (float4)( 0x1.95d48cp-1, 0x1.95d48cp-1, 0x1.95d48cp-1, 0x1.95d48cp-1 );
    const float4 p6_z  = (float4)( 0x1.07c1b6p-5, 0x1.07c1b6p-5, 0x1.07c1b6p-5, 0x1.07c1b6p-5 );
    const float4 p6_w  = (float4)( 0x1.696ecep+0, 0x1.696ecep+0, 0x1.696ecep+0, 0x1.696ecep+0 );

    float4       f0_x, r0_x, r1_x, r2_x, r3_x, r4_x, r5_x;
    float4       f0_y, r0_y, r1_y, r2_y, r3_y, r4_y, r5_y;
    float4       f0_z, r0_z, r1_z, r2_z, r3_z, r4_z, r5_z;
    float4       f0_w, r0_w, r1_w, r2_w, r3_w, r4_w, r5_w;
    int4 loc_x, loc_y;
    const int Many = 128;
    int tmp = get_global_id(0) * Many;
    int orig_loc_x = tmp;
    loc_y = get_global_id(1);
    int count;

    for (count = 0; count < Many/4; ++count)
    {
    
        loc_x = (int4)(tmp, tmp+1, tmp+2, tmp+3);

        f0_x = st_origin.x + ((float4)((float)loc_x.x,(float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.x + ((float4)((float)loc_y.x,(float)loc_y.y, (float)loc_y.z, (float)loc_y.w) + 0.5f) * st_delta.z;
        f0_y = st_origin.y + ((float4)((float)loc_x.x,(float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.y + ((float4)((float)loc_y.x,(float)loc_y.y, (float)loc_y.z, (float)loc_y.w) + 0.5f) * st_delta.w;
        f0_z = 0.0f;
        f0_w = 0.0f;

        r2_x = f0_x;
        r2_y = f0_y;
        r2_z = f0_z;
        r2_w = f0_w;

        r0_x = r2_x*l2.x + r2_y*l2.y + l2.w;
        r0_y = r2_x*l3.x + r2_y*l3.y + l3.w;
 
        r4_x = r0_x;
        r4_y = r0_y;
        r4_z = r0_z;
        r4_w = r0_w;

        read_transposed_imagef(t0, t_sampler0, r4_x, r4_y, &r1_x, &r1_y, &r1_z, &r1_w);

        r3_x = r1_x*p0_x + r1_y*p0_y + r1_z*p0_z;
        r3_y = r3_x; r3_z = r3_x; r3_w = r3_x;

        r2_x = max(p1_x, r1_w);
        r2_y = max(p1_x, r1_w);
        r2_z = max(p1_x, r1_w);
        r2_w = max(p1_x, r1_w);

        r0_x = half_recip(r2_x);
        r0_y = half_recip(r2_x);
        r0_z = half_recip(r2_x);
        r0_w = half_recip(r2_x);

        r4_x = r3_x*r0_x;
        r4_y = r3_y*r0_y;
        r4_z = r3_z*r0_z;
        r4_w = r3_w*r0_w;

        r2_x = r1_w*p2_x;
        r2_y = r1_w*p2_y;
        r2_z = r1_w*p2_z;
        r2_w = r1_w*p2_w;

        r0_x = r4_x*p3_x + p4_x;
        r0_y = r4_y*p3_y + p4_y;
        r0_z = r4_z*p3_z + p4_z;
        r0_w = r4_w*p3_w + p4_w;

        r0_x = r0_x*r4_x + p5_x;
        r0_y = r0_y*r4_y + p5_y;
        r0_z = r0_z*r4_z + p5_z;
        r0_w = r0_w*r4_w + p5_w;

        r0_x = r0_x*r4_x + p6_x;
        r0_y = r0_y*r4_y + p6_y;
        r0_z = r0_z*r4_z + p6_z;
        r0_w = r0_w*r4_w + p6_w;

        r2_x = r0_x*r3_x + r2_x;
        r2_y = r0_y*r3_y + r2_y;
        r2_z = r0_z*r3_z + r2_z;
        r2_w = r0_w*r3_w + r2_w;

        r0_x = r1_w*p1_y + -r3_x;
        r0_y = r1_w*p1_y + -r3_y;
        r0_z = r1_w*p1_y + -r3_z;
        r0_w = r1_w*p1_y + -r3_w;

        r2_x = select(r2_w,r2_x, isless(-r0_x, 0.0f));

        r0_x = r3_x*r3_x + -r3_x;
        r0_y = r3_y*r3_y + -r3_y;
        r0_z = r3_z*r3_z + -r3_z;
        r0_w = r3_w*r3_w + -r3_w;

        r0_x = select(r3_x,r2_x, isless(r0_x, 0.0f));
        r0_y = select(r3_y,r2_y, isless(r0_x, 0.0f));
        r0_z = select(r3_z,r2_z, isless(r0_x, 0.0f));
        r0_w = select(r3_w,r2_w, isless(r0_x, 0.0f));

        r0_w = r1_w;

        r0_x = r0_x*l0.x;
        r0_y = r0_y*l0.y;
        r0_z = r0_z*l0.z;
        r0_w = r0_w*l0.w;

        r0_x = mix(r1_x,r0_x, l1.x);
        r0_y = mix(r1_y,r0_y, l1.x);
        r0_z = mix(r1_z,r0_z, l1.x);
        r0_w = mix(r1_w,r0_w, l1.x);

        r0_x = min(r0_x, r0_w);
        r0_y = min(r0_y, r0_w);
        r0_z = min(r0_z, r0_w);


        int yaxis;
        int yaxisT, yaxisF;

        yaxisT = get_image_height(dest) - (loc_y.x + dim.w + 1);
        yaxisF = loc_y.x + dim.w;
        yaxis = select (yaxisF, yaxisT, flipped);

        write_transposed_imagef(dest, loc_x.x + dim.z, yaxis, r0_x, r0_y, r0_z, r0_w);

        tmp += 4;
    }
}





// Also: transposed samplers!
__kernel void programMany(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, float4 l2, float4 l3, read_only image2d_t t0, sampler_t t_sampler0)
{
    const float4 p0_x  = (float4)( 0x1.b33334p-3, 0x1.b33334p-3, 0x1.b33334p-3, 0x1.b33334p-3 );
    const float4 p0_y  = (float4)( 0x1.6e48e8p-1, 0x1.6e48e8p-1, 0x1.6e48e8p-1, 0x1.6e48e8p-1 );
    const float4 p0_z  = (float4)( 0x1.275254p-4, 0x1.275254p-4, 0x1.275254p-4, 0x1.275254p-4 );
    const float4 p0_w  = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p1_x  = (float4)( 0x1.a36e2ep-14, 0x1.a36e2ep-14, 0x1.a36e2ep-14, 0x1.a36e2ep-14 );
    const float4 p1_y  = (float4)( 0x1.5c28f6p-4, 0x1.5c28f6p-4, 0x1.5c28f6p-4, 0x1.5c28f6p-4 );
    const float4 p1_z  = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p1_w  = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p2_x  = (float4)( 0x1.d595dap-11, 0x1.d595dap-11, 0x1.d595dap-11, 0x1.d595dap-11 );
    const float4 p2_y  = (float4)( -0x1.218e3cp-10, -0x1.218e3cp-10, -0x1.218e3cp-10, -0x1.218e3cp-10 );
    const float4 p2_z  = (float4)( -0x1.3ee89ep-11, -0x1.3ee89ep-11, -0x1.3ee89ep-11, -0x1.3ee89ep-11 );
    const float4 p2_w  = (float4)( 0x1.0c7ca6p-5, 0x1.0c7ca6p-5, 0x1.0c7ca6p-5, 0x1.0c7ca6p-5 );
    const float4 p3_x  = (float4)( -0x1.c2bf7cp+11, -0x1.c2bf7cp+11, -0x1.c2bf7cp+11, -0x1.c2bf7cp+11 );
    const float4 p3_y  = (float4)( 0x1.0efb7cp-3, 0x1.0efb7cp-3, 0x1.0efb7cp-3, 0x1.0efb7cp-3 );
    const float4 p3_z  = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p3_w  = (float4)( -0x1.97e1fcp-3, -0x1.97e1fcp-3, -0x1.97e1fcp-3, -0x1.97e1fcp-3 );
    const float4 p4_x  = (float4)( 0x1.62659ep+9, 0x1.62659ep+9, 0x1.62659ep+9, 0x1.62659ep+9 );
    const float4 p4_y  = (float4)( -0x1.8fad94p-2, -0x1.8fad94p-2, -0x1.8fad94p-2, -0x1.8fad94p-2 );
    const float4 p4_z  = (float4)( -0x1.df8f8cp-5, -0x1.df8f8cp-5, -0x1.df8f8cp-5, -0x1.df8f8cp-5 );
    const float4 p4_w  = (float4)( 0x1.52ff12p-1, 0x1.52ff12p-1, 0x1.52ff12p-1, 0x1.52ff12p-1 );
    const float4 p5_x  = (float4)( -0x1.9777ap+5, -0x1.9777ap+5, -0x1.9777ap+5, -0x1.9777ap+5 );
    const float4 p5_y  = (float4)( 0x1.dca79ap-2, 0x1.dca79ap-2, 0x1.dca79ap-2, 0x1.dca79ap-2 );
    const float4 p5_z  = (float4)( 0x1.070dd8p+0, 0x1.070dd8p+0, 0x1.070dd8p+0, 0x1.070dd8p+0 );
    const float4 p5_w  = (float4)( -0x1.d0565ap-1, -0x1.d0565ap-1, -0x1.d0565ap-1, -0x1.d0565ap-1 );
    const float4 p6_x  = (float4)( 0x1.8eef1cp+1, 0x1.8eef1cp+1, 0x1.8eef1cp+1, 0x1.8eef1cp+1 );
    const float4 p6_y  = (float4)( 0x1.95d48cp-1, 0x1.95d48cp-1, 0x1.95d48cp-1, 0x1.95d48cp-1 );
    const float4 p6_z  = (float4)( 0x1.07c1b6p-5, 0x1.07c1b6p-5, 0x1.07c1b6p-5, 0x1.07c1b6p-5 );
    const float4 p6_w  = (float4)( 0x1.696ecep+0, 0x1.696ecep+0, 0x1.696ecep+0, 0x1.696ecep+0 );

    float4       f0_x, r0_x, r1_x, r2_x, r3_x, r4_x, r5_x;
    float4       f0_y, r0_y, r1_y, r2_y, r3_y, r4_y, r5_y;
    float4       f0_z, r0_z, r1_z, r2_z, r3_z, r4_z, r5_z;
    float4       f0_w, r0_w, r1_w, r2_w, r3_w, r4_w, r5_w;
    int4 loc_x, loc_y;
    const int Many = 128;
    int tmp = get_global_id(0) * Many;
    int orig_loc_x = tmp;
    loc_y = get_global_id(1);
    int count;
    float4     input_x[Many/4], input_y[Many/4], input_z[Many/4], input_w[Many/4], output_x[Many/4], output_y[Many/4], output_z[Many/4], output_w[Many/4];
    for (count = 0; count < Many/4; ++count)
    {
    
        loc_x = (int4)(tmp, tmp+1, tmp+2, tmp+3);

        f0_x = st_origin.x + ((float4)((float)loc_x.x,(float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.x + ((float4)((float)loc_y.x,(float)loc_y.y, (float)loc_y.z, (float)loc_y.w) + 0.5f) * st_delta.z;
        f0_y = st_origin.y + ((float4)((float)loc_x.x,(float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.y + ((float4)((float)loc_y.x,(float)loc_y.y, (float)loc_y.z, (float)loc_y.w) + 0.5f) * st_delta.w;
        f0_z = 0.0f;
        f0_w = 0.0f;

        r2_x = f0_x;
        r2_y = f0_y;
        r2_z = f0_z;
        r2_w = f0_w;

        r0_x = r2_x*l2.x + r2_y*l2.y + l2.w;
        r0_y = r2_x*l3.x + r2_y*l3.y + l3.w;
 
        r4_x = r0_x;
        r4_y = r0_y;
        r4_z = r0_z;
        r4_w = r0_w;

        // read inputs only once! 
        if (count == 0)
        {
            float2 stride;
            stride.x = r4_x.y - r4_x.x;
            stride.y = r4_y.y - r4_y.x;
            __async_work_group_stream_from_image(t0, t_sampler0, (float2)(r4_x.x, r4_y.x), stride, Many, input_x, input_y, input_z, input_w);
        }
        // extract the inputs you currently require
        r1_x = input_x[count];
        r1_y = input_y[count];
        r1_z = input_z[count];
        r1_w = input_w[count];


        r3_x = r1_x*p0_x + r1_y*p0_y + r1_z*p0_z;
        r3_y = r3_x; r3_z = r3_x; r3_w = r3_x;

        r2_x = max(p1_x, r1_w);
        r2_y = max(p1_x, r1_w);
        r2_z = max(p1_x, r1_w);
        r2_w = max(p1_x, r1_w);

        r0_x = half_recip(r2_x);
        r0_y = half_recip(r2_x);
        r0_z = half_recip(r2_x);
        r0_w = half_recip(r2_x);

        r4_x = r3_x*r0_x;
        r4_y = r3_y*r0_y;
        r4_z = r3_z*r0_z;
        r4_w = r3_w*r0_w;

        r2_x = r1_w*p2_x;
        r2_y = r1_w*p2_y;
        r2_z = r1_w*p2_z;
        r2_w = r1_w*p2_w;

        r0_x = r4_x*p3_x + p4_x;
        r0_y = r4_y*p3_y + p4_y;
        r0_z = r4_z*p3_z + p4_z;
        r0_w = r4_w*p3_w + p4_w;

        r0_x = r0_x*r4_x + p5_x;
        r0_y = r0_y*r4_y + p5_y;
        r0_z = r0_z*r4_z + p5_z;
        r0_w = r0_w*r4_w + p5_w;

        r0_x = r0_x*r4_x + p6_x;
        r0_y = r0_y*r4_y + p6_y;
        r0_z = r0_z*r4_z + p6_z;
        r0_w = r0_w*r4_w + p6_w;

        r2_x = r0_x*r3_x + r2_x;
        r2_y = r0_y*r3_y + r2_y;
        r2_z = r0_z*r3_z + r2_z;
        r2_w = r0_w*r3_w + r2_w;

        r0_x = r1_w*p1_y + -r3_x;
        r0_y = r1_w*p1_y + -r3_y;
        r0_z = r1_w*p1_y + -r3_z;
        r0_w = r1_w*p1_y + -r3_w;

        r2_x = select(r2_w,r2_x, isless(-r0_x, 0.0f));

        r0_x = r3_x*r3_x + -r3_x;
        r0_y = r3_y*r3_y + -r3_y;
        r0_z = r3_z*r3_z + -r3_z;
        r0_w = r3_w*r3_w + -r3_w;

        r0_x = select(r3_x,r2_x, isless(r0_x, 0.0f));
        r0_y = select(r3_y,r2_y, isless(r0_x, 0.0f));
        r0_z = select(r3_z,r2_z, isless(r0_x, 0.0f));
        r0_w = select(r3_w,r2_w, isless(r0_x, 0.0f));

        r0_w = r1_w;

        r0_x = r0_x*l0.x;
        r0_y = r0_y*l0.y;
        r0_z = r0_z*l0.z;
        r0_w = r0_w*l0.w;

        r0_x = mix(r1_x,r0_x, l1.x);
        r0_y = mix(r1_y,r0_y, l1.x);
        r0_z = mix(r1_z,r0_z, l1.x);
        r0_w = mix(r1_w,r0_w, l1.x);

        r0_x = min(r0_x, r0_w);
        r0_y = min(r0_y, r0_w);
        r0_z = min(r0_z, r0_w);


        int yaxis;
        int yaxisT, yaxisF;

        yaxisT = get_image_height(dest) - (loc_y.x + dim.w + 1);
        yaxisF = loc_y.x + dim.w;
        yaxis = select (yaxisF, yaxisT, flipped);

        output_x[count] = r0_x;
        output_y[count] = r0_y;
        output_z[count] = r0_z;
        output_w[count] = r0_w;
        if (count == Many/4-1)
        {
            __async_work_group_stream_to_image(dest, orig_loc_x + dim.z, yaxis, Many, output_x, output_y, output_z, output_w);
        }

        tmp += 4;
    }
}




// Also: transposed samplers!
__kernel void program4_4(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, float4 l2, float4 l3, read_only image2d_t t0, sampler_t t_sampler0)
{
    const float4 p0_x  = (float4)( 0x1.b33334p-3, 0x1.b33334p-3, 0x1.b33334p-3, 0x1.b33334p-3 );
    const float4 p0_y  = (float4)( 0x1.6e48e8p-1, 0x1.6e48e8p-1, 0x1.6e48e8p-1, 0x1.6e48e8p-1 );
    const float4 p0_z  = (float4)( 0x1.275254p-4, 0x1.275254p-4, 0x1.275254p-4, 0x1.275254p-4 );
    const float4 p0_w  = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p1_x  = (float4)( 0x1.a36e2ep-14, 0x1.a36e2ep-14, 0x1.a36e2ep-14, 0x1.a36e2ep-14 );
    const float4 p1_y  = (float4)( 0x1.5c28f6p-4, 0x1.5c28f6p-4, 0x1.5c28f6p-4, 0x1.5c28f6p-4 );
    const float4 p1_z  = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p1_w  = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p2_x  = (float4)( 0x1.d595dap-11, 0x1.d595dap-11, 0x1.d595dap-11, 0x1.d595dap-11 );
    const float4 p2_y  = (float4)( -0x1.218e3cp-10, -0x1.218e3cp-10, -0x1.218e3cp-10, -0x1.218e3cp-10 );
    const float4 p2_z  = (float4)( -0x1.3ee89ep-11, -0x1.3ee89ep-11, -0x1.3ee89ep-11, -0x1.3ee89ep-11 );
    const float4 p2_w  = (float4)( 0x1.0c7ca6p-5, 0x1.0c7ca6p-5, 0x1.0c7ca6p-5, 0x1.0c7ca6p-5 );
    const float4 p3_x  = (float4)( -0x1.c2bf7cp+11, -0x1.c2bf7cp+11, -0x1.c2bf7cp+11, -0x1.c2bf7cp+11 );
    const float4 p3_y  = (float4)( 0x1.0efb7cp-3, 0x1.0efb7cp-3, 0x1.0efb7cp-3, 0x1.0efb7cp-3 );
    const float4 p3_z  = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p3_w  = (float4)( -0x1.97e1fcp-3, -0x1.97e1fcp-3, -0x1.97e1fcp-3, -0x1.97e1fcp-3 );
    const float4 p4_x  = (float4)( 0x1.62659ep+9, 0x1.62659ep+9, 0x1.62659ep+9, 0x1.62659ep+9 );
    const float4 p4_y  = (float4)( -0x1.8fad94p-2, -0x1.8fad94p-2, -0x1.8fad94p-2, -0x1.8fad94p-2 );
    const float4 p4_z  = (float4)( -0x1.df8f8cp-5, -0x1.df8f8cp-5, -0x1.df8f8cp-5, -0x1.df8f8cp-5 );
    const float4 p4_w  = (float4)( 0x1.52ff12p-1, 0x1.52ff12p-1, 0x1.52ff12p-1, 0x1.52ff12p-1 );
    const float4 p5_x  = (float4)( -0x1.9777ap+5, -0x1.9777ap+5, -0x1.9777ap+5, -0x1.9777ap+5 );
    const float4 p5_y  = (float4)( 0x1.dca79ap-2, 0x1.dca79ap-2, 0x1.dca79ap-2, 0x1.dca79ap-2 );
    const float4 p5_z  = (float4)( 0x1.070dd8p+0, 0x1.070dd8p+0, 0x1.070dd8p+0, 0x1.070dd8p+0 );
    const float4 p5_w  = (float4)( -0x1.d0565ap-1, -0x1.d0565ap-1, -0x1.d0565ap-1, -0x1.d0565ap-1 );
    const float4 p6_x  = (float4)( 0x1.8eef1cp+1, 0x1.8eef1cp+1, 0x1.8eef1cp+1, 0x1.8eef1cp+1 );
    const float4 p6_y  = (float4)( 0x1.95d48cp-1, 0x1.95d48cp-1, 0x1.95d48cp-1, 0x1.95d48cp-1 );
    const float4 p6_z  = (float4)( 0x1.07c1b6p-5, 0x1.07c1b6p-5, 0x1.07c1b6p-5, 0x1.07c1b6p-5 );
    const float4 p6_w  = (float4)( 0x1.696ecep+0, 0x1.696ecep+0, 0x1.696ecep+0, 0x1.696ecep+0 );

    float4       f0_x, r0_x, r1_x, r2_x, r3_x, r4_x, r5_x;
    float4       f0_y, r0_y, r1_y, r2_y, r3_y, r4_y, r5_y;
    float4       f0_z, r0_z, r1_z, r2_z, r3_z, r4_z, r5_z;
    float4       f0_w, r0_w, r1_w, r2_w, r3_w, r4_w, r5_w;
    int4 loc_x, loc_y;

    int tmp = get_global_id(0)*4;
    loc_x = (int4)(tmp, tmp+1, tmp+2, tmp+3);
    loc_y = get_global_id(1);

    f0_x = st_origin.x + ((float4)((float)loc_x.x,(float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.x + ((float4)((float)loc_y.x,(float)loc_y.y, (float)loc_y.z, (float)loc_y.w) + 0.5f) * st_delta.z;
    f0_y = st_origin.y + ((float4)((float)loc_x.x,(float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.y + ((float4)((float)loc_y.x,(float)loc_y.y, (float)loc_y.z, (float)loc_y.w) + 0.5f) * st_delta.w;
    f0_z = 0.0f;
    f0_w = 0.0f;

    r2_x = f0_x;
    r2_y = f0_y;
    r2_z = f0_z;
    r2_w = f0_w;

    r0_x = r2_x*l2.x + r2_y*l2.y + l2.w;
    r0_y = r2_x*l3.x + r2_y*l3.y + l3.w;

    r4_x = r0_x;
    r4_y = r0_y;
    r4_z = r0_z;
    r4_w = r0_w;

    read_transposed_imagef(t0, t_sampler0, r4_x, r4_y, &r1_x, &r1_y, &r1_z, &r1_w);

//   float4 r1T0, r1T1, r1T2, r1T3;
//    r1T0 = read_imagef(t0, t_sampler0, (float2)(r4_x.x, r4_y.x));
//    r1T1 = read_imagef(t0, t_sampler0, (float2)(r4_x.y, r4_y.y));
//    r1T2 = read_imagef(t0, t_sampler0, (float2)(r4_x.z, r4_y.z));
//    r1T3 = read_imagef(t0, t_sampler0, (float2)(r4_x.w, r4_y.w));
//   // transpose back
//   r1_x = (float4)(r1T0.x, r1T1.x, r1T2.x, r1T3.x);
//   r1_y = (float4)(r1T0.y, r1T1.y, r1T2.y, r1T3.y);
//   r1_z = (float4)(r1T0.z, r1T1.z, r1T2.z, r1T3.z);
//   r1_w = (float4)(r1T0.w, r1T1.w, r1T2.w, r1T3.w);

    r3_x = r1_x*p0_x + r1_y*p0_y + r1_z*p0_z;
    r3_y = r3_x; r3_z = r3_x; r3_w = r3_x;

    r2_x = max(p1_x, r1_w);
    r2_y = max(p1_x, r1_w);
    r2_z = max(p1_x, r1_w);
    r2_w = max(p1_x, r1_w);

    r0_x = half_recip(r2_x);
    r0_y = half_recip(r2_x);
    r0_z = half_recip(r2_x);
    r0_w = half_recip(r2_x);

    r4_x = r3_x*r0_x;
    r4_y = r3_y*r0_y;
    r4_z = r3_z*r0_z;
    r4_w = r3_w*r0_w;

    r2_x = r1_w*p2_x;
    r2_y = r1_w*p2_y;
    r2_z = r1_w*p2_z;
    r2_w = r1_w*p2_w;

    r0_x = r4_x*p3_x + p4_x;
    r0_y = r4_y*p3_y + p4_y;
    r0_z = r4_z*p3_z + p4_z;
    r0_w = r4_w*p3_w + p4_w;

    r0_x = r0_x*r4_x + p5_x;
    r0_y = r0_y*r4_y + p5_y;
    r0_z = r0_z*r4_z + p5_z;
    r0_w = r0_w*r4_w + p5_w;

    r0_x = r0_x*r4_x + p6_x;
    r0_y = r0_y*r4_y + p6_y;
    r0_z = r0_z*r4_z + p6_z;
    r0_w = r0_w*r4_w + p6_w;

    r2_x = r0_x*r3_x + r2_x;
    r2_y = r0_y*r3_y + r2_y;
    r2_z = r0_z*r3_z + r2_z;
    r2_w = r0_w*r3_w + r2_w;

    r0_x = r1_w*p1_y + -r3_x;
    r0_y = r1_w*p1_y + -r3_y;
    r0_z = r1_w*p1_y + -r3_z;
    r0_w = r1_w*p1_y + -r3_w;

    r2_x = select(r2_w,r2_x, isless(-r0_x, 0.0f));

    r0_x = r3_x*r3_x + -r3_x;
    r0_y = r3_y*r3_y + -r3_y;
    r0_z = r3_z*r3_z + -r3_z;
    r0_w = r3_w*r3_w + -r3_w;

    r0_x = select(r3_x,r2_x, isless(r0_x, 0.0f));
    r0_y = select(r3_y,r2_y, isless(r0_x, 0.0f));
    r0_z = select(r3_z,r2_z, isless(r0_x, 0.0f));
    r0_w = select(r3_w,r2_w, isless(r0_x, 0.0f));

    r0_w = r1_w;

    r0_x = r0_x*l0.x;
    r0_y = r0_y*l0.y;
    r0_z = r0_z*l0.z;
    r0_w = r0_w*l0.w;

    r0_x = mix(r1_x,r0_x, l1.x);
    r0_y = mix(r1_y,r0_y, l1.x);
    r0_z = mix(r1_z,r0_z, l1.x);
    r0_w = mix(r1_w,r0_w, l1.x);

    r0_x = min(r0_x, r0_w);
    r0_y = min(r0_y, r0_w);
    r0_z = min(r0_z, r0_w);


    int yaxis;
    int yaxisT, yaxisF;

    yaxisT = get_image_height(dest) - (loc_y.x + dim.w + 1);
    yaxisF = loc_y.x + dim.w;
    yaxis = select (yaxisF, yaxisT, flipped);

    write_transposed_imagef(dest, loc_x.x + dim.z, yaxis, r0_x, r0_y, r0_z, r0_w);

//    float4 r0T0, r0T1, r0T2, r0T3;
//    r0T0 = (float4)(r0_x.x, r0_y.x, r0_z.x, r0_w.x);
//    r0T1 = (float4)(r0_x.y, r0_y.y, r0_z.y, r0_w.y);
//    r0T2 = (float4)(r0_x.z, r0_y.z, r0_z.z, r0_w.z);
//    r0T3 = (float4)(r0_x.w, r0_y.w, r0_z.w, r0_w.w);
//    write_imagef(dest, (int2)( loc_x.x + dim.z , yaxis ), r0T0);
//    write_imagef(dest, (int2)( loc_x.y + dim.z , yaxis ), r0T1);
//    write_imagef(dest, (int2)( loc_x.z + dim.z , yaxis ), r0T2);
//    write_imagef(dest, (int2)( loc_x.w + dim.z , yaxis ), r0T3);
}



// also: dot_product vectorize
__kernel void program4_3(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, float4 l2, float4 l3, read_only image2d_t t0, sampler_t t_sampler0)
{
    const float4 p0_x  = (float4)( 0x1.b33334p-3, 0x1.b33334p-3, 0x1.b33334p-3, 0x1.b33334p-3 );
    const float4 p0_y  = (float4)( 0x1.6e48e8p-1, 0x1.6e48e8p-1, 0x1.6e48e8p-1, 0x1.6e48e8p-1 );
    const float4 p0_z  = (float4)( 0x1.275254p-4, 0x1.275254p-4, 0x1.275254p-4, 0x1.275254p-4 );
    const float4 p0_w  = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p1_x  = (float4)( 0x1.a36e2ep-14, 0x1.a36e2ep-14, 0x1.a36e2ep-14, 0x1.a36e2ep-14 );
    const float4 p1_y  = (float4)( 0x1.5c28f6p-4, 0x1.5c28f6p-4, 0x1.5c28f6p-4, 0x1.5c28f6p-4 );
    const float4 p1_z  = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p1_w  = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p2_x  = (float4)( 0x1.d595dap-11, 0x1.d595dap-11, 0x1.d595dap-11, 0x1.d595dap-11 );
    const float4 p2_y  = (float4)( -0x1.218e3cp-10, -0x1.218e3cp-10, -0x1.218e3cp-10, -0x1.218e3cp-10 );
    const float4 p2_z  = (float4)( -0x1.3ee89ep-11, -0x1.3ee89ep-11, -0x1.3ee89ep-11, -0x1.3ee89ep-11 );
    const float4 p2_w  = (float4)( 0x1.0c7ca6p-5, 0x1.0c7ca6p-5, 0x1.0c7ca6p-5, 0x1.0c7ca6p-5 );
    const float4 p3_x  = (float4)( -0x1.c2bf7cp+11, -0x1.c2bf7cp+11, -0x1.c2bf7cp+11, -0x1.c2bf7cp+11 );
    const float4 p3_y  = (float4)( 0x1.0efb7cp-3, 0x1.0efb7cp-3, 0x1.0efb7cp-3, 0x1.0efb7cp-3 );
    const float4 p3_z  = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p3_w  = (float4)( -0x1.97e1fcp-3, -0x1.97e1fcp-3, -0x1.97e1fcp-3, -0x1.97e1fcp-3 );
    const float4 p4_x  = (float4)( 0x1.62659ep+9, 0x1.62659ep+9, 0x1.62659ep+9, 0x1.62659ep+9 );
    const float4 p4_y  = (float4)( -0x1.8fad94p-2, -0x1.8fad94p-2, -0x1.8fad94p-2, -0x1.8fad94p-2 );
    const float4 p4_z  = (float4)( -0x1.df8f8cp-5, -0x1.df8f8cp-5, -0x1.df8f8cp-5, -0x1.df8f8cp-5 );
    const float4 p4_w  = (float4)( 0x1.52ff12p-1, 0x1.52ff12p-1, 0x1.52ff12p-1, 0x1.52ff12p-1 );
    const float4 p5_x  = (float4)( -0x1.9777ap+5, -0x1.9777ap+5, -0x1.9777ap+5, -0x1.9777ap+5 );
    const float4 p5_y  = (float4)( 0x1.dca79ap-2, 0x1.dca79ap-2, 0x1.dca79ap-2, 0x1.dca79ap-2 );
    const float4 p5_z  = (float4)( 0x1.070dd8p+0, 0x1.070dd8p+0, 0x1.070dd8p+0, 0x1.070dd8p+0 );
    const float4 p5_w  = (float4)( -0x1.d0565ap-1, -0x1.d0565ap-1, -0x1.d0565ap-1, -0x1.d0565ap-1 );
    const float4 p6_x  = (float4)( 0x1.8eef1cp+1, 0x1.8eef1cp+1, 0x1.8eef1cp+1, 0x1.8eef1cp+1 );
    const float4 p6_y  = (float4)( 0x1.95d48cp-1, 0x1.95d48cp-1, 0x1.95d48cp-1, 0x1.95d48cp-1 );
    const float4 p6_z  = (float4)( 0x1.07c1b6p-5, 0x1.07c1b6p-5, 0x1.07c1b6p-5, 0x1.07c1b6p-5 );
    const float4 p6_w  = (float4)( 0x1.696ecep+0, 0x1.696ecep+0, 0x1.696ecep+0, 0x1.696ecep+0 );

    float4       f0_x, r0_x, r1_x, r2_x, r3_x, r4_x, r5_x;
    float4       f0_y, r0_y, r1_y, r2_y, r3_y, r4_y, r5_y;
    float4       f0_z, r0_z, r1_z, r2_z, r3_z, r4_z, r5_z;
    float4       f0_w, r0_w, r1_w, r2_w, r3_w, r4_w, r5_w;
    int4 loc_x, loc_y;

    int tmp = get_global_id(0)*4;
    loc_x = (int4)(tmp, tmp+1, tmp+2, tmp+3);
    loc_y = get_global_id(1);

    f0_x = st_origin.x + ((float4)((float)loc_x.x,(float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.x + ((float4)((float)loc_y.x,(float)loc_y.y, (float)loc_y.z, (float)loc_y.w) + 0.5f) * st_delta.z;
    f0_y = st_origin.y + ((float4)((float)loc_x.x,(float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.y + ((float4)((float)loc_y.x,(float)loc_y.y, (float)loc_y.z, (float)loc_y.w) + 0.5f) * st_delta.w;
    f0_z = 0.0f;
    f0_w = 0.0f;

    r2_x = f0_x;
    r2_y = f0_y;
    r2_z = f0_z;
    r2_w = f0_w;

//    r0.x = dot(r2.xy,l2.xy) + l2.w;
//    r0.y = dot(r2.xy,l3.xy) + l3.w;
//r0.x = r2.x*l2.x + r2.y*l2.y + l2.w;
//r0.y = r2.x*l3.x + r2.y*l3.y + l3.w;

    r0_x = r2_x*l2.x + r2_y*l2.y + l2.w;
    r0_y = r2_x*l3.x + r2_y*l3.y + l3.w;

//    r0_x.x = dot((float2)(r2_x.x, r2_y.x),l2.xy) + l2.w;
//    r0_x.y = dot((float2)(r2_x.y, r2_y.y),l2.xy) + l2.w;
//    r0_x.z = dot((float2)(r2_x.z, r2_y.z),l2.xy) + l2.w;
//    r0_x.w = dot((float2)(r2_x.w, r2_y.w),l2.xy) + l2.w; 
//    r0_y.x = dot((float2)(r2_x.x, r2_y.x),l3.xy) + l3.w;
//    r0_y.y = dot((float2)(r2_x.y, r2_y.y),l3.xy) + l3.w;
//    r0_y.z = dot((float2)(r2_x.z, r2_y.z),l3.xy) + l3.w;
//    r0_y.w = dot((float2)(r2_x.w, r2_y.w),l3.xy) + l3.w;

    r4_x = r0_x;
    r4_y = r0_y;
    r4_z = r0_z;
    r4_w = r0_w;

   float4 r1T0, r1T1, r1T2, r1T3;
    r1T0 = read_imagef(t0, t_sampler0, (float2)(r4_x.x, r4_y.x));
    r1T1 = read_imagef(t0, t_sampler0, (float2)(r4_x.y, r4_y.y));
    r1T2 = read_imagef(t0, t_sampler0, (float2)(r4_x.z, r4_y.z));
    r1T3 = read_imagef(t0, t_sampler0, (float2)(r4_x.w, r4_y.w));
   // transpose back
   r1_x = (float4)(r1T0.x, r1T1.x, r1T2.x, r1T3.x);
   r1_y = (float4)(r1T0.y, r1T1.y, r1T2.y, r1T3.y);
   r1_z = (float4)(r1T0.z, r1T1.z, r1T2.z, r1T3.z);
   r1_w = (float4)(r1T0.w, r1T1.w, r1T2.w, r1T3.w);

//    r3 = dot(r1.xyz,p0.xyz);
//r3.x = r1.x*p0.x + r1.y*p0.y + r1.z*p0.z;
//r3.y = r3.x; r3.z = r3.x; r3.w = r3.x;

    r3_x = r1_x*p0_x + r1_y*p0_y + r1_z*p0_z;
    r3_y = r3_x; r3_z = r3_x; r3_w = r3_x;

//    float4 p0T = (float4)(p0_x.x, p0_y.x, p0_z.x, p0_w.x); 
//    float4 r3T0, r3T1, r3T2, r3T3;
//    r3T0 = dot(r1T0.xyz,p0T.xyz);
//    r3T1 = dot(r1T1.xyz,p0T.xyz);
//    r3T2 = dot(r1T2.xyz,p0T.xyz);
//    r3T3 = dot(r1T3.xyz,p0T.xyz);
//   // transpose back
//   r3_x = (float4)(r3T0.x, r3T1.x, r3T2.x, r3T3.x);
//   r3_y = (float4)(r3T0.y, r3T1.y, r3T2.y, r3T3.y);
//   r3_z = (float4)(r3T0.z, r3T1.z, r3T2.z, r3T3.z);
//   r3_w = (float4)(r3T0.w, r3T1.w, r3T2.w, r3T3.w);

    r2_x = max(p1_x, r1_w);
    r2_y = max(p1_x, r1_w);
    r2_z = max(p1_x, r1_w);
    r2_w = max(p1_x, r1_w);

    r0_x = half_recip(r2_x);
    r0_y = half_recip(r2_x);
    r0_z = half_recip(r2_x);
    r0_w = half_recip(r2_x);

    r4_x = r3_x*r0_x;
    r4_y = r3_y*r0_y;
    r4_z = r3_z*r0_z;
    r4_w = r3_w*r0_w;

    r2_x = r1_w*p2_x;
    r2_y = r1_w*p2_y;
    r2_z = r1_w*p2_z;
    r2_w = r1_w*p2_w;

    r0_x = r4_x*p3_x + p4_x;
    r0_y = r4_y*p3_y + p4_y;
    r0_z = r4_z*p3_z + p4_z;
    r0_w = r4_w*p3_w + p4_w;

    r0_x = r0_x*r4_x + p5_x;
    r0_y = r0_y*r4_y + p5_y;
    r0_z = r0_z*r4_z + p5_z;
    r0_w = r0_w*r4_w + p5_w;

    r0_x = r0_x*r4_x + p6_x;
    r0_y = r0_y*r4_y + p6_y;
    r0_z = r0_z*r4_z + p6_z;
    r0_w = r0_w*r4_w + p6_w;

    r2_x = r0_x*r3_x + r2_x;
    r2_y = r0_y*r3_y + r2_y;
    r2_z = r0_z*r3_z + r2_z;
    r2_w = r0_w*r3_w + r2_w;

    r0_x = r1_w*p1_y + -r3_x;
    r0_y = r1_w*p1_y + -r3_y;
    r0_z = r1_w*p1_y + -r3_z;
    r0_w = r1_w*p1_y + -r3_w;

    r2_x = select(r2_w,r2_x, isless(-r0_x, 0.0f));

    r0_x = r3_x*r3_x + -r3_x;
    r0_y = r3_y*r3_y + -r3_y;
    r0_z = r3_z*r3_z + -r3_z;
    r0_w = r3_w*r3_w + -r3_w;

    r0_x = select(r3_x,r2_x, isless(r0_x, 0.0f));
    r0_y = select(r3_y,r2_y, isless(r0_x, 0.0f));
    r0_z = select(r3_z,r2_z, isless(r0_x, 0.0f));
    r0_w = select(r3_w,r2_w, isless(r0_x, 0.0f));

    r0_w = r1_w;

    r0_x = r0_x*l0.x;
    r0_y = r0_y*l0.y;
    r0_z = r0_z*l0.z;
    r0_w = r0_w*l0.w;

    r0_x = mix(r1_x,r0_x, l1.x);
    r0_y = mix(r1_y,r0_y, l1.x);
    r0_z = mix(r1_z,r0_z, l1.x);
    r0_w = mix(r1_w,r0_w, l1.x);

    r0_x = min(r0_x, r0_w);
    r0_y = min(r0_y, r0_w);
    r0_z = min(r0_z, r0_w);

    float4 r0T0, r0T1, r0T2, r0T3;
    r0T0 = (float4)(r0_x.x, r0_y.x, r0_z.x, r0_w.x);
    r0T1 = (float4)(r0_x.y, r0_y.y, r0_z.y, r0_w.y);
    r0T2 = (float4)(r0_x.z, r0_y.z, r0_z.z, r0_w.z);
    r0T3 = (float4)(r0_x.w, r0_y.w, r0_z.w, r0_w.w);

    int yaxis;
    int yaxisT, yaxisF;

    yaxisT = get_image_height(dest) - (loc_y.x + dim.w + 1);
    yaxisF = loc_y.x + dim.w;
    yaxis = select (yaxisF, yaxisT, flipped);

    write_imagef(dest, (int2)( loc_x.x + dim.z , yaxis ), r0T0);
    write_imagef(dest, (int2)( loc_x.y + dim.z , yaxis ), r0T1);
    write_imagef(dest, (int2)( loc_x.z + dim.z , yaxis ), r0T2);
    write_imagef(dest, (int2)( loc_x.w + dim.z , yaxis ), r0T3);
}




// // also: minimize control flow
__kernel void program4_2(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, float4 l2, float4 l3, read_only image2d_t t0, sampler_t t_sampler0)
{
    const float4 p0_x  = (float4)( 0x1.b33334p-3, 0x1.b33334p-3, 0x1.b33334p-3, 0x1.b33334p-3 );
    const float4 p0_y  = (float4)( 0x1.6e48e8p-1, 0x1.6e48e8p-1, 0x1.6e48e8p-1, 0x1.6e48e8p-1 );
    const float4 p0_z  = (float4)( 0x1.275254p-4, 0x1.275254p-4, 0x1.275254p-4, 0x1.275254p-4 );
    const float4 p0_w  = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p1_x  = (float4)( 0x1.a36e2ep-14, 0x1.a36e2ep-14, 0x1.a36e2ep-14, 0x1.a36e2ep-14 );
    const float4 p1_y  = (float4)( 0x1.5c28f6p-4, 0x1.5c28f6p-4, 0x1.5c28f6p-4, 0x1.5c28f6p-4 );
    const float4 p1_z  = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p1_w  = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p2_x  = (float4)( 0x1.d595dap-11, 0x1.d595dap-11, 0x1.d595dap-11, 0x1.d595dap-11 );
    const float4 p2_y  = (float4)( -0x1.218e3cp-10, -0x1.218e3cp-10, -0x1.218e3cp-10, -0x1.218e3cp-10 );
    const float4 p2_z  = (float4)( -0x1.3ee89ep-11, -0x1.3ee89ep-11, -0x1.3ee89ep-11, -0x1.3ee89ep-11 );
    const float4 p2_w  = (float4)( 0x1.0c7ca6p-5, 0x1.0c7ca6p-5, 0x1.0c7ca6p-5, 0x1.0c7ca6p-5 );
    const float4 p3_x  = (float4)( -0x1.c2bf7cp+11, -0x1.c2bf7cp+11, -0x1.c2bf7cp+11, -0x1.c2bf7cp+11 );
    const float4 p3_y  = (float4)( 0x1.0efb7cp-3, 0x1.0efb7cp-3, 0x1.0efb7cp-3, 0x1.0efb7cp-3 );
    const float4 p3_z  = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p3_w  = (float4)( -0x1.97e1fcp-3, -0x1.97e1fcp-3, -0x1.97e1fcp-3, -0x1.97e1fcp-3 );
    const float4 p4_x  = (float4)( 0x1.62659ep+9, 0x1.62659ep+9, 0x1.62659ep+9, 0x1.62659ep+9 );
    const float4 p4_y  = (float4)( -0x1.8fad94p-2, -0x1.8fad94p-2, -0x1.8fad94p-2, -0x1.8fad94p-2 );
    const float4 p4_z  = (float4)( -0x1.df8f8cp-5, -0x1.df8f8cp-5, -0x1.df8f8cp-5, -0x1.df8f8cp-5 );
    const float4 p4_w  = (float4)( 0x1.52ff12p-1, 0x1.52ff12p-1, 0x1.52ff12p-1, 0x1.52ff12p-1 );
    const float4 p5_x  = (float4)( -0x1.9777ap+5, -0x1.9777ap+5, -0x1.9777ap+5, -0x1.9777ap+5 );
    const float4 p5_y  = (float4)( 0x1.dca79ap-2, 0x1.dca79ap-2, 0x1.dca79ap-2, 0x1.dca79ap-2 );
    const float4 p5_z  = (float4)( 0x1.070dd8p+0, 0x1.070dd8p+0, 0x1.070dd8p+0, 0x1.070dd8p+0 );
    const float4 p5_w  = (float4)( -0x1.d0565ap-1, -0x1.d0565ap-1, -0x1.d0565ap-1, -0x1.d0565ap-1 );
    const float4 p6_x  = (float4)( 0x1.8eef1cp+1, 0x1.8eef1cp+1, 0x1.8eef1cp+1, 0x1.8eef1cp+1 );
    const float4 p6_y  = (float4)( 0x1.95d48cp-1, 0x1.95d48cp-1, 0x1.95d48cp-1, 0x1.95d48cp-1 );
    const float4 p6_z  = (float4)( 0x1.07c1b6p-5, 0x1.07c1b6p-5, 0x1.07c1b6p-5, 0x1.07c1b6p-5 );
    const float4 p6_w  = (float4)( 0x1.696ecep+0, 0x1.696ecep+0, 0x1.696ecep+0, 0x1.696ecep+0 );

    float4       f0_x, r0_x, r1_x, r2_x, r3_x, r4_x, r5_x;
    float4       f0_y, r0_y, r1_y, r2_y, r3_y, r4_y, r5_y;
    float4       f0_z, r0_z, r1_z, r2_z, r3_z, r4_z, r5_z;
    float4       f0_w, r0_w, r1_w, r2_w, r3_w, r4_w, r5_w;
    int4 loc_x, loc_y;

    int tmp = get_global_id(0)*4;
    loc_x = (int4)(tmp, tmp+1, tmp+2, tmp+3);
    loc_y = get_global_id(1);

    f0_x = st_origin.x + ((float4)((float)loc_x.x,(float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.x + ((float4)((float)loc_y.x,(float)loc_y.y, (float)loc_y.z, (float)loc_y.w) + 0.5f) * st_delta.z;
    f0_y = st_origin.y + ((float4)((float)loc_x.x,(float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.y + ((float4)((float)loc_y.x,(float)loc_y.y, (float)loc_y.z, (float)loc_y.w) + 0.5f) * st_delta.w;
    f0_z = 0.0f;
    f0_w = 0.0f;

    r2_x = f0_x;
    r2_y = f0_y;
    r2_z = f0_z;
    r2_w = f0_w;

    r0_x.x = dot((float2)(r2_x.x, r2_y.x),l2.xy) + l2.w;
    r0_x.y = dot((float2)(r2_x.y, r2_y.y),l2.xy) + l2.w;
    r0_x.z = dot((float2)(r2_x.z, r2_y.z),l2.xy) + l2.w;
    r0_x.w = dot((float2)(r2_x.w, r2_y.w),l2.xy) + l2.w;

    r0_y.x = dot((float2)(r2_x.x, r2_y.x),l3.xy) + l3.w;
    r0_y.y = dot((float2)(r2_x.y, r2_y.y),l3.xy) + l3.w;
    r0_y.z = dot((float2)(r2_x.z, r2_y.z),l3.xy) + l3.w;
    r0_y.w = dot((float2)(r2_x.w, r2_y.w),l3.xy) + l3.w;

    r4_x = r0_x;
    r4_y = r0_y;
    r4_z = r0_z;
    r4_w = r0_w;

   float4 r1T0, r1T1, r1T2, r1T3;
    r1T0 = read_imagef(t0, t_sampler0, (float2)(r4_x.x, r4_y.x));
    r1T1 = read_imagef(t0, t_sampler0, (float2)(r4_x.y, r4_y.y));
    r1T2 = read_imagef(t0, t_sampler0, (float2)(r4_x.z, r4_y.z));
    r1T3 = read_imagef(t0, t_sampler0, (float2)(r4_x.w, r4_y.w));
   // transpose back
   r1_x = (float4)(r1T0.x, r1T1.x, r1T2.x, r1T3.x);
   r1_y = (float4)(r1T0.y, r1T1.y, r1T2.y, r1T3.y);
   r1_z = (float4)(r1T0.z, r1T1.z, r1T2.z, r1T3.z);
   r1_w = (float4)(r1T0.w, r1T1.w, r1T2.w, r1T3.w);

    float4 p0T = (float4)(p0_x.x, p0_y.x, p0_z.x, p0_w.x); 
    float4 r3T0, r3T1, r3T2, r3T3;
    r3T0 = dot(r1T0.xyz,p0T.xyz);
    r3T1 = dot(r1T1.xyz,p0T.xyz);
    r3T2 = dot(r1T2.xyz,p0T.xyz);
    r3T3 = dot(r1T3.xyz,p0T.xyz);
   // transpose back
   r3_x = (float4)(r3T0.x, r3T1.x, r3T2.x, r3T3.x);
   r3_y = (float4)(r3T0.y, r3T1.y, r3T2.y, r3T3.y);
   r3_z = (float4)(r3T0.z, r3T1.z, r3T2.z, r3T3.z);
   r3_w = (float4)(r3T0.w, r3T1.w, r3T2.w, r3T3.w);

    r2_x = max(p1_x, r1_w);
    r2_y = max(p1_x, r1_w);
    r2_z = max(p1_x, r1_w);
    r2_w = max(p1_x, r1_w);

    r0_x = half_recip(r2_x);
    r0_y = half_recip(r2_x);
    r0_z = half_recip(r2_x);
    r0_w = half_recip(r2_x);

    r4_x = r3_x*r0_x;
    r4_y = r3_y*r0_y;
    r4_z = r3_z*r0_z;
    r4_w = r3_w*r0_w;

    r2_x = r1_w*p2_x;
    r2_y = r1_w*p2_y;
    r2_z = r1_w*p2_z;
    r2_w = r1_w*p2_w;

    r0_x = r4_x*p3_x + p4_x;
    r0_y = r4_y*p3_y + p4_y;
    r0_z = r4_z*p3_z + p4_z;
    r0_w = r4_w*p3_w + p4_w;

    r0_x = r0_x*r4_x + p5_x;
    r0_y = r0_y*r4_y + p5_y;
    r0_z = r0_z*r4_z + p5_z;
    r0_w = r0_w*r4_w + p5_w;

    r0_x = r0_x*r4_x + p6_x;
    r0_y = r0_y*r4_y + p6_y;
    r0_z = r0_z*r4_z + p6_z;
    r0_w = r0_w*r4_w + p6_w;

    r2_x = r0_x*r3_x + r2_x;
    r2_y = r0_y*r3_y + r2_y;
    r2_z = r0_z*r3_z + r2_z;
    r2_w = r0_w*r3_w + r2_w;

    r0_x = r1_w*p1_y + -r3_x;
    r0_y = r1_w*p1_y + -r3_y;
    r0_z = r1_w*p1_y + -r3_z;
    r0_w = r1_w*p1_y + -r3_w;

    r2_x = select(r2_w,r2_x, isless(-r0_x, 0.0f));

    r0_x = r3_x*r3_x + -r3_x;
    r0_y = r3_y*r3_y + -r3_y;
    r0_z = r3_z*r3_z + -r3_z;
    r0_w = r3_w*r3_w + -r3_w;

    r0_x = select(r3_x,r2_x, isless(r0_x, 0.0f));
    r0_y = select(r3_y,r2_y, isless(r0_x, 0.0f));
    r0_z = select(r3_z,r2_z, isless(r0_x, 0.0f));
    r0_w = select(r3_w,r2_w, isless(r0_x, 0.0f));

    r0_w = r1_w;

    r0_x = r0_x*l0.x;
    r0_y = r0_y*l0.y;
    r0_z = r0_z*l0.z;
    r0_w = r0_w*l0.w;

    r0_x = mix(r1_x,r0_x, l1.x);
    r0_y = mix(r1_y,r0_y, l1.x);
    r0_z = mix(r1_z,r0_z, l1.x);
    r0_w = mix(r1_w,r0_w, l1.x);

    r0_x = min(r0_x, r0_w);
    r0_y = min(r0_y, r0_w);
    r0_z = min(r0_z, r0_w);

    float4 r0T0, r0T1, r0T2, r0T3;
    r0T0 = (float4)(r0_x.x, r0_y.x, r0_z.x, r0_w.x);
    r0T1 = (float4)(r0_x.y, r0_y.y, r0_z.y, r0_w.y);
    r0T2 = (float4)(r0_x.z, r0_y.z, r0_z.z, r0_w.z);
    r0T3 = (float4)(r0_x.w, r0_y.w, r0_z.w, r0_w.w);
    int yaxis;
    int yaxisT, yaxisF;

    yaxisT = get_image_height(dest) - (loc_y.x + dim.w + 1);
    yaxisF = loc_y.x + dim.w;
    yaxis = select (yaxisF, yaxisT, flipped);
//    if (flipped)
//        yaxis = get_image_height(dest) - (loc_y.x + dim.w + 1);
//    else
//        yaxis = loc_y.x + dim.w;

    write_imagef(dest, (int2)( loc_x.x + dim.z , yaxis ), r0T0);
    write_imagef(dest, (int2)( loc_x.y + dim.z , yaxis ), r0T1);
    write_imagef(dest, (int2)( loc_x.z + dim.z , yaxis ), r0T2);
    write_imagef(dest, (int2)( loc_x.w + dim.z , yaxis ), r0T3);
}



// Vectorized (4). Also: scalarize the <3 x float> min/max functions
__kernel void program4(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, float4 l2, float4 l3, read_only image2d_t t0, sampler_t t_sampler0)
{
    const float4 p0_x  = (float4)( 0x1.b33334p-3, 0x1.b33334p-3, 0x1.b33334p-3, 0x1.b33334p-3 );
    const float4 p0_y  = (float4)( 0x1.6e48e8p-1, 0x1.6e48e8p-1, 0x1.6e48e8p-1, 0x1.6e48e8p-1 );
    const float4 p0_z  = (float4)( 0x1.275254p-4, 0x1.275254p-4, 0x1.275254p-4, 0x1.275254p-4 );
    const float4 p0_w  = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p1_x  = (float4)( 0x1.a36e2ep-14, 0x1.a36e2ep-14, 0x1.a36e2ep-14, 0x1.a36e2ep-14 );
    const float4 p1_y  = (float4)( 0x1.5c28f6p-4, 0x1.5c28f6p-4, 0x1.5c28f6p-4, 0x1.5c28f6p-4 );
    const float4 p1_z  = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p1_w  = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p2_x  = (float4)( 0x1.d595dap-11, 0x1.d595dap-11, 0x1.d595dap-11, 0x1.d595dap-11 );
    const float4 p2_y  = (float4)( -0x1.218e3cp-10, -0x1.218e3cp-10, -0x1.218e3cp-10, -0x1.218e3cp-10 );
    const float4 p2_z  = (float4)( -0x1.3ee89ep-11, -0x1.3ee89ep-11, -0x1.3ee89ep-11, -0x1.3ee89ep-11 );
    const float4 p2_w  = (float4)( 0x1.0c7ca6p-5, 0x1.0c7ca6p-5, 0x1.0c7ca6p-5, 0x1.0c7ca6p-5 );
    const float4 p3_x  = (float4)( -0x1.c2bf7cp+11, -0x1.c2bf7cp+11, -0x1.c2bf7cp+11, -0x1.c2bf7cp+11 );
    const float4 p3_y  = (float4)( 0x1.0efb7cp-3, 0x1.0efb7cp-3, 0x1.0efb7cp-3, 0x1.0efb7cp-3 );
    const float4 p3_z  = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p3_w  = (float4)( -0x1.97e1fcp-3, -0x1.97e1fcp-3, -0x1.97e1fcp-3, -0x1.97e1fcp-3 );
    const float4 p4_x  = (float4)( 0x1.62659ep+9, 0x1.62659ep+9, 0x1.62659ep+9, 0x1.62659ep+9 );
    const float4 p4_y  = (float4)( -0x1.8fad94p-2, -0x1.8fad94p-2, -0x1.8fad94p-2, -0x1.8fad94p-2 );
    const float4 p4_z  = (float4)( -0x1.df8f8cp-5, -0x1.df8f8cp-5, -0x1.df8f8cp-5, -0x1.df8f8cp-5 );
    const float4 p4_w  = (float4)( 0x1.52ff12p-1, 0x1.52ff12p-1, 0x1.52ff12p-1, 0x1.52ff12p-1 );
    const float4 p5_x  = (float4)( -0x1.9777ap+5, -0x1.9777ap+5, -0x1.9777ap+5, -0x1.9777ap+5 );
    const float4 p5_y  = (float4)( 0x1.dca79ap-2, 0x1.dca79ap-2, 0x1.dca79ap-2, 0x1.dca79ap-2 );
    const float4 p5_z  = (float4)( 0x1.070dd8p+0, 0x1.070dd8p+0, 0x1.070dd8p+0, 0x1.070dd8p+0 );
    const float4 p5_w  = (float4)( -0x1.d0565ap-1, -0x1.d0565ap-1, -0x1.d0565ap-1, -0x1.d0565ap-1 );
    const float4 p6_x  = (float4)( 0x1.8eef1cp+1, 0x1.8eef1cp+1, 0x1.8eef1cp+1, 0x1.8eef1cp+1 );
    const float4 p6_y  = (float4)( 0x1.95d48cp-1, 0x1.95d48cp-1, 0x1.95d48cp-1, 0x1.95d48cp-1 );
    const float4 p6_z  = (float4)( 0x1.07c1b6p-5, 0x1.07c1b6p-5, 0x1.07c1b6p-5, 0x1.07c1b6p-5 );
    const float4 p6_w  = (float4)( 0x1.696ecep+0, 0x1.696ecep+0, 0x1.696ecep+0, 0x1.696ecep+0 );

    float4       f0_x, r0_x, r1_x, r2_x, r3_x, r4_x, r5_x;
    float4       f0_y, r0_y, r1_y, r2_y, r3_y, r4_y, r5_y;
    float4       f0_z, r0_z, r1_z, r2_z, r3_z, r4_z, r5_z;
    float4       f0_w, r0_w, r1_w, r2_w, r3_w, r4_w, r5_w;
    int4 loc_x, loc_y;

    int tmp = get_global_id(0)*4;
    loc_x = (int4)(tmp, tmp+1, tmp+2, tmp+3);
    loc_y = get_global_id(1);

    f0_x = st_origin.x + ((float4)((float)loc_x.x,(float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.x + ((float4)((float)loc_y.x,(float)loc_y.y, (float)loc_y.z, (float)loc_y.w) + 0.5f) * st_delta.z;
    f0_y = st_origin.y + ((float4)((float)loc_x.x,(float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.y + ((float4)((float)loc_y.x,(float)loc_y.y, (float)loc_y.z, (float)loc_y.w) + 0.5f) * st_delta.w;
    f0_z = 0.0f;
    f0_w = 0.0f;

    r2_x = f0_x;
    r2_y = f0_y;
    r2_z = f0_z;
    r2_w = f0_w;

    r0_x.x = dot((float2)(r2_x.x, r2_y.x),l2.xy) + l2.w;
    r0_x.y = dot((float2)(r2_x.y, r2_y.y),l2.xy) + l2.w;
    r0_x.z = dot((float2)(r2_x.z, r2_y.z),l2.xy) + l2.w;
    r0_x.w = dot((float2)(r2_x.w, r2_y.w),l2.xy) + l2.w;

    r0_y.x = dot((float2)(r2_x.x, r2_y.x),l3.xy) + l3.w;
    r0_y.y = dot((float2)(r2_x.y, r2_y.y),l3.xy) + l3.w;
    r0_y.z = dot((float2)(r2_x.z, r2_y.z),l3.xy) + l3.w;
    r0_y.w = dot((float2)(r2_x.w, r2_y.w),l3.xy) + l3.w;

    r4_x = r0_x;
    r4_y = r0_y;
    r4_z = r0_z;
    r4_w = r0_w;

   float4 r1T0, r1T1, r1T2, r1T3;
    r1T0 = read_imagef(t0, t_sampler0, (float2)(r4_x.x, r4_y.x));
    r1T1 = read_imagef(t0, t_sampler0, (float2)(r4_x.y, r4_y.y));
    r1T2 = read_imagef(t0, t_sampler0, (float2)(r4_x.z, r4_y.z));
    r1T3 = read_imagef(t0, t_sampler0, (float2)(r4_x.w, r4_y.w));
   // transpose back
   r1_x = (float4)(r1T0.x, r1T1.x, r1T2.x, r1T3.x);
   r1_y = (float4)(r1T0.y, r1T1.y, r1T2.y, r1T3.y);
   r1_z = (float4)(r1T0.z, r1T1.z, r1T2.z, r1T3.z);
   r1_w = (float4)(r1T0.w, r1T1.w, r1T2.w, r1T3.w);

    float4 p0T = (float4)(p0_x.x, p0_y.x, p0_z.x, p0_w.x); 
    float4 r3T0, r3T1, r3T2, r3T3;
    r3T0 = dot(r1T0.xyz,p0T.xyz);
    r3T1 = dot(r1T1.xyz,p0T.xyz);
    r3T2 = dot(r1T2.xyz,p0T.xyz);
    r3T3 = dot(r1T3.xyz,p0T.xyz);
   // transpose back
   r3_x = (float4)(r3T0.x, r3T1.x, r3T2.x, r3T3.x);
   r3_y = (float4)(r3T0.y, r3T1.y, r3T2.y, r3T3.y);
   r3_z = (float4)(r3T0.z, r3T1.z, r3T2.z, r3T3.z);
   r3_w = (float4)(r3T0.w, r3T1.w, r3T2.w, r3T3.w);

    r2_x = max(p1_x, r1_w);
    r2_y = max(p1_x, r1_w);
    r2_z = max(p1_x, r1_w);
    r2_w = max(p1_x, r1_w);

    r0_x = half_recip(r2_x);
    r0_y = half_recip(r2_x);
    r0_z = half_recip(r2_x);
    r0_w = half_recip(r2_x);

    r4_x = r3_x*r0_x;
    r4_y = r3_y*r0_y;
    r4_z = r3_z*r0_z;
    r4_w = r3_w*r0_w;

    r2_x = r1_w*p2_x;
    r2_y = r1_w*p2_y;
    r2_z = r1_w*p2_z;
    r2_w = r1_w*p2_w;

    r0_x = r4_x*p3_x + p4_x;
    r0_y = r4_y*p3_y + p4_y;
    r0_z = r4_z*p3_z + p4_z;
    r0_w = r4_w*p3_w + p4_w;

    r0_x = r0_x*r4_x + p5_x;
    r0_y = r0_y*r4_y + p5_y;
    r0_z = r0_z*r4_z + p5_z;
    r0_w = r0_w*r4_w + p5_w;

    r0_x = r0_x*r4_x + p6_x;
    r0_y = r0_y*r4_y + p6_y;
    r0_z = r0_z*r4_z + p6_z;
    r0_w = r0_w*r4_w + p6_w;

    r2_x = r0_x*r3_x + r2_x;
    r2_y = r0_y*r3_y + r2_y;
    r2_z = r0_z*r3_z + r2_z;
    r2_w = r0_w*r3_w + r2_w;

    r0_x = r1_w*p1_y + -r3_x;
    r0_y = r1_w*p1_y + -r3_y;
    r0_z = r1_w*p1_y + -r3_z;
    r0_w = r1_w*p1_y + -r3_w;

    r2_x = select(r2_w,r2_x, isless(-r0_x, 0.0f));

    r0_x = r3_x*r3_x + -r3_x;
    r0_y = r3_y*r3_y + -r3_y;
    r0_z = r3_z*r3_z + -r3_z;
    r0_w = r3_w*r3_w + -r3_w;

    r0_x = select(r3_x,r2_x, isless(r0_x, 0.0f));
    r0_y = select(r3_y,r2_y, isless(r0_x, 0.0f));
    r0_z = select(r3_z,r2_z, isless(r0_x, 0.0f));
    r0_w = select(r3_w,r2_w, isless(r0_x, 0.0f));

    r0_w = r1_w;

    r0_x = r0_x*l0.x;
    r0_y = r0_y*l0.y;
    r0_z = r0_z*l0.z;
    r0_w = r0_w*l0.w;

    r0_x = mix(r1_x,r0_x, l1.x);
    r0_y = mix(r1_y,r0_y, l1.x);
    r0_z = mix(r1_z,r0_z, l1.x);
    r0_w = mix(r1_w,r0_w, l1.x);

    r0_x = min(r0_x, r0_w);
    r0_y = min(r0_y, r0_w);
    r0_z = min(r0_z, r0_w);

    float4 r0T0, r0T1, r0T2, r0T3;
    r0T0 = (float4)(r0_x.x, r0_y.x, r0_z.x, r0_w.x);
    r0T1 = (float4)(r0_x.y, r0_y.y, r0_z.y, r0_w.y);
    r0T2 = (float4)(r0_x.z, r0_y.z, r0_z.z, r0_w.z);
    r0T3 = (float4)(r0_x.w, r0_y.w, r0_z.w, r0_w.w);
    write_imagef(dest, (int2)( loc_x.x + dim.z , flipped ? get_image_height(dest) - (loc_y.x + dim.w + 1) : loc_y.x + dim.w ), r0T0);
    write_imagef(dest, (int2)( loc_x.y + dim.z , flipped ? get_image_height(dest) - (loc_y.y + dim.w + 1) : loc_y.y + dim.w ), r0T1);
    write_imagef(dest, (int2)( loc_x.z + dim.z , flipped ? get_image_height(dest) - (loc_y.z + dim.w + 1) : loc_y.z + dim.w ), r0T2);
    write_imagef(dest, (int2)( loc_x.w + dim.z , flipped ? get_image_height(dest) - (loc_y.w + dim.w + 1) : loc_y.w + dim.w ), r0T3);
}





__kernel void program(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, float4 l2, float4 l3, read_only image2d_t t0, sampler_t t_sampler0)
{
    const float4 p0  = (float4)( 0x1.b33334p-3, 0x1.6e48e8p-1, 0x1.275254p-4, 0x0p+0 );
    const float4 p1  = (float4)( 0x1.a36e2ep-14, 0x1.5c28f6p-4, 0x0p+0, 0x0p+0 );
    const float4 p2  = (float4)( 0x1.d595dap-11, -0x1.218e3cp-10, -0x1.3ee89ep-11, 0x1.0c7ca6p-5 );
    const float4 p3  = (float4)( -0x1.c2bf7cp+11, 0x1.0efb7cp-3, 0x0p+0, -0x1.97e1fcp-3 );
    const float4 p4  = (float4)( 0x1.62659ep+9, -0x1.8fad94p-2, -0x1.df8f8cp-5, 0x1.52ff12p-1 );
    const float4 p5  = (float4)( -0x1.9777ap+5, 0x1.dca79ap-2, 0x1.070dd8p+0, -0x1.d0565ap-1 );
    const float4 p6  = (float4)( 0x1.8eef1cp+1, 0x1.95d48cp-1, 0x1.07c1b6p-5, 0x1.696ecep+0 );
    int          dest_width = dim.x;
    int          dest_height = dim.y;
    float4       o0, r0, r1, r2, r3, r4, r5;
    float4       false_vector = (float4) 0.0f;
    float4       true_vector = (float4) 1.0f;
    int2         loc = (int2)( get_global_id(0), get_global_id(1) );

    float4 f0 = (float4)( st_origin.x + ((float)loc.x + 0.5f) * st_delta.x + ((float)loc.y + 0.5f) * st_delta.z, 
                            st_origin.y + ((float)loc.x + 0.5f) * st_delta.y + ((float)loc.y + 0.5f) * st_delta.w,  
                            0.0f, 0.0f );

    r2 = f0;
    r0.x = dot(r2.xy,l2.xy) + l2.w;
    r0.y = dot(r2.xy,l3.xy) + l3.w;
    r4 = r0;
    r1 = read_imagef(t0, t_sampler0, r4.xy);
    r3 = dot(r1.xyz,p0.xyz);
    r2 = max(p1.xxxx, r1.wwww);
    r0 = half_recip(r2.xxxx);
    r4 = r3*r0;
    r2 = r1.wwww*p2;
    r0 = r4*p3 + p4;
    r0 = r0*r4 + p5;
    r0 = r0*r4 + p6;
    r2 = r0*r3 + r2;
    r0 = r1.wwww*p1.yyyy + -r3;
    r2.x = select(r2.w,r2.x, isless(-r0.x, 0.0f));
    r0 = r3*r3 + -r3;
    r0 = select(r3,r2, isless(r0.xxxx, 0.0f));
    r0.w = r1.w;
    r0 = r0*l0;
    r0 = mix(r1,r0, l1.xxxx);
    r0.xyz = min(r0.xyz, r0.www);
    o0 = r0;
    write_imagef(dest, (int2)( loc.x + dim.z , flipped ? get_image_height(dest) - (loc.y + dim.w + 1) : loc.y + dim.w ), o0);
}




__kernel void program_trans(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, float4 l2, float4 l3, read_only image2d_t t0, sampler_t t_sampler0)
{
    float4       f0_start, f0_end, f0_delta, tf0_start[4], tf0_delta[4];
    float4       f1_start, f1_end, f1_delta, tf1_start[4], tf1_delta[4];
    float4       gr0_1_0[64], gr0_1_1[64], gr0_1_2[64], gr0_1_3[64];
    const float4 p0  = (float4)( 0x1.b33334p-3, 0x1.b33334p-3, 0x1.b33334p-3, 0x1.b33334p-3 );
    const float4 p1  = (float4)( 0x1.6e48e8p-1, 0x1.6e48e8p-1, 0x1.6e48e8p-1, 0x1.6e48e8p-1 );
    const float4 p2  = (float4)( 0x1.275254p-4, 0x1.275254p-4, 0x1.275254p-4, 0x1.275254p-4 );
    const float4 p3  = (float4)( 0x1.a36e2ep-14, 0x1.a36e2ep-14, 0x1.a36e2ep-14, 0x1.a36e2ep-14 );
    const float4 p4  = (float4)( 0x1.d595dap-11, 0x1.d595dap-11, 0x1.d595dap-11, 0x1.d595dap-11 );
    const float4 p5  = (float4)( -0x1.218e3cp-10, -0x1.218e3cp-10, -0x1.218e3cp-10, -0x1.218e3cp-10 );
    const float4 p6  = (float4)( -0x1.3ee89ep-11, -0x1.3ee89ep-11, -0x1.3ee89ep-11, -0x1.3ee89ep-11 );
    const float4 p7  = (float4)( 0x1.0c7ca6p-5, 0x1.0c7ca6p-5, 0x1.0c7ca6p-5, 0x1.0c7ca6p-5 );
    const float4 p8  = (float4)( -0x1.c2bf7cp+11, -0x1.c2bf7cp+11, -0x1.c2bf7cp+11, -0x1.c2bf7cp+11 );
    const float4 p9  = (float4)( 0x1.62659ep+9, 0x1.62659ep+9, 0x1.62659ep+9, 0x1.62659ep+9 );
    const float4 p10  = (float4)( 0x1.0efb7cp-3, 0x1.0efb7cp-3, 0x1.0efb7cp-3, 0x1.0efb7cp-3 );
    const float4 p11  = (float4)( -0x1.8fad94p-2, -0x1.8fad94p-2, -0x1.8fad94p-2, -0x1.8fad94p-2 );
    const float4 p12  = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p13  = (float4)( -0x1.df8f8cp-5, -0x1.df8f8cp-5, -0x1.df8f8cp-5, -0x1.df8f8cp-5 );
    const float4 p14  = (float4)( -0x1.97e1fcp-3, -0x1.97e1fcp-3, -0x1.97e1fcp-3, -0x1.97e1fcp-3 );
    const float4 p15  = (float4)( 0x1.52ff12p-1, 0x1.52ff12p-1, 0x1.52ff12p-1, 0x1.52ff12p-1 );
    const float4 p16  = (float4)( -0x1.9777ap+5, -0x1.9777ap+5, -0x1.9777ap+5, -0x1.9777ap+5 );
    const float4 p17  = (float4)( 0x1.dca79ap-2, 0x1.dca79ap-2, 0x1.dca79ap-2, 0x1.dca79ap-2 );
    const float4 p18  = (float4)( 0x1.070dd8p+0, 0x1.070dd8p+0, 0x1.070dd8p+0, 0x1.070dd8p+0 );
    const float4 p19  = (float4)( -0x1.d0565ap-1, -0x1.d0565ap-1, -0x1.d0565ap-1, -0x1.d0565ap-1 );
    const float4 p20  = (float4)( 0x1.8eef1cp+1, 0x1.8eef1cp+1, 0x1.8eef1cp+1, 0x1.8eef1cp+1 );
    const float4 p21  = (float4)( 0x1.95d48cp-1, 0x1.95d48cp-1, 0x1.95d48cp-1, 0x1.95d48cp-1 );
    const float4 p22  = (float4)( 0x1.07c1b6p-5, 0x1.07c1b6p-5, 0x1.07c1b6p-5, 0x1.07c1b6p-5 );
    const float4 p23  = (float4)( 0x1.696ecep+0, 0x1.696ecep+0, 0x1.696ecep+0, 0x1.696ecep+0 );
    const float4 p24  = (float4)( -0x1p+0, -0x1p+0, -0x1p+0, -0x1p+0 );
    const float4 p25  = (float4)( 0x1.5c28f6p-4, 0x1.5c28f6p-4, 0x1.5c28f6p-4, 0x1.5c28f6p-4 );
    int          index = 0;
    int          total_index = 0;
    int          write_amount = 0;
    int          read_amount = 256;
    int          write_offset = 0;
    float4       o_r[64], o_g[64], o_b[64], o_a[64];
    int          dest_width = dim.x;
    int          dest_height = dim.y;
    float4       o0, r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, r16, r17, r18, r19, r20, r21, r22, r23, r24, r25, r26, r27, r28, r29, r30, r31, r32, r33, r34, r35, r36, r37, r38, r39, r40, r41, r42, r43, r44, r45, r46, r47, r48, r49, r50, r51, r52, r53, r54, r55, r56, r57, r58, r59, r60, r61, r62, r63, r64, r65, r66, r67, r68, r69, r70, r71, r72, r73, r74, r75, r76, r77, r78, r79, r80, r81, r82, r83, r84, r85, r86, r87, r88, r89, r90, r91, r92, r93, r94, r95, r96, r97, r98, r99, r100, r101, r102, r103, r104, r105, r106, r107, r108, r109, r110;
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

    // vertex start
    f0_start = loc_start;
    r1 = loc_start;
    r0.x = dot(r1.xy,l2.xy) + l2.w;
    r0.y = dot(r1.xy,l3.xy) + l3.w;
    f1_start = r0;

    // vertex end
    f0_end = loc_end;
    r1 = loc_end;
    r0.x = dot(r1.xy,l2.xy) + l2.w;
    r0.y = dot(r1.xy,l3.xy) + l3.w;
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
    for(; loc.x<dest_width ; loc.x+=4)
    {
        r0 = gr0_1_0[index];
        r1 = gr0_1_1[index];
        r2 = gr0_1_2[index];
        r3 = gr0_1_3[index];
        r4 = r0*p0;
        r5 = r1*p1;
        r6 = r4+r5;
        r7 = r2*p2;
        r8 = r6+r7;
        r9 = r8;
        r10 = r8;
        r11 = r8;
        r12 = r8;
        r13 = max(p3, r3);
        r14 = max(p3, r3);
        r15 = max(p3, r3);
        r16 = max(p3, r3);
        r17 = half_recip(r13);
        r18 = half_recip(r13);
        r19 = half_recip(r13);
        r20 = half_recip(r13);
        r21 = r9*r17;
        r22 = r10*r18;
        r23 = r11*r19;
        r24 = r12*r20;
        r25 = r3*p4;
        r26 = r3*p5;
        r27 = r3*p6;
        r28 = r3*p7;
        r29 = r21*p8 + p9;
        r30 = r22*p10 + p11;
        r31 = r23*p12 + p13;
        r32 = r24*p14 + p15;
        r33 = r29*r21 + p16;
        r34 = r30*r22 + p17;
        r35 = r31*r23 + p18;
        r36 = r32*r24 + p19;
        r37 = r33;
        r38 = r34;
        r39 = r35;
        r40 = r36;
        r41 = r37*r21 + p20;
        r42 = r38*r22 + p21;
        r43 = r39*r23 + p22;
        r44 = r40*r24 + p23;
        r45 = r41;
        r46 = r42;
        r47 = r43;
        r48 = r44;
        r49 = r45*r9 + r25;
        r50 = r46*r10 + r26;
        r51 = r47*r11 + r27;
        r52 = r48*r12 + r28;
        r53 = r49;
        r54 = r50;
        r55 = r51;
        r56 = r52;
        r57 = r9*p24;
        r58 = r10*p24;
        r59 = r11*p24;
        r60 = r12*p24;
        r61 = r3*p25 + r57;
        r62 = r3*p25 + r58;
        r63 = r3*p25 + r59;
        r64 = r3*p25 + r60;
        r65 = r61*p24;
        r66 = r62*p24;
        r67 = r63*p24;
        r68 = r64*p24;
        r69 = select(r56,r53, isless(r65, 0.0f));
        r70 = r69;
        r71 = r9*p24;
        r72 = r10*p24;
        r73 = r11*p24;
        r74 = r12*p24;
        r75 = r9*r9 + r71;
        r76 = r10*r10 + r72;
        r77 = r11*r11 + r73;
        r78 = r12*r12 + r74;
        r79 = select(r9,r70, isless(r75, 0.0f));
        r80 = select(r10,r54, isless(r75, 0.0f));
        r81 = select(r11,r55, isless(r75, 0.0f));
        r82 = select(r12,r56, isless(r75, 0.0f));
        r83 = r79;
        r84 = r3;
        r85 = r83*l0.xxxx;
        r86 = r80*l0.yyyy;
        r87 = r81*l0.zzzz;
        r88 = r84*l0.wwww;
        r89 = r85;
        r90 = r86;
        r91 = r87;
        r92 = r88;
        r93 = mix(r0,r89, l1.xxxx);
        r94 = mix(r1,r90, l1.xxxx);
        r95 = mix(r2,r91, l1.xxxx);
        r96 = mix(r3,r92, l1.xxxx);
        r97 = r93;
        r98 = r94;
        r99 = r95;
        r100 = r96;
        r101 = min(r97, r100);
        r102 = min(r98, r100);
        r103 = min(r99, r100);
        r104 = r101;
        r105 = r102;
        r106 = r103;
        r107 = r104;
        r108 = r105;
        r109 = r106;
        r110 = r100;
        o_r[index] = r107;
        o_g[index] = r108;
        o_b[index] = r109;
        o_a[index] = r110;
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
    if (index > 0)
    {
        (void)__async_work_group_stream_to_image(dest, (size_t)(dim.z + write_offset), (size_t)(flipped ? get_image_height(dest) - (loc.y+dim.w+1): loc.y+dim.w), (size_t)(dest_width - write_offset), (const float4 *)o_r, (const float4 *)o_g, (const float4 *)o_b, (const float4 *)o_a);
    }
}
