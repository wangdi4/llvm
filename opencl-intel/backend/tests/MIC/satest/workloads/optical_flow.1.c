#ifdef __CUDACC__
// maybe since these are used frequently, we should make them softer on the
// eyes, e.g. __main instead of MAIN
#define MAIN __global__
#define DEVICE __device__
#define GLOBAL
#define SHARED __shared__
#define LOCAL
#define SYNC() __syncthreads()
#define TIDX threadIdx.x
#define TIDY threadIdx.y
#define BIDX blockIdx.x
#define BIDY blockIdx.y
#define BDMX blockDim.x
#define BDMY blockDIm.y
#define GDMX gridDim.x
#define GDMY gridDim.y
#define MOD(A, B) ((A) % (B))
#define STATIC static
#define BOOL bool
#else
#ifdef AFCL // is there not some OpenCL directive?
#define MAIN __kernel
#define DEVICE
#define GLOBAL __global
#define SHARED __local
#define LOCAL __local
#define SYNC() barrier(CLK_LOCAL_MEM_FENCE);
#define TIDX get_local_id(0)
#define TIDY get_local_id(1)
#define BIDX get_group_id(0)
#define BIDY get_group_id(1)
#define BDMX get_local_size(0)
#define BDMY get_local_size(1)
#define GDMX get_num_groups(0)
#define GDMY get_num_groups(1)
#define MOD(A, B) ((A) % (B))
#define STATIC // opencl does not like static kernels
#define BOOL uchar
#endif // AFCL
#endif // CUDACC
#define W 1
#define H 2
#define tout float
#define get_in(x, y) in[x + y * w]
void __conv2_core_check(int x, int y, int w, int h, __global tout *in,
                        __global tout *out, __constant tout *ker) {
  tout sum = 0;
  int y_ = y - ((H - 1) >> 1);
  for (int fy = H - 1; fy >= 0; fy--, y_++) {
    if ((y_ < 0) || (y_ >= h))
      continue;
    int x_ = x - ((W - 1) >> 1);
    for (int fx = W - 1; fx >= 0; fx--, x_++) {
      if ((x_ < 0) || (x_ >= w))
        continue;
      sum += ker[fx + fy * W] * get_in(x_, y_);
    }
  }
  *out = sum;
}
void __conv2_core_pass(int x, int y, int w, int h, __global tout *in,
                       __global tout *out, __constant tout *ker) {
  tout sum = 0;
  int y_ = y - ((H - 1) >> 1);
  for (int fy = H - 1; fy >= 0; fy--, y_++) {
    int x_ = x - ((W - 1) >> 1);
    for (int fx = W - 1; fx >= 0; fx--, x_++) {
      sum += ker[fx + fy * W] * get_in(x_, y_);
    }
  }
  *out = sum;
}
__kernel void __conv2_same(__global tout *d_in, __global tout *d_out,
                           __constant tout *c_filter, int w, int h, int b0,
                           int Ni, int Nf) {
  const int tile = BIDY / b0;
  const int blockIdx_y = BIDY - b0 * tile;
  const int x_block = BIDX * BDMX;
  const int y_block = blockIdx_y * BDMY;
  const int x = TIDX + x_block;
  const int y = TIDY + y_block;
  const bool is_left = (x_block <= (W - 1) / 2);
  const bool is_right = ((x_block + BDMX) >= (w - (W - 1) / 2));
  const bool is_top = (y_block <= (H - 1) / 2);
  const bool is_bot = ((y_block + BDMY) >= (h - (H - 1) / 2));
  if (x >= w || y >= h)
    return;
  int idx = x + y * w;
  int off_in = 0, off_k = 0;
  if (Ni > 1)
    off_in += (w * h * tile);
  if (Nf > 1)
    off_k += tile * W * H;
  __global tout *out = d_out + idx + tile * w * h;
  __global tout *in = d_in + off_in;
  __constant tout *ker = c_filter + off_k;
  const bool check = (is_left | is_right | is_top | is_bot);
  if (check)
    __conv2_core_check(x, y, w, h, in, out, ker);
  else
    __conv2_core_pass(x, y, w, h, in, out, ker);
}
__kernel void __conv2_valid(__global tout *d_in, __global tout *d_out,
                            __constant tout *c_filter, int w, int h, int b0,
                            int Ni, int Nf) {
  const int tile = BIDY / b0;
  const int blockIdx_y = BIDY - b0 * tile;
  int x = TIDX + BIDX * BDMX;
  int y = TIDY + blockIdx_y * BDMY;
  // bail early
  if (x >= (w - (W - 1)) || y >= (h - (H - 1)))
    return;
  int ww = w - (W - 1);
  int hh = h - (H - 1);
  int idx = x + y * ww;
  x += (W - 1) / 2;
  y += (H - 1) / 2;
  int off_in = 0, off_k = 0;
  if (Ni > 1)
    off_in += (w * h * tile);
  if (Nf > 1)
    off_k += tile * W * H;
  __global tout *out = d_out + idx + tile * w * h;
  __global tout *in = d_in + off_in;
  __constant tout *ker = c_filter + off_k;
  __conv2_core_pass(x, y, w, h, in, out, ker);
}
