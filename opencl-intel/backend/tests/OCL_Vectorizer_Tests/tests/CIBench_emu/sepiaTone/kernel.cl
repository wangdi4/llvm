
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


