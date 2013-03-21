float func(__global float* b)
{
  uint tid = get_global_id(0);
  b[tid] = 3.f;
  return 3.f;
}

__kernel
void call(__global float *a)
{
  func(a);
}
