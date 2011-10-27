__kernel void program(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, float4 l2, read_only image2d_t t0, sampler_t t_sampler0)
{
    const float4 p0  = (float4)( 0x1p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p1  = (float4)( 0x0p+0, 0x1p+0, 0x0p+0, 0x0p+0 );
    int          dest_width = dim.x;
    int          dest_height = dim.y;
    float4       o0, r0, r1, r2, r3, r4, r5, r6, r7;
    float4       false_vector = (float4) 0.0f;
    float4       true_vector = (float4) 1.0f;
    float        unused_float1;
    float2       unused_float2;
    __float3_SPI       unused_float3;
    float4       unused_float4;
    int2         loc = (int2)( get_global_id(0), get_global_id(1) );
    float4 f0 = (float4)( st_origin.x + ((float)loc.x + 0.5f) * st_delta.x + ((float)loc.y + 0.5f) * st_delta.z, st_origin.y + ((float)loc.x + 0.5f) * st_delta.y + ((float)loc.y + 0.5f) * st_delta.w, 0.0f, 0.0f );
    r4 = f0;
    r7 = r4+p0;
    r5 = r4-p1;
    r6 = r5+p0;
    r3.x = dot(r7.xy,l1.xy) + l1.w;
    r3.y = dot(r7.xy,l2.xy) + l2.w;
    r2.x = dot(r6.xy,l1.xy) + l1.w;
    r2.y = dot(r6.xy,l2.xy) + l2.w;
    r1.x = dot(r5.xy,l1.xy) + l1.w;
    r1.y = dot(r5.xy,l2.xy) + l2.w;
    r1 = read_imagef(t0, t_sampler0, r1.xy);
    r2 = read_imagef(t0, t_sampler0, r2.xy);
    r3 = read_imagef(t0, t_sampler0, r3.xy);
    r0.x = dot(r4.xy,l1.xy) + l1.w;
    r0.y = dot(r4.xy,l2.xy) + l2.w;
    r0 = read_imagef(t0, t_sampler0, r0.xy);
    r2 = r0-r2;
    r1 = r1-r3;
    r2 = r2*r2;
    r1 = r1*r1 + r2;
    r0.xyz = r1.xyz*l0.xyz;
    r0.xyz = min(r0.xyz, r0.www);
    o0 = r0;
    write_imagef(dest, (int2)( loc.x + dim.z , flipped ? get_image_height(dest) - (loc.y + dim.w + 1) : loc.y + dim.w ), o0);
}


