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
#include <intrin.h>




// ****************************************************************************
//                                 char4x4
// ****************************************************************************


#if defined(__AVX__)


void load_transpose_char4x4_common(char16 xyzwIn, char4* xOut, char4* yOut, char4* zOut, char4* wOut) {
  
  char16 xyzw = xyzwIn;                                                     // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3
  *xOut = xyzw.s048C;                                                       // x0  D  D  D x1  D  D  D x2  D  D  D x3  D  D  D 
  
  xyzw = (char16) _mm_srli_si128((__m128i)xyzw, 1);                         // y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3  D
  *yOut = xyzw.s048C;                                                       // y0  D  D  D y1  D  D  D y2  D  D  D y3  D  D  D 
  
  xyzw = (char16) _mm_srli_si128((__m128i)xyzw, 1);                         // z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3  D  D
  *zOut = xyzw.s048C;                                                       // z0  D  D  D z1  D  D  D z2  D  D  D z3  D  D  D
  
  xyzw = (char16) _mm_srli_si128((__m128i)xyzw, 1);                         // w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3  D  D  D
  *wOut = xyzw.s048C;                                                       // w0  D  D  D w1  D  D  D w2  D  D  D w3  D  D  D
}

void load_transpose_char4x4(char4* pLoadAdd, char4* xOut, char4* yOut, char4* zOut, char4* wOut) {
  // We load "char16", meaning we load the full matrix in a single load
  char* tmpLoadAdd = (char*)pLoadAdd;
  char16 xyzw = vload16(0, tmpLoadAdd); // unaligned load

  load_transpose_char4x4_common(xyzw, xOut, yOut, zOut, wOut);
}

void transpose_store_char4x4_common(char16* xyzw, char4 xIn, char4 yIn, char4 zIn, char4 wIn) {

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

void transpose_store_char4x4(char4* pStoreAdd, char4 xIn, char4 yIn, char4 zIn, char4 wIn) {

  char16 xyzw;
  transpose_store_char4x4_common(&xyzw, xIn, yIn, zIn, wIn);

  // We store "char16", meaning we store the full matrix in a single store
  char* tmpStoreAdd = (char*)pStoreAdd;
  vstore16(xyzw, 0, tmpStoreAdd);  // unaligned store
}

#endif // defined(__AVX__)


// ****************************************************************************
//                                 char4x8
// ****************************************************************************

#if defined(__AVX__)

void load_transpose_char4x8_common_AVX(char16 xyzw0In, char16 xyzw1In, char8* xOut, char8* yOut, char8* zOut, char8* wOut) {
  
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

void load_transpose_char4x8_AVX(char4* pLoadAdd, char8* xOut, char8* yOut, char8* zOut, char8* wOut) {
  // We load "char16", meaning we load the full matrix in a 2 loads
  char* tmpLoadAdd = (char*)pLoadAdd;
  char16 xyzw0 = vload16(0, tmpLoadAdd);                                  // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3 
  char16 xyzw1 = vload16(1, tmpLoadAdd);                                  // x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6 x7 y7 z7 w7
  
  load_transpose_char4x8_common_AVX(xyzw0, xyzw1, xOut, yOut, zOut, wOut);
}

void transpose_store_char4x8_common_AVX(char16* xyzw0, char16* xyzw1, char8 xIn, char8 yIn, char8 zIn, char8 wIn) {
  
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

void transpose_store_char4x8_AVX(char4* pStoreAdd, char8 xIn, char8 yIn, char8 zIn, char8 wIn) {
  
  char16 xyzw0;
  char16 xyzw1;
  transpose_store_char4x8_common_AVX(&xyzw0, &xyzw1, xIn, yIn, zIn, wIn);

  // We store "char16", meaning we store the full matrix in a 2 stores
  char* tmpStoreAdd = (char*)pStoreAdd;
  vstore16(xyzw0, 0, tmpStoreAdd);
  vstore16(xyzw1, 1, tmpStoreAdd);
}

#endif // defined(__AVX__)


#if defined(__AVX2__)

typedef __v32qi char32;

void load_transpose_char4x8_common_AVX2(char32 xyzwIn, char8* xOut, char8* yOut, char8* zOut, char8* wOut) {


  // TODO : Replace this shuffles with shuffle instead of shuffle builtin
  // when shuffle, shuffle2 with const mask passes will be supported in the BE
  char32 xyzw = xyzwIn;                                                     //x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3 | x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6 x7 y7 z7 w7
  char32 dummy;
  xyzw =  __builtin_shufflevector (xyzw, dummy,                             // x0 x1 x2 x3 y0 y1 y2 y3 z0 z1 z2 z3 w0 w1 w2 w3 | x4 x5 x6 x7 y4 y5 y6 y7 z4 z5 z6 z7 w4 w5 w6 w7
          0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15,
          16, 20, 24, 28, 17, 21, 25, 29, 18, 22, 26, 30, 19, 23, 27, 31);
  xyzw = (char32)(((int8)xyzw).s04152637);                                  // x0 x1 x2 x3 x4 x5 x6 x7 y0 y1 y2 y3 y4 y5 y6 y7 | z0 z1 z2 z3 z4 z5 z6 z7 w0 w1 w2 w3 w4 w5 w6 w7

  // TODO : Replace these unpacks with shuffle instead of unpack intrinsics
  // when shuffle, shuffle2 with const mask passes will be supported in the BE
  char32 xz = (char32) _mm256_unpacklo_epi8((__m256i)xyzw, (__m256i)dummy); // x0  D x1  D x2  D x3  D x4  D x5  D x6  D x7  D | z0  D z1  D z2  D z3  D z4  D z5  D z6  D z7  D
  char32 yw = (char32) _mm256_unpackhi_epi8((__m256i)xyzw, (__m256i)dummy); // y0  D y1  D y2  D y3  D y4  D y5  D y6  D y7  D | w0  D w1  D w2  D w3  D w4  D w5  D w6  D w7  D
  
  // TODO : Replace these extracts with shuffle instead of extract intrinsics
  // when shuffle, shuffle2 with const mask passes will be supported in the BE
  /*
  char16 x = shuffle (xz, dummy,                                            // x0  D x1  D x2  D x3  D x4  D x5  D x6  D x7  D
          {0, 1, 2, 3, 4, 5, 6, 7, 8, 9 , 10, 11, 12, 13, 14, 15});
  char16 y = shuffle (yw, dummy,                                            // y0  D y1  D y2  D y3  D y4  D y5  D y6  D y7  D
          {0, 1, 2, 3, 4, 5, 6, 7, 8, 9 , 10, 11, 12, 13, 14, 15});
  char16 z = shuffle (xz, dummy,                                            // z0  D z1  D z2  D z3  D z4  D z5  D z6  D z7  D
          {16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,  31});
  char16 w = shuffle (yw, dummy,                                            // w0  D w1  D w2  D w3  D w4  D w5  D w6  D w7  D
          {16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,  31});
  */
  // TODO : using builtin shuffles does not create optimal code
  char16 x = (char16) _mm256_extracti128_si256((__m256i)xz, 0);             // x0  D x1  D x2  D x3  D x4  D x5  D x6  D x7  D
  char16 y = (char16) _mm256_extracti128_si256((__m256i)yw, 0);             // y0  D y1  D y2  D y3  D y4  D y5  D y6  D y7  D
  char16 z = (char16) _mm256_extracti128_si256((__m256i)xz, 1);             // z0  D z1  D z2  D z3  D z4  D z5  D z6  D z7  D
  char16 w = (char16) _mm256_extracti128_si256((__m256i)yw, 1);             // w0  D w1  D w2  D w3  D w4  D w5  D w6  D w7  D


  *xOut = x.s02468ACE;
  *yOut = y.s02468ACE;
  *zOut = z.s02468ACE;
  *wOut = w.s02468ACE;
}

void load_transpose_char4x8_AVX2(char4* pLoadAdd, char8* xOut, char8* yOut, char8* zOut, char8* wOut) {

    // We load "int8", meaning we load the full matrix in a single load
  int* tmpLoadAdd = (int*)pLoadAdd;
    char32 xyzw = (char32) vload8(0, tmpLoadAdd);                           // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3 | x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6 x7 y7 z7 w7

  load_transpose_char4x8_common_AVX2(xyzw, xOut, yOut, zOut, wOut);
}

void transpose_store_char4x8_common_AVX2(char32* xyzwOut, char8 xIn, char8 yIn, char8 zIn, char8 wIn) {
  
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

  // TODO : Replace these inserts with shuffle2 instead of insert intrinsics
  // when shuffle, shuffle2 with const mask passes will be supported in the BE
  /*
  xz = shuffle2 (x, z, 
          {0, 1, 2, 3, 4, 5, 6, 7, 8, 9 , 10, 11, 12, 13, 14, 15,
          16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31});
  yw = shuffle2 (y, w, 
          {0, 1, 2, 3, 4, 5, 6, 7, 8, 9 , 10, 11, 12, 13, 14, 15
          16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31});
  */  
  xz = (char32) _mm256_inserti128_si256((__m256i)xz, (__m128i)x, 0);        // x0  D x1  D x2  D x3  D x4  D x5  D x6  D x7  D |  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D
  yw = (char32) _mm256_inserti128_si256((__m256i)yw, (__m128i)y, 0);        // y0  D y1  D y2  D y3  D y4  D y5  D y6  D y7  D |  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D
  xz = (char32) _mm256_inserti128_si256((__m256i)xz, (__m128i)z, 1);        // x0  D x1  D x2  D x3  D x4  D x5  D x6  D x7  D | z0  D z1  D z2  D z3  D z4  D z5  D z6  D z7  D
  yw = (char32) _mm256_inserti128_si256((__m256i)yw, (__m128i)w, 1);        // y0  D y1  D y2  D y3  D y4  D y5  D y6  D y7  D | w0  D w1  D w2  D w3  D w4  D w5  D w6  D w7  D

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

void transpose_store_char4x8_AVX2(char4* pStoreAdd, char8 xIn, char8 yIn, char8 zIn, char8 wIn) {
  
  char32 xyzw;
  transpose_store_char4x8_common_AVX2(&xyzw, xIn, yIn, zIn, wIn);

  // We store "int8", meaning we store the full matrix in a single store
  int* tmpStoreAdd = (int*)pStoreAdd;
  vstore8((int8)xyzw, 0, tmpStoreAdd);
}

#endif // defined(__AVX2__)

#if defined(__AVX__)

void load_transpose_char4x8(char4* pLoadAdd, char8* xOut, char8* yOut, char8* zOut, char8* wOut) {
#if defined(__AVX2__)
  load_transpose_char4x8_AVX2(pLoadAdd, xOut, yOut, zOut, wOut);
#else // defined(__AVX__)
  load_transpose_char4x8_AVX(pLoadAdd, xOut, yOut, zOut, wOut);
#endif
}

void transpose_store_char4x8(char4* pStoreAdd, char8 xIn, char8 yIn, char8 zIn, char8 wIn) {
#if defined(__AVX2__)
  transpose_store_char4x8_AVX2(pStoreAdd, xIn, yIn, zIn, wIn);
#else // defined(__AVX__)
  transpose_store_char4x8_AVX(pStoreAdd, xIn, yIn, zIn, wIn);
#endif
}

#endif // defined(__AVX__)

// ****************************************************************************
//                                 int4x4
// ****************************************************************************

#if defined(__AVX__)

void transpose_int4x4_AVX(int4 xyzw0, int4 xyzw1, int4 xyzw2, int4 xyzw3,
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

void load_transpose_int4x4_AVX(int4* pLoadAdd, int4* xOut, int4* yOut, int4* zOut, int4* wOut) {

  // pLoadAdd[0]  = x0 y0 z0 w0 
  // pLoadAdd[1]  = x1 y1 z1 w1 
  // pLoadAdd[2]  = x2 y2 z2 w2 
  // pLoadAdd[3]  = x3 y3 z3 w3 

  transpose_int4x4_AVX(pLoadAdd[0], pLoadAdd[1], pLoadAdd[2], pLoadAdd[3],
            xOut, yOut, zOut, wOut);

  // xOut      = x0 x1 x2 x3
  // yOut      = y0 y1 y2 y3
  // zOut      = z0 z1 z2 z3
  // wOut      = w0 w1 w2 w3
}

void transpose_store_int4x4_AVX(int4* pStoreAdd, int4 xIn, int4 yIn, int4 zIn, int4 wIn) {

  // xIn      = x0 x1 x2 x3
  // yIn      = y0 y1 y2 y3
  // zIn      = z0 z1 z2 z3
  // wIn      = w0 w1 w2 w3

  transpose_int4x4_AVX(xIn, yIn, zIn, wIn,
            &(pStoreAdd[0]), &(pStoreAdd[1]), &(pStoreAdd[2]), &(pStoreAdd[3]));
  
  // pStoreAdd[0] = x0 y0 z0 w0 
  // pStoreAdd[1] = x1 y1 z1 w1 
  // pStoreAdd[2] = x2 y2 z2 w2 
  // pStoreAdd[3] = x3 y3 z3 w3 
}

#endif // defined(__AVX__)


#if defined(__AVX2__)

void transpose_int4x4_common_AVX2(int8 xyzw01, int8 xyzw23,
            int8* xzOut, int8* ywOut) {
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

  xyzw01 = xyzw01.s04152637;                                                    // x0 x1 y0 y1 z0 z1 w0 w1
  xyzw23 = xyzw23.s15043726;                                                    // y2 y3 x2 x3 w2 w3 z2 z3

  // TODO : Replace this blend built-in with ?: when clang bug will be fixed
  //int8 cond = {0, 0, 0xFFFFFFFF, 0xFFFFFFFF, 0, 0, 0xFFFFFFFF, 0xFFFFFFFF};
  //int8 xz = cond ? xyzw01 : xyzw23;               // x0 x1 x2 x3 z0 z1 z2 z3
  *xzOut = (int8) _mm256_blend_epi32((__m256i)xyzw01, (__m256i)xyzw23, 0xCC);   // x0 x1 x2 x3 z0 z1 z2 z3
  
  // TODO : Replace this shuffles with shuffle2 instead of shuffle builtin
  // when shuffle, shuffle2 with const mask passes will be supported in the BE
  *ywOut = __builtin_shufflevector (xyzw01, xyzw23,                             // y0 y1 y2 y3 w0 w1 w2 w3
          2, 3, 8, 9, 6, 7, 12, 13);
}

void load_transpose_int4x4_AVX2(int4* pLoadAdd, int4* xOut, int4* yOut, int4* zOut, int4* wOut) {

  // We load "int8", meaning we load the full matrix in 2 loads
  int* tmpLoadAdd = (int*)pLoadAdd;
  int8 xyzw01 = vload8(0, tmpLoadAdd);                                      // x0 y0 z0 w0 x1 y1 z1 w1
  int8 xyzw23 = vload8(1, tmpLoadAdd);                                      // x2 y2 z2 w2 x3 y3 z3 w3

  int8 xz;
  int8 yw;

  transpose_int4x4_common_AVX2(xyzw01, xyzw23,                              // x0 x1 x2 x3 z0 z1 z2 z3  =  xz
                &xz, &yw);                                                  // y0 y1 y2 y3 w0 w1 w2 w3  =  yw

  *xOut = xz.s0123;                                                         // x0 x1 x2 x3
  *yOut = yw.s0123;                                                         // y0 y1 y2 y3
  *zOut = xz.s4567;                                                         // z0 z1 z2 z3
  *wOut = yw.s4567;                                                         // w0 w1 w2 w3
}

void transpose_store_int4x4_AVX2(int4* pStoreAdd, int4 xIn, int4 yIn, int4 zIn, int4 wIn) {

  int8 xy = {xIn, yIn};                                                     // x0 x1 x2 x3 y0 y1 y2 y3
  int8 zw = {zIn, wIn};                                                     // z0 z1 z2 z3 w0 w1 w2 w3

  xy = xy.s04261537;                                                        // x0 y0 x2 y2 x1 y1 x2 y3
  zw = zw.s26043715;                                                        // z2 w2 z0 w0 z3 w3 z1 w1

  // TODO : Replace this blend built-in with ?: when clang bug will be fixed
  //int8 cond = {0, 0, 0xFFFFFFFF, 0xFFFFFFFF, 0, 0, 0xFFFFFFFF, 0xFFFFFFFF};
  //int8 xyzw01 = cond ? xy : zw;                     // x0 y0 z0 w0 x1 y1 z1 w1
  int8 xyzw01 = (int8) _mm256_blend_epi32((__m256i)xy, (__m256i)zw, 0xCC);  // x0 y0 z0 w0 x1 y1 z1 w1

  // TODO : Replace this shuffle with shuffle2 instead of shuffle builtin
  // when shuffle, shuffle2 with const mask passes will be supported in the BE
  int8 xyzw23 = __builtin_shufflevector (xy, zw,                            // x2 y2 z2 w2 x3 y3 z3 w3
          2, 3, 8, 9, 6, 7, 12, 13);

  // We store "int8", meaning we store the full matrix in 2 stores
  int* tmpStoreAdd = (int*)pStoreAdd;
  vstore8(xyzw01, 0, tmpStoreAdd);
  vstore8(xyzw23, 1, tmpStoreAdd);
}

#endif // defined(__AVX2__)

#if defined(__AVX__)

void load_transpose_int4x4(int4* pLoadAdd, int4* xOut, int4* yOut, int4* zOut, int4* wOut) {
#if defined(__AVX2__)
  load_transpose_int4x4_AVX2(pLoadAdd, xOut, yOut, zOut, wOut);
#else // defined(__AVX__)
  load_transpose_int4x4_AVX(pLoadAdd, xOut, yOut, zOut, wOut);
#endif
}

void transpose_store_int4x4(int4* pStoreAdd, int4 xIn, int4 yIn, int4 zIn, int4 wIn)  {
#if defined(__AVX2__)
  transpose_store_int4x4_AVX2(pStoreAdd, xIn, yIn, zIn, wIn);
#else // defined(__AVX__)
  transpose_store_int4x4_AVX(pStoreAdd, xIn, yIn, zIn, wIn);
#endif
}

#endif // defined(__AVX__)


// ****************************************************************************
//                                 int4x8
// ****************************************************************************

#if defined(__AVX__)

void load_transpose_int4x8_AVX(int4* pLoadAdd, int8* xOut, int8* yOut, int8* zOut, int8* wOut) {

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

void transpose_store_int4x8_AVX(int4* pStoreAdd, int8 xIn, int8 yIn, int8 zIn, int8 wIn) {
  
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

#endif // defined(__AVX__)

#if defined(__AVX2__)

void transpose_int4x8_common_AVX2(int8 xyzw04, int8 xyzw15, int8 xyzw26, int8 xyzw37,
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

void load_transpose_int4x8_AVX2(int4* pLoadAdd, int8* xOut, int8* yOut, int8* zOut, int8* wOut) {
  // TODO: creates
  // vmovaps  48(%eax), %xmm0
  // vmovaps  112(%eax), %xmm1
  // vinsertf128  $1, %xmm1, %ymm0, %ymm0
  // instead of vpinsert from memory and not loading into xmm1

  // TODO : Replace this shuffle with { pLoadAdd[0],  pLoadAdd[4]} instead of shuffle builtin
  // when clang bug will be fixed
  int8 xyzw04 = __builtin_shufflevector (pLoadAdd[0], pLoadAdd[4],          // x0 y0 z0 w0 x4 y4 z4 w4
            0, 1, 2, 3, 4, 5, 6, 7);
  int8 xyzw15 = __builtin_shufflevector (pLoadAdd[1], pLoadAdd[5],          // x1 y1 z1 w1 x5 y5 z5 w5
            0, 1, 2, 3, 4, 5, 6, 7);
  int8 xyzw26 = __builtin_shufflevector (pLoadAdd[2], pLoadAdd[6],          // x2 y2 z2 w2 x6 y6 z6 w6
            0, 1, 2, 3, 4, 5, 6, 7);
  int8 xyzw37 = __builtin_shufflevector (pLoadAdd[3], pLoadAdd[7],          // x3 y3 z3 w3 x7 y7 z7 w7
            0, 1, 2, 3, 4, 5, 6, 7);

  transpose_int4x8_common_AVX2(xyzw04, xyzw15, xyzw26, xyzw37,
            xOut, yOut, zOut, wOut);
                                                                            // xOut = x0 x1 x2 x3 x4 x5 x6 x7
                                                                            // yOut = y0 y1 y2 y3 y4 y5 y6 y7
                                                                            // zOut = z0 z1 z2 z3 z4 z5 z6 z7
                                                                            // wOut = w0 w1 w2 w3 w4 w5 w6 w7
}

void transpose_store_int4x8_AVX2(int4* pStoreAdd, int8 xIn, int8 yIn, int8 zIn, int8 wIn) {
  
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
  
  pStoreAdd[0] = xyzw04.s0123;                                              // x0 y0 z0 w0
  pStoreAdd[1] = xyzw15.s0123;                                              // x1 y1 z1 w1
  pStoreAdd[2] = xyzw26.s0123;                                              // x2 y2 z2 w2
  pStoreAdd[3] = xyzw37.s0123;                                              // x3 y3 z3 w3 
  pStoreAdd[4] = xyzw04.s4567;                                              // x4 y4 z4 w4
  pStoreAdd[5] = xyzw15.s4567;                                              // x5 y5 z5 w5
  pStoreAdd[6] = xyzw26.s4567;                                              // x6 y6 z6 w6
  pStoreAdd[7] = xyzw37.s4567;                                              // x7 y7 z7 w7
}

#endif // defined(__AVX2__)


#if defined(__AVX__)

void load_transpose_int4x8(int4* pLoadAdd, int8* xOut, int8* yOut, int8* zOut, int8* wOut) {
#if defined(__AVX2__)
  load_transpose_int4x8_AVX2(pLoadAdd, xOut, yOut, zOut, wOut);
#else // defined(__AVX__)
  load_transpose_int4x8_AVX(pLoadAdd, xOut, yOut, zOut, wOut);
#endif
}

void transpose_store_int4x8(int4* pStoreAdd, int8 xIn, int8 yIn, int8 zIn, int8 wIn) {
#if defined(__AVX2__)
  transpose_store_int4x8_AVX2(pStoreAdd, xIn, yIn, zIn, wIn);
#else // defined(__AVX__)
  transpose_store_int4x8_AVX(pStoreAdd, xIn, yIn, zIn, wIn);
#endif
}

#endif // defined(__AVX__)


// ****************************************************************************
//                                 float4x4
// ****************************************************************************

#if defined(__AVX__)

void transpose_float4x4_AVX(float4 in0, float4 in1, float4 in2, float4 in3,
            float4* out0, float4* out1, float4* out2, float4* out3) {
  transpose_int4x4_AVX((int4)in0, (int4)in1, (int4)in2, (int4)in3,
            (int4*)out0, (int4*)out1, (int4*)out2, (int4*)out3);
}

void load_transpose_float4x4_AVX(float4* pLoadAdd, float4* xOut, float4* yOut, float4* zOut, float4* wOut) {
  load_transpose_int4x4_AVX((int4*)pLoadAdd, (int4*)xOut, (int4*)yOut, (int4*)zOut, (int4*)wOut);
}

void transpose_store_float4x4_AVX(float4* pStoreAdd, float4 xIn, float4 yIn, float4 zIn, float4 wIn) {
  transpose_store_int4x4_AVX((int4*)pStoreAdd, (int4)xIn, (int4)yIn, (int4)zIn, (int4)wIn);
}

#endif // defined(__AVX__)


#if defined(__AVX2__)
void transpose_float4x4_common_AVX2(float8 xyzw01, float8 xyzw23,
            float4* xOut, float4* yOut, float4* zOut, float4* wOut) {

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

  xyzw01 = xyzw01.s04152637;                                                    // x0 x1 y0 y1 z0 z1 w0 w1
  xyzw23 = xyzw23.s15043726;                                                    // y2 y3 x2 x3 w2 w3 z2 z3

  // TODO : Replace this blend built-in with ?: when clang bug will be fixed
  //int8 cond = {0, 0, 0xFFFFFFFF, 0xFFFFFFFF, 0, 0, 0xFFFFFFFF, 0xFFFFFFFF};
  //int8 xz = cond ? xyzw01 : xyzw23;                                           // x0 x1 x2 x3 z0 z1 z2 z3
  float8 xz = (float8) _mm256_blend_ps((__m256)xyzw01, (__m256)xyzw23, 0xCC);   // x0 x1 x2 x3 z0 z1 z2 z3
  
  // TODO : Replace this shuffles with shuffle2 instead of shuffle builtin
  // when shuffle, shuffle2 with const mask passes will be supported in the BE
  float8 yw = __builtin_shufflevector ((__v8sf)xyzw01, (__v8sf)xyzw23,          // y0 y1 y2 y3 w0 w1 w2 w3
          2, 3, 8, 9, 6, 7, 12, 13);

  *xOut = xz.s0123;
  *yOut = yw.s0123;
  *zOut = xz.s4567;
  *wOut = yw.s4567;
  
}

void load_transpose_float4x4_AVX2(float4* pLoadAdd, float4* xOut, float4* yOut, float4* zOut, float4* wOut) {

  // We load "float8", meaning we load the full matrix in 2 loads
  float* tmpLoadAdd = (float*)pLoadAdd;
  float8 xyzw01 = vload8(0, tmpLoadAdd);                                    // x0 y0 z0 w0 x1 y1 z1 w1
  float8 xyzw23 = vload8(1, tmpLoadAdd);                                    // x2 y2 z2 w2 x3 y3 z3 w3

  transpose_float4x4_common_AVX2(xyzw01, xyzw23,
            xOut, yOut, zOut, wOut);
}

void transpose_store_float4x4_AVX2(float4* pStoreAdd, float4 xIn, float4 yIn, float4 zIn, float4 wIn) {

  float8 xy = {xIn, yIn};                                                   // x0 x1 x2 x3 y0 y1 y2 y3
  float8 zw = {zIn, wIn};                                                   // z0 z1 z2 z3 w0 w1 w2 w3

  xy = xy.s04261537;                                                        // x0 y0 x2 y2 x1 y1 x2 y3
  zw = zw.s26043715;                                                        // z2 w2 z0 w0 z3 w3 z1 w1

  // TODO : Replace this blend built-in with ?: when clang bug will be fixed
  //int8 cond = {0, 0, 0xFFFFFFFF, 0xFFFFFFFF, 0, 0, 0xFFFFFFFF, 0xFFFFFFFF};
  //int8 xyzw01 = cond ? xy : zw;                                           // x0 y0 z0 w0 x1 y1 z1 w1
  float8 xyzw01 = (float8) _mm256_blend_ps((__m256)xy, (__m256)zw, 0xCC);   // x0 y0 z0 w0 x1 y1 z1 w1

  // TODO : Replace this shuffle with shuffle2 instead of shuffle builtin
  // when shuffle, shuffle2 with const mask passes will be supported in the BE
  float8 xyzw23 = __builtin_shufflevector ((__v8sf)xy, (__v8sf)zw,          // x2 y2 z2 w2 x3 y3 z3 w3
          2, 3, 8, 9, 6, 7, 12, 13);

  // We store "float8", meaning we store the full matrix in 2 stores
  float* tmpStoreAdd = (float*)pStoreAdd;
  vstore8(xyzw01, 0, tmpStoreAdd);
  vstore8(xyzw23, 1, tmpStoreAdd);
}

#endif // defined(__AVX2__)

#if defined(__AVX__)

void load_transpose_float4x4(float4* pLoadAdd, float4* xOut, float4* yOut, float4* zOut, float4* wOut) {
#if defined(__AVX2__)
  load_transpose_float4x4_AVX2(pLoadAdd, xOut, yOut, zOut, wOut);
#else // defined(__AVX__)
  load_transpose_float4x4_AVX(pLoadAdd, xOut, yOut, zOut, wOut);
#endif
}

void transpose_store_float4x4(float4* pStoreAdd, float4 xIn, float4 yIn, float4 zIn, float4 wIn) {
#if defined(__AVX2__)
  transpose_store_float4x4_AVX2(pStoreAdd, xIn, yIn, zIn, wIn);
#else // defined(__AVX__)
  transpose_store_float4x4_AVX(pStoreAdd, xIn, yIn, zIn, wIn);
#endif
}

#endif // defined(__AVX__)


// ****************************************************************************
//                                 float4x8
// ****************************************************************************

#if defined(__AVX__)

void transpose_float4x8(float4 xyzw0, float4 xyzw1, float4 xyzw2, float4 xyzw3, float4 xyzw4, float4 xyzw5, float4 xyzw6, float4 xyzw7,
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

void transpose_float8x4(float8 xIn, float8 yIn, float8 zIn, float8 wIn,
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



void load_transpose_float4x8(float4* pLoadAdd, float8* xOut, float8* yOut, float8* zOut, float8* wOut) {
  
  transpose_float4x8(pLoadAdd[0], pLoadAdd[1], pLoadAdd[2], pLoadAdd[3], pLoadAdd[4], pLoadAdd[5], pLoadAdd[6], pLoadAdd[7],
                xOut, yOut, zOut, wOut);
}

void transpose_store_float4x8(float4* pStoreAdd, float8 xIn, float8 yIn, float8 zIn, float8 wIn) {
  
  transpose_float8x4(xIn, yIn, zIn, wIn,
            &(pStoreAdd[0]), &(pStoreAdd[1]), &(pStoreAdd[2]), &(pStoreAdd[3]), &(pStoreAdd[4]), &(pStoreAdd[5]), &(pStoreAdd[6]), &(pStoreAdd[7]));
}

#endif // defined(__AVX__)
