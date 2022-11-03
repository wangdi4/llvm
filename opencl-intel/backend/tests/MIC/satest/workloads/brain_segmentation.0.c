#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#define ty float
#define ISGFOR 0
#define BLOCK_SIZE 16
#define ISCONJ 0
#if ISCONJ
ty CONJ(ty in) {
  ty out = {in.x, -in.y};
  return out;
}
#else
#define CONJ(in) (in)
#endif
#define TEMPLATE
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
TEMPLATE MAIN STATIC void _transpose(const GLOBAL ty *_idata, GLOBAL ty *_odata,
                                     unsigned w, unsigned h, unsigned gridDim_x,
                                     int i, int j) {
  SHARED ty block[BLOCK_SIZE][BLOCK_SIZE + 1];
  unsigned id = 0;
  unsigned bx = BIDX + i;
  unsigned by = BIDY + j;
  if (ISGFOR) {
    id = (bx) / gridDim_x;
    bx -= id * gridDim_x; // MOD(bx, gridDim_x);
  }
  unsigned inc = (ISGFOR ? id * w * h : 0);
  GLOBAL ty *odata = (GLOBAL ty *)_odata + inc;
  const GLOBAL ty *idata = (const GLOBAL ty *)_idata + inc;
  int blockIdx_x, blockIdx_y;
  // do diagonal reordering
  if (w == h) {
    blockIdx_y = bx;
    blockIdx_x = MOD((bx + by), gridDim_x);
  } else {
    int bid = bx + gridDim_x * by;
    blockIdx_y = MOD(bid, GDMY);
    int tmp = ((bid / GDMY) + blockIdx_y);
    blockIdx_x = MOD(tmp, gridDim_x);
  }
  // from here on the code is same as previous kernel except blockIdx_x replaces
  // BIDX and similarly for y read the matrix tile into shared memory
  unsigned x = blockIdx_x * BLOCK_SIZE + TIDX;
  unsigned y = blockIdx_y * BLOCK_SIZE + TIDY;
  if (x < w && y < h)
    block[TIDY][TIDX] = idata[y * w + x];
  SYNC(); // coalesce reads/writes
  // write the transposed matrix tile to global memory
  x = blockIdx_y * BLOCK_SIZE + TIDX;
  y = blockIdx_x * BLOCK_SIZE + TIDY;
  if (x < h && y < w)
    odata[y * h + x] = ISCONJ ? CONJ(block[TIDX][TIDY]) : block[TIDX][TIDY];
}
