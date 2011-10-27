__kernel
void hadd_test(__global int * x,
               __global int * y,
               __global int * z)
{
  uint tid = 0;
  z[tid] = hadd(x[tid], y[tid]);
  printf("%d\n", z[tid]);
}