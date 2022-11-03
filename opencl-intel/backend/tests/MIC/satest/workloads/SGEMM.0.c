// Code derived from work done by the authors quoted in the original header
// below:
//
// (c) January 24, 2008 Vasily Volkov @ UC Berkeley
//
// Other credits:
// - Paul Leventis @ Altera Corp. for prefetching and -maxrregcount techniques
// - many thanks to Wladimir J. van der Laan @ the University of Groningen
// for his cubin disassembler (http://www.cs.rug.nl/~wladimir/decuda/)
//
//
#define OPTIMIZED_NT_B_BLOCK 2
#define OPTIMIZED_NT_A_BLOCK_STRIDE 16
#define OPTIMIZED_NT_A_BLOCK 1
#define OPTIMIZED_MANUAL_PREFETCH 0
#define OPTIMIZED_PREFETCH_FOR_1_OF_16 0
#define OPTMIZED_MANUAL_UNROLLING 0
#ifdef SINGLE_PRECISION
#define FPTYPE float
#define FPTYPE16 float16
#elif K_DOUBLE_PRECISION
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#define FPTYPE double
#define FPTYPE16 double16
#elif AMD_DOUBLE_PRECISION
#pragma OPENCL EXTENSION cl_amd_fp64 : enable
#define FPTYPE double
#define FPTYPE16 double16
#endif
#define SAXPY(_A_, _BS_, _C_)                                                  \
  do {                                                                         \
    _C_[0] += _A_ * _BS_[0];                                                   \
    _C_[1] += _A_ * _BS_[1];                                                   \
    _C_[2] += _A_ * _BS_[2];                                                   \
    _C_[3] += _A_ * _BS_[3];                                                   \
    _C_[4] += _A_ * _BS_[4];                                                   \
    _C_[5] += _A_ * _BS_[5];                                                   \
    _C_[6] += _A_ * _BS_[6];                                                   \
    _C_[7] += _A_ * _BS_[7];                                                   \
    _C_[8] += _A_ * _BS_[8];                                                   \
    _C_[9] += _A_ * _BS_[9];                                                   \
    _C_[10] += _A_ * _BS_[10];                                                 \
    _C_[11] += _A_ * _BS_[11];                                                 \
    _C_[12] += _A_ * _BS_[12];                                                 \
    _C_[13] += _A_ * _BS_[13];                                                 \
    _C_[14] += _A_ * _BS_[14];                                                 \
    _C_[15] += _A_ * _BS_[15];                                                 \
  } while (0)
__kernel void sgemmNT(__global const FPTYPE *restrict A, int lda,
                      __global const FPTYPE *restrict B, int ldb,
                      __global FPTYPE *restrict C, int ldc, int k, FPTYPE alpha,
                      FPTYPE beta) {
  const int inx = get_local_id(0);
  const int iny = get_local_id(1);
  const int ibx = get_group_id(0) * 64 * OPTIMIZED_NT_A_BLOCK;
  const int iby = get_group_id(1) * 16 * OPTIMIZED_NT_B_BLOCK;
  const int id = inx + iny * 16 * OPTIMIZED_NT_A_BLOCK;
  int Aind = ibx + id;
  int Bind = iby;
  int Cind = ibx + id + (iby * ldc);
  FPTYPE c[16 * OPTIMIZED_NT_A_BLOCK * OPTIMIZED_NT_B_BLOCK];
  for (int i = 0; i < 16 * OPTIMIZED_NT_A_BLOCK * OPTIMIZED_NT_B_BLOCK; ++i) {
    c[i] = 0.0;
  }
  for (int ii = 0; ii < k; ++ii) {
    for (int ablock = 0; ablock < OPTIMIZED_NT_A_BLOCK; ++ablock) {
      for (int bblock = 0; bblock < OPTIMIZED_NT_B_BLOCK; ++bblock) {
        int Acur = Aind + ii * lda + ablock * OPTIMIZED_NT_A_BLOCK_STRIDE;
        int Bcur = Bind + ii * ldb + bblock * 16;
        FPTYPE atmp = A[Acur];
#if OPTIMIZED_MANUAL_PREFETCH
        {
#if OPTIMIZED_PREFETCH_FOR_1_OF_16
          if (inx == 0)
#endif
          {
            prefetch(A + Acur + lda, 1);
            prefetch(B + Bcur + ldb, 1);
          }
        }
#endif
#if !OPTMIZED_MANUAL_UNROLLING
        for (int jj = 0; jj < 16; ++jj) {
          c[jj + OPTIMIZED_NT_B_BLOCK * ablock * 16 + bblock * 16] +=
              atmp * B[Bcur + jj];
        }
#else
        {
#define MYMACRO1(jj)                                                           \
  c[jj + OPTIMIZED_NT_B_BLOCK * ablock * 16 + bblock * 16] +=                  \
      atmp * B[Bcur + jj];
          MYMACRO1(0)
          MYMACRO1(1)
          MYMACRO1(2)
          MYMACRO1(3)
          MYMACRO1(4)
          MYMACRO1(5)
          MYMACRO1(6)
          MYMACRO1(7)
          MYMACRO1(8)
          MYMACRO1(9)
          MYMACRO1(10)
          MYMACRO1(11)
          MYMACRO1(12)
          MYMACRO1(13)
          MYMACRO1(14)
          MYMACRO1(15)
        }
#endif
      }
    }
  }
  for (int ablock = 0; ablock < OPTIMIZED_NT_A_BLOCK; ++ablock) {
    for (int bblock = 0; bblock < OPTIMIZED_NT_B_BLOCK; ++bblock) {
      Cind = (ibx + id) + (iby * ldc) + ablock * OPTIMIZED_NT_A_BLOCK_STRIDE +
             ldc * bblock * 16;
      for (int i = 0; i < 16; i++, Cind += ldc) {
        C[Cind] =
            alpha * c[i + OPTIMIZED_NT_B_BLOCK * ablock * 16 + bblock * 16] +
            beta * C[Cind];
      }
    }
  }
}
#define AA(X, Y) A[(X) + (Y)*lda]
#define BB(X, Y) B[(X) + (Y)*ldb]
#define CC(X, Y) C[(X) + (Y)*ldc]
__kernel void sgemmNN(__global const FPTYPE *A, int lda,
                      __global const FPTYPE *B, int ldb, __global FPTYPE *C,
                      int ldc, int k, FPTYPE alpha, FPTYPE beta) {
  const int iby = get_group_id(1) * WG_SIZE_1 * WI_TILE_SIZE +
                  get_local_id(1) * WI_TILE_SIZE;
  const int id = get_local_id(0) + get_group_id(0) * WG_SIZE_0;
  FPTYPE c[WI_TILE_SIZE] = {0.0};
  for (int counter = 0; counter < k; counter += WI_TILE_SIZE)
    for (int j = 0; j < WI_TILE_SIZE; j++)
      for (int i = 0; i < WI_TILE_SIZE; i++)
        c[j] += AA(id, counter + i) * BB(counter + i, iby + j);
  for (int i = 0; i < WI_TILE_SIZE; i++)
    CC(id, iby + i) = alpha * c[i] + beta * CC(id, iby + i);
}
