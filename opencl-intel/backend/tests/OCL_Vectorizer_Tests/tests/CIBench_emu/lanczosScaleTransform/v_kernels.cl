// improved loop
__kernel void programMany_3(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, float4 l2, float4 l3, float4 l4, read_only image2d_t t0, sampler_t t_sampler0)
{
    float4       f0_x, o0_x, r0_x, r1_x, r2_x, r3_x, r4_x, r5_x, r6_x, r7_x;
    float4       f0_y, o0_y, r0_y, r1_y, r2_y, r3_y, r4_y, r5_y, r6_y, r7_y;
    float4       f0_z, o0_z, r0_z, r1_z, r2_z, r3_z, r4_z, r5_z, r6_z, r7_z;
    float4       f0_w, o0_w, r0_w, r1_w, r2_w, r3_w, r4_w, r5_w, r6_w, r7_w;
    const int Many = 128;

    float4     output_x[Many/4], output_y[Many/4], output_z[Many/4], output_w[Many/4];
    float4     input0_x[Many/4], input0_y[Many/4], input0_z[Many/4], input0_w[Many/4];
    float4     input1_x[Many/4], input1_y[Many/4], input1_z[Many/4], input1_w[Many/4];
    float4     input2_x[Many/4], input2_y[Many/4], input2_z[Many/4], input2_w[Many/4];
    float4     input3_x[Many/4], input3_y[Many/4], input3_z[Many/4], input3_w[Many/4];
    float4     input4_x[Many/4], input4_y[Many/4], input4_z[Many/4], input4_w[Many/4];
    float4     input5_x[Many/4], input5_y[Many/4], input5_z[Many/4], input5_w[Many/4];


    int         orig_loc_x = get_global_id(0)*4;
    int tmp = orig_loc_x;
    int4        loc_x;//
    int         loc_y = get_global_id(1);
    int count;

    loc_x = (int4)(tmp, tmp+1, tmp+2, tmp+3);


    f0_x = st_origin.x + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.x + ((float4)loc_y + 0.5f) * st_delta.z;
    f0_y = st_origin.y + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.y + ((float4)loc_y + 0.5f) * st_delta.w;
    f0_z = 0.0f;
    f0_w = 0.0f;

    r7_x = f0_x;
    r7_y = f0_y;
    r7_z = f0_z;
    r7_w = f0_w;

    r6_x = r7_x*l1.z;
    r6_y = r7_y*l1.w;
    r6_z = r7_z*l1.x;
    r6_w = r7_w*l1.y;

    r7_x = r6_x-l1.x;
    r7_y = r6_y-l1.y;
    r7_z = r6_z-l1.z;
    r7_w = r6_w-l1.w;

    r0_x = r7_x*l3.x + r7_y*l3.y + l3.w;
    r0_y = r7_x*l4.x + r7_y*l4.y + l4.w;

    r7_x = r0_x;
    r7_y = r0_y;
    r7_z = r0_z;
    r7_w = r0_w;

  {
    float2 stride;
    stride.x = r7_x.y - r7_x.x;
    stride.y = r7_y.y - r7_y.x;
    __async_work_group_stream_from_image(t0, t_sampler0, (float2)(r7_x.x, r7_y.x), stride, Many, input0_x, input0_y, input0_z, input0_w);
  }

    r7_x = r6_x-l0.z;
    r7_y = r6_y-l0.w;
    r7_z = r6_z-l0.x;
    r7_w = r6_w-l0.y;

    r1_x = r7_x*l3.x + r7_y*l3.y + l3.w;
    r1_y = r7_x*l4.x + r7_y*l4.y + l4.w;

    r7_x = r1_x;
    r7_y = r1_y;
    r7_z = r1_z;
    r7_w = r1_w;

  {
    float2 stride;
    stride.x = r7_x.y - r7_x.x;
    stride.y = r7_y.y - r7_y.x;
    __async_work_group_stream_from_image(t0, t_sampler0, (float2)(r7_x.x, r7_y.x), stride, Many, input1_x, input1_y, input1_z, input1_w);
  }

    r7_x = r6_x-l0.x;
    r7_y = r6_y-l0.y;
    r7_z = r6_z-l0.z;
    r7_w = r6_w-l0.w;

    r3_x = r7_x*l3.x + r7_y*l3.y + l3.w;
    r3_y = r7_x*l4.x + r7_y*l4.y + l4.w;

    r7_x = r3_x;
    r7_y = r3_y;
    r7_z = r3_z;
    r7_w = r3_w;

  {
    float2 stride;
    stride.x = r7_x.y - r7_x.x;
    stride.y = r7_y.y - r7_y.x;
    __async_work_group_stream_from_image(t0, t_sampler0, (float2)(r7_x.x, r7_y.x), stride, Many, input3_x, input3_y, input3_z, input3_w);
  }

    r7_x = r6_x+l0.x;
    r7_y = r6_y+l0.y;
    r7_z = r6_z+l0.z;
    r7_w = r6_w+l0.w;

    r5_x = r7_x*l3.x + r7_y*l3.y + l3.w;
    r5_y = r7_x*l4.x + r7_y*l4.y + l4.w;

    r7_x = r5_x;
    r7_y = r5_y;
    r7_z = r5_z;
    r7_w = r5_w;

  {
    float2 stride;
    stride.x = r7_x.y - r7_x.x;
    stride.y = r7_y.y - r7_y.x;
    __async_work_group_stream_from_image(t0, t_sampler0, (float2)(r7_x.x, r7_y.x), stride, Many, input5_x, input5_y, input5_z, input5_w);
  }

    r7_x = r6_x+l0.z;
    r7_y = r6_y+l0.w;
    r7_z = r6_z+l0.x;
    r7_w = r6_w+l0.y;

    r4_x = r7_x*l3.x + r7_y*l3.y + l3.w;
    r4_y = r7_x*l4.x + r7_y*l4.y + l4.w;

  {
    float2 stride;
    stride.x = r4_x.y - r4_x.x;
    stride.y = r4_y.y - r4_y.x;
    __async_work_group_stream_from_image(t0, t_sampler0, (float2)(r4_x.x, r4_y.x), stride, Many, input4_x, input4_y, input4_z, input4_w);
  }

    r6_x = r6_x+l1.x;
    r6_y = r6_y+l1.y;
    r6_z = r6_z+l1.z;
    r6_w = r6_w+l1.w;

    r2_x = r6_x*l3.x + r6_y*l3.y + l3.w;
    r2_y = r6_x*l4.x + r6_y*l4.y + l4.w;

  {
    float2 stride;
    stride.x = r2_x.y - r2_x.x;
    stride.y = r2_y.y - r2_y.x;
    __async_work_group_stream_from_image(t0, t_sampler0, (float2)(r2_x.x, r2_y.x), stride, Many, input2_x, input2_y, input2_z, input2_w);
  }


  do
  {
    


    r0_x = input0_x[count];
    r0_y = input0_y[count];
    r0_z = input0_z[count];
    r0_w = input0_w[count];

    r1_x = input1_x[count];
    r1_y = input1_y[count];
    r1_z = input1_z[count];
    r1_w = input1_w[count];

    r3_x = input3_x[count];
    r3_y = input3_y[count];
    r3_z = input3_z[count];
    r3_w = input3_w[count];

    r5_x = input5_x[count];
    r5_y = input5_y[count];
    r5_z = input5_z[count];
    r5_w = input5_w[count];

    r4_x = input4_x[count];
    r4_y = input4_y[count];
    r4_z = input4_z[count];
    r4_w = input4_w[count];

    r2_x = input2_x[count];
    r2_y = input2_y[count];
    r2_z = input2_z[count];
    r2_w = input2_w[count];

    r3_x = r3_x+r5_x;
    r3_y = r3_y+r5_y;
    r3_z = r3_z+r5_z;
    r3_w = r3_w+r5_w;

    r3_x = l2.x*r3_x;
    r3_y = l2.x*r3_y;
    r3_z = l2.x*r3_z;
    r3_w = l2.x*r3_w;

    r1_x = r1_x+r4_x;
    r1_y = r1_y+r4_y;
    r1_z = r1_z+r4_z;
    r1_w = r1_w+r4_w;

    r1_x = l2.y*r1_x + r3_x;
    r1_y = l2.y*r1_y + r3_y;
    r1_z = l2.y*r1_z + r3_z;
    r1_w = l2.y*r1_w + r3_w;

    r0_x = r0_x+r2_x;
    r0_y = r0_y+r2_y;
    r0_z = r0_z+r2_z;
    r0_w = r0_w+r2_w;

    r0_x = l2.z*r0_x + r1_x;
    r0_y = l2.z*r0_y + r1_y;
    r0_z = l2.z*r0_z + r1_z;
    r0_w = l2.z*r0_w + r1_w;

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

  } while (count < Many/4);

    int yaxis;
    int yaxisT, yaxisF;
    yaxisT = get_image_height(dest) - (loc_y + dim.w + 1);
    yaxisF = loc_y + dim.w;
    yaxis = select (yaxisF, yaxisT, flipped);

    {
        __async_work_group_stream_to_image(dest, orig_loc_x + dim.z, yaxis, Many, output_x, output_y, output_z, output_w);
    }

}



// stream samplers
__kernel void programMany_2(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, float4 l2, float4 l3, float4 l4, read_only image2d_t t0, sampler_t t_sampler0)
{
    float4       f0_x, o0_x, r0_x, r1_x, r2_x, r3_x, r4_x, r5_x, r6_x, r7_x;
    float4       f0_y, o0_y, r0_y, r1_y, r2_y, r3_y, r4_y, r5_y, r6_y, r7_y;
    float4       f0_z, o0_z, r0_z, r1_z, r2_z, r3_z, r4_z, r5_z, r6_z, r7_z;
    float4       f0_w, o0_w, r0_w, r1_w, r2_w, r3_w, r4_w, r5_w, r6_w, r7_w;
    const int Many = 128;

    float4     output_x[Many/4], output_y[Many/4], output_z[Many/4], output_w[Many/4];
    float4     input0_x[Many/4], input0_y[Many/4], input0_z[Many/4], input0_w[Many/4];
    float4     input1_x[Many/4], input1_y[Many/4], input1_z[Many/4], input1_w[Many/4];
    float4     input2_x[Many/4], input2_y[Many/4], input2_z[Many/4], input2_w[Many/4];
    float4     input3_x[Many/4], input3_y[Many/4], input3_z[Many/4], input3_w[Many/4];
    float4     input4_x[Many/4], input4_y[Many/4], input4_z[Many/4], input4_w[Many/4];
    float4     input5_x[Many/4], input5_y[Many/4], input5_z[Many/4], input5_w[Many/4];


    int         orig_loc_x = get_global_id(0)*4;
    int tmp = orig_loc_x;
    int4        loc_x;//
    int         loc_y = get_global_id(1);
    int count;
  for (count = 0; count < Many/4; ++count)
  {
    
    loc_x = (int4)(tmp, tmp+1, tmp+2, tmp+3);


    f0_x = st_origin.x + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.x + ((float4)loc_y + 0.5f) * st_delta.z;
    f0_y = st_origin.y + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.y + ((float4)loc_y + 0.5f) * st_delta.w;
    f0_z = 0.0f;
    f0_w = 0.0f;

    r7_x = f0_x;
    r7_y = f0_y;
    r7_z = f0_z;
    r7_w = f0_w;

    r6_x = r7_x*l1.z;
    r6_y = r7_y*l1.w;
    r6_z = r7_z*l1.x;
    r6_w = r7_w*l1.y;

    r7_x = r6_x-l1.x;
    r7_y = r6_y-l1.y;
    r7_z = r6_z-l1.z;
    r7_w = r6_w-l1.w;

    r0_x = r7_x*l3.x + r7_y*l3.y + l3.w;
    r0_y = r7_x*l4.x + r7_y*l4.y + l4.w;

    r7_x = r0_x;
    r7_y = r0_y;
    r7_z = r0_z;
    r7_w = r0_w;
  if (count == 0)
  {
    float2 stride;
    stride.x = r7_x.y - r7_x.x;
    stride.y = r7_y.y - r7_y.x;
    __async_work_group_stream_from_image(t0, t_sampler0, (float2)(r7_x.x, r7_y.x), stride, Many, input0_x, input0_y, input0_z, input0_w);
  }
    r0_x = input0_x[count];
    r0_y = input0_y[count];
    r0_z = input0_z[count];
    r0_w = input0_w[count];

    r7_x = r6_x-l0.z;
    r7_y = r6_y-l0.w;
    r7_z = r6_z-l0.x;
    r7_w = r6_w-l0.y;

    r1_x = r7_x*l3.x + r7_y*l3.y + l3.w;
    r1_y = r7_x*l4.x + r7_y*l4.y + l4.w;

    r7_x = r1_x;
    r7_y = r1_y;
    r7_z = r1_z;
    r7_w = r1_w;

  if (count == 0)
  {
    float2 stride;
    stride.x = r7_x.y - r7_x.x;
    stride.y = r7_y.y - r7_y.x;
    __async_work_group_stream_from_image(t0, t_sampler0, (float2)(r7_x.x, r7_y.x), stride, Many, input1_x, input1_y, input1_z, input1_w);
  }
    r1_x = input1_x[count];
    r1_y = input1_y[count];
    r1_z = input1_z[count];
    r1_w = input1_w[count];

    r7_x = r6_x-l0.x;
    r7_y = r6_y-l0.y;
    r7_z = r6_z-l0.z;
    r7_w = r6_w-l0.w;

    r3_x = r7_x*l3.x + r7_y*l3.y + l3.w;
    r3_y = r7_x*l4.x + r7_y*l4.y + l4.w;

    r7_x = r3_x;
    r7_y = r3_y;
    r7_z = r3_z;
    r7_w = r3_w;

  if (count == 0)
  {
    float2 stride;
    stride.x = r7_x.y - r7_x.x;
    stride.y = r7_y.y - r7_y.x;
    __async_work_group_stream_from_image(t0, t_sampler0, (float2)(r7_x.x, r7_y.x), stride, Many, input3_x, input3_y, input3_z, input3_w);
  }
    r3_x = input3_x[count];
    r3_y = input3_y[count];
    r3_z = input3_z[count];
    r3_w = input3_w[count];

    r7_x = r6_x+l0.x;
    r7_y = r6_y+l0.y;
    r7_z = r6_z+l0.z;
    r7_w = r6_w+l0.w;

    r5_x = r7_x*l3.x + r7_y*l3.y + l3.w;
    r5_y = r7_x*l4.x + r7_y*l4.y + l4.w;

    r7_x = r5_x;
    r7_y = r5_y;
    r7_z = r5_z;
    r7_w = r5_w;

  if (count == 0)
  {
    float2 stride;
    stride.x = r7_x.y - r7_x.x;
    stride.y = r7_y.y - r7_y.x;
    __async_work_group_stream_from_image(t0, t_sampler0, (float2)(r7_x.x, r7_y.x), stride, Many, input5_x, input5_y, input5_z, input5_w);
  }
    r5_x = input5_x[count];
    r5_y = input5_y[count];
    r5_z = input5_z[count];
    r5_w = input5_w[count];

    r7_x = r6_x+l0.z;
    r7_y = r6_y+l0.w;
    r7_z = r6_z+l0.x;
    r7_w = r6_w+l0.y;

    r4_x = r7_x*l3.x + r7_y*l3.y + l3.w;
    r4_y = r7_x*l4.x + r7_y*l4.y + l4.w;

  if (count == 0)
  {
    float2 stride;
    stride.x = r4_x.y - r4_x.x;
    stride.y = r4_y.y - r4_y.x;
    __async_work_group_stream_from_image(t0, t_sampler0, (float2)(r4_x.x, r4_y.x), stride, Many, input4_x, input4_y, input4_z, input4_w);
  }
    r4_x = input4_x[count];
    r4_y = input4_y[count];
    r4_z = input4_z[count];
    r4_w = input4_w[count];

    r6_x = r6_x+l1.x;
    r6_y = r6_y+l1.y;
    r6_z = r6_z+l1.z;
    r6_w = r6_w+l1.w;

    r2_x = r6_x*l3.x + r6_y*l3.y + l3.w;
    r2_y = r6_x*l4.x + r6_y*l4.y + l4.w;

  if (count == 0)
  {
    float2 stride;
    stride.x = r2_x.y - r2_x.x;
    stride.y = r2_y.y - r2_y.x;
    __async_work_group_stream_from_image(t0, t_sampler0, (float2)(r2_x.x, r2_y.x), stride, Many, input2_x, input2_y, input2_z, input2_w);
  }
    r2_x = input2_x[count];
    r2_y = input2_y[count];
    r2_z = input2_z[count];
    r2_w = input2_w[count];

    r3_x = r3_x+r5_x;
    r3_y = r3_y+r5_y;
    r3_z = r3_z+r5_z;
    r3_w = r3_w+r5_w;

    r3_x = l2.x*r3_x;
    r3_y = l2.x*r3_y;
    r3_z = l2.x*r3_z;
    r3_w = l2.x*r3_w;

    r1_x = r1_x+r4_x;
    r1_y = r1_y+r4_y;
    r1_z = r1_z+r4_z;
    r1_w = r1_w+r4_w;

    r1_x = l2.y*r1_x + r3_x;
    r1_y = l2.y*r1_y + r3_y;
    r1_z = l2.y*r1_z + r3_z;
    r1_w = l2.y*r1_w + r3_w;

    r0_x = r0_x+r2_x;
    r0_y = r0_y+r2_y;
    r0_z = r0_z+r2_z;
    r0_w = r0_w+r2_w;

    r0_x = l2.z*r0_x + r1_x;
    r0_y = l2.z*r0_y + r1_y;
    r0_z = l2.z*r0_z + r1_z;
    r0_w = l2.z*r0_w + r1_w;

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

    int yaxis;
    int yaxisT, yaxisF;
    yaxisT = get_image_height(dest) - (loc_y + dim.w + 1);
    yaxisF = loc_y + dim.w;
    yaxis = select (yaxisF, yaxisT, flipped);

    if (count == Many/4-1)
    {
        __async_work_group_stream_to_image(dest, orig_loc_x + dim.z, yaxis, Many, output_x, output_y, output_z, output_w);
    }
    tmp += 4;
  }
}





// big loop
__kernel void programMany(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, float4 l2, float4 l3, float4 l4, read_only image2d_t t0, sampler_t t_sampler0)
{
    float4       f0_x, o0_x, r0_x, r1_x, r2_x, r3_x, r4_x, r5_x, r6_x, r7_x;
    float4       f0_y, o0_y, r0_y, r1_y, r2_y, r3_y, r4_y, r5_y, r6_y, r7_y;
    float4       f0_z, o0_z, r0_z, r1_z, r2_z, r3_z, r4_z, r5_z, r6_z, r7_z;
    float4       f0_w, o0_w, r0_w, r1_w, r2_w, r3_w, r4_w, r5_w, r6_w, r7_w;
    const int Many = 128;
    int         tmp = get_global_id(0)*4;
    int4        loc_x;//
    int         loc_y = get_global_id(1);
    int count;
  for (count = 0; count < Many/4; ++count)
  {
    
    loc_x = (int4)(tmp, tmp+1, tmp+2, tmp+3);


    f0_x = st_origin.x + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.x + ((float4)loc_y + 0.5f) * st_delta.z;
    f0_y = st_origin.y + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.y + ((float4)loc_y + 0.5f) * st_delta.w;
    f0_z = 0.0f;
    f0_w = 0.0f;

    r7_x = f0_x;
    r7_y = f0_y;
    r7_z = f0_z;
    r7_w = f0_w;

    r6_x = r7_x*l1.z;
    r6_y = r7_y*l1.w;
    r6_z = r7_z*l1.x;
    r6_w = r7_w*l1.y;

    r7_x = r6_x-l1.x;
    r7_y = r6_y-l1.y;
    r7_z = r6_z-l1.z;
    r7_w = r6_w-l1.w;

    r0_x = r7_x*l3.x + r7_y*l3.y + l3.w;
    r0_y = r7_x*l4.x + r7_y*l4.y + l4.w;

    r7_x = r0_x;
    r7_y = r0_y;
    r7_z = r0_z;
    r7_w = r0_w;

    read_transposed_imagef(t0, t_sampler0, r7_x, r7_y, &r0_x, &r0_y, &r0_z, &r0_w);

    r7_x = r6_x-l0.z;
    r7_y = r6_y-l0.w;
    r7_z = r6_z-l0.x;
    r7_w = r6_w-l0.y;

    r1_x = r7_x*l3.x + r7_y*l3.y + l3.w;
    r1_y = r7_x*l4.x + r7_y*l4.y + l4.w;

    r7_x = r1_x;
    r7_y = r1_y;
    r7_z = r1_z;
    r7_w = r1_w;

    read_transposed_imagef(t0, t_sampler0, r7_x, r7_y, &r1_x, &r1_y, &r1_z, &r1_w);

    r7_x = r6_x-l0.x;
    r7_y = r6_y-l0.y;
    r7_z = r6_z-l0.z;
    r7_w = r6_w-l0.w;

    r3_x = r7_x*l3.x + r7_y*l3.y + l3.w;
    r3_y = r7_x*l4.x + r7_y*l4.y + l4.w;

    r7_x = r3_x;
    r7_y = r3_y;
    r7_z = r3_z;
    r7_w = r3_w;

    read_transposed_imagef(t0, t_sampler0, r7_x, r7_y, &r3_x, &r3_y, &r3_z, &r3_w);

    r7_x = r6_x+l0.x;
    r7_y = r6_y+l0.y;
    r7_z = r6_z+l0.z;
    r7_w = r6_w+l0.w;

    r5_x = r7_x*l3.x + r7_y*l3.y + l3.w;
    r5_y = r7_x*l4.x + r7_y*l4.y + l4.w;

    r7_x = r5_x;
    r7_y = r5_y;
    r7_z = r5_z;
    r7_w = r5_w;

    read_transposed_imagef(t0, t_sampler0, r7_x, r7_y, &r5_x, &r5_y, &r5_z, &r5_w);

    r7_x = r6_x+l0.z;
    r7_y = r6_y+l0.w;
    r7_z = r6_z+l0.x;
    r7_w = r6_w+l0.y;

    r4_x = r7_x*l3.x + r7_y*l3.y + l3.w;
    r4_y = r7_x*l4.x + r7_y*l4.y + l4.w;

    read_transposed_imagef(t0, t_sampler0, r4_x, r4_y, &r4_x, &r4_y, &r4_z, &r4_w);

    r6_x = r6_x+l1.x;
    r6_y = r6_y+l1.y;
    r6_z = r6_z+l1.z;
    r6_w = r6_w+l1.w;

    r2_x = r6_x*l3.x + r6_y*l3.y + l3.w;
    r2_y = r6_x*l4.x + r6_y*l4.y + l4.w;

    read_transposed_imagef(t0, t_sampler0, r2_x, r2_y, &r2_x, &r2_y, &r2_z, &r2_w);

    r3_x = r3_x+r5_x;
    r3_y = r3_y+r5_y;
    r3_z = r3_z+r5_z;
    r3_w = r3_w+r5_w;

    r3_x = l2.x*r3_x;
    r3_y = l2.x*r3_y;
    r3_z = l2.x*r3_z;
    r3_w = l2.x*r3_w;

    r1_x = r1_x+r4_x;
    r1_y = r1_y+r4_y;
    r1_z = r1_z+r4_z;
    r1_w = r1_w+r4_w;

    r1_x = l2.y*r1_x + r3_x;
    r1_y = l2.y*r1_y + r3_y;
    r1_z = l2.y*r1_z + r3_z;
    r1_w = l2.y*r1_w + r3_w;

    r0_x = r0_x+r2_x;
    r0_y = r0_y+r2_y;
    r0_z = r0_z+r2_z;
    r0_w = r0_w+r2_w;

    r0_x = l2.z*r0_x + r1_x;
    r0_y = l2.z*r0_y + r1_y;
    r0_z = l2.z*r0_z + r1_z;
    r0_w = l2.z*r0_w + r1_w;

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





// transposed samplers
__kernel void program4_3(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, float4 l2, float4 l3, float4 l4, read_only image2d_t t0, sampler_t t_sampler0)
{
    float4       f0_x, o0_x, r0_x, r1_x, r2_x, r3_x, r4_x, r5_x, r6_x, r7_x;
    float4       f0_y, o0_y, r0_y, r1_y, r2_y, r3_y, r4_y, r5_y, r6_y, r7_y;
    float4       f0_z, o0_z, r0_z, r1_z, r2_z, r3_z, r4_z, r5_z, r6_z, r7_z;
    float4       f0_w, o0_w, r0_w, r1_w, r2_w, r3_w, r4_w, r5_w, r6_w, r7_w;
    int         tmpx = get_global_id(0)*4;
    int4        loc_x = tmpx + (int4)(0, 1, 2, 3);
    int         loc_y = get_global_id(1);

    f0_x = st_origin.x + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.x + ((float4)loc_y + 0.5f) * st_delta.z;
    f0_y = st_origin.y + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.y + ((float4)loc_y + 0.5f) * st_delta.w;
    f0_z = 0.0f;
    f0_w = 0.0f;

    r7_x = f0_x;
    r7_y = f0_y;
    r7_z = f0_z;
    r7_w = f0_w;

    r6_x = r7_x*l1.z;
    r6_y = r7_y*l1.w;
    r6_z = r7_z*l1.x;
    r6_w = r7_w*l1.y;

    r7_x = r6_x-l1.x;
    r7_y = r6_y-l1.y;
    r7_z = r6_z-l1.z;
    r7_w = r6_w-l1.w;

    r0_x = r7_x*l3.x + r7_y*l3.y + l3.w;
    r0_y = r7_x*l4.x + r7_y*l4.y + l4.w;

    r7_x = r0_x;
    r7_y = r0_y;
    r7_z = r0_z;
    r7_w = r0_w;

    read_transposed_imagef(t0, t_sampler0, r7_x, r7_y, &r0_x, &r0_y, &r0_z, &r0_w);

    r7_x = r6_x-l0.z;
    r7_y = r6_y-l0.w;
    r7_z = r6_z-l0.x;
    r7_w = r6_w-l0.y;

    r1_x = r7_x*l3.x + r7_y*l3.y + l3.w;
    r1_y = r7_x*l4.x + r7_y*l4.y + l4.w;

    r7_x = r1_x;
    r7_y = r1_y;
    r7_z = r1_z;
    r7_w = r1_w;

    read_transposed_imagef(t0, t_sampler0, r7_x, r7_y, &r1_x, &r1_y, &r1_z, &r1_w);

    r7_x = r6_x-l0.x;
    r7_y = r6_y-l0.y;
    r7_z = r6_z-l0.z;
    r7_w = r6_w-l0.w;

    r3_x = r7_x*l3.x + r7_y*l3.y + l3.w;
    r3_y = r7_x*l4.x + r7_y*l4.y + l4.w;

    r7_x = r3_x;
    r7_y = r3_y;
    r7_z = r3_z;
    r7_w = r3_w;

    read_transposed_imagef(t0, t_sampler0, r7_x, r7_y, &r3_x, &r3_y, &r3_z, &r3_w);

    r7_x = r6_x+l0.x;
    r7_y = r6_y+l0.y;
    r7_z = r6_z+l0.z;
    r7_w = r6_w+l0.w;

    r5_x = r7_x*l3.x + r7_y*l3.y + l3.w;
    r5_y = r7_x*l4.x + r7_y*l4.y + l4.w;

    r7_x = r5_x;
    r7_y = r5_y;
    r7_z = r5_z;
    r7_w = r5_w;

    read_transposed_imagef(t0, t_sampler0, r7_x, r7_y, &r5_x, &r5_y, &r5_z, &r5_w);

    r7_x = r6_x+l0.z;
    r7_y = r6_y+l0.w;
    r7_z = r6_z+l0.x;
    r7_w = r6_w+l0.y;

    r4_x = r7_x*l3.x + r7_y*l3.y + l3.w;
    r4_y = r7_x*l4.x + r7_y*l4.y + l4.w;

    read_transposed_imagef(t0, t_sampler0, r4_x, r4_y, &r4_x, &r4_y, &r4_z, &r4_w);

    r6_x = r6_x+l1.x;
    r6_y = r6_y+l1.y;
    r6_z = r6_z+l1.z;
    r6_w = r6_w+l1.w;

    r2_x = r6_x*l3.x + r6_y*l3.y + l3.w;
    r2_y = r6_x*l4.x + r6_y*l4.y + l4.w;

    read_transposed_imagef(t0, t_sampler0, r2_x, r2_y, &r2_x, &r2_y, &r2_z, &r2_w);

    r3_x = r3_x+r5_x;
    r3_y = r3_y+r5_y;
    r3_z = r3_z+r5_z;
    r3_w = r3_w+r5_w;

    r3_x = l2.x*r3_x;
    r3_y = l2.x*r3_y;
    r3_z = l2.x*r3_z;
    r3_w = l2.x*r3_w;

    r1_x = r1_x+r4_x;
    r1_y = r1_y+r4_y;
    r1_z = r1_z+r4_z;
    r1_w = r1_w+r4_w;

    r1_x = l2.y*r1_x + r3_x;
    r1_y = l2.y*r1_y + r3_y;
    r1_z = l2.y*r1_z + r3_z;
    r1_w = l2.y*r1_w + r3_w;

    r0_x = r0_x+r2_x;
    r0_y = r0_y+r2_y;
    r0_z = r0_z+r2_z;
    r0_w = r0_w+r2_w;

    r0_x = l2.z*r0_x + r1_x;
    r0_y = l2.z*r0_y + r1_y;
    r0_z = l2.z*r0_z + r1_z;
    r0_w = l2.z*r0_w + r1_w;

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



// vect dot products
__kernel void program4_2(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, float4 l2, float4 l3, float4 l4, read_only image2d_t t0, sampler_t t_sampler0)
{
    float4       f0_x, o0_x, r0_x, r1_x, r2_x, r3_x, r4_x, r5_x, r6_x, r7_x;
    float4       f0_y, o0_y, r0_y, r1_y, r2_y, r3_y, r4_y, r5_y, r6_y, r7_y;
    float4       f0_z, o0_z, r0_z, r1_z, r2_z, r3_z, r4_z, r5_z, r6_z, r7_z;
    float4       f0_w, o0_w, r0_w, r1_w, r2_w, r3_w, r4_w, r5_w, r6_w, r7_w;
    int         tmpx = get_global_id(0)*4;
    int4        loc_x = tmpx + (int4)(0, 1, 2, 3);
    int         loc_y = get_global_id(1);

    f0_x = st_origin.x + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.x + ((float4)loc_y + 0.5f) * st_delta.z;
    f0_y = st_origin.y + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.y + ((float4)loc_y + 0.5f) * st_delta.w;
    f0_z = 0.0f;
    f0_w = 0.0f;

    r7_x = f0_x;
    r7_y = f0_y;
    r7_z = f0_z;
    r7_w = f0_w;

    r6_x = r7_x*l1.z;
    r6_y = r7_y*l1.w;
    r6_z = r7_z*l1.x;
    r6_w = r7_w*l1.y;

    r7_x = r6_x-l1.x;
    r7_y = r6_y-l1.y;
    r7_z = r6_z-l1.z;
    r7_w = r6_w-l1.w;

    r0_x = r7_x*l3.x + r7_y*l3.y + l3.w;
    r0_y = r7_x*l4.x + r7_y*l4.y + l4.w;

    r7_x = r0_x;
    r7_y = r0_y;
    r7_z = r0_z;
    r7_w = r0_w;

    float4 r0Tx = read_imagef(t0, t_sampler0, (float2)(r7_x.x, r7_y.x));
    float4 r0Ty = read_imagef(t0, t_sampler0, (float2)(r7_x.y, r7_y.y));
    float4 r0Tz = read_imagef(t0, t_sampler0, (float2)(r7_x.z, r7_y.z));
    float4 r0Tw = read_imagef(t0, t_sampler0, (float2)(r7_x.w, r7_y.w));
    r0_x = (float4)(r0Tx.x, r0Ty.x, r0Tz.x, r0Tw.x);
    r0_y = (float4)(r0Tx.y, r0Ty.y, r0Tz.y, r0Tw.y);
    r0_z = (float4)(r0Tx.z, r0Ty.z, r0Tz.z, r0Tw.z);
    r0_w = (float4)(r0Tx.w, r0Ty.w, r0Tz.w, r0Tw.w);

    r7_x = r6_x-l0.z;
    r7_y = r6_y-l0.w;
    r7_z = r6_z-l0.x;
    r7_w = r6_w-l0.y;

    r1_x = r7_x*l3.x + r7_y*l3.y + l3.w;
    r1_y = r7_x*l4.x + r7_y*l4.y + l4.w;

    r7_x = r1_x;
    r7_y = r1_y;
    r7_z = r1_z;
    r7_w = r1_w;

    float4 r1Tx = read_imagef(t0, t_sampler0, (float2)(r7_x.x, r7_y.x));
    float4 r1Ty = read_imagef(t0, t_sampler0, (float2)(r7_x.y, r7_y.y));
    float4 r1Tz = read_imagef(t0, t_sampler0, (float2)(r7_x.z, r7_y.z));
    float4 r1Tw = read_imagef(t0, t_sampler0, (float2)(r7_x.w, r7_y.w));
    r1_x = (float4)(r1Tx.x, r1Ty.x, r1Tz.x, r1Tw.x);
    r1_y = (float4)(r1Tx.y, r1Ty.y, r1Tz.y, r1Tw.y);
    r1_z = (float4)(r1Tx.z, r1Ty.z, r1Tz.z, r1Tw.z);
    r1_w = (float4)(r1Tx.w, r1Ty.w, r1Tz.w, r1Tw.w);

    r7_x = r6_x-l0.x;
    r7_y = r6_y-l0.y;
    r7_z = r6_z-l0.z;
    r7_w = r6_w-l0.w;

    r3_x = r7_x*l3.x + r7_y*l3.y + l3.w;
    r3_y = r7_x*l4.x + r7_y*l4.y + l4.w;

    r7_x = r3_x;
    r7_y = r3_y;
    r7_z = r3_z;
    r7_w = r3_w;

    float4 r3Tx = read_imagef(t0, t_sampler0, (float2)(r7_x.x, r7_y.x));
    float4 r3Ty = read_imagef(t0, t_sampler0, (float2)(r7_x.y, r7_y.y));
    float4 r3Tz = read_imagef(t0, t_sampler0, (float2)(r7_x.z, r7_y.z));
    float4 r3Tw = read_imagef(t0, t_sampler0, (float2)(r7_x.w, r7_y.w));
    r3_x = (float4)(r3Tx.x, r3Ty.x, r3Tz.x, r3Tw.x);
    r3_y = (float4)(r3Tx.y, r3Ty.y, r3Tz.y, r3Tw.y);
    r3_z = (float4)(r3Tx.z, r3Ty.z, r3Tz.z, r3Tw.z);
    r3_w = (float4)(r3Tx.w, r3Ty.w, r3Tz.w, r3Tw.w);

    r7_x = r6_x+l0.x;
    r7_y = r6_y+l0.y;
    r7_z = r6_z+l0.z;
    r7_w = r6_w+l0.w;

    r5_x = r7_x*l3.x + r7_y*l3.y + l3.w;
    r5_y = r7_x*l4.x + r7_y*l4.y + l4.w;

    r7_x = r5_x;
    r7_y = r5_y;
    r7_z = r5_z;
    r7_w = r5_w;

    float4 r5Tx = read_imagef(t0, t_sampler0, (float2)(r7_x.x, r7_y.x));
    float4 r5Ty = read_imagef(t0, t_sampler0, (float2)(r7_x.y, r7_y.y));
    float4 r5Tz = read_imagef(t0, t_sampler0, (float2)(r7_x.z, r7_y.z));
    float4 r5Tw = read_imagef(t0, t_sampler0, (float2)(r7_x.w, r7_y.w));
    r5_x = (float4)(r5Tx.x, r5Ty.x, r5Tz.x, r5Tw.x);
    r5_y = (float4)(r5Tx.y, r5Ty.y, r5Tz.y, r5Tw.y);
    r5_z = (float4)(r5Tx.z, r5Ty.z, r5Tz.z, r5Tw.z);
    r5_w = (float4)(r5Tx.w, r5Ty.w, r5Tz.w, r5Tw.w);

    r7_x = r6_x+l0.z;
    r7_y = r6_y+l0.w;
    r7_z = r6_z+l0.x;
    r7_w = r6_w+l0.y;

    r4_x = r7_x*l3.x + r7_y*l3.y + l3.w;
    r4_y = r7_x*l4.x + r7_y*l4.y + l4.w;

    float4 r4Tx = read_imagef(t0, t_sampler0, (float2)(r4_x.x, r4_y.x));
    float4 r4Ty = read_imagef(t0, t_sampler0, (float2)(r4_x.y, r4_y.y));
    float4 r4Tz = read_imagef(t0, t_sampler0, (float2)(r4_x.z, r4_y.z));
    float4 r4Tw = read_imagef(t0, t_sampler0, (float2)(r4_x.w, r4_y.w));
    r4_x = (float4)(r4Tx.x, r4Ty.x, r4Tz.x, r4Tw.x);
    r4_y = (float4)(r4Tx.y, r4Ty.y, r4Tz.y, r4Tw.y);
    r4_z = (float4)(r4Tx.z, r4Ty.z, r4Tz.z, r4Tw.z);
    r4_w = (float4)(r4Tx.w, r4Ty.w, r4Tz.w, r4Tw.w);

    r6_x = r6_x+l1.x;
    r6_y = r6_y+l1.y;
    r6_z = r6_z+l1.z;
    r6_w = r6_w+l1.w;

    r2_x = r6_x*l3.x + r6_y*l3.y + l3.w;
    r2_y = r6_x*l4.x + r6_y*l4.y + l4.w;

    float4 r2Tx = read_imagef(t0, t_sampler0, (float2)(r2_x.x, r2_y.x));
    float4 r2Ty = read_imagef(t0, t_sampler0, (float2)(r2_x.y, r2_y.y));
    float4 r2Tz = read_imagef(t0, t_sampler0, (float2)(r2_x.z, r2_y.z));
    float4 r2Tw = read_imagef(t0, t_sampler0, (float2)(r2_x.w, r2_y.w));
    r2_x = (float4)(r2Tx.x, r2Ty.x, r2Tz.x, r2Tw.x);
    r2_y = (float4)(r2Tx.y, r2Ty.y, r2Tz.y, r2Tw.y);
    r2_z = (float4)(r2Tx.z, r2Ty.z, r2Tz.z, r2Tw.z);
    r2_w = (float4)(r2Tx.w, r2Ty.w, r2Tz.w, r2Tw.w);

    r3_x = r3_x+r5_x;
    r3_y = r3_y+r5_y;
    r3_z = r3_z+r5_z;
    r3_w = r3_w+r5_w;

    r3_x = l2.x*r3_x;
    r3_y = l2.x*r3_y;
    r3_z = l2.x*r3_z;
    r3_w = l2.x*r3_w;

    r1_x = r1_x+r4_x;
    r1_y = r1_y+r4_y;
    r1_z = r1_z+r4_z;
    r1_w = r1_w+r4_w;

    r1_x = l2.y*r1_x + r3_x;
    r1_y = l2.y*r1_y + r3_y;
    r1_z = l2.y*r1_z + r3_z;
    r1_w = l2.y*r1_w + r3_w;

    r0_x = r0_x+r2_x;
    r0_y = r0_y+r2_y;
    r0_z = r0_z+r2_z;
    r0_w = r0_w+r2_w;

    r0_x = l2.z*r0_x + r1_x;
    r0_y = l2.z*r0_y + r1_y;
    r0_z = l2.z*r0_z + r1_z;
    r0_w = l2.z*r0_w + r1_w;

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




// Simple vectorize by 4
__kernel void program4(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, float4 l2, float4 l3, float4 l4, read_only image2d_t t0, sampler_t t_sampler0)
{
    float4       f0_x, o0_x, r0_x, r1_x, r2_x, r3_x, r4_x, r5_x, r6_x, r7_x;
    float4       f0_y, o0_y, r0_y, r1_y, r2_y, r3_y, r4_y, r5_y, r6_y, r7_y;
    float4       f0_z, o0_z, r0_z, r1_z, r2_z, r3_z, r4_z, r5_z, r6_z, r7_z;
    float4       f0_w, o0_w, r0_w, r1_w, r2_w, r3_w, r4_w, r5_w, r6_w, r7_w;
    int         tmpx = get_global_id(0)*4;
    int4        loc_x = tmpx + (int4)(0, 1, 2, 3);
    int         loc_y = get_global_id(1);

    f0_x = st_origin.x + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.x + ((float4)loc_y + 0.5f) * st_delta.z;
    f0_y = st_origin.y + ((float4)((float)loc_x.x, (float)loc_x.y, (float)loc_x.z, (float)loc_x.w) + 0.5f) * st_delta.y + ((float4)loc_y + 0.5f) * st_delta.w;
    f0_z = 0.0f;
    f0_w = 0.0f;

    r7_x = f0_x;
    r7_y = f0_y;
    r7_z = f0_z;
    r7_w = f0_w;

    r6_x = r7_x*l1.z;
    r6_y = r7_y*l1.w;
    r6_z = r7_z*l1.x;
    r6_w = r7_w*l1.y;

    r7_x = r6_x-l1.x;
    r7_y = r6_y-l1.y;
    r7_z = r6_z-l1.z;
    r7_w = r6_w-l1.w;

    r0_x.x = dot((float2)(r7_x.x, r7_y.x),l3.xy) + l3.w;
    r0_x.y = dot((float2)(r7_x.y, r7_y.y),l3.xy) + l3.w;
    r0_x.z = dot((float2)(r7_x.z, r7_y.z),l3.xy) + l3.w;
    r0_x.w = dot((float2)(r7_x.w, r7_y.w),l3.xy) + l3.w;

    r0_y.x = dot((float2)(r7_x.x, r7_y.x),l4.xy) + l4.w;
    r0_y.y = dot((float2)(r7_x.y, r7_y.y),l4.xy) + l4.w;
    r0_y.z = dot((float2)(r7_x.z, r7_y.z),l4.xy) + l4.w;
    r0_y.w = dot((float2)(r7_x.w, r7_y.w),l4.xy) + l4.w;

    r7_x = r0_x;
    r7_y = r0_y;
    r7_z = r0_z;
    r7_w = r0_w;

    float4 r0Tx = read_imagef(t0, t_sampler0, (float2)(r7_x.x, r7_y.x));
    float4 r0Ty = read_imagef(t0, t_sampler0, (float2)(r7_x.y, r7_y.y));
    float4 r0Tz = read_imagef(t0, t_sampler0, (float2)(r7_x.z, r7_y.z));
    float4 r0Tw = read_imagef(t0, t_sampler0, (float2)(r7_x.w, r7_y.w));
    r0_x = (float4)(r0Tx.x, r0Ty.x, r0Tz.x, r0Tw.x);
    r0_y = (float4)(r0Tx.y, r0Ty.y, r0Tz.y, r0Tw.y);
    r0_z = (float4)(r0Tx.z, r0Ty.z, r0Tz.z, r0Tw.z);
    r0_w = (float4)(r0Tx.w, r0Ty.w, r0Tz.w, r0Tw.w);

    r7_x = r6_x-l0.z;
    r7_y = r6_y-l0.w;
    r7_z = r6_z-l0.x;
    r7_w = r6_w-l0.y;

    r1_x.x = dot((float2)(r7_x.x, r7_y.x),l3.xy) + l3.w;
    r1_x.y = dot((float2)(r7_x.y, r7_y.y),l3.xy) + l3.w;
    r1_x.z = dot((float2)(r7_x.z, r7_y.z),l3.xy) + l3.w;
    r1_x.w = dot((float2)(r7_x.w, r7_y.w),l3.xy) + l3.w;
    r1_y.x = dot((float2)(r7_x.x, r7_y.x),l4.xy) + l4.w;
    r1_y.y = dot((float2)(r7_x.y, r7_y.y),l4.xy) + l4.w;
    r1_y.z = dot((float2)(r7_x.z, r7_y.z),l4.xy) + l4.w;
    r1_y.w = dot((float2)(r7_x.w, r7_y.w),l4.xy) + l4.w;

    r7_x = r1_x;
    r7_y = r1_y;
    r7_z = r1_z;
    r7_w = r1_w;

    float4 r1Tx = read_imagef(t0, t_sampler0, (float2)(r7_x.x, r7_y.x));
    float4 r1Ty = read_imagef(t0, t_sampler0, (float2)(r7_x.y, r7_y.y));
    float4 r1Tz = read_imagef(t0, t_sampler0, (float2)(r7_x.z, r7_y.z));
    float4 r1Tw = read_imagef(t0, t_sampler0, (float2)(r7_x.w, r7_y.w));
    r1_x = (float4)(r1Tx.x, r1Ty.x, r1Tz.x, r1Tw.x);
    r1_y = (float4)(r1Tx.y, r1Ty.y, r1Tz.y, r1Tw.y);
    r1_z = (float4)(r1Tx.z, r1Ty.z, r1Tz.z, r1Tw.z);
    r1_w = (float4)(r1Tx.w, r1Ty.w, r1Tz.w, r1Tw.w);

    r7_x = r6_x-l0.x;
    r7_y = r6_y-l0.y;
    r7_z = r6_z-l0.z;
    r7_w = r6_w-l0.w;

    r3_x.x = dot((float2)(r7_x.x, r7_y.x),l3.xy) + l3.w;
    r3_x.y = dot((float2)(r7_x.y, r7_y.y),l3.xy) + l3.w;
    r3_x.z = dot((float2)(r7_x.z, r7_y.z),l3.xy) + l3.w;
    r3_x.w = dot((float2)(r7_x.w, r7_y.w),l3.xy) + l3.w;
    r3_y.x = dot((float2)(r7_x.x, r7_y.x),l4.xy) + l4.w;
    r3_y.y = dot((float2)(r7_x.y, r7_y.y),l4.xy) + l4.w;
    r3_y.z = dot((float2)(r7_x.z, r7_y.z),l4.xy) + l4.w;
    r3_y.w = dot((float2)(r7_x.w, r7_y.w),l4.xy) + l4.w;

    r7_x = r3_x;
    r7_y = r3_y;
    r7_z = r3_z;
    r7_w = r3_w;

    float4 r3Tx = read_imagef(t0, t_sampler0, (float2)(r7_x.x, r7_y.x));
    float4 r3Ty = read_imagef(t0, t_sampler0, (float2)(r7_x.y, r7_y.y));
    float4 r3Tz = read_imagef(t0, t_sampler0, (float2)(r7_x.z, r7_y.z));
    float4 r3Tw = read_imagef(t0, t_sampler0, (float2)(r7_x.w, r7_y.w));
    r3_x = (float4)(r3Tx.x, r3Ty.x, r3Tz.x, r3Tw.x);
    r3_y = (float4)(r3Tx.y, r3Ty.y, r3Tz.y, r3Tw.y);
    r3_z = (float4)(r3Tx.z, r3Ty.z, r3Tz.z, r3Tw.z);
    r3_w = (float4)(r3Tx.w, r3Ty.w, r3Tz.w, r3Tw.w);

    r7_x = r6_x+l0.x;
    r7_y = r6_y+l0.y;
    r7_z = r6_z+l0.z;
    r7_w = r6_w+l0.w;

    r5_x.x = dot((float2)(r7_x.x, r7_y.x),l3.xy) + l3.w;
    r5_x.y = dot((float2)(r7_x.y, r7_y.y),l3.xy) + l3.w;
    r5_x.z = dot((float2)(r7_x.z, r7_y.z),l3.xy) + l3.w;
    r5_x.w = dot((float2)(r7_x.w, r7_y.w),l3.xy) + l3.w;
    r5_y.x = dot((float2)(r7_x.x, r7_y.x),l4.xy) + l4.w;
    r5_y.y = dot((float2)(r7_x.y, r7_y.y),l4.xy) + l4.w;
    r5_y.z = dot((float2)(r7_x.z, r7_y.z),l4.xy) + l4.w;
    r5_y.w = dot((float2)(r7_x.w, r7_y.w),l4.xy) + l4.w;

    r7_x = r5_x;
    r7_y = r5_y;
    r7_z = r5_z;
    r7_w = r5_w;

    float4 r5Tx = read_imagef(t0, t_sampler0, (float2)(r7_x.x, r7_y.x));
    float4 r5Ty = read_imagef(t0, t_sampler0, (float2)(r7_x.y, r7_y.y));
    float4 r5Tz = read_imagef(t0, t_sampler0, (float2)(r7_x.z, r7_y.z));
    float4 r5Tw = read_imagef(t0, t_sampler0, (float2)(r7_x.w, r7_y.w));
    r5_x = (float4)(r5Tx.x, r5Ty.x, r5Tz.x, r5Tw.x);
    r5_y = (float4)(r5Tx.y, r5Ty.y, r5Tz.y, r5Tw.y);
    r5_z = (float4)(r5Tx.z, r5Ty.z, r5Tz.z, r5Tw.z);
    r5_w = (float4)(r5Tx.w, r5Ty.w, r5Tz.w, r5Tw.w);

    r7_x = r6_x+l0.z;
    r7_y = r6_y+l0.w;
    r7_z = r6_z+l0.x;
    r7_w = r6_w+l0.y;

    r4_x.x = dot((float2)(r7_x.x, r7_y.x),l3.xy) + l3.w;
    r4_x.y = dot((float2)(r7_x.y, r7_y.y),l3.xy) + l3.w;
    r4_x.z = dot((float2)(r7_x.z, r7_y.z),l3.xy) + l3.w;
    r4_x.w = dot((float2)(r7_x.w, r7_y.w),l3.xy) + l3.w;
    r4_y.x = dot((float2)(r7_x.x, r7_y.x),l4.xy) + l4.w;
    r4_y.y = dot((float2)(r7_x.y, r7_y.y),l4.xy) + l4.w;
    r4_y.z = dot((float2)(r7_x.z, r7_y.z),l4.xy) + l4.w;
    r4_y.w = dot((float2)(r7_x.w, r7_y.w),l4.xy) + l4.w;

    float4 r4Tx = read_imagef(t0, t_sampler0, (float2)(r4_x.x, r4_y.x));
    float4 r4Ty = read_imagef(t0, t_sampler0, (float2)(r4_x.y, r4_y.y));
    float4 r4Tz = read_imagef(t0, t_sampler0, (float2)(r4_x.z, r4_y.z));
    float4 r4Tw = read_imagef(t0, t_sampler0, (float2)(r4_x.w, r4_y.w));
    r4_x = (float4)(r4Tx.x, r4Ty.x, r4Tz.x, r4Tw.x);
    r4_y = (float4)(r4Tx.y, r4Ty.y, r4Tz.y, r4Tw.y);
    r4_z = (float4)(r4Tx.z, r4Ty.z, r4Tz.z, r4Tw.z);
    r4_w = (float4)(r4Tx.w, r4Ty.w, r4Tz.w, r4Tw.w);

    r6_x = r6_x+l1.x;
    r6_y = r6_y+l1.y;
    r6_z = r6_z+l1.z;
    r6_w = r6_w+l1.w;

    r2_x.x = dot((float2)(r6_x.x, r6_y.x),l3.xy) + l3.w;
    r2_x.y = dot((float2)(r6_x.y, r6_y.y),l3.xy) + l3.w;
    r2_x.z = dot((float2)(r6_x.z, r6_y.z),l3.xy) + l3.w;
    r2_x.w = dot((float2)(r6_x.w, r6_y.w),l3.xy) + l3.w;
    r2_y.x = dot((float2)(r6_x.x, r6_y.x),l4.xy) + l4.w;
    r2_y.y = dot((float2)(r6_x.y, r6_y.y),l4.xy) + l4.w;
    r2_y.z = dot((float2)(r6_x.z, r6_y.z),l4.xy) + l4.w;
    r2_y.w = dot((float2)(r6_x.w, r6_y.w),l4.xy) + l4.w;

    float4 r2Tx = read_imagef(t0, t_sampler0, (float2)(r2_x.x, r2_y.x));
    float4 r2Ty = read_imagef(t0, t_sampler0, (float2)(r2_x.y, r2_y.y));
    float4 r2Tz = read_imagef(t0, t_sampler0, (float2)(r2_x.z, r2_y.z));
    float4 r2Tw = read_imagef(t0, t_sampler0, (float2)(r2_x.w, r2_y.w));
    r2_x = (float4)(r2Tx.x, r2Ty.x, r2Tz.x, r2Tw.x);
    r2_y = (float4)(r2Tx.y, r2Ty.y, r2Tz.y, r2Tw.y);
    r2_z = (float4)(r2Tx.z, r2Ty.z, r2Tz.z, r2Tw.z);
    r2_w = (float4)(r2Tx.w, r2Ty.w, r2Tz.w, r2Tw.w);

    r3_x = r3_x+r5_x;
    r3_y = r3_y+r5_y;
    r3_z = r3_z+r5_z;
    r3_w = r3_w+r5_w;

    r3_x = l2.x*r3_x;
    r3_y = l2.x*r3_y;
    r3_z = l2.x*r3_z;
    r3_w = l2.x*r3_w;

    r1_x = r1_x+r4_x;
    r1_y = r1_y+r4_y;
    r1_z = r1_z+r4_z;
    r1_w = r1_w+r4_w;

    r1_x = l2.y*r1_x + r3_x;
    r1_y = l2.y*r1_y + r3_y;
    r1_z = l2.y*r1_z + r3_z;
    r1_w = l2.y*r1_w + r3_w;

    r0_x = r0_x+r2_x;
    r0_y = r0_y+r2_y;
    r0_z = r0_z+r2_z;
    r0_w = r0_w+r2_w;

    r0_x = l2.z*r0_x + r1_x;
    r0_y = l2.z*r0_y + r1_y;
    r0_z = l2.z*r0_z + r1_z;
    r0_w = l2.z*r0_w + r1_w;

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





__kernel void programScalar(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, float4 l2, float4 l3, float4 l4, read_only image2d_t t0, sampler_t t_sampler0)
{
    float       f0_x, o0_x, r0_x, r1_x, r2_x, r3_x, r4_x, r5_x, r6_x, r7_x;
    float       f0_y, o0_y, r0_y, r1_y, r2_y, r3_y, r4_y, r5_y, r6_y, r7_y;
    float       f0_z, o0_z, r0_z, r1_z, r2_z, r3_z, r4_z, r5_z, r6_z, r7_z;
    float       f0_w, o0_w, r0_w, r1_w, r2_w, r3_w, r4_w, r5_w, r6_w, r7_w;
    int         loc_x = get_global_id(0);
    int         loc_y = get_global_id(1);

    f0_x = st_origin.x + ((float)loc_x + 0.5f) * st_delta.x + ((float)loc_y + 0.5f) * st_delta.z;
    f0_y = st_origin.y + ((float)loc_x + 0.5f) * st_delta.y + ((float)loc_y + 0.5f) * st_delta.w;
    f0_z = 0.0f;
    f0_w = 0.0f;

    r7_x = f0_x;
    r7_y = f0_y;
    r7_z = f0_z;
    r7_w = f0_w;

    r6_x = r7_x*l1.z;
    r6_y = r7_y*l1.w;
    r6_z = r7_z*l1.x;
    r6_w = r7_w*l1.y;

    r7_x = r6_x-l1.x;
    r7_y = r6_y-l1.y;
    r7_z = r6_z-l1.z;
    r7_w = r6_w-l1.w;

    r0_x = dot((float2)(r7_x, r7_y),l3.xy) + l3.w;
    r0_y = dot((float2)(r7_x, r7_y),l4.xy) + l4.w;

    r7_x = r0_x;
    r7_y = r0_y;
    r7_z = r0_z;
    r7_w = r0_w;

    float4 r0T = read_imagef(t0, t_sampler0, (float2)(r7_x, r7_y));
    r0_x = r0T.x;
    r0_y = r0T.y;
    r0_z = r0T.z;
    r0_w = r0T.w;

    r7_x = r6_x-l0.z;
    r7_y = r6_y-l0.w;
    r7_z = r6_z-l0.x;
    r7_w = r6_w-l0.y;

    r1_x = dot((float2)(r7_x, r7_y),l3.xy) + l3.w;
    r1_y = dot((float2)(r7_x, r7_y),l4.xy) + l4.w;

    r7_x = r1_x;
    r7_y = r1_y;
    r7_z = r1_z;
    r7_w = r1_w;

    float4 r1T = read_imagef(t0, t_sampler0, (float2)(r7_x, r7_y));
    r1_x = r1T.x;
    r1_y = r1T.y;
    r1_z = r1T.z;
    r1_w = r1T.w;

    r7_x = r6_x-l0.x;
    r7_y = r6_y-l0.y;
    r7_z = r6_z-l0.z;
    r7_w = r6_w-l0.w;

    r3_x = dot((float2)(r7_x, r7_y),l3.xy) + l3.w;
    r3_y = dot((float2)(r7_x, r7_y),l4.xy) + l4.w;
    r7_x = r3_x;
    r7_y = r3_y;
    r7_z = r3_z;
    r7_w = r3_w;

    float4 r3T = read_imagef(t0, t_sampler0, (float2)(r7_x, r7_y));
    r3_x = r3T.x;
    r3_y = r3T.y;
    r3_z = r3T.z;
    r3_w = r3T.w;

    r7_x = r6_x+l0.x;
    r7_y = r6_y+l0.y;
    r7_z = r6_z+l0.z;
    r7_w = r6_w+l0.w;

    r5_x = dot((float2)(r7_x, r7_y),l3.xy) + l3.w;
    r5_y = dot((float2)(r7_x, r7_y),l4.xy) + l4.w;

    r7_x = r5_x;
    r7_y = r5_y;
    r7_z = r5_z;
    r7_w = r5_w;

    float4 r5T = read_imagef(t0, t_sampler0, (float2)(r7_x, r7_y));
    r5_x = r5T.x;
    r5_y = r5T.y;
    r5_z = r5T.z;
    r5_w = r5T.w;

    r7_x = r6_x+l0.z;
    r7_y = r6_y+l0.w;
    r7_z = r6_z+l0.x;
    r7_w = r6_w+l0.y;

    r4_x = dot((float2)(r7_x, r7_y),l3.xy) + l3.w;
    r4_y = dot((float2)(r7_x, r7_y),l4.xy) + l4.w;

    float4 r4T = read_imagef(t0, t_sampler0, (float2)(r4_x, r4_y));
    r4_x = r4T.x;
    r4_y = r4T.y;
    r4_z = r4T.z;
    r4_w = r4T.w;

    r6_x = r6_x+l1.x;
    r6_y = r6_y+l1.y;
    r6_z = r6_z+l1.z;
    r6_w = r6_w+l1.w;

    r2_x = dot((float2)(r6_x, r6_y),l3.xy) + l3.w;
    r2_y = dot((float2)(r6_x, r6_y),l4.xy) + l4.w;

    float4 r2T = read_imagef(t0, t_sampler0, (float2)(r2_x, r2_y));
    r2_x = r2T.x;
    r2_y = r2T.y;
    r2_z = r2T.z;
    r2_w = r2T.w;

    r3_x = r3_x+r5_x;
    r3_y = r3_y+r5_y;
    r3_z = r3_z+r5_z;
    r3_w = r3_w+r5_w;

    r3_x = l2.x*r3_x;
    r3_y = l2.x*r3_y;
    r3_z = l2.x*r3_z;
    r3_w = l2.x*r3_w;

    r1_x = r1_x+r4_x;
    r1_y = r1_y+r4_y;
    r1_z = r1_z+r4_z;
    r1_w = r1_w+r4_w;

    r1_x = l2.y*r1_x + r3_x;
    r1_y = l2.y*r1_y + r3_y;
    r1_z = l2.y*r1_z + r3_z;
    r1_w = l2.y*r1_w + r3_w;

    r0_x = r0_x+r2_x;
    r0_y = r0_y+r2_y;
    r0_z = r0_z+r2_z;
    r0_w = r0_w+r2_w;

    r0_x = l2.z*r0_x + r1_x;
    r0_y = l2.z*r0_y + r1_y;
    r0_z = l2.z*r0_z + r1_z;
    r0_w = l2.z*r0_w + r1_w;

    r0_x = min(r0_x, r0_w);
    r0_y = min(r0_y, r0_w);
    r0_z = min(r0_z, r0_w);

    o0_x = r0_x;
    o0_y = r0_y;
    o0_z = r0_z;
    o0_w = r0_w;

    write_imagef(dest, (int2)( loc_x + dim.z , flipped ? get_image_height(dest) - (loc_y + dim.w + 1) : loc_y + dim.w ), (float4)(o0_x, o0_y, o0_z, o0_w));
}




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




__kernel void program_trans(write_only image2d_t dest, int flipped, int4 dim, float2 st_origin, float4 st_delta, float4 l0, float4 l1, float4 l2, float4 l3, float4 l4, read_only image2d_t t0, sampler_t t_sampler0)
{
    float4       f0_start, f0_end, f0_delta, tf0_start[4], tf0_delta[4];
    float4       f1_start, f1_end, f1_delta, tf1_start[4], tf1_delta[4];
    float4       f2_start, f2_end, f2_delta, tf2_start[4], tf2_delta[4];
    float4       f3_start, f3_end, f3_delta, tf3_start[4], tf3_delta[4];
    float4       f4_start, f4_end, f4_delta, tf4_start[4], tf4_delta[4];
    float4       f5_start, f5_end, f5_delta, tf5_start[4], tf5_delta[4];
    float4       f6_start, f6_end, f6_delta, tf6_start[4], tf6_delta[4];
    float4       gr0_3_0[64], gr0_3_1[64], gr0_3_2[64], gr0_3_3[64];
    float4       gr0_1_0[64], gr0_1_1[64], gr0_1_2[64], gr0_1_3[64];
    float4       gr0_5_0[64], gr0_5_1[64], gr0_5_2[64], gr0_5_3[64];
    float4       gr0_4_0[64], gr0_4_1[64], gr0_4_2[64], gr0_4_3[64];
    float4       gr0_6_0[64], gr0_6_1[64], gr0_6_2[64], gr0_6_3[64];
    float4       gr0_2_0[64], gr0_2_1[64], gr0_2_2[64], gr0_2_3[64];
    int          index = 0;
    int          total_index = 0;
    int          write_amount = 0;
    int          read_amount = 256;
    int          write_offset = 0;
    float4       o_r[64], o_g[64], o_b[64], o_a[64];
    int          dest_width = dim.x;
    int          dest_height = dim.y;
    float4       o0, r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, r16, r17, r18, r19, r20, r21, r22, r23, r24, r25, r26, r27, r28, r29, r30, r31, r32, r33, r34, r35, r36, r37, r38, r39, r40, r41, r42, r43, r44, r45, r46, r47, r48, r49, r50, r51, r52, r53, r54, r55, r56, r57, r58, r59, r60, r61, r62, r63, r64, r65, r66, r67, r68, r69, r70, r71, r72, r73, r74, r75, r76, r77, r78, r79, r80, r81;
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
    r7 = loc_start;
    r1 = r7*l1.zwxy;
    r7 = r1-l1;
    r6.x = dot(r7.xy,l3.xy) + l3.w;
    r6.y = dot(r7.xy,l4.xy) + l4.w;
    f1_start = r6;
    r6 = r1-l0.zwxy;
    r5.x = dot(r6.xy,l3.xy) + l3.w;
    r5.y = dot(r6.xy,l4.xy) + l4.w;
    f2_start = r5;
    r5 = r1-l0;
    r4.x = dot(r5.xy,l3.xy) + l3.w;
    r4.y = dot(r5.xy,l4.xy) + l4.w;
    f3_start = r4;
    r4 = r1+l0;
    r3.x = dot(r4.xy,l3.xy) + l3.w;
    r3.y = dot(r4.xy,l4.xy) + l4.w;
    f4_start = r3;
    r3 = r1+l0.zwxy;
    r2.x = dot(r3.xy,l3.xy) + l3.w;
    r2.y = dot(r3.xy,l4.xy) + l4.w;
    f5_start = r2;
    r1 = r1+l1;
    r0.x = dot(r1.xy,l3.xy) + l3.w;
    r0.y = dot(r1.xy,l4.xy) + l4.w;
    f6_start = r0;

    // vertex end
    f0_end = loc_end;
    r7 = loc_end;
    r1 = r7*l1.zwxy;
    r7 = r1-l1;
    r6.x = dot(r7.xy,l3.xy) + l3.w;
    r6.y = dot(r7.xy,l4.xy) + l4.w;
    f1_end = r6;
    r6 = r1-l0.zwxy;
    r5.x = dot(r6.xy,l3.xy) + l3.w;
    r5.y = dot(r6.xy,l4.xy) + l4.w;
    f2_end = r5;
    r5 = r1-l0;
    r4.x = dot(r5.xy,l3.xy) + l3.w;
    r4.y = dot(r5.xy,l4.xy) + l4.w;
    f3_end = r4;
    r4 = r1+l0;
    r3.x = dot(r4.xy,l3.xy) + l3.w;
    r3.y = dot(r4.xy,l4.xy) + l4.w;
    f4_end = r3;
    r3 = r1+l0.zwxy;
    r2.x = dot(r3.xy,l3.xy) + l3.w;
    r2.y = dot(r3.xy,l4.xy) + l4.w;
    f5_end = r2;
    r1 = r1+l1;
    r0.x = dot(r1.xy,l3.xy) + l3.w;
    r0.y = dot(r1.xy,l4.xy) + l4.w;
    f6_end = r0;

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

    __async_work_group_stream_from_image(t0, t_sampler0, f3_start.xy, f3_delta.xy, read_amount, (float4 *)gr0_3_0, (float4 *)gr0_3_1, (float4 *)gr0_3_2, (float4 *)gr0_3_3);
    __async_work_group_stream_from_image(t0, t_sampler0, f1_start.xy, f1_delta.xy, read_amount, (float4 *)gr0_1_0, (float4 *)gr0_1_1, (float4 *)gr0_1_2, (float4 *)gr0_1_3);
    __async_work_group_stream_from_image(t0, t_sampler0, f5_start.xy, f5_delta.xy, read_amount, (float4 *)gr0_5_0, (float4 *)gr0_5_1, (float4 *)gr0_5_2, (float4 *)gr0_5_3);
    __async_work_group_stream_from_image(t0, t_sampler0, f4_start.xy, f4_delta.xy, read_amount, (float4 *)gr0_4_0, (float4 *)gr0_4_1, (float4 *)gr0_4_2, (float4 *)gr0_4_3);
    __async_work_group_stream_from_image(t0, t_sampler0, f6_start.xy, f6_delta.xy, read_amount, (float4 *)gr0_6_0, (float4 *)gr0_6_1, (float4 *)gr0_6_2, (float4 *)gr0_6_3);
    __async_work_group_stream_from_image(t0, t_sampler0, f2_start.xy, f2_delta.xy, read_amount, (float4 *)gr0_2_0, (float4 *)gr0_2_1, (float4 *)gr0_2_2, (float4 *)gr0_2_3);
    for(; loc.x<dest_width ; loc.x+=4)
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
        r24 = r8+r12;
        r25 = r9+r13;
        r26 = r10+r14;
        r27 = r11+r15;
        r28 = r24;
        r29 = r25;
        r30 = r26;
        r31 = r27;
        r32 = l2.xxxx*r28;
        r33 = l2.xxxx*r29;
        r34 = l2.xxxx*r30;
        r35 = l2.xxxx*r31;
        r36 = r32;
        r37 = r33;
        r38 = r34;
        r39 = r35;
        r40 = r4+r16;
        r41 = r5+r17;
        r42 = r6+r18;
        r43 = r7+r19;
        r44 = r40;
        r45 = r41;
        r46 = r42;
        r47 = r43;
        r48 = l2.yyyy*r44 + r36;
        r49 = l2.yyyy*r45 + r37;
        r50 = l2.yyyy*r46 + r38;
        r51 = l2.yyyy*r47 + r39;
        r52 = r48;
        r53 = r49;
        r54 = r50;
        r55 = r51;
        r56 = r0+r20;
        r57 = r1+r21;
        r58 = r2+r22;
        r59 = r3+r23;
        r60 = r56;
        r61 = r57;
        r62 = r58;
        r63 = r59;
        r64 = l2.zzzz*r60 + r52;
        r65 = l2.zzzz*r61 + r53;
        r66 = l2.zzzz*r62 + r54;
        r67 = l2.zzzz*r63 + r55;
        r68 = r64;
        r69 = r65;
        r70 = r66;
        r71 = r67;
        r72 = min(r68, r71);
        r73 = min(r69, r71);
        r74 = min(r70, r71);
        r75 = r72;
        r76 = r73;
        r77 = r74;
        r78 = r75;
        r79 = r76;
        r80 = r77;
        r81 = r71;
        o_r[index] = r78;
        o_g[index] = r79;
        o_b[index] = r80;
        o_a[index] = r81;
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
            __async_work_group_stream_from_image(t0, t_sampler0, f4_start.xy + ((float)4*total_index) * f4_delta.xy, f4_delta.xy, read_amount, (float4 *)gr0_4_0, (float4 *)gr0_4_1, (float4 *)gr0_4_2, (float4 *)gr0_4_3);
            __async_work_group_stream_from_image(t0, t_sampler0, f6_start.xy + ((float)4*total_index) * f6_delta.xy, f6_delta.xy, read_amount, (float4 *)gr0_6_0, (float4 *)gr0_6_1, (float4 *)gr0_6_2, (float4 *)gr0_6_3);
            __async_work_group_stream_from_image(t0, t_sampler0, f2_start.xy + ((float)4*total_index) * f2_delta.xy, f2_delta.xy, read_amount, (float4 *)gr0_2_0, (float4 *)gr0_2_1, (float4 *)gr0_2_2, (float4 *)gr0_2_3);
        }
;
    }
    if (index > 0)
    {
        (void)__async_work_group_stream_to_image(dest, (size_t)(dim.z + write_offset), (size_t)(flipped ? get_image_height(dest) - (loc.y+dim.w+1): loc.y+dim.w), (size_t)(dest_width - write_offset), (const float4 *)o_r, (const float4 *)o_g, (const float4 *)o_b, (const float4 *)o_a);
    }
}

