typedef struct {
  float4 someArray[20];
  float *unsupported;
} unsupportedStruct;
__kernel void test(global float *a, global float *b,
                   global unsupportedStruct *c) {
  a[get_global_id(0)] += b[get_global_id(0)];
  c[get_global_id(0)].unsupported[0] = 2.3f;
}
