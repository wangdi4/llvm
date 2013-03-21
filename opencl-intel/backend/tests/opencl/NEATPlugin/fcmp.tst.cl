
__kernel
void fcmp(__global float4 * input, 
          __global float4 * output, 
          const    uint  buffer_size)
{
  uint tid = get_global_id(0);
  output[tid].w = input[tid].x;
  if (input[tid].x < 20.f)
  {
    if (input[tid].y <= 10.f)
    {
      output[tid].y = input[tid].x+5.f;   // x < 20; y <= 10; z, w - any
    }
    else
    {
      output[tid].x = 100.f;              // x < 20; y > 10; z, w - any
    }
  }
  else
  {
    if (input[tid].z == 41.f)
    {
      output[tid].z = 1.f;                  // x >= 20; z == 41; y, w - any
    }
    else
    {
      if (input[tid].w > 20.f)
      {
        if (input[tid].x != 30.f)
        {
          output[tid].x = 30.f;               // x >= 20 && x != 30; z != 41; w > 20
        }
        else
        {
          output[tid].y = 40.f;               // x == 30; z != 41; w > 20
        }
      }
      else
      {
        if (input[tid].x >= 41.f)
        {
          output[tid].x = 45.f;               // 
        }
        else
        {
          output[tid].x = 44.f;
        }
      }
    }
  }
}
