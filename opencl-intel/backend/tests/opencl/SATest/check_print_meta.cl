typedef struct {
  double z;
  uint3 g;
} B;
typedef struct {
  B l;
  B r;
} A;
__kernel void sample_test(__global A *c, __read_only image2d_t img,
                          __global long *buf, uint8 vectr) {}