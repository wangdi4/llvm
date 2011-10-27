struct str1
{
  int a;
  float b;
  double c;
  int3 d;
  float4 e;
  double8 f[10];
};

__kernel
void allocaTest(__global float * input,
            __global float * output,
             const uint buffer_size)
{
  float tmp = input[get_global_id(0)];
  int tmpint;
  double tmpdouble;
  int2 tmpint2;
  double4 tmpdouble4;
  struct str1 tmpstruct;
  output[get_global_id(0)] = tmp;
}
