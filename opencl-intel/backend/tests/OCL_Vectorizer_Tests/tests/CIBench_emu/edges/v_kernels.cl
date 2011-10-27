// Added: improve loop
__kernel void programMany_4(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, float4 l2, read_only image2d_t t0, sampler_t t_sampler0)
{
    const float4 p0_x = (float4)( 0x1p+0, 0x1p+0, 0x1p+0, 0x1p+0 );
    const float4 p0_y = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p0_z = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p0_w = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p1_x = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p1_y = (float4)( 0x1p+0, 0x1p+0, 0x1p+0, 0x1p+0 );
    const float4 p1_z = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p1_w = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );

    float4        r0_x, r1_x, r2_x, r3_x, r4_x, r5_x, r6_x, r7_x;
    float4        r0_y, r1_y, r2_y, r3_y, r4_y, r5_y, r6_y, r7_y;
    float4        r0_z, r1_z, r2_z, r3_z, r4_z, r5_z, r6_z, r7_z;
    float4        r0_w, r1_w, r2_w, r3_w, r4_w, r5_w, r6_w, r7_w;
    float4       f0_x, f0_y, f0_z, f0_w;

    const int Many = 128;
    int         tmp_x = get_global_id(0) * Many;
    int         orig_loc_x = tmp_x;
    int         loc_y = get_global_id(1);
    int4        loc_x;
    int count;
    float4      input0_x[Many/4], input0_y[Many/4], input0_z[Many/4], input0_w[Many/4], output_x[Many/4], output_y[Many/4], output_z[Many/4], output_w[Many/4];
    float4      input1_x[Many/4], input1_y[Many/4], input1_z[Many/4], input1_w[Many/4];
    float4      input2_x[Many/4], input2_y[Many/4], input2_z[Many/4], input2_w[Many/4];
    float4      input3_x[Many/4], input3_y[Many/4], input3_z[Many/4], input3_w[Many/4];

    loc_x = (int4)( tmp_x, tmp_x+1, tmp_x+2, tmp_x+3 );
 
    f0_x =  st_origin.x + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.x + ((float)loc_y + 0.5f) * st_delta.z;
    f0_y =  st_origin.y + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.y + ((float)loc_y + 0.5f) * st_delta.w;
    f0_z =  0.0f;
    f0_w =  0.0f;
    r4_x = f0_x;
    r4_y = f0_y;
    r4_z = f0_z;
    r4_w = f0_w;
    r7_x = r4_x+p0_x;
    r7_y = r4_y+p0_y;
    r7_z = r4_z+p0_z;
    r7_w = r4_w+p0_w;
    r5_x = r4_x-p1_x;
    r5_y = r4_y-p1_y;
    r5_z = r4_z-p1_z;
    r5_w = r4_w-p1_w;
    r6_x = r5_x+p0_x;
    r6_y = r5_y+p0_y;
    r6_z = r5_z+p0_z;
    r6_w = r5_w+p0_w;

    r3_x = r7_x * l1.x + r7_y * l1.y + l1.w;
    r3_y = r7_x * l2.x + r7_y * l2.y + l2.w;
    r2_x = r6_x * l1.x + r6_y * l1.y + l1.w;
    r2_y = r6_x * l2.x + r6_y * l2.y + l2.w;
    r1_x = r5_x * l1.x + r5_y * l1.y + l1.w;
    r1_y = r5_x * l2.x + r5_y * l2.y + l2.w;
    r0_x = r4_x * l1.x + r4_y * l1.y + l1.w;
    r0_y = r4_x * l2.x + r4_y * l2.y + l2.w;

    float2 stride1;
    stride1.x = r1_x.y - r1_x.x;
    stride1.y = r1_y.y - r1_y.x;
    __async_work_group_stream_from_image(t0, t_sampler0, (float2)(r1_x.x, r1_y.x), stride1, Many, input1_x, input1_y, input1_z, input1_w);

    float2 stride2;
    stride2.x = r2_x.y - r2_x.x;
    stride2.y = r2_y.y - r2_y.x;
    __async_work_group_stream_from_image(t0, t_sampler0, (float2)(r2_x.x, r2_y.x), stride2, Many, input2_x, input2_y, input2_z, input2_w);

    float2 stride3;
    stride3.x = r3_x.y - r3_x.x;
    stride3.y = r3_y.y - r3_y.x;
    __async_work_group_stream_from_image(t0, t_sampler0, (float2)(r3_x.x, r3_y.x), stride3, Many, input3_x, input3_y, input3_z, input3_w);

    float2 stride0;
    stride0.x = r0_x.y - r0_x.x;
    stride0.y = r0_y.y - r0_y.x;
    __async_work_group_stream_from_image(t0, t_sampler0, (float2)(r0_x.x, r0_y.x), stride0, Many, input0_x, input0_y, input0_z, input0_w);



  //for (count = 0; count < Many/4; ++count)
  do
  {

    // extract the inputs you currently require
    r1_x = input1_x[count];
    r1_y = input1_y[count];
    r1_z = input1_z[count];
    r1_w = input1_w[count];
    r2_x = input2_x[count];
    r2_y = input2_y[count];
    r2_z = input2_z[count];
    r2_w = input2_w[count];
    r3_x = input3_x[count];
    r3_y = input3_y[count];
    r3_z = input3_z[count];
    r3_w = input3_w[count];
    r0_x = input0_x[count];
    r0_y = input0_y[count];
    r0_z = input0_z[count];
    r0_w = input0_w[count];

    r2_x = r0_x-r2_x;
    r2_y = r0_y-r2_y;
    r2_z = r0_z-r2_z;
    r2_w = r0_w-r2_w;
    r1_x = r1_x-r3_x;
    r1_y = r1_y-r3_y;
    r1_z = r1_z-r3_z;
    r1_w = r1_w-r3_w;
    r2_x = r2_x*r2_x;
    r2_y = r2_y*r2_y;
    r2_z = r2_z*r2_z;
    r2_w = r2_w*r2_w;
    r1_x = r1_x*r1_x + r2_x;
    r1_y = r1_y*r1_y + r2_y;
    r1_z = r1_z*r1_z + r2_z;
    r1_w = r1_w*r1_w + r2_w;
    r0_x = r1_x*l0.x;
    r0_y = r1_y*l0.y;
    r0_z = r1_z*l0.z;

    r0_x = min(r0_x, r0_w);
    r0_y = min(r0_y, r0_w);
    r0_z = min(r0_z, r0_w);

    output_x[count] = r0_x;
    output_y[count] = r0_y;
    output_z[count] = r0_z;
    output_w[count] = r0_w;

    ++count;
  } while (count < Many/4);


    int yaxis;
    int yaxisT, yaxisF;

    yaxisT = get_image_height(dest) - (loc_y + dim.w + 1);
    yaxisF = loc_y + dim.w;
    yaxis = select (yaxisF, yaxisT, flipped);

    __async_work_group_stream_to_image(dest, orig_loc_x + dim.z, yaxis, Many, output_x, output_y, output_z, output_w);

}





// Added: join samplers conditional
__kernel void programMany_3(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, float4 l2, read_only image2d_t t0, sampler_t t_sampler0)
{
    const float4 p0_x = (float4)( 0x1p+0, 0x1p+0, 0x1p+0, 0x1p+0 );
    const float4 p0_y = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p0_z = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p0_w = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p1_x = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p1_y = (float4)( 0x1p+0, 0x1p+0, 0x1p+0, 0x1p+0 );
    const float4 p1_z = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p1_w = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );

    float4        r0_x, r1_x, r2_x, r3_x, r4_x, r5_x, r6_x, r7_x;
    float4        r0_y, r1_y, r2_y, r3_y, r4_y, r5_y, r6_y, r7_y;
    float4        r0_z, r1_z, r2_z, r3_z, r4_z, r5_z, r6_z, r7_z;
    float4        r0_w, r1_w, r2_w, r3_w, r4_w, r5_w, r6_w, r7_w;
    float4       f0_x, f0_y, f0_z, f0_w;

    const int Many = 128;
    int         tmp_x = get_global_id(0) * Many;
    int         orig_loc_x = tmp_x;
    int         loc_y = get_global_id(1);
    int4        loc_x;
    int count;
    float4      input0_x[Many/4], input0_y[Many/4], input0_z[Many/4], input0_w[Many/4], output_x[Many/4], output_y[Many/4], output_z[Many/4], output_w[Many/4];
    float4      input1_x[Many/4], input1_y[Many/4], input1_z[Many/4], input1_w[Many/4];
    float4      input2_x[Many/4], input2_y[Many/4], input2_z[Many/4], input2_w[Many/4];
    float4      input3_x[Many/4], input3_y[Many/4], input3_z[Many/4], input3_w[Many/4];
 
  for (count = 0; count < Many/4; ++count)
  {
    loc_x = (int4)( tmp_x, tmp_x+1, tmp_x+2, tmp_x+3 );

    f0_x =  st_origin.x + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.x + ((float)loc_y + 0.5f) * st_delta.z;
    f0_y =  st_origin.y + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.y + ((float)loc_y + 0.5f) * st_delta.w;
    f0_z =  0.0f;
    f0_w =  0.0f;
    r4_x = f0_x;
    r4_y = f0_y;
    r4_z = f0_z;
    r4_w = f0_w;
    r7_x = r4_x+p0_x;
    r7_y = r4_y+p0_y;
    r7_z = r4_z+p0_z;
    r7_w = r4_w+p0_w;
    r5_x = r4_x-p1_x;
    r5_y = r4_y-p1_y;
    r5_z = r4_z-p1_z;
    r5_w = r4_w-p1_w;
    r6_x = r5_x+p0_x;
    r6_y = r5_y+p0_y;
    r6_z = r5_z+p0_z;
    r6_w = r5_w+p0_w;

    r3_x = r7_x * l1.x + r7_y * l1.y + l1.w;
    r3_y = r7_x * l2.x + r7_y * l2.y + l2.w;
    r2_x = r6_x * l1.x + r6_y * l1.y + l1.w;
    r2_y = r6_x * l2.x + r6_y * l2.y + l2.w;
    r1_x = r5_x * l1.x + r5_y * l1.y + l1.w;
    r1_y = r5_x * l2.x + r5_y * l2.y + l2.w;
    r0_x = r4_x * l1.x + r4_y * l1.y + l1.w;
    r0_y = r4_x * l2.x + r4_y * l2.y + l2.w;


    if (count == 0)
    {
            float2 stride1;
            stride1.x = r1_x.y - r1_x.x;
            stride1.y = r1_y.y - r1_y.x;
            __async_work_group_stream_from_image(t0, t_sampler0, (float2)(r1_x.x, r1_y.x), stride1, Many, input1_x, input1_y, input1_z, input1_w);

            float2 stride2;
            stride2.x = r2_x.y - r2_x.x;
            stride2.y = r2_y.y - r2_y.x;
            __async_work_group_stream_from_image(t0, t_sampler0, (float2)(r2_x.x, r2_y.x), stride2, Many, input2_x, input2_y, input2_z, input2_w);

            float2 stride3;
            stride3.x = r3_x.y - r3_x.x;
            stride3.y = r3_y.y - r3_y.x;
            __async_work_group_stream_from_image(t0, t_sampler0, (float2)(r3_x.x, r3_y.x), stride3, Many, input3_x, input3_y, input3_z, input3_w);

            float2 stride0;
            stride0.x = r0_x.y - r0_x.x;
            stride0.y = r0_y.y - r0_y.x;
            __async_work_group_stream_from_image(t0, t_sampler0, (float2)(r0_x.x, r0_y.x), stride0, Many, input0_x, input0_y, input0_z, input0_w);
    }
    // extract the inputs you currently require


    r1_x = input1_x[count];
    r1_y = input1_y[count];
    r1_z = input1_z[count];
    r1_w = input1_w[count];
    r2_x = input2_x[count];
    r2_y = input2_y[count];
    r2_z = input2_z[count];
    r2_w = input2_w[count];
    r3_x = input3_x[count];
    r3_y = input3_y[count];
    r3_z = input3_z[count];
    r3_w = input3_w[count];
    r0_x = input0_x[count];
    r0_y = input0_y[count];
    r0_z = input0_z[count];
    r0_w = input0_w[count];

    r2_x = r0_x-r2_x;
    r2_y = r0_y-r2_y;
    r2_z = r0_z-r2_z;
    r2_w = r0_w-r2_w;
    r1_x = r1_x-r3_x;
    r1_y = r1_y-r3_y;
    r1_z = r1_z-r3_z;
    r1_w = r1_w-r3_w;
    r2_x = r2_x*r2_x;
    r2_y = r2_y*r2_y;
    r2_z = r2_z*r2_z;
    r2_w = r2_w*r2_w;
    r1_x = r1_x*r1_x + r2_x;
    r1_y = r1_y*r1_y + r2_y;
    r1_z = r1_z*r1_z + r2_z;
    r1_w = r1_w*r1_w + r2_w;
    r0_x = r1_x*l0.x;
    r0_y = r1_y*l0.y;
    r0_z = r1_z*l0.z;

    r0_x = min(r0_x, r0_w);
    r0_y = min(r0_y, r0_w);
    r0_z = min(r0_z, r0_w);

    int yaxis;
    int yaxisT, yaxisF;

    yaxisT = get_image_height(dest) - (loc_y + dim.w + 1);
    yaxisF = loc_y + dim.w;
    yaxis = select (yaxisF, yaxisT, flipped);


    output_x[count] = r0_x;
    output_y[count] = r0_y;
    output_z[count] = r0_z;
    output_w[count] = r0_w;
    if (count == Many/4-1)
    {
        __async_work_group_stream_to_image(dest, orig_loc_x + dim.z, yaxis, Many, output_x, output_y, output_z, output_w);
    }

    tmp_x += 4;
  }
}





// Added: sampler stream
__kernel void programMany_2(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, float4 l2, read_only image2d_t t0, sampler_t t_sampler0)
{
    const float4 p0_x = (float4)( 0x1p+0, 0x1p+0, 0x1p+0, 0x1p+0 );
    const float4 p0_y = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p0_z = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p0_w = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p1_x = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p1_y = (float4)( 0x1p+0, 0x1p+0, 0x1p+0, 0x1p+0 );
    const float4 p1_z = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p1_w = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );

    float4        r0_x, r1_x, r2_x, r3_x, r4_x, r5_x, r6_x, r7_x;
    float4        r0_y, r1_y, r2_y, r3_y, r4_y, r5_y, r6_y, r7_y;
    float4        r0_z, r1_z, r2_z, r3_z, r4_z, r5_z, r6_z, r7_z;
    float4        r0_w, r1_w, r2_w, r3_w, r4_w, r5_w, r6_w, r7_w;
    float4       f0_x, f0_y, f0_z, f0_w;

    const int Many = 128;
    int         tmp_x = get_global_id(0) * Many;
    int         orig_loc_x = tmp_x;
    int         loc_y = get_global_id(1);
    int4        loc_x;
    int count;
    float4      input0_x[Many/4], input0_y[Many/4], input0_z[Many/4], input0_w[Many/4], output_x[Many/4], output_y[Many/4], output_z[Many/4], output_w[Many/4];
    float4      input1_x[Many/4], input1_y[Many/4], input1_z[Many/4], input1_w[Many/4];
    float4      input2_x[Many/4], input2_y[Many/4], input2_z[Many/4], input2_w[Many/4];
    float4      input3_x[Many/4], input3_y[Many/4], input3_z[Many/4], input3_w[Many/4];
 
  for (count = 0; count < Many/4; ++count)
  {
    loc_x = (int4)( tmp_x, tmp_x+1, tmp_x+2, tmp_x+3 );

    f0_x =  st_origin.x + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.x + ((float)loc_y + 0.5f) * st_delta.z;
    f0_y =  st_origin.y + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.y + ((float)loc_y + 0.5f) * st_delta.w;
    f0_z =  0.0f;
    f0_w =  0.0f;
    r4_x = f0_x;
    r4_y = f0_y;
    r4_z = f0_z;
    r4_w = f0_w;
    r7_x = r4_x+p0_x;
    r7_y = r4_y+p0_y;
    r7_z = r4_z+p0_z;
    r7_w = r4_w+p0_w;
    r5_x = r4_x-p1_x;
    r5_y = r4_y-p1_y;
    r5_z = r4_z-p1_z;
    r5_w = r4_w-p1_w;
    r6_x = r5_x+p0_x;
    r6_y = r5_y+p0_y;
    r6_z = r5_z+p0_z;
    r6_w = r5_w+p0_w;

    r3_x = r7_x * l1.x + r7_y * l1.y + l1.w;
    r3_y = r7_x * l2.x + r7_y * l2.y + l2.w;
    r2_x = r6_x * l1.x + r6_y * l1.y + l1.w;
    r2_y = r6_x * l2.x + r6_y * l2.y + l2.w;
    r1_x = r5_x * l1.x + r5_y * l1.y + l1.w;
    r1_y = r5_x * l2.x + r5_y * l2.y + l2.w;

    if (count == 0)
    {
            float2 stride;
            stride.x = r1_x.y - r1_x.x;
            stride.y = r1_y.y - r1_y.x;
            __async_work_group_stream_from_image(t0, t_sampler0, (float2)(r1_x.x, r1_y.x), stride, Many, input1_x, input1_y, input1_z, input1_w);
    }
    // extract the inputs you currently require
    r1_x = input1_x[count];
    r1_y = input1_y[count];
    r1_z = input1_z[count];
    r1_w = input1_w[count];

    if (count == 0)
    {
            float2 stride;
            stride.x = r2_x.y - r2_x.x;
            stride.y = r2_y.y - r2_y.x;
            __async_work_group_stream_from_image(t0, t_sampler0, (float2)(r2_x.x, r2_y.x), stride, Many, input2_x, input2_y, input2_z, input2_w);
    }
    // extract the inputs you currently require
    r2_x = input2_x[count];
    r2_y = input2_y[count];
    r2_z = input2_z[count];
    r2_w = input2_w[count];

    if (count == 0)
    {
            float2 stride;
            stride.x = r3_x.y - r3_x.x;
            stride.y = r3_y.y - r3_y.x;
            __async_work_group_stream_from_image(t0, t_sampler0, (float2)(r3_x.x, r3_y.x), stride, Many, input3_x, input3_y, input3_z, input3_w);
    }
    // extract the inputs you currently require
    r3_x = input3_x[count];
    r3_y = input3_y[count];
    r3_z = input3_z[count];
    r3_w = input3_w[count];

    r0_x = r4_x * l1.x + r4_y * l1.y + l1.w;
    r0_y = r4_x * l2.x + r4_y * l2.y + l2.w;

    if (count == 0)
    {
            float2 stride;
            stride.x = r0_x.y - r0_x.x;
            stride.y = r0_y.y - r0_y.x;
            __async_work_group_stream_from_image(t0, t_sampler0, (float2)(r0_x.x, r0_y.x), stride, Many, input0_x, input0_y, input0_z, input0_w);
    }
    // extract the inputs you currently require
    r0_x = input0_x[count];
    r0_y = input0_y[count];
    r0_z = input0_z[count];
    r0_w = input0_w[count];

    r2_x = r0_x-r2_x;
    r2_y = r0_y-r2_y;
    r2_z = r0_z-r2_z;
    r2_w = r0_w-r2_w;
    r1_x = r1_x-r3_x;
    r1_y = r1_y-r3_y;
    r1_z = r1_z-r3_z;
    r1_w = r1_w-r3_w;
    r2_x = r2_x*r2_x;
    r2_y = r2_y*r2_y;
    r2_z = r2_z*r2_z;
    r2_w = r2_w*r2_w;
    r1_x = r1_x*r1_x + r2_x;
    r1_y = r1_y*r1_y + r2_y;
    r1_z = r1_z*r1_z + r2_z;
    r1_w = r1_w*r1_w + r2_w;
    r0_x = r1_x*l0.x;
    r0_y = r1_y*l0.y;
    r0_z = r1_z*l0.z;

    r0_x = min(r0_x, r0_w);
    r0_y = min(r0_y, r0_w);
    r0_z = min(r0_z, r0_w);

    int yaxis;
    int yaxisT, yaxisF;

    yaxisT = get_image_height(dest) - (loc_y + dim.w + 1);
    yaxisF = loc_y + dim.w;
    yaxis = select (yaxisF, yaxisT, flipped);


    output_x[count] = r0_x;
    output_y[count] = r0_y;
    output_z[count] = r0_z;
    output_w[count] = r0_w;
    if (count == Many/4-1)
    {
        __async_work_group_stream_to_image(dest, orig_loc_x + dim.z, yaxis, Many, output_x, output_y, output_z, output_w);
    }

    tmp_x += 4;
  }
}





// Added: sampler transposing
__kernel void programMany(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, float4 l2, read_only image2d_t t0, sampler_t t_sampler0)
{
    const float4 p0_x = (float4)( 0x1p+0, 0x1p+0, 0x1p+0, 0x1p+0 );
    const float4 p0_y = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p0_z = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p0_w = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p1_x = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p1_y = (float4)( 0x1p+0, 0x1p+0, 0x1p+0, 0x1p+0 );
    const float4 p1_z = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p1_w = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );

    float4        r0_x, r1_x, r2_x, r3_x, r4_x, r5_x, r6_x, r7_x;
    float4        r0_y, r1_y, r2_y, r3_y, r4_y, r5_y, r6_y, r7_y;
    float4        r0_z, r1_z, r2_z, r3_z, r4_z, r5_z, r6_z, r7_z;
    float4        r0_w, r1_w, r2_w, r3_w, r4_w, r5_w, r6_w, r7_w;
    float4       f0_x, f0_y, f0_z, f0_w;

    const int Many = 128;
    int         tmp_x = get_global_id(0) * Many;
    int         orig_loc_x = tmp_x;
    int         loc_y = get_global_id(1);
    int4        loc_x;
    int count;
 
  for (count = 0; count < Many/4; ++count)
  {
    loc_x = (int4)( tmp_x, tmp_x+1, tmp_x+2, tmp_x+3 );

    f0_x =  st_origin.x + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.x + ((float)loc_y + 0.5f) * st_delta.z;
    f0_y =  st_origin.y + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.y + ((float)loc_y + 0.5f) * st_delta.w;
    f0_z =  0.0f;
    f0_w =  0.0f;
    r4_x = f0_x;
    r4_y = f0_y;
    r4_z = f0_z;
    r4_w = f0_w;
    r7_x = r4_x+p0_x;
    r7_y = r4_y+p0_y;
    r7_z = r4_z+p0_z;
    r7_w = r4_w+p0_w;
    r5_x = r4_x-p1_x;
    r5_y = r4_y-p1_y;
    r5_z = r4_z-p1_z;
    r5_w = r4_w-p1_w;
    r6_x = r5_x+p0_x;
    r6_y = r5_y+p0_y;
    r6_z = r5_z+p0_z;
    r6_w = r5_w+p0_w;

    r3_x = r7_x * l1.x + r7_y * l1.y + l1.w;
    r3_y = r7_x * l2.x + r7_y * l2.y + l2.w;
    r2_x = r6_x * l1.x + r6_y * l1.y + l1.w;
    r2_y = r6_x * l2.x + r6_y * l2.y + l2.w;
    r1_x = r5_x * l1.x + r5_y * l1.y + l1.w;
    r1_y = r5_x * l2.x + r5_y * l2.y + l2.w;

    read_transposed_imagef(t0, t_sampler0, r1_x, r1_y, &r1_x, &r1_y, &r1_z, &r1_w);
    read_transposed_imagef(t0, t_sampler0, r2_x, r2_y, &r2_x, &r2_y, &r2_z, &r2_w);
    read_transposed_imagef(t0, t_sampler0, r3_x, r3_y, &r3_x, &r3_y, &r3_z, &r3_w);

    r0_x = r4_x * l1.x + r4_y * l1.y + l1.w;
    r0_y = r4_x * l2.x + r4_y * l2.y + l2.w;

    read_transposed_imagef(t0, t_sampler0, r0_x, r0_y, &r0_x, &r0_y, &r0_z, &r0_w);

    r2_x = r0_x-r2_x;
    r2_y = r0_y-r2_y;
    r2_z = r0_z-r2_z;
    r2_w = r0_w-r2_w;
    r1_x = r1_x-r3_x;
    r1_y = r1_y-r3_y;
    r1_z = r1_z-r3_z;
    r1_w = r1_w-r3_w;
    r2_x = r2_x*r2_x;
    r2_y = r2_y*r2_y;
    r2_z = r2_z*r2_z;
    r2_w = r2_w*r2_w;
    r1_x = r1_x*r1_x + r2_x;
    r1_y = r1_y*r1_y + r2_y;
    r1_z = r1_z*r1_z + r2_z;
    r1_w = r1_w*r1_w + r2_w;
    r0_x = r1_x*l0.x;
    r0_y = r1_y*l0.y;
    r0_z = r1_z*l0.z;

    r0_x = min(r0_x, r0_w);
    r0_y = min(r0_y, r0_w);
    r0_z = min(r0_z, r0_w);

    int yaxis;
    int yaxisT, yaxisF;

    yaxisT = get_image_height(dest) - (loc_y + dim.w + 1);
    yaxisF = loc_y + dim.w;
    yaxis = select (yaxisF, yaxisT, flipped);

    write_transposed_imagef(dest, loc_x.x + dim.z, yaxis, r0_x, r0_y, r0_z, r0_w);
    tmp_x += 4;
  }
}




// Added: sampler transposing
__kernel void program4_3(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, float4 l2, read_only image2d_t t0, sampler_t t_sampler0)
{
    const float4 p0_x = (float4)( 0x1p+0, 0x1p+0, 0x1p+0, 0x1p+0 );
    const float4 p0_y = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p0_z = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p0_w = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p1_x = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p1_y = (float4)( 0x1p+0, 0x1p+0, 0x1p+0, 0x1p+0 );
    const float4 p1_z = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p1_w = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );

    float4        r0_x, r1_x, r2_x, r3_x, r4_x, r5_x, r6_x, r7_x;
    float4        r0_y, r1_y, r2_y, r3_y, r4_y, r5_y, r6_y, r7_y;
    float4        r0_z, r1_z, r2_z, r3_z, r4_z, r5_z, r6_z, r7_z;
    float4        r0_w, r1_w, r2_w, r3_w, r4_w, r5_w, r6_w, r7_w;
    float4       f0_x, f0_y, f0_z, f0_w;

    int         tmp_x = get_global_id(0) * 4;
    int         loc_y = get_global_id(1);
    int4         loc_x = (int4)( tmp_x, tmp_x+1, tmp_x+2, tmp_x+3 );

    f0_x =  st_origin.x + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.x + ((float)loc_y + 0.5f) * st_delta.z;
    f0_y =  st_origin.y + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.y + ((float)loc_y + 0.5f) * st_delta.w;
    f0_z =  0.0f;
    f0_w =  0.0f;
    r4_x = f0_x;
    r4_y = f0_y;
    r4_z = f0_z;
    r4_w = f0_w;
    r7_x = r4_x+p0_x;
    r7_y = r4_y+p0_y;
    r7_z = r4_z+p0_z;
    r7_w = r4_w+p0_w;
    r5_x = r4_x-p1_x;
    r5_y = r4_y-p1_y;
    r5_z = r4_z-p1_z;
    r5_w = r4_w-p1_w;
    r6_x = r5_x+p0_x;
    r6_y = r5_y+p0_y;
    r6_z = r5_z+p0_z;
    r6_w = r5_w+p0_w;

    r3_x = r7_x * l1.x + r7_y * l1.y + l1.w;
    r3_y = r7_x * l2.x + r7_y * l2.y + l2.w;
    r2_x = r6_x * l1.x + r6_y * l1.y + l1.w;
    r2_y = r6_x * l2.x + r6_y * l2.y + l2.w;
    r1_x = r5_x * l1.x + r5_y * l1.y + l1.w;
    r1_y = r5_x * l2.x + r5_y * l2.y + l2.w;

    read_transposed_imagef(t0, t_sampler0, r1_x, r1_y, &r1_x, &r1_y, &r1_z, &r1_w);
    read_transposed_imagef(t0, t_sampler0, r2_x, r2_y, &r2_x, &r2_y, &r2_z, &r2_w);
    read_transposed_imagef(t0, t_sampler0, r3_x, r3_y, &r3_x, &r3_y, &r3_z, &r3_w);

    r0_x = r4_x * l1.x + r4_y * l1.y + l1.w;
    r0_y = r4_x * l2.x + r4_y * l2.y + l2.w;

    read_transposed_imagef(t0, t_sampler0, r0_x, r0_y, &r0_x, &r0_y, &r0_z, &r0_w);

    r2_x = r0_x-r2_x;
    r2_y = r0_y-r2_y;
    r2_z = r0_z-r2_z;
    r2_w = r0_w-r2_w;
    r1_x = r1_x-r3_x;
    r1_y = r1_y-r3_y;
    r1_z = r1_z-r3_z;
    r1_w = r1_w-r3_w;
    r2_x = r2_x*r2_x;
    r2_y = r2_y*r2_y;
    r2_z = r2_z*r2_z;
    r2_w = r2_w*r2_w;
    r1_x = r1_x*r1_x + r2_x;
    r1_y = r1_y*r1_y + r2_y;
    r1_z = r1_z*r1_z + r2_z;
    r1_w = r1_w*r1_w + r2_w;
    r0_x = r1_x*l0.x;
    r0_y = r1_y*l0.y;
    r0_z = r1_z*l0.z;

    r0_x = min(r0_x, r0_w);
    r0_y = min(r0_y, r0_w);
    r0_z = min(r0_z, r0_w);

    int yaxis;
    int yaxisT, yaxisF;

    yaxisT = get_image_height(dest) - (loc_y + dim.w + 1);
    yaxisF = loc_y + dim.w;
    yaxis = select (yaxisF, yaxisT, flipped);

    write_transposed_imagef(dest, loc_x.x + dim.z, yaxis, r0_x, r0_y, r0_z, r0_w);
}





// Added: dot product transposing
__kernel void program4_2(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, float4 l2, read_only image2d_t t0, sampler_t t_sampler0)
{
    const float4 p0_x = (float4)( 0x1p+0, 0x1p+0, 0x1p+0, 0x1p+0 );
    const float4 p0_y = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p0_z = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p0_w = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p1_x = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p1_y = (float4)( 0x1p+0, 0x1p+0, 0x1p+0, 0x1p+0 );
    const float4 p1_z = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p1_w = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );

    float4        r0_x, r1_x, r2_x, r3_x, r4_x, r5_x, r6_x, r7_x;
    float4        r0_y, r1_y, r2_y, r3_y, r4_y, r5_y, r6_y, r7_y;
    float4        r0_z, r1_z, r2_z, r3_z, r4_z, r5_z, r6_z, r7_z;
    float4        r0_w, r1_w, r2_w, r3_w, r4_w, r5_w, r6_w, r7_w;
    float4       f0_x, f0_y, f0_z, f0_w;

    int         tmp_x = get_global_id(0) * 4;
    int         loc_y = get_global_id(1);
    int4         loc_x = (int4)( tmp_x, tmp_x+1, tmp_x+2, tmp_x+3 );

    f0_x =  st_origin.x + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.x + ((float)loc_y + 0.5f) * st_delta.z;
    f0_y =  st_origin.y + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.y + ((float)loc_y + 0.5f) * st_delta.w;
    f0_z =  0.0f;
    f0_w =  0.0f;
    r4_x = f0_x;
    r4_y = f0_y;
    r4_z = f0_z;
    r4_w = f0_w;
    r7_x = r4_x+p0_x;
    r7_y = r4_y+p0_y;
    r7_z = r4_z+p0_z;
    r7_w = r4_w+p0_w;
    r5_x = r4_x-p1_x;
    r5_y = r4_y-p1_y;
    r5_z = r4_z-p1_z;
    r5_w = r4_w-p1_w;
    r6_x = r5_x+p0_x;
    r6_y = r5_y+p0_y;
    r6_z = r5_z+p0_z;
    r6_w = r5_w+p0_w;

    r3_x = r7_x * l1.x + r7_y * l1.y + l1.w;
    r3_y = r7_x * l2.x + r7_y * l2.y + l2.w;
    r2_x = r6_x * l1.x + r6_y * l1.y + l1.w;
    r2_y = r6_x * l2.x + r6_y * l2.y + l2.w;
    r1_x = r5_x * l1.x + r5_y * l1.y + l1.w;
    r1_y = r5_x * l2.x + r5_y * l2.y + l2.w;

    float4 r1T_x = read_imagef(t0, t_sampler0, (float2)(r1_x.x,r1_y.x));
    float4 r1T_y = read_imagef(t0, t_sampler0, (float2)(r1_x.y,r1_y.y));
    float4 r1T_z = read_imagef(t0, t_sampler0, (float2)(r1_x.z,r1_y.z));
    float4 r1T_w = read_imagef(t0, t_sampler0, (float2)(r1_x.w,r1_y.w));
    // transpose
    r1_x = (float4)(r1T_x.x, r1T_y.x, r1T_z.x, r1T_w.x);
    r1_y = (float4)(r1T_x.y, r1T_y.y, r1T_z.y, r1T_w.y);
    r1_z = (float4)(r1T_x.z, r1T_y.z, r1T_z.z, r1T_w.z);
    r1_w = (float4)(r1T_x.w, r1T_y.w, r1T_z.w, r1T_w.w);

    float4 r2T_x = read_imagef(t0, t_sampler0, (float2)(r2_x.x,r2_y.x));
    float4 r2T_y = read_imagef(t0, t_sampler0, (float2)(r2_x.y,r2_y.y));
    float4 r2T_z = read_imagef(t0, t_sampler0, (float2)(r2_x.z,r2_y.z));
    float4 r2T_w = read_imagef(t0, t_sampler0, (float2)(r2_x.w,r2_y.w));
    // transpose
    r2_x = (float4)(r2T_x.x, r2T_y.x, r2T_z.x, r2T_w.x);
    r2_y = (float4)(r2T_x.y, r2T_y.y, r2T_z.y, r2T_w.y);
    r2_z = (float4)(r2T_x.z, r2T_y.z, r2T_z.z, r2T_w.z);
    r2_w = (float4)(r2T_x.w, r2T_y.w, r2T_z.w, r2T_w.w);

    float4 r3T_x = read_imagef(t0, t_sampler0, (float2)(r3_x.x,r3_y.x));
    float4 r3T_y = read_imagef(t0, t_sampler0, (float2)(r3_x.y,r3_y.y));
    float4 r3T_z = read_imagef(t0, t_sampler0, (float2)(r3_x.z,r3_y.z));
    float4 r3T_w = read_imagef(t0, t_sampler0, (float2)(r3_x.w,r3_y.w));
    // transpose
    r3_x = (float4)(r3T_x.x, r3T_y.x, r3T_z.x, r3T_w.x);
    r3_y = (float4)(r3T_x.y, r3T_y.y, r3T_z.y, r3T_w.y);
    r3_z = (float4)(r3T_x.z, r3T_y.z, r3T_z.z, r3T_w.z);
    r3_w = (float4)(r3T_x.w, r3T_y.w, r3T_z.w, r3T_w.w);

    r0_x = r4_x * l1.x + r4_y * l1.y + l1.w;
    r0_y = r4_x * l2.x + r4_y * l2.y + l2.w;

    float4 r0T_x = read_imagef(t0, t_sampler0, (float2)(r0_x.x,r0_y.x));
    float4 r0T_y = read_imagef(t0, t_sampler0, (float2)(r0_x.y,r0_y.y));
    float4 r0T_z = read_imagef(t0, t_sampler0, (float2)(r0_x.z,r0_y.z));
    float4 r0T_w = read_imagef(t0, t_sampler0, (float2)(r0_x.w,r0_y.w));
    // transpose
    r0_x = (float4)(r0T_x.x, r0T_y.x, r0T_z.x, r0T_w.x);
    r0_y = (float4)(r0T_x.y, r0T_y.y, r0T_z.y, r0T_w.y);
    r0_z = (float4)(r0T_x.z, r0T_y.z, r0T_z.z, r0T_w.z);
    r0_w = (float4)(r0T_x.w, r0T_y.w, r0T_z.w, r0T_w.w);

    r2_x = r0_x-r2_x;
    r2_y = r0_y-r2_y;
    r2_z = r0_z-r2_z;
    r2_w = r0_w-r2_w;
    r1_x = r1_x-r3_x;
    r1_y = r1_y-r3_y;
    r1_z = r1_z-r3_z;
    r1_w = r1_w-r3_w;
    r2_x = r2_x*r2_x;
    r2_y = r2_y*r2_y;
    r2_z = r2_z*r2_z;
    r2_w = r2_w*r2_w;
    r1_x = r1_x*r1_x + r2_x;
    r1_y = r1_y*r1_y + r2_y;
    r1_z = r1_z*r1_z + r2_z;
    r1_w = r1_w*r1_w + r2_w;
    r0_x = r1_x*l0.x;
    r0_y = r1_y*l0.y;
    r0_z = r1_z*l0.z;

    r0_x = min(r0_x, r0_w);
    r0_y = min(r0_y, r0_w);
    r0_z = min(r0_z, r0_w);

    write_imagef(dest, (int2)( loc_x.x + dim.z , flipped ? get_image_height(dest) - (loc_y + dim.w + 1) : loc_y + dim.w ), (float4)(r0_x.x, r0_y.x, r0_z.x, r0_w.x));
    write_imagef(dest, (int2)( loc_x.y + dim.z , flipped ? get_image_height(dest) - (loc_y + dim.w + 1) : loc_y + dim.w ), (float4)(r0_x.y, r0_y.y, r0_z.y, r0_w.y));
    write_imagef(dest, (int2)( loc_x.z + dim.z , flipped ? get_image_height(dest) - (loc_y + dim.w + 1) : loc_y + dim.w ), (float4)(r0_x.z, r0_y.z, r0_z.z, r0_w.z));
    write_imagef(dest, (int2)( loc_x.w + dim.z , flipped ? get_image_height(dest) - (loc_y + dim.w + 1) : loc_y + dim.w ), (float4)(r0_x.w, r0_y.w, r0_z.w, r0_w.w));
}




// Added: basic(4) vectorization
__kernel void program4(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, float4 l2, read_only image2d_t t0, sampler_t t_sampler0)
{
    const float4 p0_x = (float4)( 0x1p+0, 0x1p+0, 0x1p+0, 0x1p+0 );
    const float4 p0_y = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p0_z = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p0_w = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p1_x = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p1_y = (float4)( 0x1p+0, 0x1p+0, 0x1p+0, 0x1p+0 );
    const float4 p1_z = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    const float4 p1_w = (float4)( 0x0p+0, 0x0p+0, 0x0p+0, 0x0p+0 );

    float4        r0_x, r1_x, r2_x, r3_x, r4_x, r5_x, r6_x, r7_x;
    float4        r0_y, r1_y, r2_y, r3_y, r4_y, r5_y, r6_y, r7_y;
    float4        r0_z, r1_z, r2_z, r3_z, r4_z, r5_z, r6_z, r7_z;
    float4        r0_w, r1_w, r2_w, r3_w, r4_w, r5_w, r6_w, r7_w;
    float4       f0_x, f0_y, f0_z, f0_w;

    int         tmp_x = get_global_id(0) * 4;
    int         loc_y = get_global_id(1);
    int4         loc_x = (int4)( tmp_x, tmp_x+1, tmp_x+2, tmp_x+3 );

    f0_x =  st_origin.x + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.x + ((float)loc_y + 0.5f) * st_delta.z;
    f0_y =  st_origin.y + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.y + ((float)loc_y + 0.5f) * st_delta.w;
    f0_z =  0.0f;
    f0_w =  0.0f;
    r4_x = f0_x;
    r4_y = f0_y;
    r4_z = f0_z;
    r4_w = f0_w;
    r7_x = r4_x+p0_x;
    r7_y = r4_y+p0_y;
    r7_z = r4_z+p0_z;
    r7_w = r4_w+p0_w;
    r5_x = r4_x-p1_x;
    r5_y = r4_y-p1_y;
    r5_z = r4_z-p1_z;
    r5_w = r4_w-p1_w;
    r6_x = r5_x+p0_x;
    r6_y = r5_y+p0_y;
    r6_z = r5_z+p0_z;
    r6_w = r5_w+p0_w;

    r3_x.x = dot((float2)(r7_x.x,r7_y.x),l1.xy) + l1.w;
    r3_x.y = dot((float2)(r7_x.y,r7_y.y),l1.xy) + l1.w;
    r3_x.z = dot((float2)(r7_x.z,r7_y.z),l1.xy) + l1.w;
    r3_x.w = dot((float2)(r7_x.w,r7_y.w),l1.xy) + l1.w;

    r3_y.x = dot((float2)(r7_x.x,r7_y.x),l2.xy) + l2.w;
    r3_y.y = dot((float2)(r7_x.y,r7_y.y),l2.xy) + l2.w;
    r3_y.z = dot((float2)(r7_x.z,r7_y.z),l2.xy) + l2.w;
    r3_y.w = dot((float2)(r7_x.w,r7_y.w),l2.xy) + l2.w;

    r2_x.x = dot((float2)(r6_x.x,r6_y.x),l1.xy) + l1.w;
    r2_x.y = dot((float2)(r6_x.y,r6_y.y),l1.xy) + l1.w;
    r2_x.z = dot((float2)(r6_x.z,r6_y.z),l1.xy) + l1.w;
    r2_x.w = dot((float2)(r6_x.w,r6_y.w),l1.xy) + l1.w;

    r2_y.x = dot((float2)(r6_x.x,r6_y.x),l2.xy) + l2.w;
    r2_y.y = dot((float2)(r6_x.y,r6_y.y),l2.xy) + l2.w;
    r2_y.z = dot((float2)(r6_x.z,r6_y.z),l2.xy) + l2.w;
    r2_y.w = dot((float2)(r6_x.w,r6_y.w),l2.xy) + l2.w;

    r1_x.x = dot((float2)(r5_x.x,r5_y.x),l1.xy) + l1.w;
    r1_x.y = dot((float2)(r5_x.y,r5_y.y),l1.xy) + l1.w;
    r1_x.z = dot((float2)(r5_x.z,r5_y.z),l1.xy) + l1.w;
    r1_x.w = dot((float2)(r5_x.w,r5_y.w),l1.xy) + l1.w;

    r1_y.x = dot((float2)(r5_x.x,r5_y.x),l2.xy) + l2.w;
    r1_y.y = dot((float2)(r5_x.y,r5_y.y),l2.xy) + l2.w;
    r1_y.z = dot((float2)(r5_x.z,r5_y.z),l2.xy) + l2.w;
    r1_y.w = dot((float2)(r5_x.w,r5_y.w),l2.xy) + l2.w;

    float4 r1T_x = read_imagef(t0, t_sampler0, (float2)(r1_x.x,r1_y.x));
    float4 r1T_y = read_imagef(t0, t_sampler0, (float2)(r1_x.y,r1_y.y));
    float4 r1T_z = read_imagef(t0, t_sampler0, (float2)(r1_x.z,r1_y.z));
    float4 r1T_w = read_imagef(t0, t_sampler0, (float2)(r1_x.w,r1_y.w));
    // transpose
    r1_x = (float4)(r1T_x.x, r1T_y.x, r1T_z.x, r1T_w.x);
    r1_y = (float4)(r1T_x.y, r1T_y.y, r1T_z.y, r1T_w.y);
    r1_z = (float4)(r1T_x.z, r1T_y.z, r1T_z.z, r1T_w.z);
    r1_w = (float4)(r1T_x.w, r1T_y.w, r1T_z.w, r1T_w.w);

    float4 r2T_x = read_imagef(t0, t_sampler0, (float2)(r2_x.x,r2_y.x));
    float4 r2T_y = read_imagef(t0, t_sampler0, (float2)(r2_x.y,r2_y.y));
    float4 r2T_z = read_imagef(t0, t_sampler0, (float2)(r2_x.z,r2_y.z));
    float4 r2T_w = read_imagef(t0, t_sampler0, (float2)(r2_x.w,r2_y.w));
    // transpose
    r2_x = (float4)(r2T_x.x, r2T_y.x, r2T_z.x, r2T_w.x);
    r2_y = (float4)(r2T_x.y, r2T_y.y, r2T_z.y, r2T_w.y);
    r2_z = (float4)(r2T_x.z, r2T_y.z, r2T_z.z, r2T_w.z);
    r2_w = (float4)(r2T_x.w, r2T_y.w, r2T_z.w, r2T_w.w);

    float4 r3T_x = read_imagef(t0, t_sampler0, (float2)(r3_x.x,r3_y.x));
    float4 r3T_y = read_imagef(t0, t_sampler0, (float2)(r3_x.y,r3_y.y));
    float4 r3T_z = read_imagef(t0, t_sampler0, (float2)(r3_x.z,r3_y.z));
    float4 r3T_w = read_imagef(t0, t_sampler0, (float2)(r3_x.w,r3_y.w));
    // transpose
    r3_x = (float4)(r3T_x.x, r3T_y.x, r3T_z.x, r3T_w.x);
    r3_y = (float4)(r3T_x.y, r3T_y.y, r3T_z.y, r3T_w.y);
    r3_z = (float4)(r3T_x.z, r3T_y.z, r3T_z.z, r3T_w.z);
    r3_w = (float4)(r3T_x.w, r3T_y.w, r3T_z.w, r3T_w.w);

    r0_x.x = dot((float2)(r4_x.x,r4_y.x),l1.xy) + l1.w;
    r0_x.y = dot((float2)(r4_x.y,r4_y.y),l1.xy) + l1.w;
    r0_x.z = dot((float2)(r4_x.z,r4_y.z),l1.xy) + l1.w;
    r0_x.w = dot((float2)(r4_x.w,r4_y.w),l1.xy) + l1.w;

    r0_y.x = dot((float2)(r4_x.x,r4_y.x),l2.xy) + l2.w;
    r0_y.y = dot((float2)(r4_x.y,r4_y.y),l2.xy) + l2.w;
    r0_y.z = dot((float2)(r4_x.z,r4_y.z),l2.xy) + l2.w;
    r0_y.w = dot((float2)(r4_x.w,r4_y.w),l2.xy) + l2.w;

    float4 r0T_x = read_imagef(t0, t_sampler0, (float2)(r0_x.x,r0_y.x));
    float4 r0T_y = read_imagef(t0, t_sampler0, (float2)(r0_x.y,r0_y.y));
    float4 r0T_z = read_imagef(t0, t_sampler0, (float2)(r0_x.z,r0_y.z));
    float4 r0T_w = read_imagef(t0, t_sampler0, (float2)(r0_x.w,r0_y.w));
    // transpose
    r0_x = (float4)(r0T_x.x, r0T_y.x, r0T_z.x, r0T_w.x);
    r0_y = (float4)(r0T_x.y, r0T_y.y, r0T_z.y, r0T_w.y);
    r0_z = (float4)(r0T_x.z, r0T_y.z, r0T_z.z, r0T_w.z);
    r0_w = (float4)(r0T_x.w, r0T_y.w, r0T_z.w, r0T_w.w);

    r2_x = r0_x-r2_x;
    r2_y = r0_y-r2_y;
    r2_z = r0_z-r2_z;
    r2_w = r0_w-r2_w;
    r1_x = r1_x-r3_x;
    r1_y = r1_y-r3_y;
    r1_z = r1_z-r3_z;
    r1_w = r1_w-r3_w;
    r2_x = r2_x*r2_x;
    r2_y = r2_y*r2_y;
    r2_z = r2_z*r2_z;
    r2_w = r2_w*r2_w;
    r1_x = r1_x*r1_x + r2_x;
    r1_y = r1_y*r1_y + r2_y;
    r1_z = r1_z*r1_z + r2_z;
    r1_w = r1_w*r1_w + r2_w;
    r0_x = r1_x*l0.x;
    r0_y = r1_y*l0.y;
    r0_z = r1_z*l0.z;

    r0_x = min(r0_x, r0_w);
    r0_y = min(r0_y, r0_w);
    r0_z = min(r0_z, r0_w);

    write_imagef(dest, (int2)( loc_x.x + dim.z , flipped ? get_image_height(dest) - (loc_y + dim.w + 1) : loc_y + dim.w ), (float4)(r0_x.x, r0_y.x, r0_z.x, r0_w.x));
    write_imagef(dest, (int2)( loc_x.y + dim.z , flipped ? get_image_height(dest) - (loc_y + dim.w + 1) : loc_y + dim.w ), (float4)(r0_x.y, r0_y.y, r0_z.y, r0_w.y));
    write_imagef(dest, (int2)( loc_x.z + dim.z , flipped ? get_image_height(dest) - (loc_y + dim.w + 1) : loc_y + dim.w ), (float4)(r0_x.z, r0_y.z, r0_z.z, r0_w.z));
    write_imagef(dest, (int2)( loc_x.w + dim.z , flipped ? get_image_height(dest) - (loc_y + dim.w + 1) : loc_y + dim.w ), (float4)(r0_x.w, r0_y.w, r0_z.w, r0_w.w));
}




__kernel void program_scalar(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, float4 l2, read_only image2d_t t0, sampler_t t_sampler0)
{
    const float p0_x = (float)( 0x1p+0 );
    const float p0_y = (float)( 0x0p+0 );
    const float p0_z = (float)( 0x0p+0 );
    const float p0_w = (float)( 0x0p+0 );
    const float p1_x = (float)( 0x0p+0 );
    const float p1_y = (float)( 0x1p+0 );
    const float p1_z = (float)( 0x0p+0 );
    const float p1_w = (float)( 0x0p+0 );

    float        r0_x, r1_x, r2_x, r3_x, r4_x, r5_x, r6_x, r7_x;
    float        r0_y, r1_y, r2_y, r3_y, r4_y, r5_y, r6_y, r7_y;
    float        r0_z, r1_z, r2_z, r3_z, r4_z, r5_z, r6_z, r7_z;
    float        r0_w, r1_w, r2_w, r3_w, r4_w, r5_w, r6_w, r7_w;
    float       f0_x, f0_y, f0_z, f0_w;

    int         loc_x = (int)( get_global_id(0) );
    int         loc_y = (int)( get_global_id(1) );

    f0_x =  st_origin.x + ((float)loc_x + 0.5f) * st_delta.x + ((float)loc_y + 0.5f) * st_delta.z;
    f0_y =  st_origin.y + ((float)loc_x + 0.5f) * st_delta.y + ((float)loc_y + 0.5f) * st_delta.w;
    f0_z =  0.0f;
    f0_w =  0.0f;

    r4_x = f0_x;
    r4_y = f0_y;
    r4_z = f0_z;
    r4_w = f0_w;

    r7_x = r4_x+p0_x;
    r7_y = r4_y+p0_y;
    r7_z = r4_z+p0_z;
    r7_w = r4_w+p0_w;

    r5_x = r4_x-p1_x;
    r5_y = r4_y-p1_y;
    r5_z = r4_z-p1_z;
    r5_w = r4_w-p1_w;

    r6_x = r5_x+p0_x;
    r6_y = r5_y+p0_y;
    r6_z = r5_z+p0_z;
    r6_w = r5_w+p0_w;

    r3_x = dot((float2)(r7_x,r7_y),l1.xy) + l1.w;

    r3_y = dot((float2)(r7_x,r7_y),l2.xy) + l2.w;

    r2_x = dot((float2)(r6_x,r6_y),l1.xy) + l1.w;

    r2_y = dot((float2)(r6_x,r6_y),l2.xy) + l2.w;

    r1_x = dot((float2)(r5_x,r5_y),l1.xy) + l1.w;

    r1_y = dot((float2)(r5_x,r5_y),l2.xy) + l2.w;

    float4 r1T = read_imagef(t0, t_sampler0, (float2)(r1_x,r1_y));
    r1_x = r1T.x;
    r1_y = r1T.y;
    r1_z = r1T.z;
    r1_w = r1T.w;

    float4 r2T = read_imagef(t0, t_sampler0, (float2)(r2_x,r2_y));
    r2_x = r2T.x;
    r2_y = r2T.y;
    r2_z = r2T.z;
    r2_w = r2T.w;

    float4 r3T = read_imagef(t0, t_sampler0, (float2)(r3_x,r3_y));
    r3_x = r3T.x;
    r3_y = r3T.y;
    r3_z = r3T.z;
    r3_w = r3T.w;

    r0_x = dot((float2)(r4_x,r4_y),l1.xy) + l1.w;

    r0_y = dot((float2)(r4_x,r4_y),l2.xy) + l2.w;

    float4 r0T = read_imagef(t0, t_sampler0, (float2)(r0_x,r0_y));
    r0_x = r0T.x;
    r0_y = r0T.y;
    r0_z = r0T.z;
    r0_w = r0T.w;

    r2_x = r0_x-r2_x;
    r2_y = r0_y-r2_y;
    r2_z = r0_z-r2_z;
    r2_w = r0_w-r2_w;

    r1_x = r1_x-r3_x;
    r1_y = r1_y-r3_y;
    r1_z = r1_z-r3_z;
    r1_w = r1_w-r3_w;

    r2_x = r2_x*r2_x;
    r2_y = r2_y*r2_y;
    r2_z = r2_z*r2_z;
    r2_w = r2_w*r2_w;

    r1_x = r1_x*r1_x + r2_x;
    r1_y = r1_y*r1_y + r2_y;
    r1_z = r1_z*r1_z + r2_z;
    r1_w = r1_w*r1_w + r2_w;

    r0_x = r1_x*l0.x;
    r0_y = r1_y*l0.y;
    r0_z = r1_z*l0.z;

    r0_x = min(r0_x, r0_w);
    r0_y = min(r0_y, r0_w);
    r0_z = min(r0_z, r0_w);

    write_imagef(dest, (int2)( loc_x + dim.z , flipped ? get_image_height(dest) - (loc_y + dim.w + 1) : loc_y + dim.w ), (float4)(r0_x, r0_y, r0_z, r0_w));
}



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




__kernel void program_trans(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, float4 l2, read_only image2d_t t0, sampler_t t_sampler0)
{
    float4       f0_start, f0_end, f0_delta, tf0_start[4], tf0_delta[4];
    float4       f1_start, f1_end, f1_delta, tf1_start[4], tf1_delta[4];
    float4       f2_start, f2_end, f2_delta, tf2_start[4], tf2_delta[4];
    float4       f3_start, f3_end, f3_delta, tf3_start[4], tf3_delta[4];
    float4       f4_start, f4_end, f4_delta, tf4_start[4], tf4_delta[4];
    float4       gr0_3_0[64], gr0_3_1[64], gr0_3_2[64], gr0_3_3[64];
    float4       gr0_1_0[64], gr0_1_1[64], gr0_1_2[64], gr0_1_3[64];
    float4       gr0_4_0[64], gr0_4_1[64], gr0_4_2[64], gr0_4_3[64];
    float4       gr0_2_0[64], gr0_2_1[64], gr0_2_2[64], gr0_2_3[64];
    int          index = 0;
    int          total_index = 0;
    int          write_amount = 0;
    int          read_amount = 256;
    int          write_offset = 0;
    float4       o_r[64], o_g[64], o_b[64], o_a[64];
    int          dest_width = dim.x;
    int          dest_height = dim.y;
    float4       o0, r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, r16, r17, r18, r19, r20, r21, r22, r23, r24, r25, r26, r27, r28, r29, r30, r31, r32, r33, r34, r35, r36, r37, r38, r39, r40, r41, r42, r43, r44, r45, r46, r47, r48, r49, r50, r51, r52, r53, r54, r55, r56, r57, r58, r59, r60;
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
    r3 = (float4) 0.0f;
    r4 = (float4) 0.0f;
    r5 = (float4) 0.0f;
    r6 = (float4) 0.0f;
    r7 = (float4) 0.0f;

    // vertex start
    f0_start = loc_start;
    r1 = loc_start;
    r7 = r1+(float4)( 0x1p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    r3 = r1-(float4)( 0x0p+0, 0x1p+0, 0x0p+0, 0x0p+0 );
    r5 = r3+(float4)( 0x1p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    r6.x = dot(r7.xy,l1.xy) + l1.w;
    r6.y = dot(r7.xy,l2.xy) + l2.w;
    f1_start = r6;
    r4.x = dot(r5.xy,l1.xy) + l1.w;
    r4.y = dot(r5.xy,l2.xy) + l2.w;
    f2_start = r4;
    r2.x = dot(r3.xy,l1.xy) + l1.w;
    r2.y = dot(r3.xy,l2.xy) + l2.w;
    f3_start = r2;
    r0.x = dot(r1.xy,l1.xy) + l1.w;
    r0.y = dot(r1.xy,l2.xy) + l2.w;
    f4_start = r0;

    // vertex end
    f0_end = loc_end;
    r1 = loc_end;
    r7 = r1+(float4)( 0x1p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    r3 = r1-(float4)( 0x0p+0, 0x1p+0, 0x0p+0, 0x0p+0 );
    r5 = r3+(float4)( 0x1p+0, 0x0p+0, 0x0p+0, 0x0p+0 );
    r6.x = dot(r7.xy,l1.xy) + l1.w;
    r6.y = dot(r7.xy,l2.xy) + l2.w;
    f1_end = r6;
    r4.x = dot(r5.xy,l1.xy) + l1.w;
    r4.y = dot(r5.xy,l2.xy) + l2.w;
    f2_end = r4;
    r2.x = dot(r3.xy,l1.xy) + l1.w;
    r2.y = dot(r3.xy,l2.xy) + l2.w;
    f3_end = r2;
    r0.x = dot(r1.xy,l1.xy) + l1.w;
    r0.y = dot(r1.xy,l2.xy) + l2.w;
    f4_end = r0;

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

    __async_work_group_stream_from_image(t0, t_sampler0, f3_start.xy, f3_delta.xy, read_amount, (float4 *)gr0_3_0, (float4 *)gr0_3_1, (float4 *)gr0_3_2, (float4 *)gr0_3_3);
    __async_work_group_stream_from_image(t0, t_sampler0, f1_start.xy, f1_delta.xy, read_amount, (float4 *)gr0_1_0, (float4 *)gr0_1_1, (float4 *)gr0_1_2, (float4 *)gr0_1_3);
    __async_work_group_stream_from_image(t0, t_sampler0, f4_start.xy, f4_delta.xy, read_amount, (float4 *)gr0_4_0, (float4 *)gr0_4_1, (float4 *)gr0_4_2, (float4 *)gr0_4_3);
    __async_work_group_stream_from_image(t0, t_sampler0, f2_start.xy, f2_delta.xy, read_amount, (float4 *)gr0_2_0, (float4 *)gr0_2_1, (float4 *)gr0_2_2, (float4 *)gr0_2_3);
    for(; loc.x<dest_width ; loc.x+=4)
    {
        r0 = gr0_3_0[index];
        r1 = gr0_3_1[index];
        r2 = gr0_3_2[index];
        r3 = gr0_3_3[index];
        r4 = gr0_2_0[index];
        r5 = gr0_2_1[index];
        r6 = gr0_2_2[index];
        r7 = gr0_2_3[index];
        r8 = gr0_1_0[index];
        r9 = gr0_1_1[index];
        r10 = gr0_1_2[index];
        r11 = gr0_1_3[index];
        r12 = gr0_4_0[index];
        r13 = gr0_4_1[index];
        r14 = gr0_4_2[index];
        r15 = gr0_4_3[index];
        r16 = r12-r4;
        r17 = r13-r5;
        r18 = r14-r6;
        r19 = r15-r7;
        r20 = r16;
        r21 = r17;
        r22 = r18;
        r23 = r19;
        r24 = r0-r8;
        r25 = r1-r9;
        r26 = r2-r10;
        r27 = r3-r11;
        r28 = r24;
        r29 = r25;
        r30 = r26;
        r31 = r27;
        r32 = r20*r20;
        r33 = r21*r21;
        r34 = r22*r22;
        r35 = r23*r23;
        r36 = r32;
        r37 = r33;
        r38 = r34;
        r39 = r35;
        r40 = r28*r28 + r36;
        r41 = r29*r29 + r37;
        r42 = r30*r30 + r38;
        r43 = r31*r31 + r39;
        r44 = r40;
        r45 = r41;
        r46 = r42;
        r47 = r43;
        r48 = r44*l0.xxxx;
        r49 = r45*l0.yyyy;
        r50 = r46*l0.zzzz;
        r51 = min(r48, r15);
        r52 = min(r49, r15);
        r53 = min(r50, r15);
        r54 = r51;
        r55 = r52;
        r56 = r53;
        r57 = r54;
        r58 = r55;
        r59 = r56;
        r60 = r15;
        o_r[index] = r57;
        o_g[index] = r58;
        o_b[index] = r59;
        o_a[index] = r60;
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
            __async_work_group_stream_from_image(t0, t_sampler0, f4_start.xy + ((float)4*total_index) * f4_delta.xy, f4_delta.xy, read_amount, (float4 *)gr0_4_0, (float4 *)gr0_4_1, (float4 *)gr0_4_2, (float4 *)gr0_4_3);
            __async_work_group_stream_from_image(t0, t_sampler0, f2_start.xy + ((float)4*total_index) * f2_delta.xy, f2_delta.xy, read_amount, (float4 *)gr0_2_0, (float4 *)gr0_2_1, (float4 *)gr0_2_2, (float4 *)gr0_2_3);
        }
;
    }
    if (index > 0)
    {
        (void)__async_work_group_stream_to_image(dest, (size_t)(dim.z + write_offset), (size_t)(flipped ? get_image_height(dest) - (loc.y+dim.w+1): loc.y+dim.w), (size_t)(dest_width - write_offset), (const float4 *)o_r, (const float4 *)o_g, (const float4 *)o_b, (const float4 *)o_a);
    }
}

