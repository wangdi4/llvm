__kernel void program(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, float4 l2, float4 l3, float4 l4, read_only image2d_t t0, sampler_t t_sampler0)
{
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
    r7 = f0;
    r6 = r7*l1.zwxy;
    r7 = r6-l1;
    r0.x = dot(r7.xy,l3.xy) + l3.w;
    r0.y = dot(r7.xy,l4.xy) + l4.w;
    r7 = r0;
    r0 = read_imagef(t0, t_sampler0, r7.xy);
    r7 = r6-l0.zwxy;
    r1.x = dot(r7.xy,l3.xy) + l3.w;
    r1.y = dot(r7.xy,l4.xy) + l4.w;
    r7 = r1;
    r1 = read_imagef(t0, t_sampler0, r7.xy);
    r7 = r6-l0;
    r3.x = dot(r7.xy,l3.xy) + l3.w;
    r3.y = dot(r7.xy,l4.xy) + l4.w;
    r7 = r3;
    r3 = read_imagef(t0, t_sampler0, r7.xy);
    r7 = r6+l0;
    r5.x = dot(r7.xy,l3.xy) + l3.w;
    r5.y = dot(r7.xy,l4.xy) + l4.w;
    r7 = r5;
    r5 = read_imagef(t0, t_sampler0, r7.xy);
    r7 = r6+l0.zwxy;
    r4.x = dot(r7.xy,l3.xy) + l3.w;
    r4.y = dot(r7.xy,l4.xy) + l4.w;
    r4 = read_imagef(t0, t_sampler0, r4.xy);
    r6 = r6+l1;
    r2.x = dot(r6.xy,l3.xy) + l3.w;
    r2.y = dot(r6.xy,l4.xy) + l4.w;
    r2 = read_imagef(t0, t_sampler0, r2.xy);
    r3 = r3+r5;
    r3 = l2.xxxx*r3;
    r1 = r1+r4;
    r1 = l2.yyyy*r1 + r3;
    r0 = r0+r2;
    r0 = l2.zzzz*r0 + r1;
    r0.xyz = min(r0.xyz, r0.www);
    o0 = r0;
    write_imagef(dest, (int2)( loc.x + dim.z , flipped ? get_image_height(dest) - (loc.y + dim.w + 1) : loc.y + dim.w ), o0);
}


