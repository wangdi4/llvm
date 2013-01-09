// Copyright (c) 2006-2007 Intel Corporation
// All rights reserved.
// 
// WARRANTY DISCLAIMER
// 
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

///////////////////////////////////////////////////////////
//  transpose_functions.cpp
///////////////////////////////////////////////////////////

#include "transpose_functions.h"

#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#include "masked_load_store_functions.h"
#include <intrin.h>



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
void __inline__ __attribute__((always_inline)) load_transpose_char4x4_common(char16 xyzwIn, char4* xOut, char4* yOut, char4* zOut, char4* wOut) {
  
  char16 xyzw = xyzwIn;                                                     // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3
  *xOut = xyzw.s048C;                                                       // x0  D  D  D x1  D  D  D x2  D  D  D x3  D  D  D 
  
  xyzw = (char16) _mm_srli_si128((__m128i)xyzw, 1);                         // y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3  D
  *yOut = xyzw.s048C;                                                       // y0  D  D  D y1  D  D  D y2  D  D  D y3  D  D  D 
  
  xyzw = (char16) _mm_srli_si128((__m128i)xyzw, 1);                         // z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3  D  D
  *zOut = xyzw.s048C;                                                       // z0  D  D  D z1  D  D  D z2  D  D  D z3  D  D  D
  
  xyzw = (char16) _mm_srli_si128((__m128i)xyzw, 1);                         // w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3  D  D  D
  *wOut = xyzw.s048C;                                                       // w0  D  D  D w1  D  D  D w2  D  D  D w3  D  D  D
}

void __inline__ __attribute__((always_inline)) load_transpose_char4x4(char4* pLoadAdd, char4* xOut, char4* yOut, char4* zOut, char4* wOut) {
  // We load "char16", meaning we load the full matrix in a single load
  char* tmpLoadAdd = (char*)pLoadAdd;
  char16 xyzw = vload16(0, tmpLoadAdd); // unaligned load

  load_transpose_char4x4_common(xyzw, xOut, yOut, zOut, wOut);
}

/// @brief Receives char4x4 matrix as 4 matrix rows, transposes it and outputs the matrix
///        as a whole using char16
/// @param xyzw   - This parameter will contain the transposed char4x4 matrix
/// @param xIn    - Row 0 of the matrix to be transposed
/// @param yIn    - Row 1 of the matrix to be transposed
/// @param zIn    - Row 2 of the matrix to be transposed
/// @param wIn    - Row 3 of the matrix to be transposed
void __inline__ __attribute__((always_inline)) transpose_store_char4x4_common(char16* xyzw, char4 xIn, char4 yIn, char4 zIn, char4 wIn) {

  char16 x;
  char16 y;
  char16 z;
  char16 w;

  x.s0123 = xIn;                                                            // x0 x1 x2 x3  D  D  D  D  D  D  D  D  D  D  D  D
  y.s0123 = yIn;                                                            // y0 y1 y2 y3  D  D  D  D  D  D  D  D  D  D  D  D
  z.s0123 = zIn;                                                            // z0 z1 z2 z3  D  D  D  D  D  D  D  D  D  D  D  D
  w.s0123 = wIn;                                                            // w0 w1 w2 w3  D  D  D  D  D  D  D  D  D  D  D  D
  
  // TODO : do these with shuffle2 instead of unpack intrinsics
  // when shuffle, shuffle2 with const mask passes will be supported in the BE
  short8 xy = (short8) _mm_unpacklo_epi8((__m128i)x, (__m128i)y);           // x0 y0 x1 y1 x2 y2 x3 y3  D  D  D  D  D  D  D  D
  short8 zw = (short8) _mm_unpacklo_epi8((__m128i)z, (__m128i)w);           // z0 w0 z1 w1 z2 w2 z3 w3  D  D  D  D  D  D  D  D

  *xyzw = (char16) _mm_unpacklo_epi16((__m128i)xy, (__m128i)zw);            // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3
}

void __inline__ __attribute__((always_inline)) transpose_store_char4x4(char4* pStoreAdd, char4 xIn, char4 yIn, char4 zIn, char4 wIn) {

  char16 xyzw;
  transpose_store_char4x4_common(&xyzw, xIn, yIn, zIn, wIn);

  // We store "char16", meaning we store the full matrix in a single store
  char* tmpStoreAdd = (char*)pStoreAdd;
  vstore16(xyzw, 0, tmpStoreAdd);  // unaligned store
}

#if defined(__AVX__)
void __inline__ __attribute__((always_inline)) masked_load_transpose_char4x4(char4* pLoadAdd, char4* xOut, char4* yOut, char4* zOut, char4* wOut, int4 mask) {
  char4 xyzw[4];
  masked_load_char4x4(pLoadAdd, xyzw, mask);
  load_transpose_char4x4(xyzw, xOut, yOut, zOut, wOut);
}


void __inline__ __attribute__((always_inline)) masked_transpose_store_char4x4(char4* pStoreAdd, char4 xIn, char4 yIn, char4 zIn, char4 wIn, int4 mask) {  
  char4 xyzw[4];
  transpose_store_char4x4(xyzw, xIn, yIn, zIn, wIn);
  masked_store_char4x4(pStoreAdd, xyzw, mask);
}
#endif // defined(__AVX__)

void __inline__ __attribute__((always_inline)) gather_transpose_char4x4(char4* pLoadAdd0, char4* pLoadAdd1, char4* pLoadAdd2, char4* pLoadAdd3,
                              char4* xOut, char4* yOut, char4* zOut, char4* wOut) {
                              
  
  // TODO : check with Tal if for AVX it's better to do inserts instead of broadcast + blend                              
  // Broadcast the loaded values, all but the first element in the register which will be moved there
  int4 xyzw1 = *((int*)pLoadAdd1);
  int4 xyzw2 = *((int*)pLoadAdd2);
  int4 xyzw3 = *((int*)pLoadAdd3);

  int4 xyzw;
  // TODO : Replace this blend built-in with ?: when clang bug will be fixed
  // There's no blendd in AVX, so we use blendps
  xyzw.s0 = *((int*)pLoadAdd0);                                   // x0 y0 z0 w0  D  D  D  D  D  D  D  D  D  D  D  D
  xyzw = (int4) _mm_blend_ps((__m128)xyzw, (__m128)xyzw1, 0x2);   // x0 y0 z0 w0 x1 y1 z1 w1  D  D  D  D  D  D  D  D
  xyzw = (int4) _mm_blend_ps((__m128)xyzw, (__m128)xyzw2, 0x4);   // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2  D  D  D  D
  xyzw = (int4) _mm_blend_ps((__m128)xyzw, (__m128)xyzw3, 0x8);   // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3

  load_transpose_char4x4_common((char16)xyzw, xOut, yOut, zOut, wOut);
}

void __inline__ __attribute__((always_inline)) transpose_scatter_char4x4(char4* pStoreAdd0, char4* pStoreAdd1, char4* pStoreAdd2, char4* pStoreAdd3,
                               char4 xIn, char4 yIn, char4 zIn, char4 wIn) {

  int4 xyzw = 0;
  transpose_store_char4x4_common((char16*)&xyzw, xIn, yIn, zIn, wIn);

  *((int*)pStoreAdd0) = xyzw.s0;
  *((int*)pStoreAdd1) = xyzw.s1;
  *((int*)pStoreAdd2) = xyzw.s2;
  *((int*)pStoreAdd3) = xyzw.s3;
}

#if defined(__AVX__)
void __inline__ __attribute__((always_inline)) masked_gather_transpose_char4x4(char4* pLoadAdd0, char4* pLoadAdd1, char4* pLoadAdd2, char4* pLoadAdd3,
                              char4* xOut, char4* yOut, char4* zOut, char4* wOut, int4 mask){
  if (all(mask)) {
    gather_transpose_char4x4( pLoadAdd0, pLoadAdd1, pLoadAdd2, pLoadAdd3,
                            xOut, yOut, zOut, wOut);
    return;
  }

  char4 dummy;

  char4* xyzw0 = mask.s0 ? pLoadAdd0 : &dummy;
  char4* xyzw1 = mask.s1 ? pLoadAdd1 : &dummy;
  char4* xyzw2 = mask.s2 ? pLoadAdd2 : &dummy;
  char4* xyzw3 = mask.s3 ? pLoadAdd3 : &dummy;

	gather_transpose_char4x4(xyzw0, xyzw1, xyzw2, xyzw3, xOut, yOut, zOut, wOut);
}

void __inline__ __attribute__((always_inline)) masked_transpose_scatter_char4x4(char4* pStoreAdd0, char4* pStoreAdd1, char4* pStoreAdd2, char4* pStoreAdd3,
                               char4 xIn, char4 yIn, char4 zIn, char4 wIn, int4 mask) {
  if (all(mask)) {
    transpose_scatter_char4x4(pStoreAdd0, pStoreAdd1, pStoreAdd2, pStoreAdd3,
                            xIn, yIn, zIn, wIn);
    return;
  }

  char4 dummy;

  char4* xyzw0 = mask.s0 ? pStoreAdd0 : &dummy;
  char4* xyzw1 = mask.s1 ? pStoreAdd1 : &dummy;
  char4* xyzw2 = mask.s2 ? pStoreAdd2 : &dummy;
  char4* xyzw3 = mask.s3 ? pStoreAdd3 : &dummy;

	transpose_scatter_char4x4(xyzw0, xyzw1, xyzw2, xyzw3, xIn, yIn, zIn, wIn);
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
void __inline__ __attribute__((always_inline)) load_transpose_char4x8_common_AVX(char16 xyzw0In, char16 xyzw1In, char8* xOut, char8* yOut, char8* zOut, char8* wOut) {
  
  char16 xyzw0 = xyzw0In;
  char16 xyzw1 = xyzw1In;

  xyzw0 = xyzw0.s048C159D26AE37BF;                                          // x0 x1 x2 x3 y0 y1 y2 y3 z0 z1 z2 z3 w0 w1 w2 w3
  xyzw1 = xyzw1.s048C159D26AE37BF;                                          // x4 x5 x6 x7 y4 y5 y6 y7 z4 z5 z6 z7 w4 w5 w6 w7

  // TODO : Replace these unpacks with shuffle2 instead of unpack intrinsics
  // when shuffle, shuffle2 with const mask passes will be supported in the BE
  //uint4 mask1 = {0, 4, 1, 5};
  //int4 tmp3 = shuffle2((int4)tmp1, (int4)tmp2, mask1);                    // x0 x1 x2 x3 x4 x5 x6 x7 y0 y1 y2 y3 y4 y5 y6 y7
  char16 xy = (char16) _mm_unpacklo_epi32((__m128i)xyzw0, (__m128i)xyzw1);  // x0 x1 x2 x3 x4 x5 x6 x7 y0 y1 y2 y3 y4 y5 y6 y7
  //uint4 mask2 = {2, 6, 3, 7};
  //int4 tmp4 = shuffle2((int4)tmp1, (int4)tmp2, mask2);                    // z0 z1 z2 z3 z4 z5 z6 z7 w0 w1 w2 w3 w4 w5 w6 w7
  char16 zw = (char16) _mm_unpackhi_epi32((__m128i)xyzw0, (__m128i)xyzw1);  // z0 z1 z2 z3 z4 z5 z6 z7 w0 w1 w2 w3 w4 w5 w6 w7

  // TODO : Replace these unpacks with shuffle2 instead of unpack intrinsics
  // when shuffle, shuffle2 with const mask passes will be supported in the BE
  __m128i dummy;
  char16 x = (char16) _mm_unpacklo_epi8((__m128i)xy, dummy);                // x0  D x1  D x2  D x3  D x4  D x5  D x6  D x7  D
  char16 y = (char16) _mm_unpackhi_epi8((__m128i)xy, dummy);                // y0  D y1  D y2  D y3  D y4  D y5  D y6  D y7  D
  char16 z = (char16) _mm_unpacklo_epi8((__m128i)zw, dummy);                // z0  D z1  D z2  D z3  D z4  D z5  D z6  D z7  D 
  char16 w = (char16) _mm_unpackhi_epi8((__m128i)zw, dummy);                // w0  D w1  D w2  D w3  D w4  D w5  D w6  D w7  D
  
  *xOut = x.s02468ACE;
  *yOut = y.s02468ACE;
  *zOut = z.s02468ACE;
  *wOut = w.s02468ACE;
}

void __inline__ __attribute__((always_inline)) load_transpose_char4x8_AVX(char4* pLoadAdd, char8* xOut, char8* yOut, char8* zOut, char8* wOut) {
  // We load "char16", meaning we load the full matrix in a 2 loads
  char* tmpLoadAdd = (char*)pLoadAdd;
  char16 xyzw0 = vload16(0, tmpLoadAdd);                                  // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3 
  char16 xyzw1 = vload16(1, tmpLoadAdd);                                  // x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6 x7 y7 z7 w7
  
  load_transpose_char4x8_common_AVX(xyzw0, xyzw1, xOut, yOut, zOut, wOut);
}

#if defined(__AVX__)
typedef __v32qi char32;

void __inline__ __attribute__((always_inline)) masked_load_transpose_char4x8(char4* pLoadAdd, char8* xOut, char8* yOut, char8* zOut, char8* wOut, int8 mask) {
  char32 xyzw;
  masked_load_char4x8(pLoadAdd, (char4*)&xyzw, mask);
  load_transpose_char4x8((char4*)&xyzw, xOut, yOut, zOut, wOut);
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
void __inline__ __attribute__((always_inline)) transpose_store_char4x8_common_AVX(char16* xyzw0, char16* xyzw1, char8 xIn, char8 yIn, char8 zIn, char8 wIn) {
  
  char16 x;
  char16 y;
  char16 z;
  char16 w;

  x.s01234567 = xIn;                                                      // x0 x1 x2 x3 x4 x5 x6 x7  D  D  D  D  D  D  D  D
  y.s01234567 = yIn;                                                      // y0 y1 y2 y3 y4 y5 y6 y7  D  D  D  D  D  D  D  D
  z.s01234567 = zIn;                                                      // z0 z1 z2 z3 z4 z5 z6 z7  D  D  D  D  D  D  D  D
  w.s01234567 = wIn;                                                      // w0 w1 w2 w3 w4 w5 w6 w7  D  D  D  D  D  D  D  D
  
  // TODO : do these with shuffle2 instead of unpack intrinsics
  // when shuffle, shuffle2 with const mask passes will be supported in the BE
  short8 xy = (short8) _mm_unpacklo_epi8((__m128i)x, (__m128i)y);         // x0 y0 x1 y1 x2 y2 x3 y3 x4 y4 x5 y5 x6 y6 x7 y7
  short8 zw = (short8) _mm_unpacklo_epi8((__m128i)z, (__m128i)w);         // z0 w0 z1 w1 z2 w2 z3 w3 z4 w4 z5 w5 z6 w6 z7 w7

  *xyzw0 = (char16) _mm_unpacklo_epi16((__m128i)xy, (__m128i)zw);         // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3
  *xyzw1 = (char16) _mm_unpackhi_epi16((__m128i)xy, (__m128i)zw);         // x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6 x7 y7 z7 w7
}

void __inline__ __attribute__((always_inline)) transpose_store_char4x8_AVX(char4* pStoreAdd, char8 xIn, char8 yIn, char8 zIn, char8 wIn) {
  
  char16 xyzw0;
  char16 xyzw1;
  transpose_store_char4x8_common_AVX(&xyzw0, &xyzw1, xIn, yIn, zIn, wIn);

  // We store "char16", meaning we store the full matrix in a 2 stores
  char* tmpStoreAdd = (char*)pStoreAdd;
  vstore16(xyzw0, 0, tmpStoreAdd);
  vstore16(xyzw1, 1, tmpStoreAdd);
}

#if defined(__AVX__)
void __inline__ __attribute__((always_inline)) masked_transpose_store_char4x8(char4* pStoreAdd, char8 xIn, char8 yIn, char8 zIn, char8 wIn, int8 mask) {
  
  char32 xyzw;
  transpose_store_char4x8((char4*)&xyzw, xIn, yIn, zIn, wIn);
  masked_store_char4x8(pStoreAdd, (char4*)&xyzw, mask);
}
#endif // defined(__AVX__)

void __inline__ __attribute__((always_inline)) gather_transpose_char4x8_AVX(char4* pLoadAdd0, char4* pLoadAdd1, char4* pLoadAdd2, char4* pLoadAdd3,
                                  char4* pLoadAdd4, char4* pLoadAdd5, char4* pLoadAdd6, char4* pLoadAdd7,
                                  char8* xOut, char8* yOut, char8* zOut, char8* wOut) {
  
  // TODO : check with Tal if for AVX it's better to do inserts instead of broadcast + blend
  // Broadcast the loaded values, all but the first element in the register which will be moved there
  int4 xyzw1 = *((int*)pLoadAdd1);
  int4 xyzw2 = *((int*)pLoadAdd2);
  int4 xyzw3 = *((int*)pLoadAdd3);
  
  int4 xyzw5 = *((int*)pLoadAdd5);
  int4 xyzw6 = *((int*)pLoadAdd6);
  int4 xyzw7 = *((int*)pLoadAdd7);

  int4 xyzwIn0;
  int4 xyzwIn1;
  // TODO : Replace this blend built-in with ?: when clang bug will be fixed
  // We don't have blendd in AVX, so we use blendps
  xyzwIn0.s0 = *((int*)pLoadAdd0);                                      // x0 y0 z0 w0  D  D  D  D  D  D  D  D  D  D  D  D
  xyzwIn0 = (int4) _mm_blend_ps((__m128)xyzwIn0, (__m128)xyzw1, 0x2);   // x0 y0 z0 w0 x1 y1 z1 w1  D  D  D  D  D  D  D  D
  xyzwIn0 = (int4) _mm_blend_ps((__m128)xyzwIn0, (__m128)xyzw2, 0x4);   // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2  D  D  D  D
  xyzwIn0 = (int4) _mm_blend_ps((__m128)xyzwIn0, (__m128)xyzw3, 0x8);   // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3

  xyzwIn1.s0 = *((int*)pLoadAdd4);                                      // x4 y4 z4 w4  D  D  D  D  D  D  D  D  D  D  D  D
  xyzwIn1 = (int4) _mm_blend_ps((__m128)xyzwIn1, (__m128)xyzw5, 0x2);   // x4 y4 z4 w4 x5 y5 z5 w5  D  D  D  D  D  D  D  D
  xyzwIn1 = (int4) _mm_blend_ps((__m128)xyzwIn1, (__m128)xyzw6, 0x4);   // x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6  D  D  D  D
  xyzwIn1 = (int4) _mm_blend_ps((__m128)xyzwIn1, (__m128)xyzw7, 0x8);   // x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6 x7 y7 z7 w7

  load_transpose_char4x8_common_AVX((char16)xyzwIn0, (char16)xyzwIn1, xOut, yOut, zOut, wOut);
}

void __inline__ __attribute__((always_inline)) transpose_scatter_char4x8_AVX(char4* pStoreAdd0, char4* pStoreAdd1, char4* pStoreAdd2, char4* pStoreAdd3,
                                   char4* pStoreAdd4, char4* pStoreAdd5, char4* pStoreAdd6, char4* pStoreAdd7,
                                   char8 xIn, char8 yIn, char8 zIn, char8 wIn) {

  int4 xyzw0 = 0;
  int4 xyzw1 = 0;
  transpose_store_char4x8_common_AVX((char16*)&xyzw0, (char16*)&xyzw1, xIn, yIn, zIn, wIn);

  *((int*)pStoreAdd0) = xyzw0.s0;
  *((int*)pStoreAdd1) = xyzw0.s1;
  *((int*)pStoreAdd2) = xyzw0.s2;
  *((int*)pStoreAdd3) = xyzw0.s3;
  *((int*)pStoreAdd4) = xyzw1.s0;
  *((int*)pStoreAdd5) = xyzw1.s1;
  *((int*)pStoreAdd6) = xyzw1.s2;
  *((int*)pStoreAdd7) = xyzw1.s3;
}

#if defined(__AVX__)
void __inline__ __attribute__((always_inline)) masked_gather_transpose_char4x8(char4* pLoadAdd0, char4* pLoadAdd1, char4* pLoadAdd2, char4* pLoadAdd3,
                              char4* pLoadAdd4, char4* pLoadAdd5, char4* pLoadAdd6, char4* pLoadAdd7,
                              char8* xOut, char8* yOut, char8* zOut, char8* wOut, int8 mask) {
  if (all(mask)) {
    gather_transpose_char4x8( pLoadAdd0, pLoadAdd1, pLoadAdd2, pLoadAdd3,
                            pLoadAdd4, pLoadAdd5, pLoadAdd6, pLoadAdd7,
                            xOut, yOut, zOut, wOut);
    return;
  }

  char4 dummy;  

  char4* xyzw0 = mask.s0 ? pLoadAdd0 : &dummy;
  char4* xyzw1 = mask.s1 ? pLoadAdd1 : &dummy;
  char4* xyzw2 = mask.s2 ? pLoadAdd2 : &dummy;
  char4* xyzw3 = mask.s3 ? pLoadAdd3 : &dummy;
  char4* xyzw4 = mask.s4 ? pLoadAdd4 : &dummy;
  char4* xyzw5 = mask.s5 ? pLoadAdd5 : &dummy;
  char4* xyzw6 = mask.s6 ? pLoadAdd6 : &dummy;
  char4* xyzw7 = mask.s7 ? pLoadAdd7 : &dummy;

	gather_transpose_char4x8( xyzw0, xyzw1, xyzw2, xyzw3,
                            xyzw4, xyzw5, xyzw6, xyzw7,
                            xOut, yOut, zOut, wOut);
}

void __inline__ __attribute__((always_inline)) masked_transpose_scatter_char4x8(char4* pStoreAdd0, char4* pStoreAdd1, char4* pStoreAdd2, char4* pStoreAdd3,
                               char4* pStoreAdd4, char4* pStoreAdd5, char4* pStoreAdd6, char4* pStoreAdd7,
                               char8 xIn, char8 yIn, char8 zIn, char8 wIn, int8 mask){
  if (all(mask)) {
    transpose_scatter_char4x8(pStoreAdd0, pStoreAdd1, pStoreAdd2, pStoreAdd3,
                            pStoreAdd4, pStoreAdd5, pStoreAdd6, pStoreAdd7,
                            xIn, yIn, zIn, wIn);
    return;
  }
  
  char4 dummy;
  
  char4* xyzw0 = mask.s0 ? pStoreAdd0 : &dummy;
  char4* xyzw1 = mask.s1 ? pStoreAdd1 : &dummy;
  char4* xyzw2 = mask.s2 ? pStoreAdd2 : &dummy;
  char4* xyzw3 = mask.s3 ? pStoreAdd3 : &dummy;
  char4* xyzw4 = mask.s4 ? pStoreAdd4 : &dummy;
  char4* xyzw5 = mask.s5 ? pStoreAdd5 : &dummy;
  char4* xyzw6 = mask.s6 ? pStoreAdd6 : &dummy;
  char4* xyzw7 = mask.s7 ? pStoreAdd7 : &dummy;

	transpose_scatter_char4x8(xyzw0, xyzw1, xyzw2, xyzw3,
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
void __inline__ __attribute__((always_inline)) load_transpose_char4x8_common_AVX2(char32 xyzwIn, char8* xOut, char8* yOut, char8* zOut, char8* wOut) {

  // TODO : Replace this shuffles with shuffle instead of shuffle builtin
  // when shuffle, shuffle2 with const mask passes will be supported in the BE
  char32 xyzw = xyzwIn;                                                     //x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3 | x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6 x7 y7 z7 w7
  char32 dummy = (char32)_mm256_setzero_si256();
  xyzw =  __builtin_shufflevector (xyzw, dummy,                             // x0 x1 x2 x3 y0 y1 y2 y3 z0 z1 z2 z3 w0 w1 w2 w3 | x4 x5 x6 x7 y4 y5 y6 y7 z4 z5 z6 z7 w4 w5 w6 w7
          0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15,
          16, 20, 24, 28, 17, 21, 25, 29, 18, 22, 26, 30, 19, 23, 27, 31);
  xyzw = (char32)(((int8)xyzw).s04152637);                                  // x0 x1 x2 x3 x4 x5 x6 x7 y0 y1 y2 y3 y4 y5 y6 y7 | z0 z1 z2 z3 z4 z5 z6 z7 w0 w1 w2 w3 w4 w5 w6 w7

  // TODO : Replace these unpacks with shuffle instead of unpack intrinsics
  // when shuffle, shuffle2 with const mask passes will be supported in the BE
  char32 xz = (char32) _mm256_unpacklo_epi8((__m256i)xyzw, (__m256i)dummy); // x0  D x1  D x2  D x3  D x4  D x5  D x6  D x7  D | z0  D z1  D z2  D z3  D z4  D z5  D z6  D z7  D
  char32 yw = (char32) _mm256_unpackhi_epi8((__m256i)xyzw, (__m256i)dummy); // y0  D y1  D y2  D y3  D y4  D y5  D y6  D y7  D | w0  D w1  D w2  D w3  D w4  D w5  D w6  D w7  D
  
  char16 x = (char16)(((int8)(xz)).lo);                                     // x0  D x1  D x2  D x3  D x4  D x5  D x6  D x7  D
  char16 y = (char16)(((int8)(yw)).lo);                                     // y0  D y1  D y2  D y3  D y4  D y5  D y6  D y7  D
  char16 z = (char16)(((int8)(xz)).hi);                                     // z0  D z1  D z2  D z3  D z4  D z5  D z6  D z7  D
  char16 w = (char16)(((int8)(yw)).hi);                                     // w0  D w1  D w2  D w3  D w4  D w5  D w6  D w7  D

  *xOut = x.s02468ACE;
  *yOut = y.s02468ACE;
  *zOut = z.s02468ACE;
  *wOut = w.s02468ACE;

}

void __inline__ __attribute__((always_inline)) load_transpose_char4x8_AVX2(char4* pLoadAdd, char8* xOut, char8* yOut, char8* zOut, char8* wOut) {

  // We load "int8", meaning we load the full matrix in a single load
  int* tmpLoadAdd = (int*)pLoadAdd;
  char32 xyzw = (char32) vload8(0, tmpLoadAdd);                           // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3 | x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6 x7 y7 z7 w7

  load_transpose_char4x8_common_AVX2(xyzw, xOut, yOut, zOut, wOut);
}

/// @brief Receives char4x8 matrix as 4 matrix rows, transposes it and outputs the matrix
///        as a whole using char32
/// @param xyzwOut  - This parameter will contain the transposed char4x8 matrix
/// @param xIn      - Row 0 of the matrix to be transposed
/// @param yIn      - Row 1 of the matrix to be transposed
/// @param zIn      - Row 2 of the matrix to be transposed
/// @param wIn      - Row 3 of the matrix to be transposed
void __inline__ __attribute__((always_inline)) transpose_store_char4x8_common_AVX2(char32* xyzwOut, char8 xIn, char8 yIn, char8 zIn, char8 wIn) {
  
  char16 x;
  char16 y;
  char16 z;
  char16 w;

  x.s02468ACE = xIn;                                                        // x0  D x1  D x2  D x3  D x4  D x5  D x6  D x7  D
  y.s02468ACE = yIn;                                                        // y0  D y1  D y2  D y3  D y4  D y5  D y6  D y7  D
  z.s02468ACE = zIn;                                                        // z0  D z1  D z2  D z3  D z4  D z5  D z6  D z7  D
  w.s02468ACE = wIn;                                                        // w0  D w1  D w2  D w3  D w4  D w5  D w6  D w7  D


  char32 xz;
  char32 yw;

  (*(int8*)&xz).lo = (int4)x;                                               // x0  D x1  D x2  D x3  D x4  D x5  D x6  D x7  D |  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D
  (*(int8*)&yw).lo = (int4)y;                                               // y0  D y1  D y2  D y3  D y4  D y5  D y6  D y7  D |  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D
  (*(int8*)&xz).hi = (int4)z;                                               // x0  D x1  D x2  D x3  D x4  D x5  D x6  D x7  D | z0  D z1  D z2  D z3  D z4  D z5  D z6  D z7  D
  (*(int8*)&yw).hi = (int4)w;                                               // y0  D y1  D y2  D y3  D y4  D y5  D y6  D y7  D | w0  D w1  D w2  D w3  D w4  D w5  D w6  D w7  D

  char32 dummy;
  // TODO : Replace these shuffles with shuffle instead of shuffle builtin
  // when shuffle, shuffle2 with const mask passes will be supported in the BE
  // MAKE SURE IT WILL CREATE A SINGLE SHUFFLE
  xz =  __builtin_shufflevector (xz, dummy,                                 // x0 x1 x2 x3 x4 x5 x6 x7  D  D  D  D  D  D  D  D | z0 z1 z2 z3 z4 z5 z6 z7  D  D  D  D  D  D  D  D
          0, 2, 4, 6, 8, 10, 12, 14, 0, 0, 0, 0, 0, 0, 0, 0, 
          16, 18, 20, 22, 24, 26, 28, 30, 0, 0, 0, 0, 0, 0, 0, 0);
  yw =  __builtin_shufflevector (yw, dummy,                                 // y0 y1 y2 y3 y4 y5 y6 y7  D  D  D  D  D  D  D  D | w0 w1 w2 w3 w4 w5 w6 w7  D  D  D  D  D  D  D  D
          0, 2, 4, 6, 8, 10, 12, 14, 0, 0, 0, 0, 0, 0, 0, 0, 
          16, 18, 20, 22, 24, 26, 28, 30, 0, 0, 0, 0, 0, 0, 0, 0);

  // TODO : Replace this unpack with shuffle2 instead of unpack intrinsics
  // when shuffle, shuffle2 with const mask passes will be supported in the BE
  char32 xyzw = (char32) _mm256_unpacklo_epi8((__m256i)xz, (__m256i)yw);    // x0 y0 x1 y1 x2 y2 x3 y3 x4 y4 x5 y5 x6 y6 x7 y7 | z0 w0 z1 w1 z2 w2 z3 w3 z4 w4 z5 w5 z6 w6 z7 w7
  xyzw= (char32)(((int8)xyzw).s01452367);                                   // x0 y0 x1 y1 x2 y2 x3 y3 z0 w0 z1 w1 z2 w2 z3 w3 | x4 y4 x5 y5 x6 y6 x7 y7 z4 w4 z5 w5 z6 w6 z7 w7

  // TODO : Replace this shuffles with shuffle instead of shuffle builtin
  // when shuffle, shuffle2 with const mask passes will be supported in the BE
  xyzw =  __builtin_shufflevector (xyzw, dummy,                             // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3 | x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6 x7 y7 z7 w7
          0, 1, 8, 9, 2, 3, 10, 11, 4, 5, 12, 13, 6, 7, 14, 15, 
          16, 17, 24, 25, 18, 19, 26, 27, 20, 21, 28, 29, 22, 23, 30, 31);

  *xyzwOut = xyzw;
}

void __inline__ __attribute__((always_inline)) transpose_store_char4x8_AVX2(char4* pStoreAdd, char8 xIn, char8 yIn, char8 zIn, char8 wIn) {
  
  char32 xyzw;
  transpose_store_char4x8_common_AVX2(&xyzw, xIn, yIn, zIn, wIn);

  // We store "int8", meaning we store the full matrix in a single store
  int* tmpStoreAdd = (int*)pStoreAdd;
  vstore8((int8)xyzw, 0, tmpStoreAdd);
}

void __inline__ __attribute__((always_inline)) gather_transpose_char4x8_AVX2( char4* pLoadAdd0, char4* pLoadAdd1, char4* pLoadAdd2, char4* pLoadAdd3,
                                    char4* pLoadAdd4, char4* pLoadAdd5, char4* pLoadAdd6, char4* pLoadAdd7,
                                    char8* xOut, char8* yOut, char8* zOut, char8* wOut) {

  // Broadcast the loaded values, all but the first element in the register which will be moved there
  int8 xyzw1 = *((int*)pLoadAdd1);
  int8 xyzw2 = *((int*)pLoadAdd2);
  int8 xyzw3 = *((int*)pLoadAdd3);
  int8 xyzw4 = *((int*)pLoadAdd4);
  int8 xyzw5 = *((int*)pLoadAdd5);
  int8 xyzw6 = *((int*)pLoadAdd6);
  int8 xyzw7 = *((int*)pLoadAdd7);

  int8 xyzw = 0;
  // TODO : Replace this blend built-in with ?: when clang bug will be fixed
  xyzw.s0 = *((int*)pLoadAdd0);                                           // x0 y0 z0 w0  D  D  D  D  D  D  D  D  D  D  D  D |  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D
  xyzw = (int8) _mm256_blend_epi32((__m256i)xyzw, (__m256i)xyzw1, 0x2);  // x0 y0 z0 w0 x1 y1 z1 w1  D  D  D  D  D  D  D  D |  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D
  xyzw = (int8) _mm256_blend_epi32((__m256i)xyzw, (__m256i)xyzw2, 0x4);  // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2  D  D  D  D |  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D
  xyzw = (int8) _mm256_blend_epi32((__m256i)xyzw, (__m256i)xyzw3, 0x8);  // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3 |  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D
  xyzw = (int8) _mm256_blend_epi32((__m256i)xyzw, (__m256i)xyzw4, 0x10);   // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3 | x4 y4 z4 w4  D  D  D  D  D  D  D  D  D  D  D  D
  xyzw = (int8) _mm256_blend_epi32((__m256i)xyzw, (__m256i)xyzw5, 0x20);   // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3 | x4 y4 z4 w4 x5 y5 z5 w5  D  D  D  D  D  D  D  D
  xyzw = (int8) _mm256_blend_epi32((__m256i)xyzw, (__m256i)xyzw6, 0x40);   // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3 | x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6  D  D  D  D
  xyzw = (int8) _mm256_blend_epi32((__m256i)xyzw, (__m256i)xyzw7, 0x80);   // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3 | x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6 x7 y7 z7 w7

  load_transpose_char4x8_common_AVX2((char32)xyzw, xOut, yOut, zOut, wOut);
}

void __inline__ __attribute__((always_inline)) transpose_scatter_char4x8_AVX2(char4* pStoreAdd0, char4* pStoreAdd1, char4* pStoreAdd2, char4* pStoreAdd3,
                                    char4* pStoreAdd4, char4* pStoreAdd5, char4* pStoreAdd6, char4* pStoreAdd7,
                                    char8 xIn, char8 yIn, char8 zIn, char8 wIn) {

  int8 xyzw = 0;
  transpose_store_char4x8_common_AVX2((char32*)&xyzw, xIn, yIn, zIn, wIn);

  *((int*)pStoreAdd0) = xyzw.s0;
  *((int*)pStoreAdd1) = xyzw.s1;
  *((int*)pStoreAdd2) = xyzw.s2;
  *((int*)pStoreAdd3) = xyzw.s3;
  *((int*)pStoreAdd4) = xyzw.s4;
  *((int*)pStoreAdd5) = xyzw.s5;
  *((int*)pStoreAdd6) = xyzw.s6;
  *((int*)pStoreAdd7) = xyzw.s7;
}

#endif // defined(__AVX2__)

void __inline__ __attribute__((always_inline)) load_transpose_char4x8(char4* pLoadAdd, char8* xOut, char8* yOut, char8* zOut, char8* wOut) {
#if defined(__AVX2__)
  load_transpose_char4x8_AVX2(pLoadAdd, xOut, yOut, zOut, wOut);
#else // defined(__SSE_4_2__)
  load_transpose_char4x8_AVX(pLoadAdd, xOut, yOut, zOut, wOut);
#endif
}

void __inline__ __attribute__((always_inline)) transpose_store_char4x8(char4* pStoreAdd, char8 xIn, char8 yIn, char8 zIn, char8 wIn) {
#if defined(__AVX2__)
  transpose_store_char4x8_AVX2(pStoreAdd, xIn, yIn, zIn, wIn);
#else // defined(__SSE_4_2__)
  transpose_store_char4x8_AVX(pStoreAdd, xIn, yIn, zIn, wIn);
#endif
}

void __inline__ __attribute__((always_inline)) gather_transpose_char4x8(char4* pLoadAdd0, char4* pLoadAdd1, char4* pLoadAdd2, char4* pLoadAdd3,
                              char4* pLoadAdd4, char4* pLoadAdd5, char4* pLoadAdd6, char4* pLoadAdd7,
                              char8* xOut, char8* yOut, char8* zOut, char8* wOut) {
#if defined(__AVX2__)
  gather_transpose_char4x8_AVX2(pLoadAdd0, pLoadAdd1, pLoadAdd2, pLoadAdd3,
                                pLoadAdd4, pLoadAdd5, pLoadAdd6, pLoadAdd7,
                                xOut, yOut, zOut, wOut);
#else // defined(__SSE_4_2__)
  gather_transpose_char4x8_AVX( pLoadAdd0, pLoadAdd1, pLoadAdd2, pLoadAdd3,
                                pLoadAdd4, pLoadAdd5, pLoadAdd6, pLoadAdd7,
                                xOut, yOut, zOut, wOut);
#endif
}

void __inline__ __attribute__((always_inline)) transpose_scatter_char4x8(char4* pStoreAdd0, char4* pStoreAdd1, char4* pStoreAdd2, char4* pStoreAdd3,
                               char4* pStoreAdd4, char4* pStoreAdd5, char4* pStoreAdd6, char4* pStoreAdd7,
                               char8 xIn, char8 yIn, char8 zIn, char8 wIn) {
#if defined(__AVX2__)
  transpose_scatter_char4x8_AVX2( pStoreAdd0, pStoreAdd1, pStoreAdd2, pStoreAdd3,
                                  pStoreAdd4, pStoreAdd5, pStoreAdd6, pStoreAdd7,
                                  xIn, yIn, zIn, wIn);
#else // defined(__SSE_4_2__)
  transpose_scatter_char4x8_AVX(  pStoreAdd0, pStoreAdd1, pStoreAdd2, pStoreAdd3,
                                  pStoreAdd4, pStoreAdd5, pStoreAdd6, pStoreAdd7,
                                  xIn, yIn, zIn, wIn);
#endif
}

// ****************************************************************************
//                                 int4x4
// ****************************************************************************



void __inline__ __attribute__((always_inline)) transpose_int4x4_AVX(int4 xyzw0, int4 xyzw1, int4 xyzw2, int4 xyzw3,
  int4* xOut, int4* yOut, int4* zOut, int4* wOut) {

                                                                            // xyzw0 = x0 y0 z0 w0 
                                                                            // xyzw1 = x1 y1 z1 w1
                                                                            // xyzw2 = x2 y2 z2 w2
                                                                            // xyzw3 = x3 y3 z3 w3

  // TODO : Replace these unpacks with shuffle2 instead of unpack intrinsics
  // when shuffle, shuffle2 with const mask passes will be supported in the BE
  int4 xy02 = (int4) _mm_unpacklo_epi32((__m128i)xyzw0, (__m128i)xyzw2);    // x0 x2 y0 y2
  int4 zw02 = (int4) _mm_unpackhi_epi32((__m128i)xyzw0, (__m128i)xyzw2);    // z0 z2 w0 w2
  int4 xy13 = (int4) _mm_unpacklo_epi32((__m128i)xyzw1, (__m128i)xyzw3);    // x1 x3 y1 y3
  int4 zw13 = (int4) _mm_unpackhi_epi32((__m128i)xyzw1, (__m128i)xyzw3);    // z1 z3 w1 w3

  // TODO : Replace these unpacks with shuffle2 instead of unpack intrinsics
  // when shuffle, shuffle2 with const mask passes will be supported in the BE
  *xOut = (int4) _mm_unpacklo_epi32((__m128i)xy02, (__m128i)xy13);          // x0 x1 x2 x3
  *yOut = (int4) _mm_unpackhi_epi32((__m128i)xy02, (__m128i)xy13);          // y0 y1 y2 y3
  *zOut = (int4) _mm_unpacklo_epi32((__m128i)zw02, (__m128i)zw13);          // z0 z1 z2 z3
  *wOut = (int4) _mm_unpackhi_epi32((__m128i)zw02, (__m128i)zw13);          // w0 w1 w2 w3
  
}

void __inline__ __attribute__((always_inline)) load_transpose_int4x4_AVX(int4* pLoadAdd, int4* xOut, int4* yOut, int4* zOut, int4* wOut) {

  // pLoadAdd[0]  = x0 y0 z0 w0
  // pLoadAdd[1]  = x1 y1 z1 w1
  // pLoadAdd[2]  = x2 y2 z2 w2
  // pLoadAdd[3]  = x3 y3 z3 w3

  transpose_int4x4_AVX(pLoadAdd[0], pLoadAdd[1], pLoadAdd[2], pLoadAdd[3],
            xOut, yOut, zOut, wOut);

  // xOut         = x0 x1 x2 x3
  // yOut         = y0 y1 y2 y3
  // zOut         = z0 z1 z2 z3
  // wOut         = w0 w1 w2 w3
}

void __inline__ __attribute__((always_inline)) transpose_store_int4x4_AVX(int4* pStoreAdd, int4 xIn, int4 yIn, int4 zIn, int4 wIn) {

  // xIn          = x0 x1 x2 x3
  // yIn          = y0 y1 y2 y3
  // zIn          = z0 z1 z2 z3
  // wIn          = w0 w1 w2 w3

  transpose_int4x4_AVX(xIn, yIn, zIn, wIn,
            &(pStoreAdd[0]), &(pStoreAdd[1]), &(pStoreAdd[2]), &(pStoreAdd[3]));
  
  // pStoreAdd[0] = x0 y0 z0 w0
  // pStoreAdd[1] = x1 y1 z1 w1
  // pStoreAdd[2] = x2 y2 z2 w2
  // pStoreAdd[3] = x3 y3 z3 w3
}

#if defined(__AVX__)
void __inline__ __attribute__((always_inline)) masked_load_transpose_int4x4(int4* pLoadAdd, int4* xOut, int4* yOut, int4* zOut, int4* wOut, int4 mask) {

  int4 xyzw[4];
  masked_load_int4x4(pLoadAdd, xyzw, mask);
  load_transpose_int4x4(xyzw, xOut, yOut, zOut, wOut);  
}

void __inline__ __attribute__((always_inline)) masked_transpose_store_int4x4(int4* pStoreAdd, int4 xIn, int4 yIn, int4 zIn, int4 wIn, int4 mask) {
  int4 xyzw[4];
  transpose_store_int4x4(xyzw, xIn, yIn, zIn, wIn);
  masked_store_int4x4(pStoreAdd, xyzw, mask);
}
#endif // defined(__AVX__)

void __inline__ __attribute__((always_inline)) gather_transpose_int4x4_AVX(int4* pLoadAdd0, int4* pLoadAdd1, int4* pLoadAdd2, int4* pLoadAdd3,
                                 int4* xOut, int4* yOut, int4* zOut, int4* wOut) {

  // pLoadAdd0    = x0 y0 z0 w0
  // pLoadAdd1    = x1 y1 z1 w1
  // pLoadAdd2    = x2 y2 z2 w2
  // pLoadAdd3    = x3 y3 z3 w3

  transpose_int4x4_AVX(*pLoadAdd0, *pLoadAdd1, *pLoadAdd2, *pLoadAdd3,
            xOut, yOut, zOut, wOut);

  // xOut         = x0 x1 x2 x3
  // yOut         = y0 y1 y2 y3
  // zOut         = z0 z1 z2 z3
  // wOut         = w0 w1 w2 w3
}

void __inline__ __attribute__((always_inline)) transpose_scatter_int4x4_AVX(int4* pStoreAdd0, int4* pStoreAdd1, int4* pStoreAdd2, int4* pStoreAdd3,
                               int4 xIn, int4 yIn, int4 zIn, int4 wIn) {

  // xIn          = x0 x1 x2 x3
  // yIn          = y0 y1 y2 y3
  // zIn          = z0 z1 z2 z3
  // wIn          = w0 w1 w2 w3

  transpose_int4x4_AVX(xIn, yIn, zIn, wIn,
            pStoreAdd0, pStoreAdd1, pStoreAdd2, pStoreAdd3);
  
  // pStoreAdd0   = x0 y0 z0 w0
  // pStoreAdd1   = x1 y1 z1 w1
  // pStoreAdd2   = x2 y2 z2 w2
  // pStoreAdd3   = x3 y3 z3 w3
}

#if defined(__AVX__)
void __inline__ __attribute__((always_inline)) masked_gather_transpose_int4x4(int4* pLoadAdd0, int4* pLoadAdd1, int4* pLoadAdd2, int4* pLoadAdd3,
                             int4* xOut, int4* yOut, int4* zOut, int4* wOut, int4 mask){
  if (all(mask)) {
    gather_transpose_int4x4( pLoadAdd0, pLoadAdd1, pLoadAdd2, pLoadAdd3,
                            xOut, yOut, zOut, wOut);
    return;
  }
  
  int4 dummy;

  int4* xyzw0 = mask.s0 ? pLoadAdd0 : &dummy;
  int4* xyzw1 = mask.s1 ? pLoadAdd1 : &dummy;
  int4* xyzw2 = mask.s2 ? pLoadAdd2 : &dummy;
  int4* xyzw3 = mask.s3 ? pLoadAdd3 : &dummy;

	gather_transpose_int4x4(xyzw0, xyzw1, xyzw2, xyzw3, xOut, yOut, zOut, wOut);
}

void __inline__ __attribute__((always_inline)) masked_transpose_scatter_int4x4(int4* pStoreAdd0, int4* pStoreAdd1, int4* pStoreAdd2, int4* pStoreAdd3,
                              int4 xIn, int4 yIn, int4 zIn, int4 wIn, int4 mask){

  if (all(mask)) {
    transpose_scatter_int4x4(pStoreAdd0, pStoreAdd1, pStoreAdd2, pStoreAdd3,
                            xIn, yIn, zIn, wIn);
    return;
  }


  int4 dummy;

  int4* xyzw0 = mask.s0 ? pStoreAdd0 : &dummy;
  int4* xyzw1 = mask.s1 ? pStoreAdd1 : &dummy;
  int4* xyzw2 = mask.s2 ? pStoreAdd2 : &dummy;
  int4* xyzw3 = mask.s3 ? pStoreAdd3 : &dummy;

	transpose_scatter_int4x4(xyzw0, xyzw1, xyzw2, xyzw3, xIn, yIn, zIn, wIn);
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
void __inline__ __attribute__((always_inline)) transpose_int4x4_common_AVX2(int8 xyzw01, int8 xyzw23,
            int8* xzOut, int8* ywOut, bool isLoad) {
  // TODO: load + permd creates:
  // vmovdqa  (%eax), %ymm1
  // vmovdqa  32(%eax), %ymm0
  // vmovdqa  LCPI20_0, %ymm2
  // vpermd  %ymm1, %ymm2, %ymm1
  // vmovdqa  LCPI20_1, %ymm2
  // vpermd  %ymm0, %ymm2, %ymm2
  // intead of:
  // vmovdqa  LCPI20_0, %ymm2
  // vpermd  (%eax), %ymm2, %ymm1
  // vmovdqa  LCPI20_1, %ymm2
  // vpermd  32(%eax), %ymm2, %ymm2

  if (isLoad) {
    // in this case xyzw01 =                                                       x0 y0 z0 w0 x1 y1 z1 w1
    // in this case xyzw23 =                                                       x2 y2 z2 w2 x3 y3 z3 w3
    xyzw01 = xyzw01.s04152637;                                                  // x0 x1 y0 y1 z0 z1 w0 w1
    xyzw23 = xyzw23.s15043726;                                                  // y2 y3 x2 x3 w2 w3 z2 z3
  } else { // isStore
    // in this case xyzw01 =                                                       x0 x1 x2 x3 y0 y1 y2 y3
    // in this case xyzw23 =                                                       z0 z1 z2 z3 w0 w1 w2 w3
    xyzw01 = xyzw01.s04261537;                                                  // x0 y0 x2 y2 x1 y1 x3 y3
    xyzw23 = xyzw23.s26043715;                                                  // z2 w2 z0 w0 z3 w3 z1 w1
  }

  // TODO : Replace this blend built-in with ?: when clang bug will be fixed
  //int8 cond = {0, 0, 0xFFFFFFFF, 0xFFFFFFFF, 0, 0, 0xFFFFFFFF, 0xFFFFFFFF};
  //int8 xz = cond ? xyzw01 : xyzw23;
  *xzOut = (int8) _mm256_blend_epi32((__m256i)xyzw01, (__m256i)xyzw23, 0xCC);   // x0 x1 x2 x3 z0 z1 z2 z3 for isLoad
                                                                                // x0 y0 z0 w0 x1 y1 z1 w1 for isStore
  
  // TODO : Replace this shuffles with shuffle2 instead of shuffle builtin
  // when shuffle, shuffle2 with const mask passes will be supported in the BE
  *ywOut = __builtin_shufflevector (xyzw01, xyzw23,                             // y0 y1 y2 y3 w0 w1 w2 w3 for isLoad
          2, 3, 8, 9, 6, 7, 12, 13);                                            // x2 y2 z2 w2 x3 y3 z3 w3 for isStore
}

/// @brief Receives int4x4 matrix as 2 halfs (2 int2x4 matrixes) using 2 int8, 
///        transposes it and outputs the rows of the transposed matrix.
/// @param xyzw0In  - Upper part of the int2x4 matrix to be transposed
/// @param xyzw1In  - Lower part of the int2x4 matrix to be transposed
/// @param xOut     - Row 0 of the transposed matrix
/// @param yOut     - Row 1 of the transposed matrix
/// @param zOut     - Row 2 of the transposed matrix
/// @param wOut     - Row 3 of the transposed matrix
void __inline__ __attribute__((always_inline)) load_transpose_int4x4_common_AVX2(int8 xyzw01, int8 xyzw23, int4* xOut, int4* yOut, int4* zOut, int4* wOut) {

                                                                   // xyzw01 = x0 y0 z0 w0 x1 y1 z1 w1
                                                                   // xyzw23 = x2 y2 z2 w2 x3 y3 z3 w3
  int8 xz;
  int8 yw;

  transpose_int4x4_common_AVX2(xyzw01, xyzw23,                              // x0 x1 x2 x3 z0 z1 z2 z3  =  xz
                &xz, &yw, true);                                            // y0 y1 y2 y3 w0 w1 w2 w3  =  yw

  *xOut = xz.s0123;                                                         // x0 x1 x2 x3
  *yOut = yw.s0123;                                                         // y0 y1 y2 y3
  *zOut = xz.s4567;                                                         // z0 z1 z2 z3
  *wOut = yw.s4567;                                                         // w0 w1 w2 w3
}

void __inline__ __attribute__((always_inline)) load_transpose_int4x4_AVX2(int4* pLoadAdd, int4* xOut, int4* yOut, int4* zOut, int4* wOut) {

  // We load "int8", meaning we load the full matrix in 2 loads
  int* tmpLoadAdd = (int*)pLoadAdd;
  int8 xyzw01 = vload8(0, tmpLoadAdd);                                      // x0 y0 z0 w0 x1 y1 z1 w1
  int8 xyzw23 = vload8(1, tmpLoadAdd);                                      // x2 y2 z2 w2 x3 y3 z3 w3

  load_transpose_int4x4_common_AVX2(xyzw01, xyzw23, xOut, yOut, zOut, wOut);
}

void __inline__ __attribute__((always_inline)) transpose_store_int4x4_AVX2(int4* pStoreAdd, int4 xIn, int4 yIn, int4 zIn, int4 wIn) {

  int8 xy = {xIn, yIn};                                                     // x0 x1 x2 x3 y0 y1 y2 y3
  int8 zw = {zIn, wIn};                                                     // z0 z1 z2 z3 w0 w1 w2 w3

  int8 xyzw01;
  int8 xyzw23;

  transpose_int4x4_common_AVX2(xy, zw,
                &xyzw01, &xyzw23, false);

  // We store "int8", meaning we store the full matrix in 2 stores
  int* tmpStoreAdd = (int*)pStoreAdd;
  vstore8(xyzw01, 0, tmpStoreAdd);
  vstore8(xyzw23, 1, tmpStoreAdd);
}

void __inline__ __attribute__((always_inline)) gather_transpose_int4x4_AVX2(int4* pLoadAdd0, int4* pLoadAdd1, int4* pLoadAdd2, int4* pLoadAdd3,
                                  int4* xOut, int4* yOut, int4* zOut, int4* wOut) {
  
  int8 xyzw01 = {*pLoadAdd0, *pLoadAdd1};                                   // x0 y0 z0 w0 x1 y1 z1 w1
  int8 xyzw23 = {*pLoadAdd2, *pLoadAdd3};                                   // x2 y2 z2 w2 x3 y3 z3 w3

  load_transpose_int4x4_common_AVX2(xyzw01, xyzw23, xOut, yOut, zOut, wOut);
}

void __inline__ __attribute__((always_inline)) transpose_scatter_int4x4_AVX2(int4* pStoreAdd0, int4* pStoreAdd1, int4* pStoreAdd2, int4* pStoreAdd3,
                               int4 xIn, int4 yIn, int4 zIn, int4 wIn) {

  int8 xy = {xIn, yIn};                                                     // x0 x1 x2 x3 y0 y1 y2 y3
  int8 zw = {zIn, wIn};                                                     // z0 z1 z2 z3 w0 w1 w2 w3

  int8 xyzw01;
  int8 xyzw23;

  transpose_int4x4_common_AVX2(xy, zw,
                &xyzw01, &xyzw23, false);

  *pStoreAdd0 = xyzw01.s0123;
  *pStoreAdd1 = xyzw01.s4567;
  *pStoreAdd2 = xyzw01.s0123;
  *pStoreAdd3 = xyzw01.s4567;
}

#endif // defined(__AVX2__)

void __inline__ __attribute__((always_inline)) load_transpose_int4x4(int4* pLoadAdd, int4* xOut, int4* yOut, int4* zOut, int4* wOut) {
#if defined(__AVX2__)
  load_transpose_int4x4_AVX2(pLoadAdd, xOut, yOut, zOut, wOut);
#else // defined(__SSE4_2__)
  load_transpose_int4x4_AVX(pLoadAdd, xOut, yOut, zOut, wOut);
#endif
}

void __inline__ __attribute__((always_inline)) transpose_store_int4x4(int4* pStoreAdd, int4 xIn, int4 yIn, int4 zIn, int4 wIn)  {
#if defined(__AVX2__)
  transpose_store_int4x4_AVX2(pStoreAdd, xIn, yIn, zIn, wIn);
#else // defined(__SSE4_2__)
  transpose_store_int4x4_AVX(pStoreAdd, xIn, yIn, zIn, wIn);
#endif
}

void __inline__ __attribute__((always_inline)) gather_transpose_int4x4(int4* pLoadAdd0, int4* pLoadAdd1, int4* pLoadAdd2, int4* pLoadAdd3,
                             int4* xOut, int4* yOut, int4* zOut, int4* wOut) {
#if defined(__AVX2__)
  gather_transpose_int4x4_AVX2(pLoadAdd0, pLoadAdd1, pLoadAdd2, pLoadAdd3, xOut, yOut, zOut, wOut);
#else // defined(__SSE4_2__)
  gather_transpose_int4x4_AVX(pLoadAdd0, pLoadAdd1, pLoadAdd2, pLoadAdd3, xOut, yOut, zOut, wOut);
#endif
}

void __inline__ __attribute__((always_inline)) transpose_scatter_int4x4(int4* pStoreAdd0, int4* pStoreAdd1, int4* pStoreAdd2, int4* pStoreAdd3,
                               int4 xIn, int4 yIn, int4 zIn, int4 wIn) {
#if defined(__AVX2__)
  transpose_scatter_int4x4_AVX2(pStoreAdd0, pStoreAdd1, pStoreAdd2, pStoreAdd3, xIn, yIn, zIn, wIn);
#else // defined(__SSE4_2__)
  transpose_scatter_int4x4_AVX(pStoreAdd0, pStoreAdd1, pStoreAdd2, pStoreAdd3, xIn, yIn, zIn, wIn);
#endif
}


// ****************************************************************************
//                                 int4x8
// ****************************************************************************

#if defined(__AVX__)

void __inline__ __attribute__((always_inline)) load_transpose_int4x8_AVX(int4* pLoadAdd, int8* xOut, int8* yOut, int8* zOut, int8* wOut) {

  int4 x0;
  int4 y0;
  int4 z0;
  int4 w0;

  load_transpose_int4x4_AVX(pLoadAdd, &x0, &y0, &z0, &w0);                  // x0 x1 x2 x3
                                                                            // y0 y1 y2 y3
                                                                            // z0 z1 z2 z3
                                                                            // w0 w1 w2 w3

  int4 x1;
  int4 y1;
  int4 z1;
  int4 w1;

  int4* pLoadAdd1 = &(pLoadAdd[4]);

  load_transpose_int4x4_AVX(pLoadAdd1, &x1, &y1, &z1, &w1);                 // x4 x5 x6 x7
                                                                            // y4 y5 y6 y7
                                                                            // z4 z5 z6 z7
                                                                            // w4 w5 w6 w7

  // TODO : Replace these shuffle builtins with *xOut = {x0, x1} when clang bug will be fixed
  *xOut = __builtin_shufflevector (x0, x1,                                  // x0 x1 x2 x3 x4 x5 x6 x7
          0, 1, 2, 3, 4, 5, 6, 7);
  *yOut = __builtin_shufflevector (y0, y1,                                  // y0 y1 y2 y3 y4 y5 y6 y7
          0, 1, 2, 3, 4, 5, 6, 7);
  *zOut = __builtin_shufflevector (z0, z1,                                  // z0 z1 z2 z3 z4 z5 z6 z7
          0, 1, 2, 3, 4, 5, 6, 7);
  *wOut = __builtin_shufflevector (w0, w1,                                  // w0 w1 w2 w3 w4 w5 w6 w7
          0, 1, 2, 3, 4, 5, 6, 7);

}

void __inline__ __attribute__((always_inline)) masked_load_transpose_int4x8(int4* pLoadAdd, int8* xOut, int8* yOut, int8* zOut, int8* wOut, int8 mask) {
  int4 xyzw[8];
  masked_load_int4x8(pLoadAdd, xyzw, mask);
  load_transpose_int4x8(xyzw, xOut, yOut, zOut, wOut);
}
void __inline__ __attribute__((always_inline)) transpose_store_int4x8_AVX(int4* pStoreAdd, int8 xIn, int8 yIn, int8 zIn, int8 wIn) {
  
  int4 x0 = xIn.s0123;
  int4 y0 = yIn.s0123;
  int4 z0 = zIn.s0123;
  int4 w0 = wIn.s0123;

  transpose_store_int4x4_AVX(pStoreAdd, x0, y0, z0, w0);                    // x0 y0 z0 w0
                                                                            // x1 y1 z1 w1
                                                                            // x2 y2 z2 w2
                                                                            // x3 y3 z3 w3

  int4 x1 = xIn.s4567;
  int4 y1 = yIn.s4567;
  int4 z1 = zIn.s4567;
  int4 w1 = wIn.s4567;

  int4* pStoreAdd1 = &(pStoreAdd[4]);

  transpose_store_int4x4_AVX(pStoreAdd1, x1, y1, z1, w1);                   // x4 y4 z4 w4
                                                                            // x5 y5 z5 w5
                                                                            // x6 y6 z6 w6
                                                                            // x7 y7 z7 w7
}

void __inline__ __attribute__((always_inline)) masked_transpose_store_int4x8(int4* pStoreAdd, int8 xIn, int8 yIn, int8 zIn, int8 wIn, int8 mask) {
  
  int4 xyzw[8];
  transpose_store_int4x8(xyzw, xIn, yIn, zIn, wIn);
  masked_store_int4x8(pStoreAdd, xyzw, mask);
}

void __inline__ __attribute__((always_inline)) gather_transpose_int4x8_AVX(int4* pLoadAdd0, int4* pLoadAdd1, int4* pLoadAdd2, int4* pLoadAdd3,
                                 int4* pLoadAdd4, int4* pLoadAdd5, int4* pLoadAdd6, int4* pLoadAdd7,
                                 int8* xOut, int8* yOut, int8* zOut, int8* wOut) {

  int4 x0;
  int4 y0;
  int4 z0;
  int4 w0;

  gather_transpose_int4x4_AVX(pLoadAdd0, pLoadAdd1, pLoadAdd2, pLoadAdd3,   // x0 x1 x2 x3
                              &x0, &y0, &z0, &w0);                          // y0 y1 y2 y3
                                                                            // z0 z1 z2 z3
                                                                            // w0 w1 w2 w3

  int4 x1;
  int4 y1;
  int4 z1;
  int4 w1;

  gather_transpose_int4x4_AVX(pLoadAdd4, pLoadAdd5, pLoadAdd6, pLoadAdd7,   // x4 x5 x6 x7
                              &x1, &y1, &z1, &w1);                          // y4 y5 y6 y7
                                                                            // z4 z5 z6 z7
                                                                            // w4 w5 w6 w7

  // TODO : Replace these shuffle builtins with *xOut = {x0, x1} when clang bug will be fixed
  *xOut = __builtin_shufflevector (x0, x1,                                  // x0 x1 x2 x3 x4 x5 x6 x7
          0, 1, 2, 3, 4, 5, 6, 7);
  *yOut = __builtin_shufflevector (y0, y1,                                  // y0 y1 y2 y3 y4 y5 y6 y7
          0, 1, 2, 3, 4, 5, 6, 7);
  *zOut = __builtin_shufflevector (z0, z1,                                  // z0 z1 z2 z3 z4 z5 z6 z7
          0, 1, 2, 3, 4, 5, 6, 7);
  *wOut = __builtin_shufflevector (w0, w1,                                  // w0 w1 w2 w3 w4 w5 w6 w7
          0, 1, 2, 3, 4, 5, 6, 7);
}

void __inline__ __attribute__((always_inline)) transpose_scatter_int4x8_AVX(int4* pStoreAdd0, int4* pStoreAdd1, int4* pStoreAdd2, int4* pStoreAdd3,
                                  int4* pStoreAdd4, int4* pStoreAdd5, int4* pStoreAdd6, int4* pStoreAdd7,
                                  int8 xIn, int8 yIn, int8 zIn, int8 wIn) {
  int4 x0 = xIn.s0123;
  int4 y0 = yIn.s0123;
  int4 z0 = zIn.s0123;
  int4 w0 = wIn.s0123;

  transpose_scatter_int4x4_AVX(pStoreAdd0, pStoreAdd1, pStoreAdd2, pStoreAdd3, 
                               x0, y0, z0, w0);                             // x0 y0 z0 w0
                                                                            // x1 y1 z1 w1
                                                                            // x2 y2 z2 w2
                                                                            // x3 y3 z3 w3

  int4 x1 = xIn.s4567;
  int4 y1 = yIn.s4567;
  int4 z1 = zIn.s4567;
  int4 w1 = wIn.s4567;

  transpose_scatter_int4x4_AVX(pStoreAdd4, pStoreAdd5, pStoreAdd6, pStoreAdd7, 
                               x1, y1, z1, w1);                             // x4 y4 z4 w4
                                                                            // x5 y5 z5 w5
                                                                            // x6 y6 z6 w6
                                                                            // x7 y7 z7 w7
}

void __inline__ __attribute__((always_inline)) masked_gather_transpose_int4x8(int4* pLoadAdd0, int4* pLoadAdd1, int4* pLoadAdd2, int4* pLoadAdd3,
                             int4* pLoadAdd4, int4* pLoadAdd5, int4* pLoadAdd6, int4* pLoadAdd7,
                             int8* xOut, int8* yOut, int8* zOut, int8* wOut, int8 mask) {
  if (all(mask)) {
    gather_transpose_int4x8( pLoadAdd0, pLoadAdd1, pLoadAdd2, pLoadAdd3,
                            pLoadAdd4, pLoadAdd5, pLoadAdd6, pLoadAdd7,
                            xOut, yOut, zOut, wOut);
    return;
  }

  int4 dummy;

  int4* xyzw0 = mask.s0 ? pLoadAdd0 : &dummy;
  int4* xyzw1 = mask.s1 ? pLoadAdd1 : &dummy;
  int4* xyzw2 = mask.s2 ? pLoadAdd2 : &dummy;
  int4* xyzw3 = mask.s3 ? pLoadAdd3 : &dummy;
  int4* xyzw4 = mask.s4 ? pLoadAdd4 : &dummy;
  int4* xyzw5 = mask.s5 ? pLoadAdd5 : &dummy;
  int4* xyzw6 = mask.s6 ? pLoadAdd6 : &dummy;
  int4* xyzw7 = mask.s7 ? pLoadAdd7 : &dummy;

	gather_transpose_int4x8( xyzw0, xyzw1, xyzw2, xyzw3,
                            xyzw4, xyzw5, xyzw6, xyzw7,
                            xOut, yOut, zOut, wOut);
}

void __inline__ __attribute__((always_inline)) masked_transpose_scatter_int4x8(int4* pStoreAdd0, int4* pStoreAdd1, int4* pStoreAdd2, int4* pStoreAdd3,
                              int4* pStoreAdd4, int4* pStoreAdd5, int4* pStoreAdd6, int4* pStoreAdd7,
                              int8 xIn, int8 yIn, int8 zIn, int8 wIn, int8 mask){
  if (all(mask)) {
    transpose_scatter_int4x8(pStoreAdd0, pStoreAdd1, pStoreAdd2, pStoreAdd3,
                            pStoreAdd4, pStoreAdd5, pStoreAdd6, pStoreAdd7,
                            xIn, yIn, zIn, wIn);
    return;
  }

  int4 dummy;
  
  int4* xyzw0 = mask.s0 ? pStoreAdd0 : &dummy;
  int4* xyzw1 = mask.s1 ? pStoreAdd1 : &dummy;
  int4* xyzw2 = mask.s2 ? pStoreAdd2 : &dummy;
  int4* xyzw3 = mask.s3 ? pStoreAdd3 : &dummy;
  int4* xyzw4 = mask.s4 ? pStoreAdd4 : &dummy;
  int4* xyzw5 = mask.s5 ? pStoreAdd5 : &dummy;
  int4* xyzw6 = mask.s6 ? pStoreAdd6 : &dummy;
  int4* xyzw7 = mask.s7 ? pStoreAdd7 : &dummy;

	transpose_scatter_int4x8(xyzw0, xyzw1, xyzw2, xyzw3,
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
void __inline__ __attribute__((always_inline)) transpose_int4x8_common_AVX2(int8 xyzw04, int8 xyzw15, int8 xyzw26, int8 xyzw37,
             int8* xOut, int8* yOut, int8* zOut, int8* wOut) {
  
  // TODO : Replace these unpacks with shuffle2 instead of unpack intrinsics
  // when shuffle, shuffle2 with const mask passes will be supported in the BE
  int8 xy0246 = (int8) _mm256_unpacklo_epi32((__m256i)xyzw04, (__m256i)xyzw26);    // x0 x2 y0 y2 x4 x6 y4 y6
  int8 zw0246 = (int8) _mm256_unpackhi_epi32((__m256i)xyzw04, (__m256i)xyzw26);    // z0 z2 w0 w2 z4 z6 w4 w6
  int8 xy1357 = (int8) _mm256_unpacklo_epi32((__m256i)xyzw15, (__m256i)xyzw37);    // x1 x3 y1 y3 x5 x7 y5 y7
  int8 zw1357 = (int8) _mm256_unpackhi_epi32((__m256i)xyzw15, (__m256i)xyzw37);    // z1 z3 w1 w3 z5 z7 w5 w7

  // TODO : Replace these unpacks with shuffle2 instead of unpack intrinsics
  // when shuffle, shuffle2 with const mask passes will be supported in the BE
  *xOut = (int8) _mm256_unpacklo_epi32((__m256i)xy0246, (__m256i)xy1357);         // x0 x1 x2 x3 x4 x5 x6 x7
  *yOut = (int8) _mm256_unpackhi_epi32((__m256i)xy0246, (__m256i)xy1357);         // y0 y1 y2 y3 y4 y5 y6 y7
  *zOut = (int8) _mm256_unpacklo_epi32((__m256i)zw0246, (__m256i)zw1357);         // z0 z1 z2 z3 z4 z5 z6 z7
  *wOut = (int8) _mm256_unpackhi_epi32((__m256i)zw0246, (__m256i)zw1357);         // w0 w1 w2 w3 w4 w5 w6 w7
}



void __inline__ __attribute__((always_inline)) gather_transpose_int4x8_AVX2(int4* pLoadAdd0, int4* pLoadAdd1, int4* pLoadAdd2, int4* pLoadAdd3,
                                  int4* pLoadAdd4, int4* pLoadAdd5, int4* pLoadAdd6, int4* pLoadAdd7,
                                  int8* xOut, int8* yOut, int8* zOut, int8* wOut) {
                                  // TODO: creates
  // vmovaps  48(%eax), %xmm0
  // vmovaps  112(%eax), %xmm1
  // vinsertf128  $1, %xmm1, %ymm0, %ymm0
  // instead of vpinsert from memory and not loading into xmm1

  // TODO : Replace this shuffle with { pLoadAdd0,  pLoadAdd4} instead of shuffle builtin
  // when clang bug will be fixed
  int8 xyzw04 = __builtin_shufflevector (*pLoadAdd0, *pLoadAdd4,            // x0 y0 z0 w0 x4 y4 z4 w4
            0, 1, 2, 3, 4, 5, 6, 7);
  int8 xyzw15 = __builtin_shufflevector (*pLoadAdd1, *pLoadAdd5,            // x1 y1 z1 w1 x5 y5 z5 w5
            0, 1, 2, 3, 4, 5, 6, 7); 
  int8 xyzw26 = __builtin_shufflevector (*pLoadAdd2, *pLoadAdd6,            // x2 y2 z2 w2 x6 y6 z6 w6
            0, 1, 2, 3, 4, 5, 6, 7);
  int8 xyzw37 = __builtin_shufflevector (*pLoadAdd3, *pLoadAdd7,            // x3 y3 z3 w3 x7 y7 z7 w7
            0, 1, 2, 3, 4, 5, 6, 7);

  transpose_int4x8_common_AVX2(xyzw04, xyzw15, xyzw26, xyzw37,
            xOut, yOut, zOut, wOut);
                                                                            // xOut = x0 x1 x2 x3 x4 x5 x6 x7
                                                                            // yOut = y0 y1 y2 y3 y4 y5 y6 y7
                                                                            // zOut = z0 z1 z2 z3 z4 z5 z6 z7
                                                                            // wOut = w0 w1 w2 w3 w4 w5 w6 w7
}

void __inline__ __attribute__((always_inline)) transpose_scatter_int4x8_AVX2(int4* pStoreAdd0, int4* pStoreAdd1, int4* pStoreAdd2, int4* pStoreAdd3,
                                   int4* pStoreAdd4, int4* pStoreAdd5, int4* pStoreAdd6, int4* pStoreAdd7,
                                   int8 xIn, int8 yIn, int8 zIn, int8 wIn) {
  int8 xyzw04;
  int8 xyzw15;
  int8 xyzw26;
  int8 xyzw37;

  transpose_int4x8_common_AVX2(xIn, yIn, zIn, wIn,
            &xyzw04, &xyzw15, &xyzw26, &xyzw37);

                                                                            // xyzw04 = x0 y0 z0 w0 x4 y4 z4 w4
                                                                            // xyzw15 = x1 y1 z1 w1 x5 y5 z5 w5
                                                                            // xyzw26 = x2 y2 z2 w2 x6 y6 z6 w6
                                                                            // xyzw37 = x3 y3 z3 w3 x7 y7 z7 w7

  // TODO: creates
  // vextractf128  $1, %ymm4, %xmm2
  // vmovapd  %xmm2, 64(%eax)
  // instead of extracting directly to memory
  
  *pStoreAdd0 = xyzw04.s0123;                                               // x0 y0 z0 w0
  *pStoreAdd1 = xyzw15.s0123;                                               // x1 y1 z1 w1
  *pStoreAdd2 = xyzw26.s0123;                                               // x2 y2 z2 w2
  *pStoreAdd3 = xyzw37.s0123;                                               // x3 y3 z3 w3
  *pStoreAdd4 = xyzw04.s4567;                                               // x4 y4 z4 w4
  *pStoreAdd5 = xyzw15.s4567;                                               // x5 y5 z5 w5
  *pStoreAdd6 = xyzw26.s4567;                                               // x6 y6 z6 w6
  *pStoreAdd7 = xyzw37.s4567;                                               // x7 y7 z7 w7
}

void __inline__ __attribute__((always_inline)) load_transpose_int4x8_AVX2(int4* pLoadAdd, int8* xOut, int8* yOut, int8* zOut, int8* wOut) {
  gather_transpose_int4x8_AVX2(&(pLoadAdd[0]), &(pLoadAdd[1]), &(pLoadAdd[2]), &(pLoadAdd[3]),
                               &(pLoadAdd[4]), &(pLoadAdd[5]), &(pLoadAdd[6]), &(pLoadAdd[7]),
                               xOut, yOut, zOut, wOut);
}

void __inline__ __attribute__((always_inline)) transpose_store_int4x8_AVX2(int4* pStoreAdd, int8 xIn, int8 yIn, int8 zIn, int8 wIn) {

  transpose_scatter_int4x8_AVX2(&(pStoreAdd[0]), &(pStoreAdd[1]), &(pStoreAdd[2]), &(pStoreAdd[3]),
                                &(pStoreAdd[4]), &(pStoreAdd[5]), &(pStoreAdd[6]), &(pStoreAdd[7]),
                                xIn, yIn, zIn, wIn);
}

#endif // defined(__AVX2__)


#if defined(__AVX__)

void __inline__ __attribute__((always_inline)) load_transpose_int4x8(int4* pLoadAdd, int8* xOut, int8* yOut, int8* zOut, int8* wOut) {
#if defined(__AVX2__)
  load_transpose_int4x8_AVX2(pLoadAdd, xOut, yOut, zOut, wOut);
#else // defined(__AVX__)
  load_transpose_int4x8_AVX(pLoadAdd, xOut, yOut, zOut, wOut);
#endif
}

void __inline__ __attribute__((always_inline)) transpose_store_int4x8(int4* pStoreAdd, int8 xIn, int8 yIn, int8 zIn, int8 wIn) {
#if defined(__AVX2__)
  transpose_store_int4x8_AVX2(pStoreAdd, xIn, yIn, zIn, wIn);
#else // defined(__AVX__)
  transpose_store_int4x8_AVX(pStoreAdd, xIn, yIn, zIn, wIn);
#endif
}

void __inline__ __attribute__((always_inline)) gather_transpose_int4x8(int4* pLoadAdd0, int4* pLoadAdd1, int4* pLoadAdd2, int4* pLoadAdd3,
                             int4* pLoadAdd4, int4* pLoadAdd5, int4* pLoadAdd6, int4* pLoadAdd7,
                             int8* xOut, int8* yOut, int8* zOut, int8* wOut) {
#if defined(__AVX2__)
  gather_transpose_int4x8_AVX2(pLoadAdd0, pLoadAdd1, pLoadAdd2, pLoadAdd3,
                               pLoadAdd4, pLoadAdd5, pLoadAdd6, pLoadAdd7,
                               xOut, yOut, zOut, wOut);
#else // defined(__AVX__)
  gather_transpose_int4x8_AVX(pLoadAdd0, pLoadAdd1, pLoadAdd2, pLoadAdd3,
                               pLoadAdd4, pLoadAdd5, pLoadAdd6, pLoadAdd7,
                               xOut, yOut, zOut, wOut);
#endif
}

void __inline__ __attribute__((always_inline)) transpose_scatter_int4x8(int4* pStoreAdd0, int4* pStoreAdd1, int4* pStoreAdd2, int4* pStoreAdd3,
                              int4* pStoreAdd4, int4* pStoreAdd5, int4* pStoreAdd6, int4* pStoreAdd7,
                              int8 xIn, int8 yIn, int8 zIn, int8 wIn) {
#if defined(__AVX2__)
  transpose_scatter_int4x8_AVX2(pStoreAdd0, pStoreAdd1, pStoreAdd2, pStoreAdd3,
                                pStoreAdd4, pStoreAdd5, pStoreAdd6, pStoreAdd7,
                                xIn, yIn, zIn, wIn);
#else // defined(__AVX__)
  transpose_scatter_int4x8_AVX(pStoreAdd0, pStoreAdd1, pStoreAdd2, pStoreAdd3,
                               pStoreAdd4, pStoreAdd5, pStoreAdd6, pStoreAdd7,
                               xIn, yIn, zIn, wIn);
#endif
}

#endif // defined(__AVX__)


// ****************************************************************************
//                                 float4x4
// ****************************************************************************



void __inline__ __attribute__((always_inline)) transpose_float4x4_AVX(float4 xyzw0, float4 xyzw1, float4 xyzw2, float4 xyzw3,
  float4* xOut, float4* yOut, float4* zOut, float4* wOut) {
    //transpose_int4x4_AVX((int4)xyzw0, (int4)xyzw1, (int4)xyzw2, (int4)xyzw3,
  //(int4*)xOut, (int4*)yOut, (int4*)zOut, (int4*)wOut);

                                                                            // xyzw0 = x0 y0 z0 w0 
                                                                            // xyzw1 = x1 y1 z1 w1
                                                                            // xyzw2 = x2 y2 z2 w2
                                                                            // xyzw3 = x3 y3 z3 w3

  // TODO : Replace these unpacks with shuffle2 instead of unpack intrinsics
  // when shuffle, shuffle2 with const mask passes will be supported in the BE
  float4 xy02 = (float4) _mm_unpacklo_ps((__m128)xyzw0, (__m128)xyzw2);     // x0 x2 y0 y2
  float4 zw02 = (float4) _mm_unpackhi_ps((__m128)xyzw0, (__m128)xyzw2);     // z0 z2 w0 w2
  float4 xy13 = (float4) _mm_unpacklo_ps((__m128)xyzw1, (__m128)xyzw3);     // x1 x3 y1 y3
  float4 zw13 = (float4) _mm_unpackhi_ps((__m128)xyzw1, (__m128)xyzw3);     // z1 z3 w1 w3

  // TODO : Replace these unpacks with shuffle2 instead of unpack intrinsics
  // when shuffle, shuffle2 with const mask passes will be supported in the BE
  *xOut = (float4) _mm_unpacklo_ps((__m128)xy02, (__m128)xy13);             // x0 x1 x2 x3
  *yOut = (float4) _mm_unpackhi_ps((__m128)xy02, (__m128)xy13);             // y0 y1 y2 y3
  *zOut = (float4) _mm_unpacklo_ps((__m128)zw02, (__m128)zw13);             // z0 z1 z2 z3
  *wOut = (float4) _mm_unpackhi_ps((__m128)zw02, (__m128)zw13);             // w0 w1 w2 w3
  
}

void __inline__ __attribute__((always_inline)) load_transpose_float4x4_AVX(float4* pLoadAdd, float4* xOut, float4* yOut, float4* zOut, float4* wOut) {
  // pLoadAdd[0]  = x0 y0 z0 w0
  // pLoadAdd[1]  = x1 y1 z1 w1
  // pLoadAdd[2]  = x2 y2 z2 w2
  // pLoadAdd[3]  = x3 y3 z3 w3

  transpose_float4x4_AVX(pLoadAdd[0], pLoadAdd[1], pLoadAdd[2], pLoadAdd[3],
            xOut, yOut, zOut, wOut);

  // xOut         = x0 x1 x2 x3
  // yOut         = y0 y1 y2 y3
  // zOut         = z0 z1 z2 z3
  // wOut         = w0 w1 w2 w3
}

void transpose_store_float4x4_AVX(float4* pStoreAdd, float4 xIn, float4 yIn, float4 zIn, float4 wIn) {
  // xIn          = x0 x1 x2 x3
  // yIn          = y0 y1 y2 y3
  // zIn          = z0 z1 z2 z3
  // wIn          = w0 w1 w2 w3

  transpose_float4x4_AVX(xIn, yIn, zIn, wIn,
            &(pStoreAdd[0]), &(pStoreAdd[1]), &(pStoreAdd[2]), &(pStoreAdd[3]));
  
  // pStoreAdd[0] = x0 y0 z0 w0
  // pStoreAdd[1] = x1 y1 z1 w1
  // pStoreAdd[2] = x2 y2 z2 w2
  // pStoreAdd[3] = x3 y3 z3 w3
}

#if defined(__AVX__)
void __inline__ __attribute__((always_inline)) masked_load_transpose_float4x4(float4* pLoadAdd, float4* xOut, float4* yOut, float4* zOut, float4* wOut, int4 mask) {
  float4 xyzw[4] = {0};
  masked_load_float4x4(pLoadAdd, xyzw, mask);
  load_transpose_float4x4(xyzw, xOut, yOut, zOut, wOut);
}

void __inline__ __attribute__((always_inline)) masked_transpose_store_float4x4(float4* pStoreAdd, float4 xIn, float4 yIn, float4 zIn, float4 wIn, int4 mask) {
  float4 xyzw[4];
  transpose_store_float4x4(xyzw, xIn, yIn, zIn, wIn);
  masked_store_float4x4(pStoreAdd, xyzw, mask);  
}
#endif // defined(__AVX__)

void __inline__ __attribute__((always_inline)) gather_transpose_float4x4_AVX(float4* pLoadAdd0, float4* pLoadAdd1, float4* pLoadAdd2, float4* pLoadAdd3,
                              float4* xOut, float4* yOut, float4* zOut, float4* wOut) {
  transpose_float4x4_AVX(*pLoadAdd0, *pLoadAdd1, *pLoadAdd2, *pLoadAdd3,
                        xOut, yOut, zOut, wOut);
}

void __inline__ __attribute__((always_inline)) transpose_scatter_float4x4_AVX(float4* pStoreAdd0, float4* pStoreAdd1, float4* pStoreAdd2, float4* pStoreAdd3,
                               float4 xIn, float4 yIn, float4 zIn, float4 wIn) {
  transpose_float4x4_AVX(xIn, yIn, zIn, wIn,
                        pStoreAdd0, pStoreAdd1, pStoreAdd2, pStoreAdd3);
}

#if defined(__AVX__)
void __inline__ __attribute__((always_inline)) masked_gather_transpose_float4x4(float4* pLoadAdd0, float4* pLoadAdd1, float4* pLoadAdd2, float4* pLoadAdd3,
                               float4* xOut, float4* yOut, float4* zOut, float4* wOut, int4 mask){
  if (all(mask)) {
    gather_transpose_float4x4(pLoadAdd0, pLoadAdd1, pLoadAdd2, pLoadAdd3, xOut, yOut, zOut, wOut);
    return;
  }
  
  float4 dummy = 0;

  float4* xyzw0 = mask.s0 ? pLoadAdd0 : &dummy;
  float4* xyzw1 = mask.s1 ? pLoadAdd1 : &dummy;
  float4* xyzw2 = mask.s2 ? pLoadAdd2 : &dummy;
  float4* xyzw3 = mask.s3 ? pLoadAdd3 : &dummy;

	gather_transpose_float4x4( xyzw0, xyzw1, xyzw2, xyzw3,
                            xOut, yOut, zOut, wOut);
}

void __inline__ __attribute__((always_inline)) masked_transpose_scatter_float4x4(float4* pStoreAdd0, float4* pStoreAdd1, float4* pStoreAdd2, float4* pStoreAdd3,
                                float4 xIn, float4 yIn, float4 zIn, float4 wIn, int4 mask) {
  if (all(mask)) {
    transpose_scatter_float4x4(pStoreAdd0, pStoreAdd1, pStoreAdd2, pStoreAdd3,
                            xIn, yIn, zIn, wIn);
    return;
  }

  float4 dummy;

  float4* xyzw0 = mask.s0 ? pStoreAdd0 : &dummy;
  float4* xyzw1 = mask.s1 ? pStoreAdd1 : &dummy;
  float4* xyzw2 = mask.s2 ? pStoreAdd2 : &dummy;
  float4* xyzw3 = mask.s3 ? pStoreAdd3 : &dummy;

	transpose_scatter_float4x4(xyzw0, xyzw1, xyzw2, xyzw3, xIn, yIn, zIn, wIn);
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
void __inline__ __attribute__((always_inline)) transpose_float4x4_common_AVX2(float8 xyzw01, float8 xyzw23,
            float8* xz, float8* yw, bool isLoad) {

  // TODO: load + permd creates:
  // vmovdqa  (%eax), %ymm1
  // vmovdqa  32(%eax), %ymm0
  // vmovdqa  LCPI20_0, %ymm2
  // vpermd  %ymm1, %ymm2, %ymm1
  // vmovdqa  LCPI20_1, %ymm2
  // vpermd  %ymm0, %ymm2, %ymm2
  // intead of:
  // vmovdqa  LCPI20_0, %ymm2
  // vpermd  (%eax), %ymm2, %ymm1
  // vmovdqa  LCPI20_1, %ymm2
  // vpermd  32(%eax), %ymm2, %ymm2

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
  *xz = (float8) _mm256_blend_ps((__m256)xyzw01, (__m256)xyzw23, 0xCC);   // x0 x1 x2 x3 z0 z1 z2 z3 for isLoad
                                                                                // x0 y0 z0 w0 x1 y1 z1 w1 for isStore

  // TODO : Replace this shuffles with shuffle2 instead of shuffle builtin
  // when shuffle, shuffle2 with const mask passes will be supported in the BE
  *yw = __builtin_shufflevector ((__v8sf)xyzw01, (__v8sf)xyzw23,          // y0 y1 y2 y3 w0 w1 w2 w3 for isLoad
          2, 3, 8, 9, 6, 7, 12, 13);                                            // x2 y2 z2 w2 x3 y3 z3 w3 for isStore
}

/// @brief Receives float4x4 matrix as 2 halfs (2 float2x4 matrixes) using 2 float8, 
///        transposes it and outputs the rows of the transposed matrix.
/// @param xyzw0In  - Upper part of the int2x4 matrix to be transposed
/// @param xyzw1In  - Lower part of the int2x4 matrix to be transposed
/// @param xOut     - Row 0 of the transposed matrix
/// @param yOut     - Row 1 of the transposed matrix
/// @param zOut     - Row 2 of the transposed matrix
/// @param wOut     - Row 3 of the transposed matrix
void __inline__ __attribute__((always_inline)) load_transpose_float4x4_common_AVX2(float8 xyzw01, float8 xyzw23, float4* xOut, float4* yOut, float4* zOut, float4* wOut) {

  float8 xz;
  float8 yw;

  transpose_float4x4_common_AVX2(xyzw01, xyzw23,
            &xz, &yw, true);

  *xOut = xz.s0123;
  *yOut = yw.s0123;
  *zOut = xz.s4567;
  *wOut = yw.s4567;
}

void __inline__ __attribute__((always_inline)) load_transpose_float4x4_AVX2(float4* pLoadAdd, float4* xOut, float4* yOut, float4* zOut, float4* wOut) {

  // We load "float8", meaning we load the full matrix in 2 loads
  float* tmpLoadAdd = (float*)pLoadAdd;
  float8 xyzw01 = vload8(0, tmpLoadAdd);                                    // x0 y0 z0 w0 x1 y1 z1 w1
  float8 xyzw23 = vload8(1, tmpLoadAdd);                                    // x2 y2 z2 w2 x3 y3 z3 w3

  load_transpose_float4x4_common_AVX2(xyzw01, xyzw23,
            xOut, yOut, zOut, wOut);
}


void __inline__ __attribute__((always_inline)) transpose_store_float4x4_AVX2(float4* pStoreAdd, float4 xIn, float4 yIn, float4 zIn, float4 wIn) {

  float8 xy = {xIn, yIn};                                                   // x0 x1 x2 x3 y0 y1 y2 y3
  float8 zw = {zIn, wIn};                                                   // z0 z1 z2 z3 w0 w1 w2 w3

  float8 xyzw01;
  float8 xyzw23;

  transpose_float4x4_common_AVX2(xy, zw,
            &xyzw01, &xyzw23, false);

  // We store "float8", meaning we store the full matrix in 2 stores
  float* tmpStoreAdd = (float*)pStoreAdd;
  vstore8(xyzw01, 0, tmpStoreAdd);
  vstore8(xyzw23, 1, tmpStoreAdd);
}

void __inline__ __attribute__((always_inline)) gather_transpose_float4x4_AVX2(float4* pLoadAdd0, float4* pLoadAdd1, float4* pLoadAdd2, float4* pLoadAdd3,
                                    float4* xOut, float4* yOut, float4* zOut, float4* wOut) {

  float8 xyzw01 = {*pLoadAdd0, *pLoadAdd1};                                // x0 y0 z0 w0 x1 y1 z1 w1
  float8 xyzw23 = {*pLoadAdd2, *pLoadAdd3};                                // x2 y2 z2 w2 x3 y3 z3 w3

  load_transpose_float4x4_common_AVX2(xyzw01, xyzw23,
            xOut, yOut, zOut, wOut);
}

void __inline__ __attribute__((always_inline)) transpose_scatter_float4x4_AVX2(float4* pStoreAdd0, float4* pStoreAdd1, float4* pStoreAdd2, float4* pStoreAdd3,
                               float4 xIn, float4 yIn, float4 zIn, float4 wIn) {
  float8 xy = {xIn, yIn};                                                   // x0 x1 x2 x3 y0 y1 y2 y3
  float8 zw = {zIn, wIn};                                                   // z0 z1 z2 z3 w0 w1 w2 w3

  float8 xyzw01;
  float8 xyzw23;

  transpose_float4x4_common_AVX2(xy, zw,
            &xyzw01, &xyzw23, false);

  *pStoreAdd0 = xyzw01.s0123;
  *pStoreAdd1 = xyzw01.s4567;
  *pStoreAdd2 = xyzw23.s0123;
  *pStoreAdd3 = xyzw23.s4567;
}

#endif // defined(__AVX2__)

void __inline__ __attribute__((always_inline)) load_transpose_float4x4(float4* pLoadAdd, float4* xOut, float4* yOut, float4* zOut, float4* wOut) {
#if defined(__AVX2__)
  load_transpose_float4x4_AVX2(pLoadAdd, xOut, yOut, zOut, wOut);
#else // defined(__SSE4_2__)
  load_transpose_float4x4_AVX(pLoadAdd, xOut, yOut, zOut, wOut);
#endif
}

void __inline__ __attribute__((always_inline)) transpose_store_float4x4(float4* pStoreAdd, float4 xIn, float4 yIn, float4 zIn, float4 wIn) {
#if defined(__AVX2__)
  transpose_store_float4x4_AVX2(pStoreAdd, xIn, yIn, zIn, wIn);
#else // defined(__SSE4_2__)
  transpose_store_float4x4_AVX(pStoreAdd, xIn, yIn, zIn, wIn);
#endif
}

void __inline__ __attribute__((always_inline)) gather_transpose_float4x4(float4* pLoadAdd0, float4* pLoadAdd1, float4* pLoadAdd2, float4* pLoadAdd3,
                               float4* xOut, float4* yOut, float4* zOut, float4* wOut) {
#if defined(__AVX2__)
  gather_transpose_float4x4_AVX2(pLoadAdd0, pLoadAdd1, pLoadAdd2, pLoadAdd3, xOut, yOut, zOut, wOut);
#else // defined(__SSE4_2__)
  gather_transpose_float4x4_AVX(pLoadAdd0, pLoadAdd1, pLoadAdd2, pLoadAdd3, xOut, yOut, zOut, wOut);
#endif
}

void __inline__ __attribute__((always_inline)) transpose_scatter_float4x4(float4* pStoreAdd0, float4* pStoreAdd1, float4* pStoreAdd2, float4* pStoreAdd3,
                               float4 xIn, float4 yIn, float4 zIn, float4 wIn) {
#if defined(__AVX2__)
  transpose_scatter_float4x4_AVX2(pStoreAdd0, pStoreAdd1, pStoreAdd2, pStoreAdd3, xIn, yIn, zIn, wIn);
#else // defined(__SSE4_2__)
  transpose_scatter_float4x4_AVX(pStoreAdd0, pStoreAdd1, pStoreAdd2, pStoreAdd3, xIn, yIn, zIn, wIn);
#endif
}




// ****************************************************************************
//                                 float4x8
// ****************************************************************************

#if defined(__AVX__)

void __inline__ __attribute__((always_inline)) transpose_float4x8(float4 xyzw0, float4 xyzw1, float4 xyzw2, float4 xyzw3, float4 xyzw4, float4 xyzw5, float4 xyzw6, float4 xyzw7,
            float8* xOut, float8* yOut, float8* zOut, float8* wOut) {
  
  // TODO : Replace this shuffle with { xyzw0,  xyzw4} instead of shuffle builtin
  // when clang bug will be fixed
  float8 xyzw04 = __builtin_shufflevector ((__v4sf)xyzw0, (__v4sf)xyzw4,          // x0 y0 z0 w0 x4 y4 z4 w4
            0, 1, 2, 3, 4, 5, 6, 7);
  float8 xyzw15 = __builtin_shufflevector ((__v4sf)xyzw1, (__v4sf)xyzw5,          // x1 y1 z1 w1 x5 y5 z5 w5
            0, 1, 2, 3, 4, 5, 6, 7);
  float8 xyzw26 = __builtin_shufflevector ((__v4sf)xyzw2, (__v4sf)xyzw6,          // x2 y2 z2 w2 x6 y6 z6 w6
            0, 1, 2, 3, 4, 5, 6, 7);
  float8 xyzw37 = __builtin_shufflevector ((__v4sf)xyzw3, (__v4sf)xyzw7,          // x3 y3 z3 w3 x7 y7 z7 w7
            0, 1, 2, 3, 4, 5, 6, 7);

  // TODO : Replace these unpacks with shuffle2 instead of unpack intrinsics
  // when shuffle, shuffle2 with const mask passes will be supported in the BE
  float8 xy0246 = (float8) _mm256_unpacklo_ps((__m256)xyzw04, (__m256)xyzw26);    // x0 x2 y0 y2 x4 x6 y4 y6
  float8 zw0246 = (float8) _mm256_unpackhi_ps((__m256)xyzw04, (__m256)xyzw26);    // z0 z2 w0 w2 z4 z6 w4 w6
  float8 xy1357 = (float8) _mm256_unpacklo_ps((__m256)xyzw15, (__m256)xyzw37);    // x1 x3 y1 y3 x5 x7 y5 y7
  float8 zw1357 = (float8) _mm256_unpackhi_ps((__m256)xyzw15, (__m256)xyzw37);    // z1 z3 w1 w3 z5 z7 w5 w7

  // TODO : Replace these unpacks with shuffle2 instead of unpack intrinsics
  // when shuffle, shuffle2 with const mask passes will be supported in the BE
  *xOut = (float8) _mm256_unpacklo_ps((__m256)xy0246, (__m256)xy1357);            // x0 x1 x2 x3 x4 x5 x6 x7
  *yOut = (float8) _mm256_unpackhi_ps((__m256)xy0246, (__m256)xy1357);            // y0 y1 y2 y3 y4 y5 y6 y7
  *zOut = (float8) _mm256_unpacklo_ps((__m256)zw0246, (__m256)zw1357);            // z0 z1 z2 z3 z4 z5 z6 z7
  *wOut = (float8) _mm256_unpackhi_ps((__m256)zw0246, (__m256)zw1357);            // w0 w1 w2 w3 w4 w5 w6 w7
}

void __inline__ __attribute__((always_inline)) transpose_float8x4(float8 xIn, float8 yIn, float8 zIn, float8 wIn,
            float4* xyzw0, float4* xyzw1, float4* xyzw2, float4* xyzw3, float4* xyzw4, float4* xyzw5, float4* xyzw6, float4* xyzw7) {
  
  // TODO : Replace these unpacks with shuffle2 instead of unpack intrinsics
  // when shuffle, shuffle2 with const mask passes will be supported in the BE
  float8 xy0145 = (float8) _mm256_unpacklo_ps((__m256)xIn, (__m256)yIn);          // x0 y0 x1 y1 x4 y4 x5 y5
  float8 xy2367 = (float8) _mm256_unpackhi_ps((__m256)xIn, (__m256)yIn);          // x2 y2 x3 y3 x6 y6 x7 y7
  float8 zw0145 = (float8) _mm256_unpacklo_ps((__m256)zIn, (__m256)wIn);          // z0 w0 z1 w1 z4 w4 z5 w5
  float8 zw2367 = (float8) _mm256_unpackhi_ps((__m256)zIn, (__m256)wIn);          // z2 w2 z3 w3 z6 w6 z7 w7
  
  
  // TODO : Replace these unpacks with shuffle2 instead of unpack intrinsics
  // when shuffle, shuffle2 with const mask passes will be supported in the BE
  float8 xyzw04 = (float8) _mm256_unpacklo_pd((__m256d)xy0145, (__m256d)zw0145);  // x0 y0 z0 w0 x4 y4 z4 w4
  float8 xyzw15 = (float8) _mm256_unpackhi_pd((__m256d)xy0145, (__m256d)zw0145);  // x1 y1 z1 w1 x5 y5 z5 w5
  float8 xyzw26 = (float8) _mm256_unpacklo_pd((__m256d)xy2367, (__m256d)zw2367);  // x2 y2 z2 w2 x6 y6 z6 w6
  float8 xyzw37 = (float8) _mm256_unpackhi_pd((__m256d)xy2367, (__m256d)zw2367);  // x3 y3 z3 w3 x7 y7 z7 w7
  
  *xyzw0 = xyzw04.s0123;
  *xyzw1 = xyzw15.s0123;
  *xyzw2 = xyzw26.s0123;
  *xyzw3 = xyzw37.s0123;
  *xyzw4 = xyzw04.s4567;
  *xyzw5 = xyzw15.s4567;
  *xyzw6 = xyzw26.s4567;
  *xyzw7 = xyzw37.s4567;
}


void __inline__ __attribute__((always_inline)) load_transpose_float4x8(float4* pLoadAdd, float8* xOut, float8* yOut, float8* zOut, float8* wOut) {
  
  transpose_float4x8(pLoadAdd[0], pLoadAdd[1], pLoadAdd[2], pLoadAdd[3], pLoadAdd[4], pLoadAdd[5], pLoadAdd[6], pLoadAdd[7],
                xOut, yOut, zOut, wOut);
}

void __inline__ __attribute__((always_inline)) transpose_store_float4x8(float4* pStoreAdd, float8 xIn, float8 yIn, float8 zIn, float8 wIn) {
  
  transpose_float8x4(xIn, yIn, zIn, wIn,
            &(pStoreAdd[0]), &(pStoreAdd[1]), &(pStoreAdd[2]), &(pStoreAdd[3]), &(pStoreAdd[4]), &(pStoreAdd[5]), &(pStoreAdd[6]), &(pStoreAdd[7]));
}

void __inline__ __attribute__((always_inline)) masked_load_transpose_float4x8(float4* pLoadAdd, float8* xOut, float8* yOut, float8* zOut, float8* wOut, int8 mask) {
  float4 xyzw[8] = {0};
  masked_load_float4x8(pLoadAdd, xyzw, mask);
  load_transpose_float4x8(xyzw, xOut, yOut, zOut, wOut);
}

void __inline__ __attribute__((always_inline)) masked_transpose_store_float4x8(float4* pStoreAdd, float8 xIn, float8 yIn, float8 zIn, float8 wIn, int8 mask) {
  float4 xyzw[8];
  transpose_store_float4x8(xyzw, xIn, yIn, zIn, wIn);
  masked_store_float4x8(pStoreAdd, xyzw, mask);
}

void __inline__ __attribute__((always_inline)) gather_transpose_float4x8(float4* pLoadAdd0, float4* pLoadAdd1, float4* pLoadAdd2, float4* pLoadAdd3,
                               float4* pLoadAdd4, float4* pLoadAdd5, float4* pLoadAdd6, float4* pLoadAdd7,
                               float8* xOut, float8* yOut, float8* zOut, float8* wOut) {
  transpose_float4x8(*pLoadAdd0, *pLoadAdd1, *pLoadAdd2, *pLoadAdd3, *pLoadAdd4, *pLoadAdd5, *pLoadAdd6, *pLoadAdd7,
                xOut, yOut, zOut, wOut);
}

void __inline__ __attribute__((always_inline)) transpose_scatter_float4x8(float4* pStoreAdd0, float4* pStoreAdd1, float4* pStoreAdd2, float4* pStoreAdd3,
                                float4* pStoreAdd4, float4* pStoreAdd5, float4* pStoreAdd6, float4* pStoreAdd7,
                                float8 xIn, float8 yIn, float8 zIn, float8 wIn) {
  transpose_float8x4(xIn, yIn, zIn, wIn,
            pStoreAdd0, pStoreAdd1, pStoreAdd2, pStoreAdd3, pStoreAdd4, pStoreAdd5, pStoreAdd6, pStoreAdd7);
}

void __inline__ __attribute__((always_inline)) masked_gather_transpose_float4x8(float4* pLoadAdd0, float4* pLoadAdd1, float4* pLoadAdd2, float4* pLoadAdd3,
                               float4* pLoadAdd4, float4* pLoadAdd5, float4* pLoadAdd6, float4* pLoadAdd7,
                               float8* xOut, float8* yOut, float8* zOut, float8* wOut, int8 mask){  
  if (all(mask)) {
    gather_transpose_float4x8( pLoadAdd0, pLoadAdd1, pLoadAdd2, pLoadAdd3,
                            pLoadAdd4, pLoadAdd5, pLoadAdd6, pLoadAdd7,
                            xOut, yOut, zOut, wOut);
    return;
  }

  float4 dummy = 0;

  float4* xyzw0 = mask.s0 ? pLoadAdd0 : &dummy;
  float4* xyzw1 = mask.s1 ? pLoadAdd1 : &dummy;
  float4* xyzw2 = mask.s2 ? pLoadAdd2 : &dummy;
  float4* xyzw3 = mask.s3 ? pLoadAdd3 : &dummy;
  float4* xyzw4 = mask.s4 ? pLoadAdd4 : &dummy;
  float4* xyzw5 = mask.s5 ? pLoadAdd5 : &dummy;
  float4* xyzw6 = mask.s6 ? pLoadAdd6 : &dummy;
  float4* xyzw7 = mask.s7 ? pLoadAdd7 : &dummy;

	gather_transpose_float4x8( xyzw0, xyzw1, xyzw2, xyzw3,
                            xyzw4, xyzw5, xyzw6, xyzw7,
                            xOut, yOut, zOut, wOut);
}

void __inline__ __attribute__((always_inline)) masked_transpose_scatter_float4x8(float4* pStoreAdd0, float4* pStoreAdd1, float4* pStoreAdd2, float4* pStoreAdd3,
                              float4* pStoreAdd4, float4* pStoreAdd5, float4* pStoreAdd6, float4* pStoreAdd7,
                              float8 xIn, float8 yIn, float8 zIn, float8 wIn, int8 mask){
  if (all(mask)) {
    transpose_scatter_float4x8(pStoreAdd0, pStoreAdd1, pStoreAdd2, pStoreAdd3,
                            pStoreAdd4, pStoreAdd5, pStoreAdd6, pStoreAdd7,
                            xIn, yIn, zIn, wIn);
    return;
  }

  float4 dummy;
  
  float4* xyzw0 = mask.s0 ? pStoreAdd0 : &dummy;
  float4* xyzw1 = mask.s1 ? pStoreAdd1 : &dummy;
  float4* xyzw2 = mask.s2 ? pStoreAdd2 : &dummy;
  float4* xyzw3 = mask.s3 ? pStoreAdd3 : &dummy;
  float4* xyzw4 = mask.s4 ? pStoreAdd4 : &dummy;
  float4* xyzw5 = mask.s5 ? pStoreAdd5 : &dummy;
  float4* xyzw6 = mask.s6 ? pStoreAdd6 : &dummy;
  float4* xyzw7 = mask.s7 ? pStoreAdd7 : &dummy;

	transpose_scatter_float4x8(xyzw0, xyzw1, xyzw2, xyzw3,
                            xyzw4, xyzw5, xyzw6, xyzw7,
                            xIn, yIn, zIn, wIn);
}
#endif // defined((__AVX__)

#endif // defined((__SSE4_2__)
