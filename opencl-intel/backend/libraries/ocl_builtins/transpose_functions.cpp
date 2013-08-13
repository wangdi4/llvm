/*=================================================================================
 Copyright (c) 2012, Intel Corporation
 Subject to the terms and conditions of the Master Development License
 Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
 OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
 ==================================================================================*/

///////////////////////////////////////////////////////////
//  transpose_functions.cpp
///////////////////////////////////////////////////////////

#include "transpose_functions.h"
#include "masked_load_store_functions.h"

#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#define __OPENCL__
#ifdef __APPLE__
  #include <immintrin.h>
#else
  #include <intrin.h>
#endif

// TODO: when porting that file to Tablegen look into CSSD100015383

#if defined(__SSE4_2__)

// ****************************************************************************
//                                 char4x4
// ****************************************************************************

/// @brief Receives char4x4 matrix as char16, transposes it and outputs the rows
///        of the transposed matrix
/// @param xyzwIn - char4x4 matrix to be transposed
/// @param xOut   - Row 0 of the transposed matrix
/// @param yOut   - Row 1 of the transposed matrix
/// @param zOut   - Row 2 of the transposed matrix
/// @param wOut   - Row 3 of the transposed matrix
void INTERNAL_INLINE_ATTRIBUTE __ocl_load_transpose_char4x4_common(char16 xyzwIn, private char4* xOut, private char4* yOut, private char4* zOut, private char4* wOut) {

  char16 xyzw = xyzwIn;                                                     // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3
  *xOut = xyzw.s048C;                                                       // x0  D  D  D x1  D  D  D x2  D  D  D x3  D  D  D

  xyzw = as_char16(_mm_srli_si128((__m128i)xyzw, 1));                       // y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3  D
  *yOut = xyzw.s048C;                                                       // y0  D  D  D y1  D  D  D y2  D  D  D y3  D  D  D

  xyzw = as_char16(_mm_srli_si128((__m128i)xyzw, 1));                       // z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3  D  D
  *zOut = xyzw.s048C;                                                       // z0  D  D  D z1  D  D  D z2  D  D  D z3  D  D  D

  xyzw = as_char16(_mm_srli_si128((__m128i)xyzw, 1));                       // w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3  D  D  D
  *wOut = xyzw.s048C;                                                       // w0  D  D  D w1  D  D  D w2  D  D  D w3  D  D  D
}

void INLINE_ATTRIBUTE __ocl_load_transpose_char4x4(private char4* pLoadAdd, private char4* xOut, private char4* yOut, private char4* zOut, private char4* wOut) {
  // We load "char16", meaning we load the full matrix in a single load
  private char* tmpLoadAdd = (private char*)pLoadAdd;
  char16 xyzw = vload16(0, tmpLoadAdd); // unaligned load

  __ocl_load_transpose_char4x4_common(xyzw, xOut, yOut, zOut, wOut);
}

/// @brief Receives char4x4 matrix as 4 matrix rows, transposes it and outputs the matrix
///        as a whole using char16
/// @param xyzw   - This parameter will contain the transposed char4x4 matrix
/// @param xIn    - Row 0 of the matrix to be transposed
/// @param yIn    - Row 1 of the matrix to be transposed
/// @param zIn    - Row 2 of the matrix to be transposed
/// @param wIn    - Row 3 of the matrix to be transposed
void INTERNAL_INLINE_ATTRIBUTE __ocl_transpose_store_char4x4_common(private char16* xyzw, char4 xIn, char4 yIn, char4 zIn, char4 wIn) {

  char16 x;
  char16 y;
  char16 z;
  char16 w;

  x.s0123 = xIn;                                                            // x0 x1 x2 x3  D  D  D  D  D  D  D  D  D  D  D  D
  y.s0123 = yIn;                                                            // y0 y1 y2 y3  D  D  D  D  D  D  D  D  D  D  D  D
  z.s0123 = zIn;                                                            // z0 z1 z2 z3  D  D  D  D  D  D  D  D  D  D  D  D
  w.s0123 = wIn;                                                            // w0 w1 w2 w3  D  D  D  D  D  D  D  D  D  D  D  D

  uchar16 low16 = {0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23};
  short8 xy = as_short8(shuffle2(x, y, low16));                             // x0 y0 x1 y1 x2 y2 x3 y3  D  D  D  D  D  D  D  D
  short8 zw = as_short8(shuffle2(z, w, low16));                             // z0 w0 z1 w1 z2 w2 z3 w3  D  D  D  D  D  D  D  D

  ushort8 low8 = {0, 8, 1, 9, 2, 10, 3, 11};
  *xyzw = as_char16(shuffle2(xy, zw, low8));                                // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3
}

void INLINE_ATTRIBUTE __ocl_transpose_store_char4x4(private char4* pStoreAdd, char4 xIn, char4 yIn, char4 zIn, char4 wIn) {

  char16 xyzw;
  __ocl_transpose_store_char4x4_common(&xyzw, xIn, yIn, zIn, wIn);

  // We store "char16", meaning we store the full matrix in a single store
  private char* tmpStoreAdd = (private char*)pStoreAdd;
  vstore16(xyzw, 0, tmpStoreAdd);  // unaligned store
}

#if defined(__AVX__)
void INLINE_ATTRIBUTE __ocl_masked_load_transpose_char4x4(private char4* pLoadAdd, private char4* xOut, private char4* yOut, private char4* zOut, private char4* wOut, int4 mask) {
  char4 xyzw[4];
  __ocl_masked_load_char4x4(pLoadAdd, xyzw, mask);
  __ocl_load_transpose_char4x4(xyzw, xOut, yOut, zOut, wOut);
}


void INLINE_ATTRIBUTE __ocl_masked_transpose_store_char4x4(private char4* pStoreAdd, char4 xIn, char4 yIn, char4 zIn, char4 wIn, int4 mask) {
  char4 xyzw[4];
  __ocl_transpose_store_char4x4(xyzw, xIn, yIn, zIn, wIn);
  __ocl_masked_store_char4x4(pStoreAdd, xyzw, mask);
}
#endif // defined(__AVX__)

void INLINE_ATTRIBUTE __ocl_gather_transpose_char4x4(private char4* pLoadAdd0, private char4* pLoadAdd1, private char4* pLoadAdd2, private char4* pLoadAdd3,
                              private char4* xOut, private char4* yOut, private char4* zOut, private char4* wOut) {

  // Broadcast the loaded values, all but the first element in the register which will be moved there
  int4 xyzw1 = *((private int*)pLoadAdd1);                                          // x1 y1 z1 w1 x1 y1 z1 w1 x1 y1 z1 w1 x1 y1 z1 w1
  int4 xyzw2 = *((private int*)pLoadAdd2);                                          // x2 y2 z2 w2 x2 y2 z2 w2 x2 y2 z2 w2 x2 y2 z2 w2
  int4 xyzw3 = *((private int*)pLoadAdd3);                                          // x3 y3 z3 w3 x3 y3 z3 w3 x3 y3 z3 w3 x3 y3 z3 w3

  int4 xyzw;
  // TODO : Replace this blend built-in with ?: when clang bug will be fixed
  // There's no blendd in AVX, so we use blendps
  xyzw.s0 = *((private int*)pLoadAdd0);                                             // x0 y0 z0 w0  D  D  D  D  D  D  D  D  D  D  D  D
  xyzw = as_int4(_mm_blend_ps((__m128)xyzw, (__m128)xyzw1, 0x2));           // x0 y0 z0 w0 x1 y1 z1 w1  D  D  D  D  D  D  D  D
  xyzw = as_int4(_mm_blend_ps((__m128)xyzw, (__m128)xyzw2, 0x4));           // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2  D  D  D  D
  xyzw = as_int4(_mm_blend_ps((__m128)xyzw, (__m128)xyzw3, 0x8));           // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3

  __ocl_load_transpose_char4x4_common(as_char16(xyzw), xOut, yOut, zOut, wOut);
}

void INLINE_ATTRIBUTE __ocl_transpose_scatter_char4x4(private char4* pStoreAdd0, private char4* pStoreAdd1, private char4* pStoreAdd2, private char4* pStoreAdd3,
                               char4 xIn, char4 yIn, char4 zIn, char4 wIn) {

  int4 xyzw = 0;
  __ocl_transpose_store_char4x4_common((private char16*)&xyzw, xIn, yIn, zIn, wIn);

  *((private int*)pStoreAdd0) = xyzw.s0;
  *((private int*)pStoreAdd1) = xyzw.s1;
  *((private int*)pStoreAdd2) = xyzw.s2;
  *((private int*)pStoreAdd3) = xyzw.s3;
}

#if defined(__AVX__)
void INLINE_ATTRIBUTE __ocl_masked_gather_transpose_char4x4(private char4* pLoadAdd0, private char4* pLoadAdd1, private char4* pLoadAdd2, private char4* pLoadAdd3,
                              private char4* xOut, private char4* yOut, private char4* zOut, private char4* wOut, int4 mask){
  if (all(mask)) {
   __ocl_gather_transpose_char4x4( pLoadAdd0, pLoadAdd1, pLoadAdd2, pLoadAdd3,
                            xOut, yOut, zOut, wOut);
    return;
  }

  char4 dummy;

  private char4* xyzw0 = mask.s0 ? pLoadAdd0 : &dummy;
  private char4* xyzw1 = mask.s1 ? pLoadAdd1 : &dummy;
  private char4* xyzw2 = mask.s2 ? pLoadAdd2 : &dummy;
  private char4* xyzw3 = mask.s3 ? pLoadAdd3 : &dummy;

	__ocl_gather_transpose_char4x4(xyzw0, xyzw1, xyzw2, xyzw3, xOut, yOut, zOut, wOut);
}

void INLINE_ATTRIBUTE __ocl_masked_transpose_scatter_char4x4(private char4* pStoreAdd0, private char4* pStoreAdd1, private char4* pStoreAdd2, private char4* pStoreAdd3,
                               char4 xIn, char4 yIn, char4 zIn, char4 wIn, int4 mask) {
  if (all(mask)) {
    __ocl_transpose_scatter_char4x4(pStoreAdd0, pStoreAdd1, pStoreAdd2, pStoreAdd3,
                            xIn, yIn, zIn, wIn);
    return;
  }

  char4 dummy;

  private char4* xyzw0 = mask.s0 ? pStoreAdd0 : &dummy;
  private char4* xyzw1 = mask.s1 ? pStoreAdd1 : &dummy;
  private char4* xyzw2 = mask.s2 ? pStoreAdd2 : &dummy;
  private char4* xyzw3 = mask.s3 ? pStoreAdd3 : &dummy;

	__ocl_transpose_scatter_char4x4(xyzw0, xyzw1, xyzw2, xyzw3, xIn, yIn, zIn, wIn);
}
#endif // defined(__AVX__)


// ****************************************************************************
//                                 char4x8
// ****************************************************************************


/// @brief Receives char8x4 matrix as 2 halfs (2 char4x4 matrixes) using 2 char16,
///        transposes it and outputs the rows of the transposed matrix.
/// @param xyzw0In  - Upper part of the char8x4 matrix to be transposed
/// @param xyzw1In  - Lower part of the char8x4 matrix to be transposed
/// @param xIn      - Row 0 of the transposed matrix
/// @param yIn      - Row 1 of the transposed matrix
/// @param zIn      - Row 2 of the transposed matrix
/// @param wIn      - Row 3 of the transposed matrix
void INTERNAL_INLINE_ATTRIBUTE __ocl_load_transpose_char4x8_common_AVX(char16 xyzw0In, char16 xyzw1In, private char8* xOut, private char8* yOut, private char8* zOut, private char8* wOut) {

  char16 xyzw0 = xyzw0In;
  char16 xyzw1 = xyzw1In;

  xyzw0 = xyzw0.s048C159D26AE37BF;                                          // x0 x1 x2 x3 y0 y1 y2 y3 z0 z1 z2 z3 w0 w1 w2 w3
  xyzw1 = xyzw1.s048C159D26AE37BF;                                          // x4 x5 x6 x7 y4 y5 y6 y7 z4 z5 z6 z7 w4 w5 w6 w7

  uint4 low4 = {0, 4, 1, 5};
  uint4 high4 = {2, 6, 3, 7};
  char16 xy = as_char16(shuffle2(as_int4(xyzw0), as_int4(xyzw1), low4));    // x0 x1 x2 x3 x4 x5 x6 x7 y0 y1 y2 y3 y4 y5 y6 y7
  char16 zw = as_char16(shuffle2(as_int4(xyzw0), as_int4(xyzw1), high4));   // z0 z1 z2 z3 z4 z5 z6 z7 w0 w1 w2 w3 w4 w5 w6 w7

  char16 x;
  char16 y;
  char16 z;
  char16 w;

  x.s02468ACE = xy.lo;                                                      // x0  D x1  D x2  D x3  D x4  D x5  D x6  D x7  D
  y.s02468ACE = xy.hi;                                                      // y0  D y1  D y2  D y3  D y4  D y5  D y6  D y7  D
  z.s02468ACE = zw.lo;                                                      // z0  D z1  D z2  D z3  D z4  D z5  D z6  D z7  D
  w.s02468ACE = zw.hi;                                                      // w0  D w1  D w2  D w3  D w4  D w5  D w6  D w7  D

  *xOut = x.s02468ACE;
  *yOut = y.s02468ACE;
  *zOut = z.s02468ACE;
  *wOut = w.s02468ACE;
}

void INTERNAL_INLINE_ATTRIBUTE __ocl_load_transpose_char4x8_AVX(private char4* pLoadAdd, private char8* xOut, private char8* yOut, private char8* zOut, private char8* wOut) {
  // We load "char16", meaning we load the full matrix in a 2 loads
  private char* tmpLoadAdd = (private char*)pLoadAdd;
  char16 xyzw0 = vload16(0, tmpLoadAdd);                                    // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3
  char16 xyzw1 = vload16(1, tmpLoadAdd);                                    // x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6 x7 y7 z7 w7

  __ocl_load_transpose_char4x8_common_AVX(xyzw0, xyzw1, xOut, yOut, zOut, wOut);
}

#if defined(__AVX__)
typedef __v32qi my_char32;

my_char32 INLINE_ATTRIBUTE as_my_char32(int8 x) {
  return __builtin_astype(x, my_char32);
}

void INLINE_ATTRIBUTE __ocl_masked_load_transpose_char4x8(private char4* pLoadAdd, private char8* xOut, private char8* yOut, private char8* zOut, private char8* wOut, int8 mask) {
  my_char32 xyzw;
  __ocl_masked_load_char4x8(pLoadAdd, (private char4*)&xyzw, mask);
  __ocl_load_transpose_char4x8((private char4*)&xyzw, xOut, yOut, zOut, wOut);
}
#endif // defined(__AVX__)

/// @brief Receives char4x8 matrix as 4 matrix rows, transposes it and outputs the matrix
///        2 halfs (2 char4x4 matrixes) using 2 char16, which create one char4x8 matrix
/// @param xyzw0  - Upper part of the transposed char4x8
/// @param xyzw1  - Lower part of the transposed char4x8
/// @param xIn    - Row 0 of the matrix to be transposed
/// @param yIn    - Row 1 of the matrix to be transposed
/// @param zIn    - Row 2 of the matrix to be transposed
/// @param wIn    - Row 3 of the matrix to be transposed
void inline INTERNAL_INLINE_ATTRIBUTE __ocl_transpose_store_char4x8_common_AVX(private char16* xyzw0, private char16* xyzw1, char8 xIn, char8 yIn, char8 zIn, char8 wIn) {

  char16 x;
  char16 y;
  char16 z;
  char16 w;

  x.s01234567 = xIn;                                                        // x0 x1 x2 x3 x4 x5 x6 x7  D  D  D  D  D  D  D  D
  y.s01234567 = yIn;                                                        // y0 y1 y2 y3 y4 y5 y6 y7  D  D  D  D  D  D  D  D
  z.s01234567 = zIn;                                                        // z0 z1 z2 z3 z4 z5 z6 z7  D  D  D  D  D  D  D  D
  w.s01234567 = wIn;                                                        // w0 w1 w2 w3 w4 w5 w6 w7  D  D  D  D  D  D  D  D

  uchar16 low16 = {0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23};
  short8 xy = as_short8(shuffle2(x, y, low16));                             // x0 y0 x1 y1 x2 y2 x3 y3 x4 y4 x5 y5 x6 y6 x7 y7
  short8 zw = as_short8(shuffle2(z, w, low16));                             // z0 w0 z1 w1 z2 w2 z3 w3 z4 w4 z5 w5 z6 w6 z7 w7

  ushort8 low8 = {0, 8, 1, 9, 2, 10, 3, 11};
  ushort8 high8 = {4, 12, 5, 13, 6, 14, 7, 15};
  *xyzw0 = as_char16(shuffle2(xy, zw, low8));                               // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3
  *xyzw1 = as_char16(shuffle2(xy, zw, high8));                              // x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6 x7 y7 z7 w7
}

void INTERNAL_INLINE_ATTRIBUTE __ocl_transpose_store_char4x8_AVX(char4* pStoreAdd, char8 xIn, char8 yIn, char8 zIn, char8 wIn) {

  char16 xyzw0;
  char16 xyzw1;
  __ocl_transpose_store_char4x8_common_AVX(&xyzw0, &xyzw1, xIn, yIn, zIn, wIn);

  // We store "char16", meaning we store the full matrix in a 2 stores
  char* tmpStoreAdd = (char*)pStoreAdd;
  vstore16(xyzw0, 0, tmpStoreAdd);
  vstore16(xyzw1, 1, tmpStoreAdd);
}

#if defined(__AVX__)
void INLINE_ATTRIBUTE __ocl_masked_transpose_store_char4x8(private char4* pStoreAdd, char8 xIn, char8 yIn, char8 zIn, char8 wIn, int8 mask) {

  my_char32 xyzw;
  __ocl_transpose_store_char4x8((private char4*)&xyzw, xIn, yIn, zIn, wIn);
  __ocl_masked_store_char4x8(pStoreAdd, (private char4*)&xyzw, mask);
}
#endif // defined(__AVX__)

void INTERNAL_INLINE_ATTRIBUTE __ocl_gather_transpose_char4x8_AVX(private char4* pLoadAdd0, private char4* pLoadAdd1, private char4* pLoadAdd2, private char4* pLoadAdd3,
                                  private char4* pLoadAdd4, private char4* pLoadAdd5, private char4* pLoadAdd6, private char4* pLoadAdd7,
                                  private char8* xOut, private char8* yOut, private char8* zOut, private char8* wOut) {

  // Broadcast the loaded values, all but the first element in the register which will be moved there
  int4 xyzw1 = *((private int*)pLoadAdd1);                                          // x1 y1 z1 w1 x1 y1 z1 w1 x1 y1 z1 w1 x1 y1 z1 w1
  int4 xyzw2 = *((private int*)pLoadAdd2);                                          // x2 y2 z2 w2 x2 y2 z2 w2 x2 y2 z2 w2 x2 y2 z2 w2
  int4 xyzw3 = *((private int*)pLoadAdd3);                                          // x3 y3 z3 w3 x3 y3 z3 w3 x3 y3 z3 w3 x3 y3 z3 w3

  int4 xyzw5 = *((private int*)pLoadAdd5);                                          // x5 y5 z5 w5 x5 y5 z5 w5 x5 y5 z5 w5 x5 y5 z5 w5
  int4 xyzw6 = *((private int*)pLoadAdd6);                                          // x6 y6 z6 w6 x6 y6 z6 w6 x6 y6 z6 w6 x6 y6 z6 w6
  int4 xyzw7 = *((private int*)pLoadAdd7);                                          // x7 y7 z7 w7 x7 y7 z7 w7 x7 y7 z7 w7 x7 y7 z7 w7

  int4 xyzwIn0;
  int4 xyzwIn1;
  // TODO : Replace this blend built-in with ?: when clang bug will be fixed
  // We don't have blendd in AVX, so we use blendps
  xyzwIn0.s0 = *((int*)pLoadAdd0);                                          // x0 y0 z0 w0  D  D  D  D  D  D  D  D  D  D  D  D
  xyzwIn0 = as_int4(_mm_blend_ps((__m128)xyzwIn0, (__m128)xyzw1, 0x2));     // x0 y0 z0 w0 x1 y1 z1 w1  D  D  D  D  D  D  D  D
  xyzwIn0 = as_int4(_mm_blend_ps((__m128)xyzwIn0, (__m128)xyzw2, 0x4));     // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2  D  D  D  D
  xyzwIn0 = as_int4(_mm_blend_ps((__m128)xyzwIn0, (__m128)xyzw3, 0x8));     // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3

  xyzwIn1.s0 = *((private int*)pLoadAdd4);                                          // x4 y4 z4 w4  D  D  D  D  D  D  D  D  D  D  D  D
  xyzwIn1 = as_int4(_mm_blend_ps((__m128)xyzwIn1, (__m128)xyzw5, 0x2));     // x4 y4 z4 w4 x5 y5 z5 w5  D  D  D  D  D  D  D  D
  xyzwIn1 = as_int4(_mm_blend_ps((__m128)xyzwIn1, (__m128)xyzw6, 0x4));     // x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6  D  D  D  D
  xyzwIn1 = as_int4(_mm_blend_ps((__m128)xyzwIn1, (__m128)xyzw7, 0x8));     // x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6 x7 y7 z7 w7

  __ocl_load_transpose_char4x8_common_AVX(as_char16(xyzwIn0), as_char16(xyzwIn1), xOut, yOut, zOut, wOut);
}

void INTERNAL_INLINE_ATTRIBUTE __ocl_transpose_scatter_char4x8_AVX(private char4* pStoreAdd0, private char4* pStoreAdd1, private char4* pStoreAdd2, private char4* pStoreAdd3,
                                   private char4* pStoreAdd4, private char4* pStoreAdd5, private char4* pStoreAdd6, private char4* pStoreAdd7,
                                   char8 xIn, char8 yIn, char8 zIn, char8 wIn) {

  int4 xyzw0 = 0;
  int4 xyzw1 = 0;
  __ocl_transpose_store_char4x8_common_AVX((private char16*)&xyzw0, (private char16*)&xyzw1, xIn, yIn, zIn, wIn);

  *((private int*)pStoreAdd0) = xyzw0.s0;
  *((private int*)pStoreAdd1) = xyzw0.s1;
  *((private int*)pStoreAdd2) = xyzw0.s2;
  *((private int*)pStoreAdd3) = xyzw0.s3;
  *((private int*)pStoreAdd4) = xyzw1.s0;
  *((private int*)pStoreAdd5) = xyzw1.s1;
  *((private int*)pStoreAdd6) = xyzw1.s2;
  *((private int*)pStoreAdd7) = xyzw1.s3;
}

#if defined(__AVX__)
void INLINE_ATTRIBUTE __ocl_masked_gather_transpose_char4x8(private char4* pLoadAdd0, private char4* pLoadAdd1, private char4* pLoadAdd2, private char4* pLoadAdd3,
                              private char4* pLoadAdd4, private char4* pLoadAdd5, private char4* pLoadAdd6, private char4* pLoadAdd7,
                              private char8* xOut, private char8* yOut, private char8* zOut, private char8* wOut, int8 mask) {
  if (all(mask)) {
   __ocl_gather_transpose_char4x8( pLoadAdd0, pLoadAdd1, pLoadAdd2, pLoadAdd3,
                            pLoadAdd4, pLoadAdd5, pLoadAdd6, pLoadAdd7,
                            xOut, yOut, zOut, wOut);
    return;
  }

  char4 dummy;

  private char4* xyzw0 = mask.s0 ? pLoadAdd0 : &dummy;
  private char4* xyzw1 = mask.s1 ? pLoadAdd1 : &dummy;
  private char4* xyzw2 = mask.s2 ? pLoadAdd2 : &dummy;
  private char4* xyzw3 = mask.s3 ? pLoadAdd3 : &dummy;
  private char4* xyzw4 = mask.s4 ? pLoadAdd4 : &dummy;
  private char4* xyzw5 = mask.s5 ? pLoadAdd5 : &dummy;
  private char4* xyzw6 = mask.s6 ? pLoadAdd6 : &dummy;
  private char4* xyzw7 = mask.s7 ? pLoadAdd7 : &dummy;

	__ocl_gather_transpose_char4x8( xyzw0, xyzw1, xyzw2, xyzw3,
                            xyzw4, xyzw5, xyzw6, xyzw7,
                            xOut, yOut, zOut, wOut);
}

void INLINE_ATTRIBUTE __ocl_masked_transpose_scatter_char4x8(private char4* pStoreAdd0, private char4* pStoreAdd1, private char4* pStoreAdd2, private char4* pStoreAdd3,
                               private char4* pStoreAdd4, private char4* pStoreAdd5, private char4* pStoreAdd6, private char4* pStoreAdd7,
                               char8 xIn, char8 yIn, char8 zIn, char8 wIn, int8 mask){
  if (all(mask)) {
    __ocl_transpose_scatter_char4x8(pStoreAdd0, pStoreAdd1, pStoreAdd2, pStoreAdd3,
                            pStoreAdd4, pStoreAdd5, pStoreAdd6, pStoreAdd7,
                            xIn, yIn, zIn, wIn);
    return;
  }

  char4 dummy;

  private char4* xyzw0 = mask.s0 ? pStoreAdd0 : &dummy;
  private char4* xyzw1 = mask.s1 ? pStoreAdd1 : &dummy;
  private char4* xyzw2 = mask.s2 ? pStoreAdd2 : &dummy;
  private char4* xyzw3 = mask.s3 ? pStoreAdd3 : &dummy;
  private char4* xyzw4 = mask.s4 ? pStoreAdd4 : &dummy;
  private char4* xyzw5 = mask.s5 ? pStoreAdd5 : &dummy;
  private char4* xyzw6 = mask.s6 ? pStoreAdd6 : &dummy;
  private char4* xyzw7 = mask.s7 ? pStoreAdd7 : &dummy;

	__ocl_transpose_scatter_char4x8(xyzw0, xyzw1, xyzw2, xyzw3,
                            xyzw4, xyzw5, xyzw6, xyzw7,
                            xIn, yIn, zIn, wIn);
}
#endif // defined(__AVX__)


#if defined(__AVX2__)

/// @brief Receives char8x4 matrix as char32, transposes it and outputs the rows
///        of the transposed matrix
/// @param xyzwIn - char8x4 matrix to be transposed
/// @param xOut   - Row 0 of the transposed matrix
/// @param yOut   - Row 1 of the transposed matrix
/// @param zOut   - Row 2 of the transposed matrix
/// @param wOut   - Row 3 of the transposed matrix
void INTERNAL_INLINE_ATTRIBUTE __ocl_load_transpose_char4x8_common_AVX2(my_char32 xyzwIn, private char8* xOut, private char8* yOut, private char8* zOut, private char8* wOut) {

  my_char32 xyzw = xyzwIn;                                                  // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3 | x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6 x7 y7 z7 w7
  my_char32 dummy;
  // The reason we use here __builtin_shufflevector and not shuffle(), shuffle2() OpenCL functions
  // is because the input typeses here are "char32", which are not legal OpenCL built-in types.
  // We do not want to sue "__builtin_shufflevector" everywhere because it's not a legal OpenCL function,
  // it's kind of a hack that makes Clang create LLVM shufflevector instructions.
  xyzw = __builtin_shufflevector (xyzw, dummy,                              // x0 x1 x2 x3 y0 y1 y2 y3 z0 z1 z2 z3 w0 w1 w2 w3 | x4 x5 x6 x7 y4 y5 y6 y7 z4 z5 z6 z7 w4 w5 w6 w7
          0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15,
          16, 20, 24, 28, 17, 21, 25, 29, 18, 22, 26, 30, 19, 23, 27, 31);
  xyzw = as_my_char32((as_int8(xyzw)).s04152637);                           // x0 x1 x2 x3 x4 x5 x6 x7 y0 y1 y2 y3 y4 y5 y6 y7 | z0 z1 z2 z3 z4 z5 z6 z7 w0 w1 w2 w3 w4 w5 w6 w7

  my_char32 xz = __builtin_shufflevector (xyzw, dummy,                      // x0  D x1  D x2  D x3  D x4  D x5  D x6  D x7  D | z0  D z1  D z2  D z3  D z4  D z5  D z6  D z7  D
          0, 32, 1, 33, 2, 34, 3, 35, 4, 36, 5, 37, 6, 38, 7, 39,
          16, 48, 17, 49, 18, 50, 19, 51, 20, 52, 21, 53, 22, 54, 23, 55);
  my_char32 yw = __builtin_shufflevector (xyzw, dummy,                      // y0  D y1  D y2  D y3  D y4  D y5  D y6  D y7  D | w0  D w1  D w2  D w3  D w4  D w5  D w6  D w7  D
         8, 40, 9, 41, 10, 42, 11, 43, 12, 44, 13, 45, 14, 46, 15, 47,
         24, 56, 25, 57, 26, 58, 27, 59, 28, 60, 29, 61, 30, 62, 31, 63);

  char16 x = as_char16((as_int8(xz)).lo);                                   // x0  D x1  D x2  D x3  D x4  D x5  D x6  D x7  D
  char16 y = as_char16((as_int8(yw)).lo);                                   // y0  D y1  D y2  D y3  D y4  D y5  D y6  D y7  D
  char16 z = as_char16((as_int8(xz)).hi);                                   // z0  D z1  D z2  D z3  D z4  D z5  D z6  D z7  D
  char16 w = as_char16((as_int8(yw)).hi);                                   // w0  D w1  D w2  D w3  D w4  D w5  D w6  D w7  D

  *xOut = x.s02468ACE;
  *yOut = y.s02468ACE;
  *zOut = z.s02468ACE;
  *wOut = w.s02468ACE;

}

void INTERNAL_INLINE_ATTRIBUTE __ocl_load_transpose_char4x8_AVX2(private char4* pLoadAdd, private char8* xOut, private char8* yOut, private char8* zOut, private char8* wOut) {

  // We load "int8", meaning we load the full matrix in a single load
  private int* tmpLoadAdd = (private int*)pLoadAdd;
  my_char32 xyzw = (my_char32) vload8(0, tmpLoadAdd);                       // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3 | x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6 x7 y7 z7 w7

  __ocl_load_transpose_char4x8_common_AVX2(xyzw, xOut, yOut, zOut, wOut);
}

/// @brief Receives char4x8 matrix as 4 matrix rows, transposes it and outputs the matrix
///        as a whole using char32
/// @param xyzwOut  - This parameter will contain the transposed char4x8 matrix
/// @param xIn      - Row 0 of the matrix to be transposed
/// @param yIn      - Row 1 of the matrix to be transposed
/// @param zIn      - Row 2 of the matrix to be transposed
/// @param wIn      - Row 3 of the matrix to be transposed
void INTERNAL_INLINE_ATTRIBUTE __ocl_transpose_store_char4x8_common_AVX2(private my_char32* xyzwOut, char8 xIn, char8 yIn, char8 zIn, char8 wIn) {

  char16 x;
  char16 y;
  char16 z;
  char16 w;

  x.s02468ACE = xIn;                                                        // x0  D x1  D x2  D x3  D x4  D x5  D x6  D x7  D
  y.s02468ACE = yIn;                                                        // y0  D y1  D y2  D y3  D y4  D y5  D y6  D y7  D
  z.s02468ACE = zIn;                                                        // z0  D z1  D z2  D z3  D z4  D z5  D z6  D z7  D
  w.s02468ACE = wIn;                                                        // w0  D w1  D w2  D w3  D w4  D w5  D w6  D w7  D


  my_char32 xz;
  my_char32 yw;

  (*(private int8*)&xz).lo = as_int4(x);                                            // x0  D x1  D x2  D x3  D x4  D x5  D x6  D x7  D |  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D
  (*(private int8*)&yw).lo = as_int4(y);                                            // y0  D y1  D y2  D y3  D y4  D y5  D y6  D y7  D |  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D
  (*(private int8*)&xz).hi = as_int4(z);                                            // x0  D x1  D x2  D x3  D x4  D x5  D x6  D x7  D | z0  D z1  D z2  D z3  D z4  D z5  D z6  D z7  D
  (*(private int8*)&yw).hi = as_int4(w);                                            // y0  D y1  D y2  D y3  D y4  D y5  D y6  D y7  D | w0  D w1  D w2  D w3  D w4  D w5  D w6  D w7  D

  // The reason we use here __builtin_shufflevector and not shuffle(), shuffle2() OpenCL functions
  // is because the input typeses here are "char32", which are not legal OpenCL built-in types.
  // We do not want to sue "__builtin_shufflevector" everywhere because it's not a legal OpenCL function,
  // it's kind of a hack that makes Clang create LLVM shufflevector instructions.
  my_char32 dummy;
  xz = __builtin_shufflevector (xz, dummy,                                  // x0 x1 x2 x3 x4 x5 x6 x7  D  D  D  D  D  D  D  D | z0 z1 z2 z3 z4 z5 z6 z7  D  D  D  D  D  D  D  D
          0, 2, 4, 6, 8, 10, 12, 14, 0, 0, 0, 0, 0, 0, 0, 0,
          16, 18, 20, 22, 24, 26, 28, 30, 0, 0, 0, 0, 0, 0, 0, 0);
  yw = __builtin_shufflevector (yw, dummy,                                  // y0 y1 y2 y3 y4 y5 y6 y7  D  D  D  D  D  D  D  D | w0 w1 w2 w3 w4 w5 w6 w7  D  D  D  D  D  D  D  D
          0, 2, 4, 6, 8, 10, 12, 14, 0, 0, 0, 0, 0, 0, 0, 0,
          16, 18, 20, 22, 24, 26, 28, 30, 0, 0, 0, 0, 0, 0, 0, 0);


  my_char32 xyzw = __builtin_shufflevector (xz, yw,                         // x0 y0 x1 y1 x2 y2 x3 y3 x4 y4 x5 y5 x6 y6 x7 y7 | z0 w0 z1 w1 z2 w2 z3 w3 z4 w4 z5 w5 z6 w6 z7 w7
          0, 32, 1, 33, 2, 34, 3, 35, 4, 36, 5, 37, 6, 38, 7, 39,
          16, 48, 17, 49, 18, 50, 19, 51, 20, 52, 21, 53, 22, 54, 23, 55);
  xyzw = as_my_char32((as_int8(xyzw)).s01452367);                           // x0 y0 x1 y1 x2 y2 x3 y3 z0 w0 z1 w1 z2 w2 z3 w3 | x4 y4 x5 y5 x6 y6 x7 y7 z4 w4 z5 w5 z6 w6 z7 w7


  xyzw =  __builtin_shufflevector (xyzw, dummy,                             // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3 | x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6 x7 y7 z7 w7
          0, 1, 8, 9, 2, 3, 10, 11, 4, 5, 12, 13, 6, 7, 14, 15,
          16, 17, 24, 25, 18, 19, 26, 27, 20, 21, 28, 29, 22, 23, 30, 31);

  *xyzwOut = xyzw;
}

void INTERNAL_INLINE_ATTRIBUTE __ocl_transpose_store_char4x8_AVX2(private char4* pStoreAdd, char8 xIn, char8 yIn, char8 zIn, char8 wIn) {

  my_char32 xyzw;
  __ocl_transpose_store_char4x8_common_AVX2(&xyzw, xIn, yIn, zIn, wIn);

  // We store "int8", meaning we store the full matrix in a single store
  private int* tmpStoreAdd = (private int*)pStoreAdd;
  vstore8(as_int8(xyzw), 0, tmpStoreAdd);
}

void INTERNAL_INLINE_ATTRIBUTE __ocl_gather_transpose_char4x8_AVX2( private char4* pLoadAdd0, private char4* pLoadAdd1, private char4* pLoadAdd2, private char4* pLoadAdd3,
                                    private char4* pLoadAdd4, private char4* pLoadAdd5, private char4* pLoadAdd6, private char4* pLoadAdd7,
                                    private char8* xOut, private char8* yOut, private char8* zOut, private char8* wOut) {

  // Broadcast the loaded values, all but the first element in the register which will be moved there
  int8 xyzw1 = *((private int*)pLoadAdd1);                                          // x1 y1 z1 w1 x1 y1 z1 w1 x1 y1 z1 w1 x1 y1 z1 w1
  int8 xyzw2 = *((private int*)pLoadAdd2);                                          // x2 y2 z2 w2 x2 y2 z2 w2 x2 y2 z2 w2 x2 y2 z2 w2
  int8 xyzw3 = *((private int*)pLoadAdd3);                                          // x3 y3 z3 w3 x3 y3 z3 w3 x3 y3 z3 w3 x3 y3 z3 w3
  int8 xyzw4 = *((private int*)pLoadAdd4);                                          // x4 y4 z4 w4 x4 y4 z4 w4 x4 y4 z4 w4 x4 y4 z4 w4
  int8 xyzw5 = *((private int*)pLoadAdd5);                                          // x5 y5 z5 w5 x5 y5 z5 w5 x5 y5 z5 w5 x5 y5 z5 w5
  int8 xyzw6 = *((private int*)pLoadAdd6);                                          // x6 y6 z6 w6 x6 y6 z6 w6 x6 y6 z6 w6 x6 y6 z6 w6
  int8 xyzw7 = *((private int*)pLoadAdd7);                                          // x7 y7 z7 w7 x7 y7 z7 w7 x7 y7 z7 w7 x7 y7 z7 w7

  int8 xyzw = 0;
  // TODO : Replace this blend built-in with ?: when clang bug will be fixed
  xyzw.s0 = *((int*)pLoadAdd0);                                             // x0 y0 z0 w0  D  D  D  D  D  D  D  D  D  D  D  D |  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D
  xyzw = as_int8(_mm256_blend_epi32((__m256i)xyzw, (__m256i)xyzw1, 0x2));   // x0 y0 z0 w0 x1 y1 z1 w1  D  D  D  D  D  D  D  D |  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D
  xyzw = as_int8(_mm256_blend_epi32((__m256i)xyzw, (__m256i)xyzw2, 0x4));   // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2  D  D  D  D |  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D
  xyzw = as_int8(_mm256_blend_epi32((__m256i)xyzw, (__m256i)xyzw3, 0x8));   // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3 |  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D
  xyzw = as_int8(_mm256_blend_epi32((__m256i)xyzw, (__m256i)xyzw4, 0x10));  // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3 | x4 y4 z4 w4  D  D  D  D  D  D  D  D  D  D  D  D
  xyzw = as_int8(_mm256_blend_epi32((__m256i)xyzw, (__m256i)xyzw5, 0x20));  // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3 | x4 y4 z4 w4 x5 y5 z5 w5  D  D  D  D  D  D  D  D
  xyzw = as_int8(_mm256_blend_epi32((__m256i)xyzw, (__m256i)xyzw6, 0x40));  // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3 | x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6  D  D  D  D
  xyzw = as_int8(_mm256_blend_epi32((__m256i)xyzw, (__m256i)xyzw7, 0x80));  // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3 | x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6 x7 y7 z7 w7

  __ocl_load_transpose_char4x8_common_AVX2((my_char32)xyzw, xOut, yOut, zOut, wOut);
}

void INTERNAL_INLINE_ATTRIBUTE __ocl_transpose_scatter_char4x8_AVX2(private char4* pStoreAdd0, private char4* pStoreAdd1, private char4* pStoreAdd2, private char4* pStoreAdd3,
                                    private char4* pStoreAdd4, private char4* pStoreAdd5, private char4* pStoreAdd6, private char4* pStoreAdd7,
                                    char8 xIn, char8 yIn, char8 zIn, char8 wIn) {

  int8 xyzw = 0;
  __ocl_transpose_store_char4x8_common_AVX2((private my_char32*)&xyzw, xIn, yIn, zIn, wIn);

  *((private int*)pStoreAdd0) = xyzw.s0;
  *((private int*)pStoreAdd1) = xyzw.s1;
  *((private int*)pStoreAdd2) = xyzw.s2;
  *((private int*)pStoreAdd3) = xyzw.s3;
  *((private int*)pStoreAdd4) = xyzw.s4;
  *((private int*)pStoreAdd5) = xyzw.s5;
  *((private int*)pStoreAdd6) = xyzw.s6;
  *((private int*)pStoreAdd7) = xyzw.s7;
}

#endif // defined(__AVX2__)

void INLINE_ATTRIBUTE __ocl_load_transpose_char4x8(private char4* pLoadAdd, private char8* xOut, private char8* yOut, private char8* zOut, private char8* wOut) {
#if defined(__AVX2__)
  __ocl_load_transpose_char4x8_AVX2(pLoadAdd, xOut, yOut, zOut, wOut);
#else // defined(__SSE4_2__)
  __ocl_load_transpose_char4x8_AVX(pLoadAdd, xOut, yOut, zOut, wOut);
#endif
}

void INLINE_ATTRIBUTE __ocl_transpose_store_char4x8(private char4* pStoreAdd, char8 xIn, char8 yIn, char8 zIn, char8 wIn) {
#if defined(__AVX2__)
  __ocl_transpose_store_char4x8_AVX2(pStoreAdd, xIn, yIn, zIn, wIn);
#else // defined(__SSE4_2__)
  __ocl_transpose_store_char4x8_AVX(pStoreAdd, xIn, yIn, zIn, wIn);
#endif
}

void INLINE_ATTRIBUTE __ocl_gather_transpose_char4x8(private char4* pLoadAdd0, private char4* pLoadAdd1, private char4* pLoadAdd2, private char4* pLoadAdd3,
                              private char4* pLoadAdd4, private char4* pLoadAdd5, private char4* pLoadAdd6, private char4* pLoadAdd7,
                              private char8* xOut, private char8* yOut, private char8* zOut, private char8* wOut) {
#if defined(__AVX2__)
 __ocl_gather_transpose_char4x8_AVX2(pLoadAdd0, pLoadAdd1, pLoadAdd2, pLoadAdd3,
                                pLoadAdd4, pLoadAdd5, pLoadAdd6, pLoadAdd7,
                                xOut, yOut, zOut, wOut);
#else // defined(__SSE4_2__)
 __ocl_gather_transpose_char4x8_AVX( pLoadAdd0, pLoadAdd1, pLoadAdd2, pLoadAdd3,
                                pLoadAdd4, pLoadAdd5, pLoadAdd6, pLoadAdd7,
                                xOut, yOut, zOut, wOut);
#endif
}

void INLINE_ATTRIBUTE __ocl_transpose_scatter_char4x8(private char4* pStoreAdd0, private char4* pStoreAdd1, private char4* pStoreAdd2, private char4* pStoreAdd3,
                               private char4* pStoreAdd4, private char4* pStoreAdd5, private char4* pStoreAdd6, private char4* pStoreAdd7,
                               char8 xIn, char8 yIn, char8 zIn, char8 wIn) {
#if defined(__AVX2__)
  __ocl_transpose_scatter_char4x8_AVX2( pStoreAdd0, pStoreAdd1, pStoreAdd2, pStoreAdd3,
                                  pStoreAdd4, pStoreAdd5, pStoreAdd6, pStoreAdd7,
                                  xIn, yIn, zIn, wIn);
#else // defined(__SSE4_2__)
  __ocl_transpose_scatter_char4x8_AVX(  pStoreAdd0, pStoreAdd1, pStoreAdd2, pStoreAdd3,
                                  pStoreAdd4, pStoreAdd5, pStoreAdd6, pStoreAdd7,
                                  xIn, yIn, zIn, wIn);
#endif
}


// ****************************************************************************
//                                 short4x8
// ****************************************************************************


#if defined(__AVX__)

void INTERNAL_INLINE_ATTRIBUTE __ocl_load_transpose_short4x8_AVX_common(short16 xyzw0, short16 xyzw1,
                        private short8* xOut, private short8* yOut, private short8* zOut, private short8* wOut) {

  ushort16 maskUnpackLo = {0, 16, 1, 17, 2, 18, 3, 19,  8, 24,  9, 25, 10, 26, 11, 27};
  ushort16 maskUnpackHi = {4, 20, 5, 21, 6, 22, 7, 23, 12, 28, 13, 29, 14, 30, 15, 31};
  uint8 maskPermute = {0, 4, 1, 5, 2, 6, 3, 7};

  short16 tmp1 = shuffle2(xyzw0, xyzw1, maskUnpackLo);              // x0 x4 y0 y4 z0 z4 w0 w4 x2 x6 y2 y6 z2 z6 w2 w6
  short16 tmp2 = shuffle2(xyzw0, xyzw1, maskUnpackHi);              // x1 x5 y1 y5 z1 z5 w1 w5 x3 x7 y3 y7 z3 z7 w3 w7

  short16 tmp3 = shuffle2(tmp1, tmp2, maskUnpackLo);                // x0 x1 x4 x5 y0 y1 y4 y5 x2 x3 x6 x7 y2 y3 y6 y7
  short16 tmp4 = shuffle2(tmp1, tmp2, maskUnpackHi);                // z0 z1 z4 z5 w0 w1 w4 w5 z2 z3 z6 z7 w2 w3 w6 w7

  short16 tmp5 = as_short16(shuffle(as_float8(tmp3), maskPermute)); // x0 x1 x2 x3 x4 x5 x6 x7 y0 y1 y2 y3 y4 y5 y6 y7
  short16 tmp6 = as_short16(shuffle(as_float8(tmp4), maskPermute)); // z0 z1 z2 z3 z4 z5 z6 z7 w0 w1 w2 w3 w4 w5 w6 w7

  *xOut = tmp5.lo;                                                  // x0 x1 x2 x3 x4 x5 x6 x7
  *yOut = tmp5.hi;                                                  // y0 y1 y2 y3 y4 y5 y6 y7
  *zOut = tmp6.lo;                                                  // z0 z1 z2 z3 z4 z5 z6 z7
  *wOut = tmp6.hi;                                                  // w0 w1 w2 w3 w4 w5 w6 w7
}


void INTERNAL_INLINE_ATTRIBUTE __ocl_transpose_store_short4x8_AVX_common(private short16* xyzw0, private short16* xyzw1,
                        short8 xIn, short8 yIn, short8 zIn, short8 wIn) {

  ushort16 low16 = {0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15};
  float8 xy = as_float8(shuffle2(xIn, yIn, low16));                 // x0 y0 x1 y1 x2 y2 x3 y3 x4 y4 x5 y5 x6 y6 x7 y7
  float8 zw = as_float8(shuffle2(zIn, wIn, low16));                 // z0 w0 z1 w1 z2 w2 z3 w3 z4 w4 z5 w5 z6 w6 z7 w7

  uint8 low8  = {0, 8, 1, 9, 2, 10, 3, 11};
  uint8 high8 = {4, 12, 5, 13, 6, 14, 7, 15};
  *xyzw0 = as_short16(shuffle2(xy, zw, low8));                      // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3
  *xyzw1 = as_short16(shuffle2(xy, zw, high8));                     // x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6 x7 y7 z7 w7
}


void INTERNAL_INLINE_ATTRIBUTE __ocl_load_transpose_short4x8_AVX(private short4* pLoadAdd, private short8* xOut, private short8* yOut, private short8* zOut, private short8* wOut) {

  short16 xyzw0 = vload16(0, (short*)pLoadAdd);                     // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3
  short16 xyzw1 = vload16(1, (short*)pLoadAdd);                     // x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6 x7 y7 z7 w7
  __ocl_load_transpose_short4x8_AVX_common(xyzw0, xyzw1, xOut, yOut, zOut, wOut);
}


void INTERNAL_INLINE_ATTRIBUTE __ocl_transpose_store_short4x8_AVX(private short4* pStoreAdd, short8 xIn, short8 yIn, short8 zIn, short8 wIn) {

  short16 xyzw0, xyzw1;
  __ocl_transpose_store_short4x8_AVX_common(&xyzw0, &xyzw1, xIn, yIn, zIn, wIn);

  vstore16(xyzw0, 0, (private short*)pStoreAdd);
  vstore16(xyzw1, 1, (private short*)pStoreAdd);
}


#if defined(__AVX2__)


void INTERNAL_INLINE_ATTRIBUTE __ocl_load_transpose_short4x8_AVX2_common(short16 xyzw0, short16 xyzw1,
                    private short8* xOut, private short8* yOut, private short8* zOut, private short8* wOut) {

  ushort16 maskUnpackLo = {0, 16, 1, 17, 2, 18, 3, 19,  8, 24,  9, 25, 10, 26, 11, 27};
  ushort16 maskUnpackHi = {4, 20, 5, 21, 6, 22, 7, 23, 12, 28, 13, 29, 14, 30, 15, 31};
  uint8    maskPermute  = {0, 4, 1, 5, 2, 6, 3, 7};

  short16 tmp1 = shuffle2(xyzw0, xyzw1, maskUnpackLo);              // x0 x4 y0 y4 z0 z4 w0 w4 x2 x6 y2 y6 z2 z6 w2 w6
  short16 tmp2 = shuffle2(xyzw0, xyzw1, maskUnpackHi);              // x1 x5 y1 y5 z1 z5 w1 w5 x3 x7 y3 y7 z3 z7 w3 w7

  short16 tmp3 = shuffle2(tmp1, tmp2, maskUnpackLo);                // x0 x1 x4 x5 y0 y1 y4 y5 x2 x3 x6 x7 y2 y3 y6 y7
  short16 tmp4 = shuffle2(tmp1, tmp2, maskUnpackHi);                // z0 z1 z4 z5 w0 w1 w4 w5 z2 z3 z6 z7 w2 w3 w6 w7

  short16 tmp5 = as_short16(shuffle(as_int8(tmp3), maskPermute));   // x0 x1 x2 x3 x4 x5 x6 x7 y0 y1 y2 y3 y4 y5 y6 y7
  short16 tmp6 = as_short16(shuffle(as_int8(tmp4), maskPermute));   // z0 z1 z2 z3 z4 z5 z6 z7 w0 w1 w2 w3 w4 w5 w6 w7

  *xOut = tmp5.lo;                                                  // x0 x1 x2 x3 x4 x5 x6 x7
  *yOut = tmp5.hi;                                                  // y0 y1 y2 y3 y4 y5 y6 y7
  *zOut = tmp6.lo;                                                  // z0 z1 z2 z3 z4 z5 z6 z7
  *wOut = tmp6.hi;                                                  // w0 w1 w2 w3 w4 w5 w6 w7
}


void INTERNAL_INLINE_ATTRIBUTE __ocl_transpose_store_short4x8_AVX2_common(private short16* xyzw0, private short16* xyzw1,
                    short8 xIn, short8 yIn, short8 zIn, short8 wIn) {

 ushort16 low16 = {0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15};

  int8 xy = as_int8(shuffle2(xIn, yIn, low16));                     // x0 y0 x1 y1 x2 y2 x3 y3 x4 y4 x5 y5 x6 y6 x7 y7
  int8 zw = as_int8(shuffle2(zIn, wIn, low16));                     // z0 w0 z1 w1 z2 w2 z3 w3 z4 w4 z5 w5 z6 w6 z7 w7

  uint8 low8  = {0, 8, 1, 9, 2, 10, 3, 11};
  uint8 high8 = {4, 12, 5, 13, 6, 14, 7, 15};
  *xyzw0 = as_short16(shuffle2(xy, zw, low8));                      // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3
  *xyzw1 = as_short16(shuffle2(xy, zw, high8));                     // x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6 x7 y7 z7 w7
}


void INTERNAL_INLINE_ATTRIBUTE __ocl_load_transpose_short4x8_AVX2(private short4* pLoadAdd, private short8* xOut, private short8* yOut, private short8* zOut, private short8* wOut) {

  short16 xyzw0 = vload16(0, (private short*)pLoadAdd);                     // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3
  short16 xyzw1 = vload16(1, (private short*)pLoadAdd);                     // x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6 x7 y7 z7 w7

  __ocl_load_transpose_short4x8_AVX2_common(xyzw0, xyzw1, xOut, yOut, zOut, wOut);
}


void INTERNAL_INLINE_ATTRIBUTE __ocl_transpose_store_short4x8_AVX2(private short4* pStoreAdd, short8 xIn, short8 yIn, short8 zIn, short8 wIn) {

  short16 xyzw0, xyzw1;
  __ocl_transpose_store_short4x8_AVX2_common(&xyzw0, &xyzw1, xIn, yIn, zIn, wIn);

  vstore16(xyzw0, 0, (private short*)pStoreAdd);
  vstore16(xyzw1, 1, (private short*)pStoreAdd);
}


#endif // defined(__AVX2__)


void INLINE_ATTRIBUTE __ocl_load_transpose_short4x8(private short4* pLoadAdd, private short8* xOut, private short8* yOut, private short8* zOut, private short8* wOut) {
#if defined(__AVX2__)
  __ocl_load_transpose_short4x8_AVX2(pLoadAdd, xOut, yOut, zOut, wOut);
#else // defined(__AVX__)
  __ocl_load_transpose_short4x8_AVX(pLoadAdd, xOut, yOut, zOut, wOut);
#endif
}


void INLINE_ATTRIBUTE __ocl_transpose_store_short4x8(private short4* pStoreAdd, short8 xIn, short8 yIn, short8 zIn, short8 wIn) {
#if defined(__AVX2__)
  __ocl_transpose_store_short4x8_AVX2(pStoreAdd, xIn, yIn, zIn, wIn);
#else // defined(__AVX__)
  __ocl_transpose_store_short4x8_AVX(pStoreAdd, xIn, yIn, zIn, wIn);
#endif
}


void INLINE_ATTRIBUTE __ocl_gather_transpose_short4x8(private short4* pLoadAdd0, private short4* pLoadAdd1, private short4* pLoadAdd2, private short4* pLoadAdd3,
                                private short4* pLoadAdd4, private short4* pLoadAdd5, private short4* pLoadAdd6, private short4* pLoadAdd7,
                                private short8* xOut, private short8* yOut, private short8* zOut, private short8* wOut) {
  double4 xyzw0123;
  xyzw0123.s0 = *((private double*)pLoadAdd0);                                              // x0 y0 z0 w0  D  D  D  D  D  D  D  D  D  D  D  D
  xyzw0123.s1 = *((private double*)pLoadAdd1);                                              // x0 y0 z0 w0 x1 y1 z1 w1  D  D  D  D  D  D  D  D
  xyzw0123.s2 = *((private double*)pLoadAdd2);                                              // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2  D  D  D  D
  xyzw0123.s3 = *((private double*)pLoadAdd3);                                              // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3

  double4 xyzw4567;
  xyzw4567.s0 = *((private double*)pLoadAdd4);                                              // x4 y4 z4 w4  D  D  D  D  D  D  D  D  D  D  D  D
  xyzw4567.s1 = *((private double*)pLoadAdd5);                                              // x4 y4 z4 w4 x5 y5 z5 w5  D  D  D  D  D  D  D  D
  xyzw4567.s2 = *((private double*)pLoadAdd6);                                              // x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6  D  D  D  D
  xyzw4567.s3 = *((private double*)pLoadAdd7);                                              // x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6 x7 y7 z7 w7

#if defined(__AVX2__)
  __ocl_load_transpose_short4x8_AVX2_common(as_short16(xyzw0123), as_short16(xyzw4567), xOut, yOut, zOut, wOut);
#else // defined(__AVX__)
  __ocl_load_transpose_short4x8_AVX_common(as_short16(xyzw0123), as_short16(xyzw4567), xOut, yOut, zOut, wOut);
#endif
}


void INLINE_ATTRIBUTE __ocl_transpose_scatter_short4x8(private short4* pStoreAdd0, private short4* pStoreAdd1, private short4* pStoreAdd2, private short4* pStoreAdd3,
                                private short4* pStoreAdd4, private short4* pStoreAdd5, private short4* pStoreAdd6, private short4* pStoreAdd7,
                                short8 xIn, short8 yIn, short8 zIn, short8 wIn) {

  short16 xyzw0, xyzw1;

#if defined(__AVX2__)
  __ocl_transpose_store_short4x8_AVX2_common(&xyzw0, &xyzw1, xIn, yIn, zIn, wIn);
#else // defined(__AVX__)
  __ocl_transpose_store_short4x8_AVX_common(&xyzw0, &xyzw1, xIn, yIn, zIn, wIn);
#endif

  double4 temp0 = as_double4(xyzw0);
  double4 temp1 = as_double4(xyzw1);

  *((private double*)pStoreAdd0) = temp0.s0;                      // x0 y0 z0 w0
  *((private double*)pStoreAdd1) = temp0.s1;                      // x1 y1 z1 w1
  *((private double*)pStoreAdd2) = temp0.s2;                      // x2 y2 z2 w2
  *((private double*)pStoreAdd3) = temp0.s3;                      // x3 y3 z3 w3
  *((private double*)pStoreAdd4) = temp1.s0;                      // x4 y4 z4 w4
  *((private double*)pStoreAdd5) = temp1.s1;                      // x5 y5 z5 w5
  *((private double*)pStoreAdd6) = temp1.s2;                      // x6 y6 z6 w6
  *((private double*)pStoreAdd7) = temp1.s3;                      // x7 y7 z7 w7
}


void INLINE_ATTRIBUTE __ocl_masked_load_transpose_short4x8(private short4* pLoadAdd, private short8* xOut, private short8* yOut, private short8* zOut, private short8* wOut, int8 mask) {
  short4 xyzw[8];
  __ocl_masked_load_short4x8(pLoadAdd, xyzw, mask);
  __ocl_load_transpose_short4x8(xyzw, xOut, yOut, zOut, wOut);
}


void INLINE_ATTRIBUTE __ocl_masked_transpose_store_short4x8(private short4* pStoreAdd, short8 xIn, short8 yIn, short8 zIn, short8 wIn, int8 mask) {
  short4 xyzw[8];
  __ocl_transpose_store_short4x8(xyzw, xIn, yIn, zIn, wIn);
  __ocl_masked_store_short4x8(pStoreAdd, xyzw, mask);
}


void INLINE_ATTRIBUTE __ocl_masked_gather_transpose_short4x8(private short4* pLoadAdd0, private short4* pLoadAdd1, private short4* pLoadAdd2, private short4* pLoadAdd3,
                                      private short4* pLoadAdd4, private short4* pLoadAdd5, private short4* pLoadAdd6, private short4* pLoadAdd7,
                                      private short8* xOut, private short8* yOut, private short8* zOut, private short8* wOut, int8 mask) {
  if (all(mask)) {
    __ocl_gather_transpose_short4x8(pLoadAdd0, pLoadAdd1, pLoadAdd2, pLoadAdd3,
                              pLoadAdd4, pLoadAdd5, pLoadAdd6, pLoadAdd7,
                              xOut, yOut, zOut, wOut);
    return;
  }

  short4 dummy;

  private short4* xyzw0 = mask.s0 ? pLoadAdd0 : &dummy;
  private short4* xyzw1 = mask.s1 ? pLoadAdd1 : &dummy;
  private short4* xyzw2 = mask.s2 ? pLoadAdd2 : &dummy;
  private short4* xyzw3 = mask.s3 ? pLoadAdd3 : &dummy;
  private short4* xyzw4 = mask.s4 ? pLoadAdd4 : &dummy;
  private short4* xyzw5 = mask.s5 ? pLoadAdd5 : &dummy;
  private short4* xyzw6 = mask.s6 ? pLoadAdd6 : &dummy;
  private short4* xyzw7 = mask.s7 ? pLoadAdd7 : &dummy;

  __ocl_gather_transpose_short4x8(xyzw0, xyzw1, xyzw2, xyzw3,
                            xyzw4, xyzw5, xyzw6, xyzw7,
                            xOut, yOut, zOut, wOut);
}


void INLINE_ATTRIBUTE __ocl_masked_transpose_scatter_short4x8(private short4* pStoreAdd0, private short4* pStoreAdd1, private short4* pStoreAdd2, private short4* pStoreAdd3,
                                      private short4* pStoreAdd4, private short4* pStoreAdd5, private short4* pStoreAdd6, private short4* pStoreAdd7,
                                      short8 xIn, short8 yIn, short8 zIn, short8 wIn, int8 mask) {
  if (all(mask)) {
    __ocl_transpose_scatter_short4x8(pStoreAdd0, pStoreAdd1, pStoreAdd2, pStoreAdd3,
                              pStoreAdd4, pStoreAdd5, pStoreAdd6, pStoreAdd7,
                              xIn, yIn, zIn, wIn);
    return;
  }

  short4 dummy;

  private short4* xyzw0 = mask.s0 ? pStoreAdd0 : &dummy;
  private short4* xyzw1 = mask.s1 ? pStoreAdd1 : &dummy;
  private short4* xyzw2 = mask.s2 ? pStoreAdd2 : &dummy;
  private short4* xyzw3 = mask.s3 ? pStoreAdd3 : &dummy;
  private short4* xyzw4 = mask.s4 ? pStoreAdd4 : &dummy;
  private short4* xyzw5 = mask.s5 ? pStoreAdd5 : &dummy;
  private short4* xyzw6 = mask.s6 ? pStoreAdd6 : &dummy;
  private short4* xyzw7 = mask.s7 ? pStoreAdd7 : &dummy;

  __ocl_transpose_scatter_short4x8(xyzw0, xyzw1, xyzw2, xyzw3,
                            xyzw4, xyzw5, xyzw6, xyzw7,
                            xIn, yIn, zIn, wIn);
}

#endif // defined(__AVX__)


// ****************************************************************************
//                                 int4x4
// ****************************************************************************



void INTERNAL_INLINE_ATTRIBUTE __ocl_transpose_int4x4_AVX(int4 xyzw0, int4 xyzw1, int4 xyzw2, int4 xyzw3,
  private int4* xOut, private int4* yOut, private int4* zOut, private int4* wOut) {

                                                                            // xyzw0 = x0 y0 z0 w0
                                                                            // xyzw1 = x1 y1 z1 w1
                                                                            // xyzw2 = x2 y2 z2 w2
                                                                            // xyzw3 = x3 y3 z3 w3
  uint4 low4 = {0, 4, 1, 5};
  uint4 high4 = {2, 6, 3, 7};

  int4 xy02 = shuffle2(xyzw0, xyzw2, low4);                                 // x0 x2 y0 y2
  int4 zw02 = shuffle2(xyzw0, xyzw2, high4);                                // z0 z2 w0 w2
  int4 xy13 = shuffle2(xyzw1, xyzw3, low4);                                 // x1 x3 y1 y3
  int4 zw13 = shuffle2(xyzw1, xyzw3, high4);                                // z1 z3 w1 w3

  *xOut = shuffle2(xy02, xy13, low4);                                       // x0 x1 x2 x3
  *yOut = shuffle2(xy02, xy13, high4);                                      // y0 y1 y2 y3
  *zOut = shuffle2(zw02, zw13, low4);                                       // z0 z1 z2 z3
  *wOut = shuffle2(zw02, zw13, high4);                                      // w0 w1 w2 w3

}

void INTERNAL_INLINE_ATTRIBUTE __ocl_load_transpose_int4x4_AVX(private int4* pLoadAdd, private int4* xOut, private int4* yOut, private int4* zOut, private int4* wOut) {

  // pLoadAdd[0]  = x0 y0 z0 w0
  // pLoadAdd[1]  = x1 y1 z1 w1
  // pLoadAdd[2]  = x2 y2 z2 w2
  // pLoadAdd[3]  = x3 y3 z3 w3

  __ocl_transpose_int4x4_AVX(pLoadAdd[0], pLoadAdd[1], pLoadAdd[2], pLoadAdd[3],
            xOut, yOut, zOut, wOut);

  // xOut         = x0 x1 x2 x3
  // yOut         = y0 y1 y2 y3
  // zOut         = z0 z1 z2 z3
  // wOut         = w0 w1 w2 w3
}

void INTERNAL_INLINE_ATTRIBUTE __ocl_transpose_store_int4x4_AVX(private int4* pStoreAdd, int4 xIn, int4 yIn, int4 zIn, int4 wIn) {

  // xIn          = x0 x1 x2 x3
  // yIn          = y0 y1 y2 y3
  // zIn          = z0 z1 z2 z3
  // wIn          = w0 w1 w2 w3

  __ocl_transpose_int4x4_AVX(xIn, yIn, zIn, wIn,
            &(pStoreAdd[0]), &(pStoreAdd[1]), &(pStoreAdd[2]), &(pStoreAdd[3]));

  // pStoreAdd[0] = x0 y0 z0 w0
  // pStoreAdd[1] = x1 y1 z1 w1
  // pStoreAdd[2] = x2 y2 z2 w2
  // pStoreAdd[3] = x3 y3 z3 w3
}

#if defined(__AVX__)
void INLINE_ATTRIBUTE __ocl_masked_load_transpose_int4x4(private int4* pLoadAdd, private int4* xOut, private int4* yOut, private int4* zOut, private int4* wOut, int4 mask) {

  int4 xyzw[4];
  __ocl_masked_load_int4x4(pLoadAdd, xyzw, mask);
  __ocl_load_transpose_int4x4(xyzw, xOut, yOut, zOut, wOut);
}

void INLINE_ATTRIBUTE __ocl_masked_transpose_store_int4x4(private int4* pStoreAdd, int4 xIn, int4 yIn, int4 zIn, int4 wIn, int4 mask) {
  int4 xyzw[4];
  __ocl_transpose_store_int4x4(xyzw, xIn, yIn, zIn, wIn);
  __ocl_masked_store_int4x4(pStoreAdd, xyzw, mask);
}
#endif // defined(__AVX__)

void INTERNAL_INLINE_ATTRIBUTE __ocl_gather_transpose_int4x4_AVX(private int4* pLoadAdd0, private int4* pLoadAdd1, private int4* pLoadAdd2, private int4* pLoadAdd3,
                                 private int4* xOut, private int4* yOut, private int4* zOut, private int4* wOut) {

  // pLoadAdd0    = x0 y0 z0 w0
  // pLoadAdd1    = x1 y1 z1 w1
  // pLoadAdd2    = x2 y2 z2 w2
  // pLoadAdd3    = x3 y3 z3 w3

  __ocl_transpose_int4x4_AVX(*pLoadAdd0, *pLoadAdd1, *pLoadAdd2, *pLoadAdd3,
            xOut, yOut, zOut, wOut);

  // xOut         = x0 x1 x2 x3
  // yOut         = y0 y1 y2 y3
  // zOut         = z0 z1 z2 z3
  // wOut         = w0 w1 w2 w3
}

void INTERNAL_INLINE_ATTRIBUTE __ocl_transpose_scatter_int4x4_AVX(private int4* pStoreAdd0, private int4* pStoreAdd1, private int4* pStoreAdd2, private int4* pStoreAdd3,
                               int4 xIn, int4 yIn, int4 zIn, int4 wIn) {

  // xIn          = x0 x1 x2 x3
  // yIn          = y0 y1 y2 y3
  // zIn          = z0 z1 z2 z3
  // wIn          = w0 w1 w2 w3

  __ocl_transpose_int4x4_AVX(xIn, yIn, zIn, wIn,
            pStoreAdd0, pStoreAdd1, pStoreAdd2, pStoreAdd3);

  // pStoreAdd0   = x0 y0 z0 w0
  // pStoreAdd1   = x1 y1 z1 w1
  // pStoreAdd2   = x2 y2 z2 w2
  // pStoreAdd3   = x3 y3 z3 w3
}

#if defined(__AVX__)
void INLINE_ATTRIBUTE __ocl_masked_gather_transpose_int4x4(private int4* pLoadAdd0, private int4* pLoadAdd1, private int4* pLoadAdd2, private int4* pLoadAdd3,
                             private int4* xOut, private int4* yOut, private int4* zOut, private int4* wOut, int4 mask){
  if (all(mask)) {
   __ocl_gather_transpose_int4x4( pLoadAdd0, pLoadAdd1, pLoadAdd2, pLoadAdd3,
                            xOut, yOut, zOut, wOut);
    return;
  }

  int4 dummy;

  private int4* xyzw0 = mask.s0 ? pLoadAdd0 : &dummy;
  private int4* xyzw1 = mask.s1 ? pLoadAdd1 : &dummy;
  private int4* xyzw2 = mask.s2 ? pLoadAdd2 : &dummy;
  private int4* xyzw3 = mask.s3 ? pLoadAdd3 : &dummy;

	__ocl_gather_transpose_int4x4(xyzw0, xyzw1, xyzw2, xyzw3, xOut, yOut, zOut, wOut);
}

void INLINE_ATTRIBUTE __ocl_masked_transpose_scatter_int4x4(private int4* pStoreAdd0, private int4* pStoreAdd1, private int4* pStoreAdd2, private int4* pStoreAdd3,
                              int4 xIn, int4 yIn, int4 zIn, int4 wIn, int4 mask){

  if (all(mask)) {
    __ocl_transpose_scatter_int4x4(pStoreAdd0, pStoreAdd1, pStoreAdd2, pStoreAdd3,
                            xIn, yIn, zIn, wIn);
    return;
  }


  int4 dummy;

  private int4* xyzw0 = mask.s0 ? pStoreAdd0 : &dummy;
  private int4* xyzw1 = mask.s1 ? pStoreAdd1 : &dummy;
  private int4* xyzw2 = mask.s2 ? pStoreAdd2 : &dummy;
  private int4* xyzw3 = mask.s3 ? pStoreAdd3 : &dummy;

	__ocl_transpose_scatter_int4x4(xyzw0, xyzw1, xyzw2, xyzw3, xIn, yIn, zIn, wIn);
}

#endif // defined(__AVX__)


#if defined(__AVX2__)

/// @brief Receives int4x4 matrix as 2 halfs (2 int2x4 matrixes) using 2 int8,
///        transposes it and outputs the rows of the transposed matrix.
/// @param xyzw0In  - Part of the int2x4 matrix to be transposed
/// @param xyzw1In  - Part of the int2x4 matrix to be transposed
/// @param xzOut  - Part of the transposed int4x2 matrix.
///                 In case of isLoad is true xzOut will contain rows 0,2 of the transposed matrix,
///                 otherwise it will contain rows 0,1.
/// @param ywOut  - Part of the transposed int4x2 matrix
///                 In case of isLoad is true ywOut will contain rows 1,3 of the transposed matrix,
///                 otherwise it will contain rows 2,3.
/// @param isLoad   - indicates if the origen of the operation is load\store.
void INTERNAL_INLINE_ATTRIBUTE __ocl_transpose_int4x4_common_AVX2(int8 xyzw01, int8 xyzw23,
            private int8* xzOut, private int8* ywOut, bool isLoad) {

  if (isLoad) {
    // in this case xyzw01 =                                                   x0 y0 z0 w0 x1 y1 z1 w1
    // in this case xyzw23 =                                                   x2 y2 z2 w2 x3 y3 z3 w3
    xyzw01 = xyzw01.s04152637;                                              // x0 x1 y0 y1 z0 z1 w0 w1
    xyzw23 = xyzw23.s15043726;                                              // y2 y3 x2 x3 w2 w3 z2 z3
  } else { // isStore
    // in this case xyzw01 =                                                   x0 x1 x2 x3 y0 y1 y2 y3
    // in this case xyzw23 =                                                   z0 z1 z2 z3 w0 w1 w2 w3
    xyzw01 = xyzw01.s04261537;                                              // x0 y0 x2 y2 x1 y1 x3 y3
    xyzw23 = xyzw23.s26043715;                                              // z2 w2 z0 w0 z3 w3 z1 w1
  }

  // TODO : Replace this blend built-in with ?: when clang bug will be fixed
  //int8 cond = {0, 0, 0xFFFFFFFF, 0xFFFFFFFF, 0, 0, 0xFFFFFFFF, 0xFFFFFFFF};
  //int8 xz = cond ? xyzw01 : xyzw23;
  *xzOut = as_int8(_mm256_blend_epi32((__m256i)xyzw01,                      // x0 x1 x2 x3 z0 z1 z2 z3 for isLoad
                                          (__m256i)xyzw23, 0xCC));          // x0 y0 z0 w0 x1 y1 z1 w1 for isStore

  uint8 ywIndices = {2, 3, 8, 9, 6, 7, 12, 13};
  *ywOut = shuffle2(xyzw01, xyzw23,ywIndices );                             // y0 y1 y2 y3 w0 w1 w2 w3 for isLoad
                                                                            // x2 y2 z2 w2 x3 y3 z3 w3 for isStore
}

/// @brief Receives int4x4 matrix as 2 halfs (2 int2x4 matrixes) using 2 int8,
///        transposes it and outputs the rows of the transposed matrix.
/// @param xyzw0In  - Upper part of the int2x4 matrix to be transposed
/// @param xyzw1In  - Lower part of the int2x4 matrix to be transposed
/// @param xOut     - Row 0 of the transposed matrix
/// @param yOut     - Row 1 of the transposed matrix
/// @param zOut     - Row 2 of the transposed matrix
/// @param wOut     - Row 3 of the transposed matrix
void INTERNAL_INLINE_ATTRIBUTE __ocl_load_transpose_int4x4_common_AVX2(int8 xyzw01, int8 xyzw23, private int4* xOut, private int4* yOut, private int4* zOut, private int4* wOut) {

                                                                            // xyzw01 = x0 y0 z0 w0 x1 y1 z1 w1
                                                                            // xyzw23 = x2 y2 z2 w2 x3 y3 z3 w3
  int8 xz;
  int8 yw;

  __ocl_transpose_int4x4_common_AVX2(xyzw01, xyzw23,                              // x0 x1 x2 x3 z0 z1 z2 z3  =  xz
                &xz, &yw, true);                                            // y0 y1 y2 y3 w0 w1 w2 w3  =  yw

  *xOut = xz.s0123;                                                         // x0 x1 x2 x3
  *yOut = yw.s0123;                                                         // y0 y1 y2 y3
  *zOut = xz.s4567;                                                         // z0 z1 z2 z3
  *wOut = yw.s4567;                                                         // w0 w1 w2 w3
}

void INTERNAL_INLINE_ATTRIBUTE __ocl_load_transpose_int4x4_AVX2(private int4* pLoadAdd, private int4* xOut, private int4* yOut, private int4* zOut, private int4* wOut) {

  // We load "int8", meaning we load the full matrix in 2 loads
  private int* tmpLoadAdd = (private int*)pLoadAdd;
  int8 xyzw01 = vload8(0, tmpLoadAdd);                                      // x0 y0 z0 w0 x1 y1 z1 w1
  int8 xyzw23 = vload8(1, tmpLoadAdd);                                      // x2 y2 z2 w2 x3 y3 z3 w3

  __ocl_load_transpose_int4x4_common_AVX2(xyzw01, xyzw23, xOut, yOut, zOut, wOut);
}

void INTERNAL_INLINE_ATTRIBUTE __ocl_transpose_store_int4x4_AVX2(private int4* pStoreAdd, int4 xIn, int4 yIn, int4 zIn, int4 wIn) {

  int8 xy = {xIn, yIn};                                                     // x0 x1 x2 x3 y0 y1 y2 y3
  int8 zw = {zIn, wIn};                                                     // z0 z1 z2 z3 w0 w1 w2 w3

  int8 xyzw01;
  int8 xyzw23;

  __ocl_transpose_int4x4_common_AVX2(xy, zw,
                &xyzw01, &xyzw23, false);

  // We store "int8", meaning we store the full matrix in 2 stores
  int* tmpStoreAdd = (private int*)pStoreAdd;
  vstore8(xyzw01, 0, tmpStoreAdd);
  vstore8(xyzw23, 1, tmpStoreAdd);
}

void INTERNAL_INLINE_ATTRIBUTE __ocl_gather_transpose_int4x4_AVX2(private int4* pLoadAdd0, private int4* pLoadAdd1, private int4* pLoadAdd2, private int4* pLoadAdd3,
                                  private int4* xOut, private int4* yOut, private int4* zOut, private int4* wOut) {

  int8 xyzw01 = {*pLoadAdd0, *pLoadAdd1};                                   // x0 y0 z0 w0 x1 y1 z1 w1
  int8 xyzw23 = {*pLoadAdd2, *pLoadAdd3};                                   // x2 y2 z2 w2 x3 y3 z3 w3

  __ocl_load_transpose_int4x4_common_AVX2(xyzw01, xyzw23, xOut, yOut, zOut, wOut);
}

void INTERNAL_INLINE_ATTRIBUTE __ocl_transpose_scatter_int4x4_AVX2(private int4* pStoreAdd0, private int4* pStoreAdd1, private int4* pStoreAdd2, private int4* pStoreAdd3,
                               int4 xIn, int4 yIn, int4 zIn, int4 wIn) {

  int8 xy = {xIn, yIn};                                                     // x0 x1 x2 x3 y0 y1 y2 y3
  int8 zw = {zIn, wIn};                                                     // z0 z1 z2 z3 w0 w1 w2 w3

  int8 xyzw01;
  int8 xyzw23;

  __ocl_transpose_int4x4_common_AVX2(xy, zw,
                &xyzw01, &xyzw23, false);

  *pStoreAdd0 = xyzw01.s0123;
  *pStoreAdd1 = xyzw01.s4567;
  *pStoreAdd2 = xyzw01.s0123;
  *pStoreAdd3 = xyzw01.s4567;
}

#endif // defined(__AVX2__)

void INLINE_ATTRIBUTE __ocl_load_transpose_int4x4(private int4* pLoadAdd, private int4* xOut, private int4* yOut, private int4* zOut, private int4* wOut) {
#if defined(__AVX2__)
  __ocl_load_transpose_int4x4_AVX2(pLoadAdd, xOut, yOut, zOut, wOut);
#else // defined(__SSE4_2__)
  __ocl_load_transpose_int4x4_AVX(pLoadAdd, xOut, yOut, zOut, wOut);
#endif
}

void INLINE_ATTRIBUTE __ocl_transpose_store_int4x4(int4* pStoreAdd, int4 xIn, int4 yIn, int4 zIn, int4 wIn)  {
#if defined(__AVX2__)
  __ocl_transpose_store_int4x4_AVX2(pStoreAdd, xIn, yIn, zIn, wIn);
#else // defined(__SSE4_2__)
  __ocl_transpose_store_int4x4_AVX(pStoreAdd, xIn, yIn, zIn, wIn);
#endif
}

void INLINE_ATTRIBUTE __ocl_gather_transpose_int4x4(private int4* pLoadAdd0, private int4* pLoadAdd1, private int4* pLoadAdd2, private int4* pLoadAdd3,
                             private int4* xOut, private int4* yOut, private int4* zOut, private int4* wOut) {
#if defined(__AVX2__)
 __ocl_gather_transpose_int4x4_AVX2(pLoadAdd0, pLoadAdd1, pLoadAdd2, pLoadAdd3, xOut, yOut, zOut, wOut);
#else // defined(__SSE4_2__)
 __ocl_gather_transpose_int4x4_AVX(pLoadAdd0, pLoadAdd1, pLoadAdd2, pLoadAdd3, xOut, yOut, zOut, wOut);
#endif
}

void INLINE_ATTRIBUTE __ocl_transpose_scatter_int4x4(private int4* pStoreAdd0, private int4* pStoreAdd1, private int4* pStoreAdd2, private int4* pStoreAdd3,
                               int4 xIn, int4 yIn, int4 zIn, int4 wIn) {
#if defined(__AVX2__)
  __ocl_transpose_scatter_int4x4_AVX2(pStoreAdd0, pStoreAdd1, pStoreAdd2, pStoreAdd3, xIn, yIn, zIn, wIn);
#else // defined(__SSE4_2__)
  __ocl_transpose_scatter_int4x4_AVX(pStoreAdd0, pStoreAdd1, pStoreAdd2, pStoreAdd3, xIn, yIn, zIn, wIn);
#endif
}


// ****************************************************************************
//                                 int4x8
// ****************************************************************************

#if defined(__AVX__)

void INTERNAL_INLINE_ATTRIBUTE __ocl_load_transpose_int4x8_AVX(private int4* pLoadAdd, private int8* xOut, private int8* yOut, private int8* zOut, private int8* wOut) {

  int4 x0;
  int4 y0;
  int4 z0;
  int4 w0;

  __ocl_load_transpose_int4x4_AVX(pLoadAdd, &x0, &y0, &z0, &w0);                  // x0 x1 x2 x3
                                                                            // y0 y1 y2 y3
                                                                            // z0 z1 z2 z3
                                                                            // w0 w1 w2 w3

  int4 x1;
  int4 y1;
  int4 z1;
  int4 w1;

  private int4* pLoadAdd1 = &(pLoadAdd[4]);

  __ocl_load_transpose_int4x4_AVX(pLoadAdd1, &x1, &y1, &z1, &w1);                 // x4 x5 x6 x7
                                                                            // y4 y5 y6 y7
                                                                            // z4 z5 z6 z7
                                                                            // w4 w5 w6 w7

  // TODO : Replace these shuffle builtins with *xOut = {x0, x1} when clang bug will be fixed
  uint8 concatVectors4 = {0, 1, 2, 3, 4, 5, 6, 7};
  *xOut = shuffle2(x0, x1, concatVectors4);                                 // x0 x1 x2 x3 x4 x5 x6 x7
  *yOut = shuffle2(y0, y1, concatVectors4);                                 // y0 y1 y2 y3 y4 y5 y6 y7
  *zOut = shuffle2(z0, z1, concatVectors4);                                 // z0 z1 z2 z3 z4 z5 z6 z7
  *wOut = shuffle2(w0, w1, concatVectors4);                                 // w0 w1 w2 w3 w4 w5 w6 w7
}

void INLINE_ATTRIBUTE __ocl_masked_load_transpose_int4x8(private int4* pLoadAdd, private int8* xOut, private int8* yOut, private int8* zOut, private int8* wOut, int8 mask) {
  int4 xyzw[8];
  __ocl_masked_load_int4x8(pLoadAdd, xyzw, mask);
  __ocl_load_transpose_int4x8(xyzw, xOut, yOut, zOut, wOut);
}
void INTERNAL_INLINE_ATTRIBUTE __ocl_transpose_store_int4x8_AVX(private int4* pStoreAdd, int8 xIn, int8 yIn, int8 zIn, int8 wIn) {

  int4 x0 = xIn.s0123;
  int4 y0 = yIn.s0123;
  int4 z0 = zIn.s0123;
  int4 w0 = wIn.s0123;

  __ocl_transpose_store_int4x4_AVX(pStoreAdd, x0, y0, z0, w0);                    // x0 y0 z0 w0
                                                                            // x1 y1 z1 w1
                                                                            // x2 y2 z2 w2
                                                                            // x3 y3 z3 w3

  int4 x1 = xIn.s4567;
  int4 y1 = yIn.s4567;
  int4 z1 = zIn.s4567;
  int4 w1 = wIn.s4567;

  private int4* pStoreAdd1 = &(pStoreAdd[4]);

  __ocl_transpose_store_int4x4_AVX(pStoreAdd1, x1, y1, z1, w1);                   // x4 y4 z4 w4
                                                                            // x5 y5 z5 w5
                                                                            // x6 y6 z6 w6
                                                                            // x7 y7 z7 w7
}

void INLINE_ATTRIBUTE __ocl_masked_transpose_store_int4x8(private int4* pStoreAdd, int8 xIn, int8 yIn, int8 zIn, int8 wIn, int8 mask) {

  int4 xyzw[8];
  __ocl_transpose_store_int4x8(xyzw, xIn, yIn, zIn, wIn);
  __ocl_masked_store_int4x8(pStoreAdd, xyzw, mask);
}

void INTERNAL_INLINE_ATTRIBUTE __ocl_gather_transpose_int4x8_AVX(private int4* pLoadAdd0, private int4* pLoadAdd1, private int4* pLoadAdd2, private int4* pLoadAdd3,
                                 private int4* pLoadAdd4, private int4* pLoadAdd5, private int4* pLoadAdd6, private int4* pLoadAdd7,
                                 private int8* xOut, private int8* yOut, private int8* zOut, private int8* wOut) {

  int4 x0;
  int4 y0;
  int4 z0;
  int4 w0;

 __ocl_gather_transpose_int4x4_AVX(pLoadAdd0, pLoadAdd1, pLoadAdd2, pLoadAdd3,   // x0 x1 x2 x3
                              &x0, &y0, &z0, &w0);                          // y0 y1 y2 y3
                                                                            // z0 z1 z2 z3
                                                                            // w0 w1 w2 w3

  int4 x1;
  int4 y1;
  int4 z1;
  int4 w1;

 __ocl_gather_transpose_int4x4_AVX(pLoadAdd4, pLoadAdd5, pLoadAdd6, pLoadAdd7,   // x4 x5 x6 x7
                              &x1, &y1, &z1, &w1);                          // y4 y5 y6 y7
                                                                            // z4 z5 z6 z7
                                                                            // w4 w5 w6 w7

  // TODO : Replace these shuffle builtins with *xOut = {x0, x1} when clang bug will be fixed
  uint8 concatVectors4 = {0, 1, 2, 3, 4, 5, 6, 7};
  *xOut = shuffle2(x0, x1, concatVectors4);                                 // x0 x1 x2 x3 x4 x5 x6 x7
  *yOut = shuffle2(y0, y1, concatVectors4);                                 // y0 y1 y2 y3 y4 y5 y6 y7
  *zOut = shuffle2(z0, z1, concatVectors4);                                 // z0 z1 z2 z3 z4 z5 z6 z7
  *wOut = shuffle2(w0, w1, concatVectors4);                                 // w0 w1 w2 w3 w4 w5 w6 w7
}

void INTERNAL_INLINE_ATTRIBUTE __ocl_transpose_scatter_int4x8_AVX(private int4* pStoreAdd0, private int4* pStoreAdd1, private int4* pStoreAdd2, private int4* pStoreAdd3,
                                  private int4* pStoreAdd4, private int4* pStoreAdd5, private int4* pStoreAdd6, private int4* pStoreAdd7,
                                  int8 xIn, int8 yIn, int8 zIn, int8 wIn) {
  int4 x0 = xIn.s0123;
  int4 y0 = yIn.s0123;
  int4 z0 = zIn.s0123;
  int4 w0 = wIn.s0123;

  __ocl_transpose_scatter_int4x4_AVX(pStoreAdd0, pStoreAdd1, pStoreAdd2, pStoreAdd3,
                               x0, y0, z0, w0);                             // x0 y0 z0 w0
                                                                            // x1 y1 z1 w1
                                                                            // x2 y2 z2 w2
                                                                            // x3 y3 z3 w3

  int4 x1 = xIn.s4567;
  int4 y1 = yIn.s4567;
  int4 z1 = zIn.s4567;
  int4 w1 = wIn.s4567;

  __ocl_transpose_scatter_int4x4_AVX(pStoreAdd4, pStoreAdd5, pStoreAdd6, pStoreAdd7,
                               x1, y1, z1, w1);                             // x4 y4 z4 w4
                                                                            // x5 y5 z5 w5
                                                                            // x6 y6 z6 w6
                                                                            // x7 y7 z7 w7
}

void INLINE_ATTRIBUTE __ocl_masked_gather_transpose_int4x8(private int4* pLoadAdd0, private int4* pLoadAdd1, private int4* pLoadAdd2, private int4* pLoadAdd3,
                             private int4* pLoadAdd4, private int4* pLoadAdd5, private int4* pLoadAdd6, private int4* pLoadAdd7,
                             private int8* xOut, private int8* yOut, private int8* zOut, private int8* wOut, int8 mask) {
  if (all(mask)) {
   __ocl_gather_transpose_int4x8( pLoadAdd0, pLoadAdd1, pLoadAdd2, pLoadAdd3,
                            pLoadAdd4, pLoadAdd5, pLoadAdd6, pLoadAdd7,
                            xOut, yOut, zOut, wOut);
    return;
  }

  int4 dummy;

  private int4* xyzw0 = mask.s0 ? pLoadAdd0 : &dummy;
  private int4* xyzw1 = mask.s1 ? pLoadAdd1 : &dummy;
  private int4* xyzw2 = mask.s2 ? pLoadAdd2 : &dummy;
  private int4* xyzw3 = mask.s3 ? pLoadAdd3 : &dummy;
  private int4* xyzw4 = mask.s4 ? pLoadAdd4 : &dummy;
  private int4* xyzw5 = mask.s5 ? pLoadAdd5 : &dummy;
  private int4* xyzw6 = mask.s6 ? pLoadAdd6 : &dummy;
  private int4* xyzw7 = mask.s7 ? pLoadAdd7 : &dummy;

	__ocl_gather_transpose_int4x8( xyzw0, xyzw1, xyzw2, xyzw3,
                            xyzw4, xyzw5, xyzw6, xyzw7,
                            xOut, yOut, zOut, wOut);
}

void INLINE_ATTRIBUTE __ocl_masked_transpose_scatter_int4x8(private int4* pStoreAdd0, private int4* pStoreAdd1, private int4* pStoreAdd2, private int4* pStoreAdd3,
                              private int4* pStoreAdd4, private int4* pStoreAdd5, private int4* pStoreAdd6, private int4* pStoreAdd7,
                              int8 xIn, int8 yIn, int8 zIn, int8 wIn, int8 mask){
  if (all(mask)) {
    __ocl_transpose_scatter_int4x8(pStoreAdd0, pStoreAdd1, pStoreAdd2, pStoreAdd3,
                            pStoreAdd4, pStoreAdd5, pStoreAdd6, pStoreAdd7,
                            xIn, yIn, zIn, wIn);
    return;
  }

  int4 dummy;

  private int4* xyzw0 = mask.s0 ? pStoreAdd0 : &dummy;
  private int4* xyzw1 = mask.s1 ? pStoreAdd1 : &dummy;
  private int4* xyzw2 = mask.s2 ? pStoreAdd2 : &dummy;
  private int4* xyzw3 = mask.s3 ? pStoreAdd3 : &dummy;
  private int4* xyzw4 = mask.s4 ? pStoreAdd4 : &dummy;
  private int4* xyzw5 = mask.s5 ? pStoreAdd5 : &dummy;
  private int4* xyzw6 = mask.s6 ? pStoreAdd6 : &dummy;
  private int4* xyzw7 = mask.s7 ? pStoreAdd7 : &dummy;

	__ocl_transpose_scatter_int4x8(xyzw0, xyzw1, xyzw2, xyzw3,
                            xyzw4, xyzw5, xyzw6, xyzw7,
                            xIn, yIn, zIn, wIn);
}

#endif // defined(__AVX__)

#if defined(__AVX2__)

/// @brief Receives int8x4 matrix as 4 double rows (4 int2x4 matrixes) using 4 int8,
///        transposes it and outputs the rows of the transposed matrix.
/// @param xyzw04  - Rows 0,4 of the matrix to be transposed
/// @param xyzw15  - Rows 1,5 of the matrix to be transposed
/// @param xyzw26  - Rows 2,6 og the matrix to be transposed
/// @param xyzw37  - Rows 3,7 matrix to be transposed
/// @param xOut    - Row 0 of the transposed matrix
/// @param yOut    - Row 1 of the transposed matrix
/// @param zOut    - Row 2 of the transposed matrix
/// @param wOut    - Row 3 of the transposed matrix
void INTERNAL_INLINE_ATTRIBUTE __ocl_transpose_int4x8_common_AVX2(int8 xyzw04, int8 xyzw15, int8 xyzw26, int8 xyzw37,
             private int8* xOut, private int8* yOut, private int8* zOut, private int8* wOut) {

  uint8 lowLane8 = {0, 8, 1, 9, 4, 12, 5, 13};
  uint8 highLane8 = {2, 10, 3, 11, 6, 14, 7, 15};

  int8 xy0246 = shuffle2(xyzw04, xyzw26, lowLane8);                         // x0 x2 y0 y2 x4 x6 y4 y6
  int8 zw0246 = shuffle2(xyzw04, xyzw26, highLane8);                        // z0 z2 w0 w2 z4 z6 w4 w6
  int8 xy1357 = shuffle2(xyzw15, xyzw37, lowLane8);                         // x1 x3 y1 y3 x5 x7 y5 y7
  int8 zw1357 = shuffle2(xyzw15, xyzw37, highLane8);                        // z1 z3 w1 w3 z5 z7 w5 w7

  *xOut = shuffle2(xy0246, xy1357, lowLane8);                               // x0 x1 x2 x3 x4 x5 x6 x7
  *yOut = shuffle2(xy0246, xy1357, highLane8);                              // y0 y1 y2 y3 y4 y5 y6 y7
  *zOut = shuffle2(zw0246, zw1357, lowLane8);                               // z0 z1 z2 z3 z4 z5 z6 z7
  *wOut = shuffle2(zw0246, zw1357, highLane8);                              // w0 w1 w2 w3 w4 w5 w6 w7
}



void INTERNAL_INLINE_ATTRIBUTE __ocl_gather_transpose_int4x8_AVX2(private int4* pLoadAdd0, private int4* pLoadAdd1, private int4* pLoadAdd2, private int4* pLoadAdd3,
                                  private int4* pLoadAdd4, private int4* pLoadAdd5, private int4* pLoadAdd6, private int4* pLoadAdd7,
                                  private int8* xOut, private int8* yOut, private int8* zOut, private int8* wOut) {
  // TODO: creates
  // vmovaps  48(%eax), %xmm0
  // vmovaps  112(%eax), %xmm1
  // vinsertf128  $1, %xmm1, %ymm0, %ymm0
  // instead of vpinsert from memory and not loading into xmm1

  // TODO : Replace this shuffle with { pLoadAdd0,  pLoadAdd4} instead of shuffle builtin
  // when clang bug will be fixed
  uint8 concatVectors4 = {0, 1, 2, 3, 4, 5, 6, 7};
  int8 xyzw04 = shuffle2(*pLoadAdd0, *pLoadAdd4, concatVectors4);           // x0 y0 z0 w0 x4 y4 z4 w4
  int8 xyzw15 = shuffle2(*pLoadAdd1, *pLoadAdd5, concatVectors4);           // x1 y1 z1 w1 x5 y5 z5 w5
  int8 xyzw26 = shuffle2(*pLoadAdd2, *pLoadAdd6, concatVectors4);           // x2 y2 z2 w2 x6 y6 z6 w6
  int8 xyzw37 = shuffle2(*pLoadAdd3, *pLoadAdd7, concatVectors4);           // x3 y3 z3 w3 x7 y7 z7 w7

  __ocl_transpose_int4x8_common_AVX2(xyzw04, xyzw15, xyzw26, xyzw37,
            xOut, yOut, zOut, wOut);
                                                                            // xOut = x0 x1 x2 x3 x4 x5 x6 x7
                                                                            // yOut = y0 y1 y2 y3 y4 y5 y6 y7
                                                                            // zOut = z0 z1 z2 z3 z4 z5 z6 z7
                                                                            // wOut = w0 w1 w2 w3 w4 w5 w6 w7
}

void INTERNAL_INLINE_ATTRIBUTE __ocl_transpose_scatter_int4x8_AVX2(private int4* pStoreAdd0, private int4* pStoreAdd1, private int4* pStoreAdd2, private int4* pStoreAdd3,
                                   private int4* pStoreAdd4, private int4* pStoreAdd5, private int4* pStoreAdd6, private int4* pStoreAdd7,
                                   int8 xIn, int8 yIn, int8 zIn, int8 wIn) {
  int8 xyzw04;
  int8 xyzw15;
  int8 xyzw26;
  int8 xyzw37;

  __ocl_transpose_int4x8_common_AVX2(xIn, yIn, zIn, wIn,
            &xyzw04, &xyzw15, &xyzw26, &xyzw37);

                                                                            // xyzw04 = x0 y0 z0 w0 x4 y4 z4 w4
                                                                            // xyzw15 = x1 y1 z1 w1 x5 y5 z5 w5
                                                                            // xyzw26 = x2 y2 z2 w2 x6 y6 z6 w6
                                                                            // xyzw37 = x3 y3 z3 w3 x7 y7 z7 w7

  // TODO: creates
  // vextractf128  $1, %ymm4, %xmm2
  // vmovapd  %xmm2, 64(%eax)
  // instead of extracting directly to memory

  *pStoreAdd0 = xyzw04.lo;                                                  // x0 y0 z0 w0
  *pStoreAdd1 = xyzw15.lo;                                                  // x1 y1 z1 w1
  *pStoreAdd2 = xyzw26.lo;                                                  // x2 y2 z2 w2
  *pStoreAdd3 = xyzw37.lo;                                                  // x3 y3 z3 w3
  *pStoreAdd4 = xyzw04.hi;                                                  // x4 y4 z4 w4
  *pStoreAdd5 = xyzw15.hi;                                                  // x5 y5 z5 w5
  *pStoreAdd6 = xyzw26.hi;                                                  // x6 y6 z6 w6
  *pStoreAdd7 = xyzw37.hi;                                                  // x7 y7 z7 w7
}

void INTERNAL_INLINE_ATTRIBUTE __ocl_load_transpose_int4x8_AVX2(private int4* pLoadAdd, private int8* xOut, private int8* yOut, private int8* zOut, private int8* wOut) {
 __ocl_gather_transpose_int4x8_AVX2(&(pLoadAdd[0]), &(pLoadAdd[1]), &(pLoadAdd[2]), &(pLoadAdd[3]),
                               &(pLoadAdd[4]), &(pLoadAdd[5]), &(pLoadAdd[6]), &(pLoadAdd[7]),
                               xOut, yOut, zOut, wOut);
}

void INTERNAL_INLINE_ATTRIBUTE __ocl_transpose_store_int4x8_AVX2(private int4* pStoreAdd, int8 xIn, int8 yIn, int8 zIn, int8 wIn) {

  __ocl_transpose_scatter_int4x8_AVX2(&(pStoreAdd[0]), &(pStoreAdd[1]), &(pStoreAdd[2]), &(pStoreAdd[3]),
                                &(pStoreAdd[4]), &(pStoreAdd[5]), &(pStoreAdd[6]), &(pStoreAdd[7]),
                                xIn, yIn, zIn, wIn);
}

#endif // defined(__AVX2__)


#if defined(__AVX__)

void INLINE_ATTRIBUTE __ocl_load_transpose_int4x8(private int4* pLoadAdd, private int8* xOut, private int8* yOut, private int8* zOut, private int8* wOut) {
#if defined(__AVX2__)
  __ocl_load_transpose_int4x8_AVX2(pLoadAdd, xOut, yOut, zOut, wOut);
#else // defined(__AVX__)
  __ocl_load_transpose_int4x8_AVX(pLoadAdd, xOut, yOut, zOut, wOut);
#endif
}

void INLINE_ATTRIBUTE __ocl_transpose_store_int4x8(private int4* pStoreAdd, int8 xIn, int8 yIn, int8 zIn, int8 wIn) {
#if defined(__AVX2__)
  __ocl_transpose_store_int4x8_AVX2(pStoreAdd, xIn, yIn, zIn, wIn);
#else // defined(__AVX__)
  __ocl_transpose_store_int4x8_AVX(pStoreAdd, xIn, yIn, zIn, wIn);
#endif
}

void INLINE_ATTRIBUTE __ocl_gather_transpose_int4x8(private int4* pLoadAdd0, private int4* pLoadAdd1, private int4* pLoadAdd2, private int4* pLoadAdd3,
                             private int4* pLoadAdd4, private int4* pLoadAdd5, private int4* pLoadAdd6, private int4* pLoadAdd7,
                             private int8* xOut, private int8* yOut, private int8* zOut, private int8* wOut) {
#if defined(__AVX2__)
 __ocl_gather_transpose_int4x8_AVX2(pLoadAdd0, pLoadAdd1, pLoadAdd2, pLoadAdd3,
                               pLoadAdd4, pLoadAdd5, pLoadAdd6, pLoadAdd7,
                               xOut, yOut, zOut, wOut);
#else // defined(__AVX__)
 __ocl_gather_transpose_int4x8_AVX(pLoadAdd0, pLoadAdd1, pLoadAdd2, pLoadAdd3,
                               pLoadAdd4, pLoadAdd5, pLoadAdd6, pLoadAdd7,
                               xOut, yOut, zOut, wOut);
#endif
}

void INLINE_ATTRIBUTE __ocl_transpose_scatter_int4x8(private int4* pStoreAdd0, private int4* pStoreAdd1, private int4* pStoreAdd2, private int4* pStoreAdd3,
                              private int4* pStoreAdd4, private int4* pStoreAdd5, private int4* pStoreAdd6, private int4* pStoreAdd7,
                              int8 xIn, int8 yIn, int8 zIn, int8 wIn) {
#if defined(__AVX2__)
  __ocl_transpose_scatter_int4x8_AVX2(pStoreAdd0, pStoreAdd1, pStoreAdd2, pStoreAdd3,
                                pStoreAdd4, pStoreAdd5, pStoreAdd6, pStoreAdd7,
                                xIn, yIn, zIn, wIn);
#else // defined(__AVX__)
  __ocl_transpose_scatter_int4x8_AVX(pStoreAdd0, pStoreAdd1, pStoreAdd2, pStoreAdd3,
                               pStoreAdd4, pStoreAdd5, pStoreAdd6, pStoreAdd7,
                               xIn, yIn, zIn, wIn);
#endif
}

#endif // defined(__AVX__)


// ****************************************************************************
//                                 float4x4
// ****************************************************************************



void INTERNAL_INLINE_ATTRIBUTE __ocl_transpose_float4x4_AVX(float4 xyzw0, float4 xyzw1, float4 xyzw2, float4 xyzw3,
  private float4* xOut, private float4* yOut, private float4* zOut, private float4* wOut) {
                                                                            // xyzw0 = x0 y0 z0 w0
                                                                            // xyzw1 = x1 y1 z1 w1
                                                                            // xyzw2 = x2 y2 z2 w2
                                                                            // xyzw3 = x3 y3 z3 w3
  uint4 low4 = {0, 4, 1, 5};
  uint4 high4 = {2, 6, 3, 7};

  float4 xy02 = shuffle2(xyzw0, xyzw2, low4);                               // x0 x2 y0 y2
  float4 zw02 = shuffle2(xyzw0, xyzw2, high4);                              // z0 z2 w0 w2
  float4 xy13 = shuffle2(xyzw1, xyzw3, low4);                               // x1 x3 y1 y3
  float4 zw13 = shuffle2(xyzw1,  xyzw3, high4);                             // z1 z3 w1 w3

  *xOut = shuffle2(xy02, xy13, low4);                                       // x0 x1 x2 x3
  *yOut = shuffle2(xy02, xy13, high4);                                      // y0 y1 y2 y3
  *zOut = shuffle2(zw02, zw13, low4);                                       // z0 z1 z2 z3
  *wOut = shuffle2(zw02, zw13, high4);                                      // w0 w1 w2 w3

}

void INTERNAL_INLINE_ATTRIBUTE __ocl_load_transpose_float4x4_AVX(private float4* pLoadAdd, private float4* xOut, private float4* yOut, private float4* zOut, private float4* wOut) {
  // pLoadAdd[0]  = x0 y0 z0 w0
  // pLoadAdd[1]  = x1 y1 z1 w1
  // pLoadAdd[2]  = x2 y2 z2 w2
  // pLoadAdd[3]  = x3 y3 z3 w3

  __ocl_transpose_float4x4_AVX(pLoadAdd[0], pLoadAdd[1], pLoadAdd[2], pLoadAdd[3],
            xOut, yOut, zOut, wOut);

  // xOut         = x0 x1 x2 x3
  // yOut         = y0 y1 y2 y3
  // zOut         = z0 z1 z2 z3
  // wOut         = w0 w1 w2 w3
}

void INTERNAL_INLINE_ATTRIBUTE __ocl_transpose_store_float4x4_AVX(private float4* pStoreAdd, float4 xIn, float4 yIn, float4 zIn, float4 wIn) {
  // xIn          = x0 x1 x2 x3
  // yIn          = y0 y1 y2 y3
  // zIn          = z0 z1 z2 z3
  // wIn          = w0 w1 w2 w3

  __ocl_transpose_float4x4_AVX(xIn, yIn, zIn, wIn,
            &(pStoreAdd[0]), &(pStoreAdd[1]), &(pStoreAdd[2]), &(pStoreAdd[3]));

  // pStoreAdd[0] = x0 y0 z0 w0
  // pStoreAdd[1] = x1 y1 z1 w1
  // pStoreAdd[2] = x2 y2 z2 w2
  // pStoreAdd[3] = x3 y3 z3 w3
}

#if defined(__AVX__)
void INLINE_ATTRIBUTE __ocl_masked_load_transpose_float4x4(private float4* pLoadAdd, private float4* xOut, private float4* yOut, private float4* zOut, private float4* wOut, int4 mask) {
  float4 xyzw[4] = {0};
  __ocl_masked_load_float4x4(pLoadAdd, xyzw, mask);
  __ocl_load_transpose_float4x4(xyzw, xOut, yOut, zOut, wOut);
}

void INLINE_ATTRIBUTE __ocl_masked_transpose_store_float4x4(private float4* pStoreAdd, float4 xIn, float4 yIn, float4 zIn, float4 wIn, int4 mask) {
  float4 xyzw[4];
  __ocl_transpose_store_float4x4(xyzw, xIn, yIn, zIn, wIn);
  __ocl_masked_store_float4x4(pStoreAdd, xyzw, mask);
}
#endif // defined(__AVX__)

void INTERNAL_INLINE_ATTRIBUTE __ocl_gather_transpose_float4x4_AVX(private float4* pLoadAdd0, private float4* pLoadAdd1, private float4* pLoadAdd2, private float4* pLoadAdd3,
                              private float4* xOut, private float4* yOut, private float4* zOut, private float4* wOut) {
  __ocl_transpose_float4x4_AVX(*pLoadAdd0, *pLoadAdd1, *pLoadAdd2, *pLoadAdd3,
                        xOut, yOut, zOut, wOut);
}

void INTERNAL_INLINE_ATTRIBUTE __ocl_transpose_scatter_float4x4_AVX(private float4* pStoreAdd0, private float4* pStoreAdd1, private float4* pStoreAdd2, private float4* pStoreAdd3,
                               float4 xIn, float4 yIn, float4 zIn, float4 wIn) {
  __ocl_transpose_float4x4_AVX(xIn, yIn, zIn, wIn,
                        pStoreAdd0, pStoreAdd1, pStoreAdd2, pStoreAdd3);
}

#if defined(__AVX__)
void INLINE_ATTRIBUTE __ocl_masked_gather_transpose_float4x4(private float4* pLoadAdd0, private float4* pLoadAdd1, private float4* pLoadAdd2, private float4* pLoadAdd3,
                               private float4* xOut, private float4* yOut, private float4* zOut, private float4* wOut, int4 mask){
  if (all(mask)) {
   __ocl_gather_transpose_float4x4(pLoadAdd0, pLoadAdd1, pLoadAdd2, pLoadAdd3, xOut, yOut, zOut, wOut);
    return;
  }

  float4 dummy = 0;

  private float4* xyzw0 = mask.s0 ? pLoadAdd0 : &dummy;
  private float4* xyzw1 = mask.s1 ? pLoadAdd1 : &dummy;
  private float4* xyzw2 = mask.s2 ? pLoadAdd2 : &dummy;
  private float4* xyzw3 = mask.s3 ? pLoadAdd3 : &dummy;

	__ocl_gather_transpose_float4x4( xyzw0, xyzw1, xyzw2, xyzw3,
                            xOut, yOut, zOut, wOut);
}

void INLINE_ATTRIBUTE __ocl_masked_transpose_scatter_float4x4(private float4* pStoreAdd0, private float4* pStoreAdd1, private float4* pStoreAdd2, private float4* pStoreAdd3,
                                float4 xIn, float4 yIn, float4 zIn, float4 wIn, int4 mask) {
  if (all(mask)) {
    __ocl_transpose_scatter_float4x4(pStoreAdd0, pStoreAdd1, pStoreAdd2, pStoreAdd3,
                            xIn, yIn, zIn, wIn);
    return;
  }

  float4 dummy;

  private float4* xyzw0 = mask.s0 ? pStoreAdd0 : &dummy;
  private float4* xyzw1 = mask.s1 ? pStoreAdd1 : &dummy;
  private float4* xyzw2 = mask.s2 ? pStoreAdd2 : &dummy;
  private float4* xyzw3 = mask.s3 ? pStoreAdd3 : &dummy;

	__ocl_transpose_scatter_float4x4(xyzw0, xyzw1, xyzw2, xyzw3, xIn, yIn, zIn, wIn);
}
#endif // defined(__AVX__)



#if defined(__AVX2__)

/// @brief Receives float4x4 matrix as 2 halfs (2 float2x4 matrixes) using 2 float8,
///        transposes it and outputs the rows of the transposed matrix.
/// @param xyzw0In  - Part of the float2x4 matrix to be transposed
/// @param xyzw1In  - Part of the float2x4 matrix to be transposed
/// @param xzOut  - Part of the transposed float4x2 matrix.
///                 In case of isLoad is true xzOut will contain rows 0,2 of the transposed matrix,
///                 otherwise it will contain rows 0,1.
/// @param ywOut  - Part of the transposed int4x2 matrix
///                 In case of isLoad is true ywOut will contain rows 1,3 of the transposed matrix,
///                 otherwise it will contain rows 2,3.
/// @param isLoad   - indicates if the origen of the operation is load\store.
void INTERNAL_INLINE_ATTRIBUTE __ocl_transpose_float4x4_common_AVX2(float8 xyzw01, float8 xyzw23,
            private float8* xz, private float8* yw, bool isLoad) {

  if (isLoad) {
    // in this case xyzw01 =                                                       x0 y0 z0 w0 x1 y1 z1 w1
    // in this case xyzw01 =                                                       x2 y2 w2 z2 x3 y3 z3 w3
    xyzw01 = xyzw01.s04152637;                                                  // x0 x1 y0 y1 z0 z1 w0 w1
    xyzw23 = xyzw23.s15043726;                                                  // y2 y3 x2 x3 w2 w3 z2 z3
  } else { // isStore
    // in this case xyzw01 =                                                       x0 x1 x2 x3 y0 y1 y2 y3
    // in this case xyzw01 =                                                       z0 z1 z2 z3 w0 w1 w2 w3
    xyzw01 = xyzw01.s04261537;                                                  // x0 y0 x2 y2 x1 y1 x3 y3
    xyzw23 = xyzw23.s26043715;                                                  // z2 w2 z0 w0 z3 w3 z1 w1
  }

  // TODO : Replace this blend built-in with ?: when clang bug will be fixed
  //int8 cond = {0, 0, 0xFFFFFFFF, 0xFFFFFFFF, 0, 0, 0xFFFFFFFF, 0xFFFFFFFF};
  //int8 xz = cond ? xyzw01 : xyzw23;
  *xz = as_float8(_mm256_blend_ps((__m256)xyzw01, (__m256)xyzw23, 0xCC));       // x0 x1 x2 x3 z0 z1 z2 z3 for isLoad
                                                                                // x0 y0 z0 w0 x1 y1 z1 w1 for isStore

  uint8 ywShuffle = {2, 3, 8, 9, 6, 7, 12, 13};
  *yw = shuffle2(xyzw01, xyzw23, ywShuffle);                                    // y0 y1 y2 y3 w0 w1 w2 w3 for isLoad
                                                                                // x2 y2 z2 w2 x3 y3 z3 w3 for isStore
}

/// @brief Receives float4x4 matrix as 2 halfs (2 float2x4 matrixes) using 2 float8,
///        transposes it and outputs the rows of the transposed matrix.
/// @param xyzw0In  - Upper part of the int2x4 matrix to be transposed
/// @param xyzw1In  - Lower part of the int2x4 matrix to be transposed
/// @param xOut     - Row 0 of the transposed matrix
/// @param yOut     - Row 1 of the transposed matrix
/// @param zOut     - Row 2 of the transposed matrix
/// @param wOut     - Row 3 of the transposed matrix
void INTERNAL_INLINE_ATTRIBUTE __ocl_load_transpose_float4x4_common_AVX2(float8 xyzw01, float8 xyzw23, private float4* xOut, private float4* yOut, private float4* zOut, private float4* wOut) {

  float8 xz;
  float8 yw;

  __ocl_transpose_float4x4_common_AVX2(xyzw01, xyzw23,
            &xz, &yw, true);

  *xOut = xz.s0123;
  *yOut = yw.s0123;
  *zOut = xz.s4567;
  *wOut = yw.s4567;
}

void INTERNAL_INLINE_ATTRIBUTE __ocl_load_transpose_float4x4_AVX2(private float4* pLoadAdd, private float4* xOut, private float4* yOut, private float4* zOut, private float4* wOut) {

  // We load "float8", meaning we load the full matrix in 2 loads
  private float* tmpLoadAdd = (float*)pLoadAdd;
  float8 xyzw01 = vload8(0, tmpLoadAdd);                                    // x0 y0 z0 w0 x1 y1 z1 w1
  float8 xyzw23 = vload8(1, tmpLoadAdd);                                    // x2 y2 z2 w2 x3 y3 z3 w3

  __ocl_load_transpose_float4x4_common_AVX2(xyzw01, xyzw23,
            xOut, yOut, zOut, wOut);
}


void INTERNAL_INLINE_ATTRIBUTE __ocl_transpose_store_float4x4_AVX2(private float4* pStoreAdd, float4 xIn, float4 yIn, float4 zIn, float4 wIn) {

  //float8 xy = {xIn, yIn};                                                   // x0 x1 x2 x3 y0 y1 y2 y3
  //float8 zw = {zIn, wIn};                                                   // z0 z1 z2 z3 w0 w1 w2 w3
  // TODO : Replace this shuffle with  {*pLoadAdd0, *pLoadAdd1} instead of shuffle builtin
  // when clang bug will be fixed
  float8 xy;
  float8 zw;

  xy.lo = xIn;                                                              // x0 x1 x2 x3 D  D  D  D
  xy.hi = yIn;                                                              // x0 x1 x2 x3 y0 y1 y2 y3
  zw.lo = zIn;                                                              // z0 z1 z2 z3 D  D  D  D
  zw.hi = wIn;                                                              // z0 z1 z2 z3 w0 w1 w2 w3

  float8 xyzw01;
  float8 xyzw23;

  __ocl_transpose_float4x4_common_AVX2(xy, zw,
            &xyzw01, &xyzw23, false);

  // We store "float8", meaning we store the full matrix in 2 stores
  private float* tmpStoreAdd = (private float*)pStoreAdd;
  vstore8(xyzw01, 0, tmpStoreAdd);
  vstore8(xyzw23, 1, tmpStoreAdd);
}

void INTERNAL_INLINE_ATTRIBUTE __ocl_gather_transpose_float4x4_AVX2(private float4* pLoadAdd0, private float4* pLoadAdd1, private float4* pLoadAdd2, private float4* pLoadAdd3,
                                    private float4* xOut, private float4* yOut, private float4* zOut, private float4* wOut) {

  //float8 xyzw01 = {*pLoadAdd0, *pLoadAdd1};                               // x0 y0 z0 w0 x1 y1 z1 w1
  //float8 xyzw23 = {*pLoadAdd2, *pLoadAdd3};                               // x2 y2 z2 w2 x3 y3 z3 w3
  // TODO : Replace this shuffle with  {*pLoadAdd0, *pLoadAdd1} instead of shuffle builtin
  // when clang bug will be fixed
  float8 xyzw01;
  float8 xyzw23;

  xyzw01.lo = *pLoadAdd0;                                                   // x0 y0 z0 w0 D  D  D  D
  xyzw01.hi = *pLoadAdd1;                                                   // x0 y0 z0 w0 x4 y4 z4 w4
  xyzw23.lo = *pLoadAdd2;                                                   // x0 y0 z0 w0 D  D  D  D
  xyzw23.hi = *pLoadAdd3;                                                   // x0 y0 z0 w0 x4 y4 z4 w4

  __ocl_load_transpose_float4x4_common_AVX2(xyzw01, xyzw23,
            xOut, yOut, zOut, wOut);
}

void INTERNAL_INLINE_ATTRIBUTE __ocl_transpose_scatter_float4x4_AVX2(private float4* pStoreAdd0, private float4* pStoreAdd1, private float4* pStoreAdd2, private float4* pStoreAdd3,
                               float4 xIn, float4 yIn, float4 zIn, float4 wIn) {
  //float8 xy = {xIn, yIn};                                                   // x0 x1 x2 x3 y0 y1 y2 y3
  //float8 zw = {zIn, wIn};                                                   // z0 z1 z2 z3 w0 w1 w2 w3
  // TODO : Replace this shuffle with  {*pLoadAdd0, *pLoadAdd1} instead of shuffle builtin
  // when clang bug will be fixed
  float8 xy;
  float8 zw;

  xy.lo = xIn;                                                              // x0 x1 x2 x3 D  D  D  D
  xy.hi = yIn;                                                              // x0 x1 x2 x3 y0 y1 y2 y3
  zw.lo = zIn;                                                              // z0 z1 z2 z3 D  D  D  D
  zw.hi = wIn;                                                              // z0 z1 z2 z3 w0 w1 w2 w3

  float8 xyzw01;
  float8 xyzw23;

  __ocl_transpose_float4x4_common_AVX2(xy, zw,
            &xyzw01, &xyzw23, false);

  *pStoreAdd0 = xyzw01.lo;
  *pStoreAdd1 = xyzw01.hi;
  *pStoreAdd2 = xyzw23.lo;
  *pStoreAdd3 = xyzw23.hi;
}

#endif // defined(__AVX2__)

void INLINE_ATTRIBUTE __ocl_load_transpose_float4x4(private float4* pLoadAdd, private float4* xOut, private float4* yOut, private float4* zOut, private float4* wOut) {
#if defined(__AVX2__)
  __ocl_load_transpose_float4x4_AVX2(pLoadAdd, xOut, yOut, zOut, wOut);
#else // defined(__SSE4_2__)
  __ocl_load_transpose_float4x4_AVX(pLoadAdd, xOut, yOut, zOut, wOut);
#endif
}

void INLINE_ATTRIBUTE __ocl_transpose_store_float4x4(private float4* pStoreAdd, float4 xIn, float4 yIn, float4 zIn, float4 wIn) {
#if defined(__AVX2__)
  __ocl_transpose_store_float4x4_AVX2(pStoreAdd, xIn, yIn, zIn, wIn);
#else // defined(__SSE4_2__)
  __ocl_transpose_store_float4x4_AVX(pStoreAdd, xIn, yIn, zIn, wIn);
#endif
}

void INLINE_ATTRIBUTE __ocl_gather_transpose_float4x4(private float4* pLoadAdd0, private float4* pLoadAdd1, private float4* pLoadAdd2, private float4* pLoadAdd3,
                               private float4* xOut, private float4* yOut, private float4* zOut, private float4* wOut) {
#if defined(__AVX2__)
 __ocl_gather_transpose_float4x4_AVX2(pLoadAdd0, pLoadAdd1, pLoadAdd2, pLoadAdd3, xOut, yOut, zOut, wOut);
#else // defined(__SSE4_2__)
 __ocl_gather_transpose_float4x4_AVX(pLoadAdd0, pLoadAdd1, pLoadAdd2, pLoadAdd3, xOut, yOut, zOut, wOut);
#endif
}

void INLINE_ATTRIBUTE __ocl_transpose_scatter_float4x4(private float4* pStoreAdd0, private float4* pStoreAdd1, private float4* pStoreAdd2, private float4* pStoreAdd3,
                               float4 xIn, float4 yIn, float4 zIn, float4 wIn) {
#if defined(__AVX2__)
  __ocl_transpose_scatter_float4x4_AVX2(pStoreAdd0, pStoreAdd1, pStoreAdd2, pStoreAdd3, xIn, yIn, zIn, wIn);
#else // defined(__SSE4_2__)
  __ocl_transpose_scatter_float4x4_AVX(pStoreAdd0, pStoreAdd1, pStoreAdd2, pStoreAdd3, xIn, yIn, zIn, wIn);
#endif
}




// ****************************************************************************
//                                 float4x8
// ****************************************************************************

#if defined(__AVX__)

void INTERNAL_INLINE_ATTRIBUTE __ocl_transpose_float4x8(float4 xyzw0, float4 xyzw1, float4 xyzw2, float4 xyzw3, float4 xyzw4, float4 xyzw5, float4 xyzw6, float4 xyzw7,
            private float8* xOut, private float8* yOut, private float8* zOut, private float8* wOut) {

  // TODO : Replace this shuffle with { xyzw0,  xyzw4} instead of shuffle builtin
  // when clang bug will be fixed
  uint8 concatVectors4 = {0, 1, 2, 3, 4, 5, 6, 7};
  float8 xyzw04 = shuffle2(xyzw0, xyzw4, concatVectors4);                   // x0 y0 z0 w0 x4 y4 z4 w4
  float8 xyzw15 = shuffle2(xyzw1, xyzw5, concatVectors4);                   // x1 y1 z1 w1 x5 y5 z5 w5
  float8 xyzw26 = shuffle2(xyzw2, xyzw6, concatVectors4);                   // x2 y2 z2 w2 x6 y6 z6 w6
  float8 xyzw37 = shuffle2(xyzw3, xyzw7, concatVectors4);                   // x3 y3 z3 w3 x7 y7 z7 w7

  uint8 lowLane8 = {0, 8, 1, 9, 4, 12, 5, 13};
  uint8 highLane8 = {2, 10, 3, 11, 6, 14, 7, 15};

  float8 xy0246 = shuffle2(xyzw04, xyzw26, lowLane8);                       // x0 x2 y0 y2 x4 x6 y4 y6
  float8 zw0246 = shuffle2(xyzw04, xyzw26, highLane8);                      // z0 z2 w0 w2 z4 z6 w4 w6
  float8 xy1357 = shuffle2(xyzw15, xyzw37, lowLane8);                       // x1 x3 y1 y3 x5 x7 y5 y7
  float8 zw1357 = shuffle2(xyzw15, xyzw37, highLane8);                      // z1 z3 w1 w3 z5 z7 w5 w7

  *xOut = shuffle2(xy0246, xy1357, lowLane8);                               // x0 x1 x2 x3 x4 x5 x6 x7
  *yOut = shuffle2(xy0246, xy1357, highLane8);                              // y0 y1 y2 y3 y4 y5 y6 y7
  *zOut = shuffle2(zw0246, zw1357, lowLane8);                               // z0 z1 z2 z3 z4 z5 z6 z7
  *wOut = shuffle2(zw0246, zw1357, highLane8);                              // w0 w1 w2 w3 w4 w5 w6 w7
}

void INTERNAL_INLINE_ATTRIBUTE __ocl_transpose_float8x4(float8 xIn, float8 yIn, float8 zIn, float8 wIn,
            private float4* xyzw0, private float4* xyzw1, private float4* xyzw2, private float4* xyzw3, private float4* xyzw4, private float4* xyzw5, private float4* xyzw6, private float4* xyzw7) {

  uint8 lowLane8 = {0, 8, 1, 9, 4, 12, 5, 13};
  uint8 highLane8 = {2, 10, 3, 11, 6, 14, 7, 15};

  float8 xy0145 = shuffle2(xIn, yIn, lowLane8);                             // x0 y0 x1 y1 x4 y4 x5 y5
  float8 xy2367 = shuffle2(xIn, yIn, highLane8);                            // x2 y2 x3 y3 x6 y6 x7 y7
  float8 zw0145 = shuffle2(zIn, wIn, lowLane8);                             // z0 w0 z1 w1 z4 w4 z5 w5
  float8 zw2367 = shuffle2(zIn, wIn, highLane8);                            // z2 w2 z3 w3 z6 w6 z7 w7


  ulong4 lowLane4 = {0, 4, 2, 6};
  ulong4 highLane4 = {1, 5, 3, 7};
  float8 xyzw04 = as_float8(shuffle2(as_double4(xy0145), as_double4(zw0145), lowLane4));  // x0 y0 z0 w0 x4 y4 z4 w4
  float8 xyzw15 = as_float8(shuffle2(as_double4(xy0145), as_double4(zw0145), highLane4)); // x1 y1 z1 w1 x5 y5 z5 w5
  float8 xyzw26 = as_float8(shuffle2(as_double4(xy2367), as_double4(zw2367), lowLane4));  // x2 y2 z2 w2 x6 y6 z6 w6
  float8 xyzw37 = as_float8(shuffle2(as_double4(xy2367), as_double4(zw2367), highLane4)); // x3 y3 z3 w3 x7 y7 z7 w7

  *xyzw0 = xyzw04.lo;                                                       // x0 y0 z0 w0
  *xyzw1 = xyzw15.lo;                                                       // x1 y1 z1 w1
  *xyzw2 = xyzw26.lo;                                                       // x2 y2 z2 w2
  *xyzw3 = xyzw37.lo;                                                       // x3 y3 z3 w3
  *xyzw4 = xyzw04.hi;                                                       // x4 y4 z4 w4
  *xyzw5 = xyzw15.hi;                                                       // x5 y5 z5 w5
  *xyzw6 = xyzw26.hi;                                                       // x6 y6 z6 w6
  *xyzw7 = xyzw37.hi;                                                       // x7 y7 z7 w7
}


void INLINE_ATTRIBUTE __ocl_load_transpose_float4x8(private float4* pLoadAdd, private float8* xOut, private float8* yOut, private float8* zOut, private float8* wOut) {

  __ocl_transpose_float4x8(pLoadAdd[0], pLoadAdd[1], pLoadAdd[2], pLoadAdd[3], pLoadAdd[4], pLoadAdd[5], pLoadAdd[6], pLoadAdd[7],
                xOut, yOut, zOut, wOut);
}

void INLINE_ATTRIBUTE __ocl_transpose_store_float4x8(float4* pStoreAdd, float8 xIn, float8 yIn, float8 zIn, float8 wIn) {

  __ocl_transpose_float8x4(xIn, yIn, zIn, wIn,
            &(pStoreAdd[0]), &(pStoreAdd[1]), &(pStoreAdd[2]), &(pStoreAdd[3]), &(pStoreAdd[4]), &(pStoreAdd[5]), &(pStoreAdd[6]), &(pStoreAdd[7]));
}

void INLINE_ATTRIBUTE __ocl_masked_load_transpose_float4x8(private float4* pLoadAdd, private float8* xOut, private float8* yOut, private float8* zOut, private float8* wOut, int8 mask) {
  float4 xyzw[8] = {0};
  __ocl_masked_load_float4x8(pLoadAdd, xyzw, mask);
  __ocl_load_transpose_float4x8(xyzw, xOut, yOut, zOut, wOut);
}

void INLINE_ATTRIBUTE __ocl_masked_transpose_store_float4x8(private float4* pStoreAdd, float8 xIn, float8 yIn, float8 zIn, float8 wIn, int8 mask) {
  float4 xyzw[8];
  __ocl_transpose_store_float4x8(xyzw, xIn, yIn, zIn, wIn);
  __ocl_masked_store_float4x8(pStoreAdd, xyzw, mask);
}

void INLINE_ATTRIBUTE __ocl_gather_transpose_float4x8(private float4* pLoadAdd0, private float4* pLoadAdd1, private float4* pLoadAdd2, private float4* pLoadAdd3,
                               private float4* pLoadAdd4, private float4* pLoadAdd5, private float4* pLoadAdd6, private float4* pLoadAdd7,
                               private float8* xOut, private float8* yOut, private float8* zOut, private float8* wOut) {
  __ocl_transpose_float4x8(*pLoadAdd0, *pLoadAdd1, *pLoadAdd2, *pLoadAdd3, *pLoadAdd4, *pLoadAdd5, *pLoadAdd6, *pLoadAdd7,
                xOut, yOut, zOut, wOut);
}

void INLINE_ATTRIBUTE __ocl_transpose_scatter_float4x8(private float4* pStoreAdd0, private float4* pStoreAdd1, private float4* pStoreAdd2, private float4* pStoreAdd3,
                                private float4* pStoreAdd4, private float4* pStoreAdd5, private float4* pStoreAdd6, private float4* pStoreAdd7,
                                float8 xIn, float8 yIn, float8 zIn, float8 wIn) {
  __ocl_transpose_float8x4(xIn, yIn, zIn, wIn,
            pStoreAdd0, pStoreAdd1, pStoreAdd2, pStoreAdd3, pStoreAdd4, pStoreAdd5, pStoreAdd6, pStoreAdd7);
}

void INLINE_ATTRIBUTE __ocl_masked_gather_transpose_float4x8(private float4* pLoadAdd0, private float4* pLoadAdd1, private float4* pLoadAdd2, private float4* pLoadAdd3,
                               private float4* pLoadAdd4, private float4* pLoadAdd5, private float4* pLoadAdd6, private float4* pLoadAdd7,
                               private float8* xOut, private float8* yOut, private float8* zOut, private float8* wOut, int8 mask){
  if (all(mask)) {
   __ocl_gather_transpose_float4x8( pLoadAdd0, pLoadAdd1, pLoadAdd2, pLoadAdd3,
                            pLoadAdd4, pLoadAdd5, pLoadAdd6, pLoadAdd7,
                            xOut, yOut, zOut, wOut);
    return;
  }

  float4 dummy = 0;

  private float4* xyzw0 = mask.s0 ? pLoadAdd0 : &dummy;
  private float4* xyzw1 = mask.s1 ? pLoadAdd1 : &dummy;
  private float4* xyzw2 = mask.s2 ? pLoadAdd2 : &dummy;
  private float4* xyzw3 = mask.s3 ? pLoadAdd3 : &dummy;
  private float4* xyzw4 = mask.s4 ? pLoadAdd4 : &dummy;
  private float4* xyzw5 = mask.s5 ? pLoadAdd5 : &dummy;
  private float4* xyzw6 = mask.s6 ? pLoadAdd6 : &dummy;
  private float4* xyzw7 = mask.s7 ? pLoadAdd7 : &dummy;

	__ocl_gather_transpose_float4x8( xyzw0, xyzw1, xyzw2, xyzw3,
                            xyzw4, xyzw5, xyzw6, xyzw7,
                            xOut, yOut, zOut, wOut);
}

void INLINE_ATTRIBUTE __ocl_masked_transpose_scatter_float4x8(private float4* pStoreAdd0, private float4* pStoreAdd1, private float4* pStoreAdd2, private float4* pStoreAdd3,
                              private float4* pStoreAdd4, private float4* pStoreAdd5, private float4* pStoreAdd6, private float4* pStoreAdd7,
                              float8 xIn, float8 yIn, float8 zIn, float8 wIn, int8 mask){
  if (all(mask)) {
    __ocl_transpose_scatter_float4x8(pStoreAdd0, pStoreAdd1, pStoreAdd2, pStoreAdd3,
                            pStoreAdd4, pStoreAdd5, pStoreAdd6, pStoreAdd7,
                            xIn, yIn, zIn, wIn);
    return;
  }

  float4 dummy;

  private float4* xyzw0 = mask.s0 ? pStoreAdd0 : &dummy;
  private float4* xyzw1 = mask.s1 ? pStoreAdd1 : &dummy;
  private float4* xyzw2 = mask.s2 ? pStoreAdd2 : &dummy;
  private float4* xyzw3 = mask.s3 ? pStoreAdd3 : &dummy;
  private float4* xyzw4 = mask.s4 ? pStoreAdd4 : &dummy;
  private float4* xyzw5 = mask.s5 ? pStoreAdd5 : &dummy;
  private float4* xyzw6 = mask.s6 ? pStoreAdd6 : &dummy;
  private float4* xyzw7 = mask.s7 ? pStoreAdd7 : &dummy;

	__ocl_transpose_scatter_float4x8(xyzw0, xyzw1, xyzw2, xyzw3,
                            xyzw4, xyzw5, xyzw6, xyzw7,
                            xIn, yIn, zIn, wIn);
}
#endif // defined((__AVX__)

#endif // defined((__SSE4_2__)

// Implement naive version of transpose built-ins until we have optimized in built-ins library
// if AVX is not defined then simulate missing transposes
#if !defined(__AVX__)
#if !defined(__SSE4_2__) 
void __ocl_transpose_char4x4(char4 xyzw0, char4 xyzw1, char4 xyzw2, char4 xyzw3,
                              private char4* xOut, private char4* yOut, private char4* zOut, private char4* wOut) {
 (*xOut).s0 = xyzw0.s0;
 (*xOut).s1 = xyzw1.s0;
 (*xOut).s2 = xyzw2.s0;
 (*xOut).s3 = xyzw3.s0;

 (*yOut).s0 = xyzw0.s1;
 (*yOut).s1 = xyzw1.s1;
 (*yOut).s2 = xyzw2.s1;
 (*yOut).s3 = xyzw3.s1;

 (*zOut).s0 = xyzw0.s2;
 (*zOut).s1 = xyzw1.s2;
 (*zOut).s2 = xyzw2.s2;
 (*zOut).s3 = xyzw3.s2;

 (*wOut).s0 = xyzw0.s3;
 (*wOut).s1 = xyzw1.s3;
 (*wOut).s2 = xyzw2.s3;
 (*wOut).s3 = xyzw3.s3;
}

void __inline__ __attribute__((always_inline)) __ocl_transpose_char4x8(char4 xyzw0, char4 xyzw1, char4 xyzw2, char4 xyzw3,
                              char4 xyzw4, char4 xyzw5, char4 xyzw6, char4 xyzw7,
                              private char8* xOut, private char8* yOut, private char8* zOut, private char8* wOut) {
 char4 xLow;
 char4 yLow;
 char4 zLow;
 char4 wLow;

 __ocl_transpose_char4x4(xyzw0, xyzw1, xyzw2, xyzw3,
                              &xLow, &yLow, &zLow, &wLow);

 char4 xHigh;
 char4 yHigh;
 char4 zHigh;
 char4 wHigh;

 __ocl_transpose_char4x4(xyzw0, xyzw1, xyzw2, xyzw3,
                              &xHigh, &yHigh, &zHigh, &wHigh);

 (*xOut).lo = xLow;
 (*xOut).hi = xHigh;
 (*yOut).lo = yLow;
 (*yOut).hi = yHigh;
 (*zOut).lo = zLow;
 (*zOut).hi = zHigh;
 (*wOut).lo = wLow;
 (*wOut).hi = wHigh;
}

void __inline__ __attribute__((always_inline)) __ocl_transpose_char8x4( char8 xIn, char8 yIn, char8 zIn, char8 wIn,
                              private char4* xyzw0, private char4* xyzw1, private char4* xyzw2, private char4* xyzw3,
                              private char4* xyzw4, private char4* xyzw5, private char4* xyzw6, private char4* xyzw7) {
 char4 xLow = xIn.lo;
 char4 yLow = yIn.lo;
 char4 zLow = zIn.lo;
 char4 wLow = wIn.lo;

 __ocl_transpose_char4x4(xLow, yLow, zLow, wLow,
                            xyzw0, xyzw1, xyzw2, xyzw3);

 char4 xHigh = xIn.hi;
 char4 yHigh = yIn.hi;
 char4 zHigh = zIn.hi;
 char4 wHigh = wIn.hi;

 __ocl_transpose_char4x4(xHigh, yHigh, zHigh, wHigh,
                            xyzw4, xyzw5, xyzw6, xyzw7);
}


void __inline__ __attribute__((always_inline)) __ocl_gather_transpose_char4x4(private char4* pLoadAdd0, private char4* pLoadAdd1, private char4* pLoadAdd2, private char4* pLoadAdd3,
                              private char4* xOut, private char4* yOut, private char4* zOut, private char4* wOut) {
 char4 xyzw0 = *pLoadAdd0;
 char4 xyzw1 = *pLoadAdd1;
 char4 xyzw2 = *pLoadAdd2;
 char4 xyzw3 = *pLoadAdd3;

 __ocl_transpose_char4x4(xyzw0, xyzw1, xyzw2, xyzw3,
                              xOut, yOut, zOut, wOut);
}

void __inline__ __attribute__((always_inline)) __ocl_transpose_scatter_char4x4(private char4* pStoreAdd0, private char4* pStoreAdd1, private char4* pStoreAdd2, private char4* pStoreAdd3,
                               char4 xIn, char4 yIn, char4 zIn, char4 wIn) {
  __ocl_transpose_char4x4(xIn, yIn, zIn, wIn,
                              pStoreAdd0, pStoreAdd1, pStoreAdd2, pStoreAdd3);
}

void __inline__ __attribute__((always_inline)) __ocl_gather_transpose_char4x8(private char4* pLoadAdd0, private char4* pLoadAdd1, private char4* pLoadAdd2, private char4* pLoadAdd3,
                              private char4* pLoadAdd4, private char4* pLoadAdd5, private char4* pLoadAdd6, private char4* pLoadAdd7,
                              private char8* xOut, private char8* yOut, private char8* zOut, private char8* wOut) {
 char4 xyzw0 = *pLoadAdd0;
 char4 xyzw1 = *pLoadAdd1;
 char4 xyzw2 = *pLoadAdd2;
 char4 xyzw3 = *pLoadAdd3;
 char4 xyzw4 = *pLoadAdd4;
 char4 xyzw5 = *pLoadAdd5;
 char4 xyzw6 = *pLoadAdd6;
 char4 xyzw7 = *pLoadAdd7;

 __ocl_transpose_char4x8( xyzw0, xyzw1, xyzw2, xyzw3,
                    xyzw4, xyzw5, xyzw6, xyzw7,
                    xOut, yOut, zOut, wOut);
}

void __ocl_transpose_scatter_char4x8(private char4* pStoreAdd0, private char4* pStoreAdd1, private char4* pStoreAdd2, private char4* pStoreAdd3,
                               private char4* pStoreAdd4, private char4* pStoreAdd5, private char4* pStoreAdd6, private char4* pStoreAdd7,
                               char8 xIn, char8 yIn, char8 zIn, char8 wIn) {
  __ocl_transpose_char8x4(xIn, yIn, zIn, wIn,
                              pStoreAdd0, pStoreAdd1, pStoreAdd2, pStoreAdd3,
                              pStoreAdd4, pStoreAdd5, pStoreAdd6, pStoreAdd7);
}

#endif

// Helper functions to speed up mask analyzing
// tblgen generated
int __attribute__((const)) __attribute__((overloadable)) intel_movemask(int4);
int __attribute__((const)) __attribute__((overloadable)) intel_movemask(int8);

// simulate masked transposes. they are not implemented in transpose_functions.cpp
void __ocl_masked_gather_transpose_char4x4(private char4* pLoadAdd0, private char4* pLoadAdd1, private char4* pLoadAdd2, private char4* pLoadAdd3,
                              private char4* xOut, private char4* yOut, private char4* zOut, private char4* wOut, int4 mask)
{
  // get mask as bits in int
  const int rescmp = intel_movemask(mask);
  // ALL 4 elements in mask are -1
  if(rescmp == 0xF){
    __ocl_gather_transpose_char4x4(pLoadAdd0, pLoadAdd1, pLoadAdd2, pLoadAdd3,
                              xOut, yOut, zOut, wOut);
    return;
  }
  // ALL elements in mask are zero
  if(rescmp == 0){
      return;
  }
  // mask addresses to stub variable
  char4 stub;
  pLoadAdd0 = mask.s0 ? pLoadAdd0 : &stub;
  pLoadAdd1 = mask.s1 ? pLoadAdd1 : &stub;
  pLoadAdd2 = mask.s2 ? pLoadAdd2 : &stub;
  pLoadAdd3 = mask.s3 ? pLoadAdd3 : &stub;

  __ocl_gather_transpose_char4x4(pLoadAdd0, pLoadAdd1, pLoadAdd2, pLoadAdd3,
                              xOut, yOut, zOut, wOut);
}

void __ocl_masked_gather_transpose_char4x8(private char4* pLoadAdd0, private char4* pLoadAdd1, private char4* pLoadAdd2, private char4* pLoadAdd3,
                              private char4* pLoadAdd4, private char4* pLoadAdd5, private char4* pLoadAdd6, private char4* pLoadAdd7,
                              private char8* xOut, private char8* yOut, private char8* zOut, private char8* wOut, int8 mask)
{
  // get mask as bits in int
  const int rescmp = intel_movemask(mask);
  
  // ALL 8 elements in mask are -1
  if(rescmp == 0xFF){
       __ocl_gather_transpose_char4x8(
           pLoadAdd0, pLoadAdd1, pLoadAdd2, pLoadAdd3,
           pLoadAdd4, pLoadAdd5, pLoadAdd6, pLoadAdd7,
           xOut, yOut, zOut, wOut);	
       return;
  }
  // ALL elements in mask are zero
  if(rescmp == 0){
      return;
  }
  // mask addresses to stub variable
  char4 stub;
  pLoadAdd0 = mask.s0 ? pLoadAdd0 : &stub;
  pLoadAdd1 = mask.s1 ? pLoadAdd1 : &stub;
  pLoadAdd2 = mask.s2 ? pLoadAdd2 : &stub;
  pLoadAdd3 = mask.s3 ? pLoadAdd3 : &stub;
  pLoadAdd4 = mask.s4 ? pLoadAdd0 : &stub;
  pLoadAdd5 = mask.s5 ? pLoadAdd1 : &stub;
  pLoadAdd6 = mask.s6 ? pLoadAdd2 : &stub;
  pLoadAdd7 = mask.s7 ? pLoadAdd3 : &stub;

  __ocl_gather_transpose_char4x8(
       pLoadAdd0, pLoadAdd1, pLoadAdd2, pLoadAdd3,
       pLoadAdd4, pLoadAdd5, pLoadAdd6, pLoadAdd7,
       xOut, yOut, zOut, wOut);
}

#endif // __AVX__
