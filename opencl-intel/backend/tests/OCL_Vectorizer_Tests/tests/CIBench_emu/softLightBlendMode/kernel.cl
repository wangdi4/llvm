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


