#define BLOCK_SIZE 1024
#if (BLOCK_SIZE == 64)
#define SHIFT_FACTOR 6
#elif (BLOCK_SIZE == 512)
#define SHIFT_FACTOR 9
#elif (BLOCK_SIZE == 1024)
#define SHIFT_FACTOR 10
#endif
/*
 * For  GPU, the kernels can be made to use local buffers by enabling the
 * following hash definition. Otherwise the input global buffer will be used.
 */
#define USE_LOCAL_BUFFERS
#ifdef USE_LOCAL_BUFFERS
#define BUF_ADDR_SPACE __local
#else
#define BUF_ADDR_SPACE __global
#endif
__attribute__((always_inline)) void bFFTRad2(BUF_ADDR_SPACE float4 *pReal,
                                             BUF_ADDR_SPACE float4 *pImag,
                                             int blockSize);
__attribute__((always_inline)) void bFFTRad4(BUF_ADDR_SPACE float4 *pReal,
                                             BUF_ADDR_SPACE float4 *pImag,
                                             int blockSize);
__attribute__((always_inline)) void
bFFTRad8(BUF_ADDR_SPACE float4 *pReal, BUF_ADDR_SPACE float4 *pImag,
         __constant float *pCos, __constant float *pSin, int fftSize,
         int stride, int blockSize);
__attribute__((always_inline)) void
bFFTRad16(BUF_ADDR_SPACE float8 *pReal, BUF_ADDR_SPACE float8 *pImag,
          __constant float *pCos, __constant float *pSin, int fftSize,
          int stride, int blockSize);
__attribute__((always_inline)) void
bFFTRad32(BUF_ADDR_SPACE float16 *pReal, BUF_ADDR_SPACE float16 *pImag,
          __constant float *pCos, __constant float *pSin, int fftSize,
          int stride, int blockSize);
__attribute__((always_inline)) void
bFFTRad64(BUF_ADDR_SPACE float16 *pReal, BUF_ADDR_SPACE float16 *pImag,
          __constant float *pCos, __constant float *pSin, int fftSize,
          int stride, int blockSize);
__attribute__((always_inline)) void
bFFTRadN(BUF_ADDR_SPACE float16 *pReal, BUF_ADDR_SPACE float16 *pImag,
         __local float16 *pCosL, __local float16 *pSinL, int fftSize);
/*
        Minimum Vector size supported is 32 and Maximum supported vector size is
        1024
*/
__attribute__((always_inline)) void bitReverse(BUF_ADDR_SPACE float *pReal,
                                               BUF_ADDR_SPACE float *pImag,
                                               int numBits, int fftSize) {
  int j = 0;
  for (int i = 0; i < (fftSize >> 1); i++) {
    if (j > i) {
      float temp = pReal[i];
      pReal[i] = pReal[j];
      pReal[j] = temp;
      temp = pImag[i];
      pImag[i] = pImag[j];
      pImag[j] = temp;
      if (j < (fftSize >> 1)) {
        temp = pReal[fftSize - (i + 1)];
        pReal[fftSize - (i + 1)] = pReal[fftSize - (j + 1)];
        pReal[fftSize - (j + 1)] = temp;
        temp = pImag[fftSize - (i + 1)];
        pImag[fftSize - (i + 1)] = pImag[fftSize - (j + 1)];
        pImag[fftSize - (j + 1)] = temp;
      }
    }
    int m = (fftSize >> 1);
    while ((m >= 1) && (j >= m)) {
      j -= m;
      m = m >> 1;
    }
    j += m;
  }
}
__kernel void blockFFT(__global float16 *pReal16, __global float16 *pImag16,
                       __constant float *pCos, __constant float *pSin,
                       int numBits, int fftSize) {
  int id = get_global_id(0);
#ifdef USE_LOCAL_BUFFERS
  __local float16 pReal[BLOCK_SIZE >> 4];
  __local float16 pImag[BLOCK_SIZE >> 4];
#endif
  int blkSize = min(BLOCK_SIZE, fftSize);
  /*
  Twiddle factors for N=BLOCK_SIZE is stored.
  Twiddle factors for N < BLOCK_SIZE, is derived from the same table
  The stride for computing the twiddle factors is
  manipulated accordingly
  */
  // compute stride for Radix 8 Butterflies
  int stride = (fftSize >> 3);
  /*
  Modify stride as twiddle factors are stored for N=BLOCK_SIZE
  Numbits in BLOCK_SIZE = SHIFT_FACTOR */
  stride = stride << (SHIFT_FACTOR - numBits);
  pReal16 += (id * (blkSize >> 4));
  pImag16 += (id * (blkSize >> 4));
#ifdef USE_LOCAL_BUFFERS
  // Copy the data from the global buffers to the local buffers
  for (int i = 0; i < (blkSize >> 4); i++) {
    pReal[i] = pReal16[i];
    pImag[i] = pImag16[i];
  }
#else
  __global float16 *pReal = pReal16;
  __global float16 *pImag = pImag16;
#endif
  bitReverse((BUF_ADDR_SPACE float *)pReal, (BUF_ADDR_SPACE float *)pImag,
             numBits, fftSize);
  bFFTRad2((BUF_ADDR_SPACE float4 *)pReal, (BUF_ADDR_SPACE float4 *)pImag,
           blkSize);
  bFFTRad4((BUF_ADDR_SPACE float4 *)pReal, (BUF_ADDR_SPACE float4 *)pImag,
           blkSize);
  bFFTRad8((BUF_ADDR_SPACE float4 *)pReal, (BUF_ADDR_SPACE float4 *)pImag, pCos,
           pSin, fftSize, stride, blkSize);
  stride = stride >> 1;
  bFFTRad16((BUF_ADDR_SPACE float8 *)pReal, (BUF_ADDR_SPACE float8 *)pImag,
            pCos, pSin, fftSize, stride, blkSize);
  if (fftSize >= 32) {
    stride = stride >> 1;
    bFFTRad32((BUF_ADDR_SPACE float16 *)pReal, (BUF_ADDR_SPACE float16 *)pImag,
              pCos, pSin, fftSize, stride, blkSize);
    if (fftSize >= 64) {
      stride = stride >> 1;
      bFFTRad64((BUF_ADDR_SPACE float16 *)pReal,
                (BUF_ADDR_SPACE float16 *)pImag, pCos, pSin, fftSize, stride,
                blkSize);
    }
    for (int radix = 128, stage = 6; radix <= fftSize; radix <<= 1, stage++) {
      stride = stride >> 1;
      __local float16 twid16C[(BLOCK_SIZE >> 1) >> 4];
      __local float16 twid16S[(BLOCK_SIZE >> 1) >> 4];
      __local float *pTwidC = (__local float *)twid16C;
      __local float *pTwidS = (__local float *)twid16S;
      __constant float *pCosC = pCos;
      __constant float *pSinC = pSin;
      for (int i = 0; i < (radix >> 1); i++) {
        *pTwidC++ = *pCosC;
        pCosC += stride;
        *pTwidS++ = *pSinC;
        pSinC += stride;
      }
      for (int j = 0; j < (fftSize >> (stage + 1)); j++) {
        bFFTRadN((pReal + (j * (radix >> 4))), (pImag + (j * (radix >> 4))),
                 twid16C, twid16S, radix);
      }
    }
  }
#ifdef USE_LOCAL_BUFFERS
  // Copy the data back to the global buffers from the local buffers
  for (int i = 0; i < (blkSize >> 4); i++) {
    pReal16[i] = pReal[i];
    pImag16[i] = pImag[i];
  }
#endif
}
__attribute__((always_inline)) void bFFTRad2(BUF_ADDR_SPACE float4 *pReal,
                                             BUF_ADDR_SPACE float4 *pImag,
                                             int blockSize) {
  for (int i = 0; i < (blockSize >> 2); i++) {
    float4 r0, i0, r1, i1;
    r1 = *pReal;
    i1 = *pImag;
    r0.x = r1.x + r1.y;
    r0.y = r1.x - r1.y;
    i0.x = i1.x + i1.y;
    i0.y = i1.x - i1.y;
    r0.z = r1.z + r1.w;
    r0.w = r1.z - r1.w;
    i0.z = i1.z + i1.w;
    i0.w = i1.z - i1.w;
    *pReal++ = r0;
    *pImag++ = i0;
  }
}
__attribute__((always_inline)) void bFFTRad4(BUF_ADDR_SPACE float4 *pReal,
                                             BUF_ADDR_SPACE float4 *pImag,
                                             int blockSize) {
  float4 r0, r1, i0, i1, r2, i2, i3, r3;
  for (int i = 0; i < (blockSize >> 3); i++) {
    r0 = *pReal;
    r2 = pReal[1];
    i0 = *pImag;
    i2 = pImag[1];
    r1.x = r0.x + r0.z;
    r1.z = r0.x - r0.z;
    r1.y = r0.y + i0.w;
    r1.w = r0.y - i0.w;
    i1.x = i0.x + i0.z;
    i1.z = i0.x - i0.z;
    i1.y = i0.y - r0.w;
    i1.w = i0.y + r0.w;
    *pReal++ = r1;
    *pImag++ = i1;
    r3.x = r2.x + r2.z;
    r3.z = r2.x - r2.z;
    r3.y = r2.y + i2.w;
    r3.w = r2.y - i2.w;
    i3.x = i2.x + i2.z;
    i3.z = i2.x - i2.z;
    i3.y = i2.y - r2.w;
    i3.w = i2.y + r2.w;
    *pReal++ = r3;
    *pImag++ = i3;
  }
}
__attribute__((always_inline)) void
bFFTRad8(BUF_ADDR_SPACE float4 *pReal, BUF_ADDR_SPACE float4 *pImag,
         __constant float *pCos, __constant float *pSin, int fftSize,
         int stride, int blockSize) {
  float4 twid[2];
  float *pTwidC = (float *)twid;
  float *pTwidS = (float *)(twid + 1);
  for (int i = 0; i < 4; i++) {
    *pTwidC++ = *pCos;
    pCos += stride;
    *pTwidS++ = *pSin;
    pSin += stride;
  }
  for (int i = 0; i < (blockSize >> 3); i++) {
    float4 cr, si, ci, sr, r0, i0, r1, i1;
    cr = pReal[1] * twid[0];
    si = pImag[1] * twid[1];
    sr = pReal[1] * twid[1];
    ci = pImag[1] * twid[0];
    float4 r2, i2;
    r2 = cr + si;
    i2 = ci - sr;
    r0 = pReal[0] + r2;
    r1 = pReal[0] - r2;
    i0 = pImag[0] + i2;
    i1 = pImag[0] - i2;
    *pReal++ = r0;
    *pReal++ = r1;
    *pImag++ = i0;
    *pImag++ = i1;
  }
}
__attribute__((always_inline)) void
bFFTRad16(BUF_ADDR_SPACE float8 *pReal, BUF_ADDR_SPACE float8 *pImag,
          __constant float *pCos, __constant float *pSin, int fftSize,
          int stride, int blockSize) {
  float8 twid[2];
  float *pTwidC = (float *)twid;
  float *pTwidS = (float *)(twid + 1);
  for (int i = 0; i < 8; i++) {
    *pTwidC++ = *pCos;
    pCos += stride;
    *pTwidS++ = *pSin;
    pSin += stride;
  }
  for (int i = 0; i < (blockSize >> 4); i++) {
    float8 cr, si, ci, sr, r0, i0, r1, i1;
    cr = pReal[1] * twid[0];
    si = pImag[1] * twid[1];
    sr = pReal[1] * twid[1];
    ci = pImag[1] * twid[0];
    float8 r2, i2;
    r2 = cr + si;
    i2 = ci - sr;
    r0 = pReal[0] + r2;
    r1 = pReal[0] - r2;
    i0 = pImag[0] + i2;
    i1 = pImag[0] - i2;
    *pReal++ = r0;
    *pReal++ = r1;
    *pImag++ = i0;
    *pImag++ = i1;
  }
}
__attribute__((always_inline)) void
bFFTRad32(BUF_ADDR_SPACE float16 *pReal, BUF_ADDR_SPACE float16 *pImag,
          __constant float *pCos, __constant float *pSin, int fftSize,
          int stride, int blockSize) {
  float16 twid[2];
  float *pTwidC = (float *)twid;
  float *pTwidS = (float *)(twid + 1);
  for (int i = 0; i < 16; i++) {
    *pTwidC++ = *pCos;
    pCos += stride;
    *pTwidS++ = *pSin;
    pSin += stride;
  }
  for (int i = 0; i < (blockSize >> 5); i++) {
    float16 cr, si, ci, sr, r0, i0, r1, i1;
    cr = pReal[1] * twid[0];
    si = pImag[1] * twid[1];
    sr = pReal[1] * twid[1];
    ci = pImag[1] * twid[0];
    float16 r2, i2;
    r2 = cr + si;
    i2 = ci - sr;
    r0 = pReal[0] + r2;
    r1 = pReal[0] - r2;
    i0 = pImag[0] + i2;
    i1 = pImag[0] - i2;
    *pReal++ = r0;
    *pReal++ = r1;
    *pImag++ = i0;
    *pImag++ = i1;
  }
}
__attribute__((always_inline)) void
bFFTRad64(BUF_ADDR_SPACE float16 *pReal, BUF_ADDR_SPACE float16 *pImag,
          __constant float *pCos, __constant float *pSin, int fftSize,
          int stride, int blockSize) {
  float16 twid[4];
  float *pTwidC = (float *)twid;
  float *pTwidS = (float *)(twid + 2);
  for (int i = 0; i < 32; i++) {
    *pTwidC++ = *pCos;
    pCos += stride;
    *pTwidS++ = *pSin;
    pSin += stride;
  }
  float16 *pTwid16C = twid;
  float16 *pTwid16S = twid + 2;
  BUF_ADDR_SPACE float16 *pReal2 = pReal + 2;
  BUF_ADDR_SPACE float16 *pImag2 = pImag + 2;
#if (BLOCK_SIZE > 64)
  for (int j = 0; j < (blockSize >> 6); j++) {
    pTwid16C = twid;
    pTwid16S = twid + 2;
#endif
    for (int i = 0; i < 2; i++) {
      float16 cr, si, ci, sr, r0, i0, r1, i1;
      float16 r2, i2;
      cr = *pReal2 * *pTwid16C;
      si = *pImag2 * *pTwid16S;
      sr = *pReal2 * *pTwid16S++;
      r2 = cr + si;
      ci = *pImag2 * *pTwid16C++;
      i2 = ci - sr;
      r0 = *pReal + r2;
      r1 = *pReal - r2;
      i0 = *pImag + i2;
      i1 = *pImag - i2;
      *pReal++ = r0;
      *pReal2++ = r1;
      *pImag++ = i0;
      *pImag2++ = i1;
    }
#if (BLOCK_SIZE > 64)
    pReal += 2;
    pReal2 += 2;
    pImag += 2;
    pImag2 += 2;
  }
#endif
}
__attribute__((always_inline)) void
bFFTRadN(BUF_ADDR_SPACE float16 *pReal, BUF_ADDR_SPACE float16 *pImag,
         __local float16 *pCosL, __local float16 *pSinL, int radix) {
  BUF_ADDR_SPACE float16 *pReal2 = pReal + (radix >> 5);
  BUF_ADDR_SPACE float16 *pImag2 = pImag + (radix >> 5);
  for (int i = 0; i < (radix >> 5); i++) {
    float16 cr, si, ci, sr, r0, i0, r1, i1;
    cr = *pReal2 * *pCosL;
    si = *pImag2 * *pSinL;
    sr = *pReal2 * *pSinL++;
    ci = *pImag2 * *pCosL++;
    float16 r2, i2;
    r2 = cr + si;
    i2 = ci - sr;
    r0 = *pReal + r2;
    r1 = *pReal - r2;
    i0 = *pImag + i2;
    i1 = *pImag - i2;
    *pReal++ = r0;
    *pReal2++ = r1;
    *pImag++ = i0;
    *pImag2++ = i1;
  }
}
/*
 * Columnwise FFT
 * The code is same as that of rowwise fft EXCEPT THAT
 * columnwise elements are copied to a local buffers
 * local buffer is used for taking FFT.
 * To avoid duplication of the code, the rowwise FFT
 * can be modified such that an extra argument - a global
 * buffer of size BLOCK_SIZE is passed and that be used.
 * Needs minor modifications in blockFFT code
 */
#define BLOCK_SIZE 1024
#if (BLOCK_SIZE == 64)
#define SHIFT_FACTOR 6
#elif (BLOCK_SIZE == 512)
#define SHIFT_FACTOR 9
#elif (BLOCK_SIZE == 1024)
#define SHIFT_FACTOR 10
#endif
__attribute__((always_inline)) void
bColFFTRad2(__local float4 *pReal, __local float4 *pImag, int blockSize);
__attribute__((always_inline)) void
bColFFTRad4(__local float4 *pReal, __local float4 *pImag, int blockSize);
__attribute__((always_inline)) void
bColFFTRad8(__local float4 *pReal, __local float4 *pImag,
            __constant float *pCos, __constant float *pSin, int fftSize,
            int stride, int blockSize);
__attribute__((always_inline)) void
bColFFTRad16(__local float8 *pReal, __local float8 *pImag,
             __constant float *pCos, __constant float *pSin, int fftSize,
             int stride, int blockSize);
__attribute__((always_inline)) void
bColFFTRad32(__local float16 *pReal, __local float16 *pImag,
             __constant float *pCos, __constant float *pSin, int fftSize,
             int stride, int blockSize);
__attribute__((always_inline)) void
bColFFTRad64(__local float16 *pReal, __local float16 *pImag,
             __constant float *pCos, __constant float *pSin, int fftSize,
             int stride, int blockSize);
__attribute__((always_inline)) void
bColFFTRadN(__local float16 *pReal, __local float16 *pImag,
            __local float16 *pCosL, __local float16 *pSinL, int fftSize);
/*
        Minimum Vector size supported is 32
*/
__attribute__((always_inline)) void colBitReverse(__local float *pReal,
                                                  __local float *pImag,
                                                  int numBits, int fftSize) {
  int j = 0;
  for (int i = 0; i < (fftSize >> 1); i++) {
    if (j > i) {
      float temp = pReal[i];
      pReal[i] = pReal[j];
      pReal[j] = temp;
      temp = pImag[i];
      pImag[i] = pImag[j];
      pImag[j] = temp;
      if (j < (fftSize >> 1)) {
        temp = pReal[fftSize - (i + 1)];
        pReal[fftSize - (i + 1)] = pReal[fftSize - (j + 1)];
        pReal[fftSize - (j + 1)] = temp;
        temp = pImag[fftSize - (i + 1)];
        pImag[fftSize - (i + 1)] = pImag[fftSize - (j + 1)];
        pImag[fftSize - (j + 1)] = temp;
      }
    }
    int m = (fftSize >> 1);
    while ((m >= 1) && (j >= m)) {
      j -= m;
      m = m >> 1;
    }
    j += m;
  }
}
__kernel void columnFFT(__global float *pRealG, __global float *pImagG,
                        __constant float *pCos, __constant float *pSin,
                        int numBits, int fftSize) {
  int id = get_global_id(0);
  __local float16 pReal[BLOCK_SIZE >> 4];
  __local float16 pImag[BLOCK_SIZE >> 4];
  int blkSize = min(BLOCK_SIZE, fftSize);
  /*
  Twiddle factors for N=BLOCK_SIZE is stored.
  Twiddle factors for N < BLOCK_SIZE, is derived from the same table
  The stride for computing the twiddle factors is
  manipulated accordingly
  */
  // compute stride for Radix 8 Butterflies
  int stride = (fftSize >> 3);
  /*
  Modify stride as twiddle factors are stored for N=BLOCK_SIZE
  Numbits in BLOCK_SIZE = SHIFT_FACTOR */
  stride = stride << (SHIFT_FACTOR - numBits);
  /*
          pRealG += (id * ( blkSize >> 4));
          pImagG += (id * ( blkSize >> 4));
  */
  // Copy the data from the global buffers to the local buffers
  __local float *pRealL = (__local float *)pReal;
  __local float *pImagL = (__local float *)pImag;
  for (int i = 0; i < blkSize; i++) {
    pRealL[i] = pRealG[id + i * fftSize];
    pImagL[i] = pImagG[id + i * fftSize];
  }
  colBitReverse((__local float *)pReal, (__local float *)pImag, numBits,
                fftSize);
  bColFFTRad2((__local float4 *)pReal, (__local float4 *)pImag, blkSize);
  bColFFTRad4((__local float4 *)pReal, (__local float4 *)pImag, blkSize);
  bColFFTRad8((__local float4 *)pReal, (__local float4 *)pImag, pCos, pSin,
              fftSize, stride, blkSize);
  stride = stride >> 1;
  bColFFTRad16((__local float8 *)pReal, (__local float8 *)pImag, pCos, pSin,
               fftSize, stride, blkSize);
  if (fftSize >= 32) {
    stride = stride >> 1;
    bColFFTRad32((__local float16 *)pReal, (__local float16 *)pImag, pCos, pSin,
                 fftSize, stride, blkSize);
    if (fftSize >= 64) {
      stride = stride >> 1;
      bColFFTRad64((__local float16 *)pReal, (__local float16 *)pImag, pCos,
                   pSin, fftSize, stride, blkSize);
    }
    for (int radix = 128, stage = 6; radix <= fftSize; radix <<= 1, stage++) {
      stride = stride >> 1;
      __local float16 twid16C[(BLOCK_SIZE >> 1) >> 4];
      __local float16 twid16S[(BLOCK_SIZE >> 1) >> 4];
      __local float *pTwidC = (__local float *)twid16C;
      __local float *pTwidS = (__local float *)twid16S;
      __constant float *pCosC = pCos;
      __constant float *pSinC = pSin;
      for (int i = 0; i < (radix >> 1); i++) {
        *pTwidC++ = *pCosC;
        pCosC += stride;
        *pTwidS++ = *pSinC;
        pSinC += stride;
      }
      for (int j = 0; j < (fftSize >> (stage + 1)); j++) {
        bColFFTRadN((pReal + (j * (radix >> 4))), (pImag + (j * (radix >> 4))),
                    twid16C, twid16S, radix);
      }
    }
  }
  pRealL = (__local float *)pReal;
  pImagL = (__local float *)pImag;
  // Copy the data back to the global buffers from the local buffers
  for (int i = 0; i < blkSize; i++) {
    pRealG[id + i * fftSize] = pRealL[i];
    pImagG[id + i * fftSize] = pImagL[i];
  }
}
__attribute__((always_inline)) void
bColFFTRad2(__local float4 *pReal, __local float4 *pImag, int blockSize) {
  for (int i = 0; i < (blockSize >> 2); i++) {
    float4 r0, i0, r1, i1;
    r1 = *pReal;
    i1 = *pImag;
    r0.x = r1.x + r1.y;
    r0.y = r1.x - r1.y;
    i0.x = i1.x + i1.y;
    i0.y = i1.x - i1.y;
    r0.z = r1.z + r1.w;
    r0.w = r1.z - r1.w;
    i0.z = i1.z + i1.w;
    i0.w = i1.z - i1.w;
    *pReal++ = r0;
    *pImag++ = i0;
  }
}
__attribute__((always_inline)) void
bColFFTRad4(__local float4 *pReal, __local float4 *pImag, int blockSize) {
  float4 r0, r1, i0, i1, r2, i2, i3, r3;
  for (int i = 0; i < (blockSize >> 3); i++) {
    r0 = *pReal;
    r2 = pReal[1];
    i0 = *pImag;
    i2 = pImag[1];
    r1.x = r0.x + r0.z;
    r1.z = r0.x - r0.z;
    r1.y = r0.y + i0.w;
    r1.w = r0.y - i0.w;
    i1.x = i0.x + i0.z;
    i1.z = i0.x - i0.z;
    i1.y = i0.y - r0.w;
    i1.w = i0.y + r0.w;
    *pReal++ = r1;
    *pImag++ = i1;
    r3.x = r2.x + r2.z;
    r3.z = r2.x - r2.z;
    r3.y = r2.y + i2.w;
    r3.w = r2.y - i2.w;
    i3.x = i2.x + i2.z;
    i3.z = i2.x - i2.z;
    i3.y = i2.y - r2.w;
    i3.w = i2.y + r2.w;
    *pReal++ = r3;
    *pImag++ = i3;
  }
}
__attribute__((always_inline)) void
bColFFTRad8(__local float4 *pReal, __local float4 *pImag,
            __constant float *pCos, __constant float *pSin, int fftSize,
            int stride, int blockSize) {
  float4 twid[2];
  float *pTwidC = (float *)twid;
  float *pTwidS = (float *)(twid + 1);
  for (int i = 0; i < 4; i++) {
    *pTwidC++ = *pCos;
    pCos += stride;
    *pTwidS++ = *pSin;
    pSin += stride;
  }
  for (int i = 0; i < (blockSize >> 3); i++) {
    float4 cr, si, ci, sr, r0, i0, r1, i1;
    cr = pReal[1] * twid[0];
    si = pImag[1] * twid[1];
    sr = pReal[1] * twid[1];
    ci = pImag[1] * twid[0];
    float4 r2, i2;
    r2 = cr + si;
    i2 = ci - sr;
    r0 = pReal[0] + r2;
    r1 = pReal[0] - r2;
    i0 = pImag[0] + i2;
    i1 = pImag[0] - i2;
    *pReal++ = r0;
    *pReal++ = r1;
    *pImag++ = i0;
    *pImag++ = i1;
  }
}
__attribute__((always_inline)) void
bColFFTRad16(__local float8 *pReal, __local float8 *pImag,
             __constant float *pCos, __constant float *pSin, int fftSize,
             int stride, int blockSize) {
  float8 twid[2];
  float *pTwidC = (float *)twid;
  float *pTwidS = (float *)(twid + 1);
  for (int i = 0; i < 8; i++) {
    *pTwidC++ = *pCos;
    pCos += stride;
    *pTwidS++ = *pSin;
    pSin += stride;
  }
  for (int i = 0; i < (blockSize >> 4); i++) {
    float8 cr, si, ci, sr, r0, i0, r1, i1;
    cr = pReal[1] * twid[0];
    si = pImag[1] * twid[1];
    sr = pReal[1] * twid[1];
    ci = pImag[1] * twid[0];
    float8 r2, i2;
    r2 = cr + si;
    i2 = ci - sr;
    r0 = pReal[0] + r2;
    r1 = pReal[0] - r2;
    i0 = pImag[0] + i2;
    i1 = pImag[0] - i2;
    *pReal++ = r0;
    *pReal++ = r1;
    *pImag++ = i0;
    *pImag++ = i1;
  }
}
__attribute__((always_inline)) void
bColFFTRad32(__local float16 *pReal, __local float16 *pImag,
             __constant float *pCos, __constant float *pSin, int fftSize,
             int stride, int blockSize) {
  float16 twid[2];
  float *pTwidC = (float *)twid;
  float *pTwidS = (float *)(twid + 1);
  for (int i = 0; i < 16; i++) {
    *pTwidC++ = *pCos;
    pCos += stride;
    *pTwidS++ = *pSin;
    pSin += stride;
  }
  for (int i = 0; i < (blockSize >> 5); i++) {
    float16 cr, si, ci, sr, r0, i0, r1, i1;
    cr = pReal[1] * twid[0];
    si = pImag[1] * twid[1];
    sr = pReal[1] * twid[1];
    ci = pImag[1] * twid[0];
    float16 r2, i2;
    r2 = cr + si;
    i2 = ci - sr;
    r0 = pReal[0] + r2;
    r1 = pReal[0] - r2;
    i0 = pImag[0] + i2;
    i1 = pImag[0] - i2;
    *pReal++ = r0;
    *pReal++ = r1;
    *pImag++ = i0;
    *pImag++ = i1;
  }
}
__attribute__((always_inline)) void
bColFFTRad64(__local float16 *pReal, __local float16 *pImag,
             __constant float *pCos, __constant float *pSin, int fftSize,
             int stride, int blockSize) {
  float16 twid[4];
  float *pTwidC = (float *)twid;
  float *pTwidS = (float *)(twid + 2);
  for (int i = 0; i < 32; i++) {
    *pTwidC++ = *pCos;
    pCos += stride;
    *pTwidS++ = *pSin;
    pSin += stride;
  }
  float16 *pTwid16C = twid;
  float16 *pTwid16S = twid + 2;
  __local float16 *pReal2 = pReal + 2;
  __local float16 *pImag2 = pImag + 2;
#if (BLOCK_SIZE > 64)
  for (int j = 0; j < (blockSize >> 6); j++) {
    pTwid16C = twid;
    pTwid16S = twid + 2;
#endif
    for (int i = 0; i < 2; i++) {
      float16 cr, si, ci, sr, r0, i0, r1, i1;
      float16 r2, i2;
      cr = *pReal2 * *pTwid16C;
      si = *pImag2 * *pTwid16S;
      sr = *pReal2 * *pTwid16S++;
      r2 = cr + si;
      ci = *pImag2 * *pTwid16C++;
      i2 = ci - sr;
      r0 = *pReal + r2;
      r1 = *pReal - r2;
      i0 = *pImag + i2;
      i1 = *pImag - i2;
      *pReal++ = r0;
      *pReal2++ = r1;
      *pImag++ = i0;
      *pImag2++ = i1;
    }
#if (BLOCK_SIZE > 64)
    pReal += 2;
    pReal2 += 2;
    pImag += 2;
    pImag2 += 2;
  }
#endif
}
__attribute__((always_inline)) void
bColFFTRadN(__local float16 *pReal, __local float16 *pImag,
            __local float16 *pCosL, __local float16 *pSinL, int radix) {
  __local float16 *pReal2 = pReal + (radix >> 5);
  __local float16 *pImag2 = pImag + (radix >> 5);
  for (int i = 0; i < (radix >> 5); i++) {
    float16 cr, si, ci, sr, r0, i0, r1, i1;
    cr = *pReal2 * *pCosL;
    si = *pImag2 * *pSinL;
    sr = *pReal2 * *pSinL++;
    ci = *pImag2 * *pCosL++;
    float16 r2, i2;
    r2 = cr + si;
    i2 = ci - sr;
    r0 = *pReal + r2;
    r1 = *pReal - r2;
    i0 = *pImag + i2;
    i1 = *pImag - i2;
    *pReal++ = r0;
    *pReal2++ = r1;
    *pImag++ = i0;
    *pImag2++ = i1;
  }
}
