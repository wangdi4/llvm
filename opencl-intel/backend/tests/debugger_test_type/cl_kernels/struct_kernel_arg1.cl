typedef struct __attribute__((packed)) {
  float f1, f2;
  float4 position;

  uint size;

  float rotation[16];
} KernelArg;

__kernel void main_kernel(__global uchar *buf_in, __global uchar *buf_out,
                          __global KernelArg *kernel_arg) {
  float kf1 = kernel_arg->f1;
  float kf2 = kernel_arg->f2;
  float4 kposition = kernel_arg->position;
  uint ksize = kernel_arg->size;

  float ksum_rotation = 0;
  int i;
  for (i = 0; i < 16; ++i)
    ksum_rotation += kernel_arg->rotation[i];

  uint sizeof_kernelarg = sizeof(*kernel_arg);

  KernelArg kcopy = *kernel_arg;

  buf_out[0] = 0;
}
