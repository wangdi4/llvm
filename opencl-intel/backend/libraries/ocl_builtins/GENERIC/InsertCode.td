// ****************************************************************************
//                           code_to_inject.td
//     can be used to avoid code duplication without declaring a new functions
//     def name: insert_code<string>;
//     then #{name}# can be used to inject the string
// ****************************************************************************

class insert_code<string v>{
string value=v;
}

// ****************************************************************************
//                                 masked load store
// ****************************************************************************

def __ocl_masked_load_float_4x4_common: insert_code<"\n"#
"    float8 extendedMask = as_float8($Arg2VarName.s01230123);\n"#
"    float8 maskLow = extendedMask.s00001111;\n"#
"    float8 maskHigh = extendedMask.s22223333;\n"#
"    float8* tmpLoadAdd = (float8*)$Arg0VarName;\n"#
"    float8* tmpLoadedValues = (float8*)$Arg1VarName;\n"#
"    tmpLoadedValues[0] = as_float8(_mm256_maskload_ps((float*)&(tmpLoadAdd[0]), (__m256)maskLow));\n"#
"    tmpLoadedValues[1] = as_float8(_mm256_maskload_ps((float*)&(tmpLoadAdd[1]), (__m256)maskHigh));\n">;

// ****************************************************************************
//                                 transpose functions
// ****************************************************************************

/// @brief Receives char4x8 matrix as 4 matrix rows, transposes it and outputs the matrix
///        2 halfs (2 char4x4 matrixes) using 2 char16, which create one char4x8 matrix
/// @param Arg0  - char16* xyzw0  - Upper part of the transposed char4x8
/// @param Arg1  - char16* xyzw1  - Lower part of the transposed char4x8
/// @param Arg2  - char8   xIn    - Row 0 of the matrix to be transposed
/// @param Arg3  - char8   yIn    - Row 1 of the matrix to be transposed
/// @param Arg4  - char8   zIn    - Row 2 of the matrix to be transposed
/// @param Arg5  - char8   wIn    - Row 3 of the matrix to be transposed
def __ocl_transpose_store_char4x8_common_sse4:  insert_code<"\n"#
"    char16 x;\n"#
"    char16 y;\n"#
"    char16 z;\n"#
"    char16 w;\n"#
"    x.lo = Arg2;  // x0 x1 x2 x3 x4 x5 x6 x7  D  D  D  D  D  D  D  D\n"#
"    y.lo = Arg3;  // y0 y1 y2 y3 y4 y5 y6 y7  D  D  D  D  D  D  D  D\n"#
"    z.lo = Arg4;  // z0 z1 z2 z3 z4 z5 z6 z7  D  D  D  D  D  D  D  D\n"#
"    w.lo = Arg5;  // w0 w1 w2 w3 w4 w5 w6 w7  D  D  D  D  D  D  D  D\n"#
"    uchar16 low16 = {0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23};\n"#
"    short8 xy = as_short8(shuffle2(x, y, low16));  // x0 y0 x1 y1 x2 y2 x3 y3 x4 y4 x5 y5 x6 y6 x7 y7\n"#
"    short8 zw = as_short8(shuffle2(z, w, low16));  // z0 w0 z1 w1 z2 w2 z3 w3 z4 w4 z5 w5 z6 w6 z7 w7\n"#
"    ushort8 low8 = {0, 8, 1, 9, 2, 10, 3, 11};\n"#
"    ushort8 high8 = {4, 12, 5, 13, 6, 14, 7, 15};\n"#
"    *Arg0 = as_char16(shuffle2(xy, zw, low8));   // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3\n"#
"    *Arg1 = as_char16(shuffle2(xy, zw, high8));  // x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6 x7 y7 z7 w7\n">;

/// @brief Receives char8x4 matrix as 2 halfs (2 char4x4 matrixes) using 2 char16,
///        transposes it and outputs the rows of the transposed matrix.
/// @param Arg0  - char16  xyzw0In  - Upper part of the char8x4 matrix to be transposed
/// @param Arg1  - char16  xyzw1In  - Lower part of the char8x4 matrix to be transposed
/// @param Arg2  - char8*  xOut      - Row 0 of the transposed matrix
/// @param Arg3  - char8*  yOut      - Row 1 of the transposed matrix
/// @param Arg4  - char8*  zOut      - Row 2 of the transposed matrix
/// @param Arg5  - char8*  wOut      - Row 3 of the transposed matrix

def __ocl_load_transpose_char4x8_common_sse4:  insert_code<"\n"#
"    Arg0 = Arg0.s048C159D26AE37BF;  // x0 x1 x2 x3 y0 y1 y2 y3 z0 z1 z2 z3 w0 w1 w2 w3\n"#
"    Arg1 = Arg1.s048C159D26AE37BF;  // x4 x5 x6 x7 y4 y5 y6 y7 z4 z5 z6 z7 w4 w5 w6 w7\n\n"#
"    uint4 low4 = {0, 4, 1, 5};\n"#
"    uint4 high4 = {2, 6, 3, 7};\n"#
"    char16 xy = as_char16(shuffle2(as_int4(Arg0), as_int4(Arg1), low4));   // x0 x1 x2 x3 x4 x5 x6 x7 y0 y1 y2 y3 y4 y5 y6 y7\n"#
"    char16 zw = as_char16(shuffle2(as_int4(Arg0), as_int4(Arg1), high4));  // z0 z1 z2 z3 z4 z5 z6 z7 w0 w1 w2 w3 w4 w5 w6 w7\n"#
"    *Arg2 = xy.lo;  // x0  D x1  D x2  D x3  D x4  D x5  D x6  D x7  D\n"#
"    *Arg3 = xy.hi;  // y0  D y1  D y2  D y3  D y4  D y5  D y6  D y7  D\n"#
"    *Arg4 = zw.lo;  // z0  D z1  D z2  D z3  D z4  D z5  D z6  D z7  D\n"#
"    *Arg5 = zw.hi;  // w0  D w1  D w2  D w3  D w4  D w5  D w6  D w7  D\n">;

/// @brief Arg0-4 types
/// @param Arg0  - char16  xyzwIn - char4x4 matrix to be transposed
/// @param Arg1  - char4*  xOut   - Row 0 of the transposed matrix
/// @param Arg2  - char4*  yOut   - Row 1 of the transposed matrix
/// @param Arg3  - char4*  zOut   - Row 2 of the transposed matrix
/// @param Arg4  - char4*  wOut   - Row 3 of the transposed matrix

def __ocl_load_transpose_char4x4common_sse4:  insert_code<"\n"#
"    char16 txyzw = Arg0;                                   // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3\n"#
"    *Arg1 = txyzw.s048C;                                   // x0  D  D  D x1  D  D  D x2  D  D  D x3  D  D  D\n"#
"    txyzw = as_char16(_mm_srli_si128((__m128i)txyzw, 1));  // y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3  D\n"#
"    *Arg2 = txyzw.s048C;                                   // y0  D  D  D y1  D  D  D y2  D  D  D y3  D  D  D\n"#
"    txyzw = as_char16(_mm_srli_si128((__m128i)txyzw, 1));  // z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3  D  D\n"#
"    *Arg3 = txyzw.s048C;                                   // z0  D  D  D z1  D  D  D z2  D  D  D z3  D  D  D\n"#
"    txyzw = as_char16(_mm_srli_si128((__m128i)txyzw, 1));  // w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3  D  D  D\n"#
"    *Arg4 = txyzw.s048C;\n ">;

/// @brief Receives char4x4 matrix as 4 matrix rows, transposes it and outputs the matrix
///        as a whole using char16
/// @param Arg0  - char16*  xyzw   - This parameter will contain the transposed char4x4 matrix
/// @param Arg1  - char4    xIn    - Row 0 of the matrix to be transposed
/// @param Arg2  - char4    yIn    - Row 1 of the matrix to be transposed
/// @param Arg3  - char4    zIn    - Row 2 of the matrix to be transposed
/// @param Arg4  - char4    wIn    - Row 3 of the matrix to be transposed

def __ocl_transpose_store_char4x4common_sse4:  insert_code<"\n"#
"    char16 x;\n"#
"    char16 y;\n"#
"    char16 z;\n"#
"    char16 w;\n"#
"    x.s0123 = Arg1;  // x0 x1 x2 x3  D  D  D  D  D  D  D  D  D  D  D  D\n"#
"    y.s0123 = Arg2;  // y0 y1 y2 y3  D  D  D  D  D  D  D  D  D  D  D  D\n"#
"    z.s0123 = Arg3;  // z0 z1 z2 z3  D  D  D  D  D  D  D  D  D  D  D  D\n"#
"    w.s0123 = Arg4;  // w0 w1 w2 w3  D  D  D  D  D  D  D  D  D  D  D  D\n"#
"    uchar16 low16 = {0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23};\n"#
"    short8 xy = as_short8(shuffle2(x, y, low16));  // x0 y0 x1 y1 x2 y2 x3 y3  D  D  D  D  D  D  D  D\n"#
"    short8 zw = as_short8(shuffle2(z, w, low16));  // z0 w0 z1 w1 z2 w2 z3 w3  D  D  D  D  D  D  D  D\n"#
"    ushort8 low8 = {0, 8, 1, 9, 2, 10, 3, 11};\n"#
"    *Arg0 = as_char16(shuffle2(xy, zw, low8));     // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3\n">;

// ****************************************************************************
//                                 AVX
// ****************************************************************************

/// @brief Arg0-5 types
/// @param Arg0  - short16  xyzw0
/// @param Arg1  - short16  xyzw1
/// @param Arg2  - short8*  xOut
/// @param Arg3  - short8*  yOut
/// @param Arg4  - short8*  zOut
/// @param Arg5  - short8*  wOut

def __ocl_load_transpose_short4x8_common_avx:  insert_code <"\n"#
"    ushort16 maskUnpackLo = {0, 16, 1, 17, 2, 18, 3, 19,  8, 24,  9, 25, 10, 26, 11, 27};\n"#
"    ushort16 maskUnpackHi = {4, 20, 5, 21, 6, 22, 7, 23, 12, 28, 13, 29, 14, 30, 15, 31};\n"#
"    uint8 maskPermute = {0, 4, 1, 5, 2, 6, 3, 7};\n"#
"    short16 tmp1 = shuffle2(Arg0, Arg1, maskUnpackLo);                 // x0 x4 y0 y4 z0 z4 w0 w4 x2 x6 y2 y6 z2 z6 w2 w6\n"#
"    short16 tmp2 = shuffle2(Arg0, Arg1, maskUnpackHi);                 // x1 x5 y1 y5 z1 z5 w1 w5 x3 x7 y3 y7 z3 z7 w3 w7\n"#
"    short16 tmp3 = shuffle2(tmp1, tmp2, maskUnpackLo);                 // x0 x1 x4 x5 y0 y1 y4 y5 x2 x3 x6 x7 y2 y3 y6 y7\n"#
"    short16 tmp4 = shuffle2(tmp1, tmp2, maskUnpackHi);                 // z0 z1 z4 z5 w0 w1 w4 w5 z2 z3 z6 z7 w2 w3 w6 w7\n"#
"    short16 tmp5 = as_short16(shuffle(as_float8(tmp3), maskPermute));  // x0 x1 x2 x3 x4 x5 x6 x7 y0 y1 y2 y3 y4 y5 y6 y7\n"#
"    short16 tmp6 = as_short16(shuffle(as_float8(tmp4), maskPermute));  // z0 z1 z2 z3 z4 z5 z6 z7 w0 w1 w2 w3 w4 w5 w6 w7\n"#
"    *Arg2 = tmp5.lo;                                                   // x0 x1 x2 x3 x4 x5 x6 x7\n"#
"    *Arg3 = tmp5.hi;                                                   // y0 y1 y2 y3 y4 y5 y6 y7\n"#
"    *Arg4 = tmp6.lo;                                                   // z0 z1 z2 z3 z4 z5 z6 z7\n"#
"    *Arg5 = tmp6.hi;                                                   // w0 w1 w2 w3 w4 w5 w6 w7\n">;

/// @brief Arg0-5 types
/// @param Arg0  - short16*  xyzw0
/// @param Arg1  - short16*  xyzw1
/// @param Arg2  - short8    xIn
/// @param Arg3  - short8    yIn
/// @param Arg4  - short8    zIn
/// @param Arg5  - short8    wIn

def __ocl_transpose_store_short4x8_common_avx:  insert_code<"\n"#
"    ushort16 low16 = {0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15};\n"#
"    float8 xy = as_float8(shuffle2(Arg2, Arg3, low16));  // x0 y0 x1 y1 x2 y2 x3 y3 x4 y4 x5 y5 x6 y6 x7 y7\n"#
"    float8 zw = as_float8(shuffle2(Arg4, Arg5, low16));  // z0 w0 z1 w1 z2 w2 z3 w3 z4 w4 z5 w5 z6 w6 z7 w7\n"#
"    uint8 low8  = {0, 8, 1, 9, 2, 10, 3, 11};\n"#
"    uint8 high8 = {4, 12, 5, 13, 6, 14, 7, 15};\n"#
"    *Arg0 = as_short16(shuffle2(xy, zw, low8));   // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3\n"#
"    *Arg1 = as_short16(shuffle2(xy, zw, high8));  // x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6 x7 y7 z7 w7\n">;

// ****************************************************************************
//                                 AVX2
// ****************************************************************************

/// @brief Arg0-5 types
/// @param Arg0  - short16  xyzw0
/// @param Arg1  - short16  xyzw1
/// @param Arg2  - short8*  xOut
/// @param Arg3  - short8*  yOut
/// @param Arg4  - short8*  zOut
/// @param Arg5  - short8*  wOut

def __ocl_load_transpose_common_short4x8_avx2:  insert_code<"\n"#
"    ushort16 maskUnpackLo = {0, 16, 1, 17, 2, 18, 3, 19,  8, 24,  9, 25, 10, 26, 11, 27};\n"#
"    ushort16 maskUnpackHi = {4, 20, 5, 21, 6, 22, 7, 23, 12, 28, 13, 29, 14, 30, 15, 31};\n"#
"    uint8    maskPermute  = {0, 4, 1, 5, 2, 6, 3, 7};\n"#
"    short16 tmp1 = shuffle2(Arg0, Arg1, maskUnpackLo);               // x0 x4 y0 y4 z0 z4 w0 w4 x2 x6 y2 y6 z2 z6 w2 w6\n"#
"    short16 tmp2 = shuffle2(Arg0, Arg1, maskUnpackHi);               // x1 x5 y1 y5 z1 z5 w1 w5 x3 x7 y3 y7 z3 z7 w3 w7\n"#
"    short16 tmp3 = shuffle2(tmp1, tmp2, maskUnpackLo);               // x0 x1 x4 x5 y0 y1 y4 y5 x2 x3 x6 x7 y2 y3 y6 y7\n"#
"    short16 tmp4 = shuffle2(tmp1, tmp2, maskUnpackHi);               // z0 z1 z4 z5 w0 w1 w4 w5 z2 z3 z6 z7 w2 w3 w6 w7\n"#
"    short16 tmp5 = as_short16(shuffle(as_int8(tmp3), maskPermute));  // x0 x1 x2 x3 x4 x5 x6 x7 y0 y1 y2 y3 y4 y5 y6 y7\n"#
"    short16 tmp6 = as_short16(shuffle(as_int8(tmp4), maskPermute));  // z0 z1 z2 z3 z4 z5 z6 z7 w0 w1 w2 w3 w4 w5 w6 w7\n"#
"    *Arg2 = tmp5.lo;                                                 // x0 x1 x2 x3 x4 x5 x6 x7\n"#
"    *Arg3 = tmp5.hi;                                                 // y0 y1 y2 y3 y4 y5 y6 y7\n"#
"    *Arg4 = tmp6.lo;                                                 // z0 z1 z2 z3 z4 z5 z6 z7\n"#
"    *Arg5 = tmp6.hi;                                                 // w0 w1 w2 w3 w4 w5 w6 w7">;

/// @brief Arg0-5 types
/// @param Arg0  - short16*  xyzw0
/// @param Arg1  - short16*  xyzw1
/// @param Arg2  - short8    xIn
/// @param Arg3  - short8    yIn
/// @param Arg4  - short8    zIn
/// @param Arg5  - short8    wIn

def __ocl_transpose_store_common_short4x8_avx2:  insert_code<"\n"#
"    ushort16 low16 = {0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15};\n"#
"    int8 xy = as_int8(shuffle2(Arg2, Arg3, low16));  // x0 y0 x1 y1 x2 y2 x3 y3 x4 y4 x5 y5 x6 y6 x7 y7\n"#
"    int8 zw = as_int8(shuffle2(Arg4, Arg5, low16));  // z0 w0 z1 w1 z2 w2 z3 w3 z4 w4 z5 w5 z6 w6 z7 w7\n"#
"    uint8 low8  = {0, 8, 1, 9, 2, 10, 3, 11};\n"#
"    uint8 high8 = {4, 12, 5, 13, 6, 14, 7, 15};\n"#
"    *Arg0 = as_short16(shuffle2(xy, zw, low8));   // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3\n"#
"    *Arg1 = as_short16(shuffle2(xy, zw, high8));  // x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6 x7 y7 z7 w7">;

/// @brief Receives int4x4 matrix as 2 halfs (2 int2x4 matrixes) using 2 int8,
///        transposes it and outputs the rows of the transposed matrix.
/// @param Arg0  - int8  xyzw0In  - Part of the int2x4 matrix to be transposed
/// @param Arg1  - int8  xyzw1In  - Part of the int2x4 matrix to be transposed
/// @param Arg2  - int8* xzOut  - Part of the transposed int4x2 matrix.
///                 In case of isLoad is true xzOut will contain rows 0,2 of the transposed matrix,
///                 otherwise it will contain rows 0,1.
/// @param Arg3  - int8* ywOut  - Part of the transposed int4x2 matrix
///                 In case of isLoad is true ywOut will contain rows 1,3 of the transposed matrix,
///                 otherwise it will contain rows 2,3.
/// @param Arg4  - bool  isLoad   - indicates if the origen of the operation is load\store.

def __ocl_transpose_common4_int4x4_avx2:  insert_code<"\n"#
"    if (Arg4) {\n"#
"      // in this case Arg0 = x0 y0 z0 w0 x1 y1 z1 w1\n"#
"      // in this case Arg1 = x2 y2 z2 w2 x3 y3 z3 w3\n"#
"      Arg0 = Arg0.s04152637;  // x0 x1 y0 y1 z0 z1 w0 w1\n"#
"      Arg1 = Arg1.s15043726;  // y2 y3 x2 x3 w2 w3 z2 z3\n"#
"    } else { // isStore\n"#
"      // in this case Arg0 = x0 x1 x2 x3 y0 y1 y2 y3\n"#
"      // in this case Arg1 = z0 z1 z2 z3 w0 w1 w2 w3\n"#
"      Arg0 = Arg0.s04261537;  // x0 y0 x2 y2 x1 y1 x3 y3\n"#
"      Arg1 = Arg1.s26043715;  // z2 w2 z0 w0 z3 w3 z1 w1\n"#
"    }\n"#
"    // TODO : Replace this blend built-in with ?: when clang bug will be fixed\n"#
"    //int8 cond = {0, 0, 0xFFFFFFFF, 0xFFFFFFFF, 0, 0, 0xFFFFFFFF, 0xFFFFFFFF};\n"#
"    //int8 xz = cond ? Arg1 : Arg2;\n"#
"    *Arg2 = as_int8(_mm256_blend_epi32((__m256i)Arg0,          // x0 x1 x2 x3 z0 z1 z2 z3 for isLoad\n"#
"                                       (__m256i)Arg1, 0xCC));  // x0 y0 z0 w0 x1 y1 z1 w1 for isStore\n"#
"    uint8 ywIndices = {2, 3, 8, 9, 6, 7, 12, 13};\n"#
"    *Arg3 = shuffle2(Arg0, Arg1, ywIndices);  // y0 y1 y2 y3 w0 w1 w2 w3 for isLoad\n"#
"                                              // x2 y2 z2 w2 x3 y3 z3 w3 for isStore">;
/// @brief Receives int4x4 matrix as 2 halfs (2 int2x4 matrixes) using 2 int8,
///        transposes it and outputs the rows of the transposed matrix.
/// @param Arg0  - int8   xyzw0In  - Upper part of the int2x4 matrix to be transposed
/// @param Arg1  - int8   xyzw1In  - Lower part of the int2x4 matrix to be transposed
/// @param Arg2  - int8*  xzOut
/// @param Arg3  - int8*  ywOut
/// @param Arg4  - bool*  isLoad
/// @param Arg5  - int4*  xOut     - Row 0 of the transposed matrix
/// @param Arg6  - int4*  yOut     - Row 1 of the transposed matrix
/// @param Arg7  - int4*  zOut     - Row 2 of the transposed matrix
/// @param Arg8  - int4*  wOut     - Row 3 of the transposed matrix



def __ocl_load_transpose_common_int4x4_avx2:  insert_code<"\n"#
"    // xyzw0 = x0 y0 z0 w0 x1 y1 z1 w1\n"#
"    // xyzw1 = x2 y2 z2 w2 x3 y3 z3 w3\n"#
"    int8 xz;\n"#
"    int8 yw;\n"#
"    Arg2 = &xz;\n"#
"    Arg3 = &yw;\n"#
"    Arg4 = true;\n\n"#
"        //__ocl_transpose_common4_int4x4_avx2(int8 Arg0, int8 Arg1, bool Arg4)\n"#
"    #{__ocl_transpose_common4_int4x4_avx2}#\n"#
"    // x0 x1 x2 x3 z0 z1 z2 z3 = xz\n"#
"    // y0 y1 y2 y3 w0 w1 w2 w3 = yw\n"#
"    *Arg5 = xz.lo;  // x0 x1 x2 x3\n"#
"    *Arg6 = yw.lo;  // y0 y1 y2 y3\n"#
"    *Arg7 = xz.hi;  // z0 z1 z2 z3\n"#
"    *Arg8 = yw.hi;  // w0 w1 w2 w3">;
/// @brief Receives int8x4 matrix as 4 double rows (4 int2x4 matrixes) using 4 int8,
///        transposes it and outputs the rows of the transposed matrix.
/// @param Arg0  - int8   xyzw04  - Rows 0,4 of the matrix to be transposed
/// @param Arg1  - int8   xyzw15  - Rows 1,5 of the matrix to be transposed
/// @param Arg2  - int8   xyzw26  - Rows 2,6 og the matrix to be transposed
/// @param Arg3  - int8   xyzw37  - Rows 3,7 matrix to be transposed
/// @param Arg4  - int8*  xOut    - Row 0 of the transposed matrix
/// @param Arg5  - int8*  yOut    - Row 1 of the transposed matrix
/// @param Arg6  - int8*  zOut    - Row 2 of the transposed matrix
/// @param Arg7  - int8*  wOut    - Row 3 of the transposed matrix

def __ocl_transpose_common_int4x8_avx2:  insert_code<"\n"#
"    uint8 lowLane8 = {0, 8, 1, 9, 4, 12, 5, 13};\n"#
"    uint8 highLane8 = {2, 10, 3, 11, 6, 14, 7, 15};\n"#
"    int8 xy0246 = shuffle2(Arg0, Arg2, lowLane8);   // x0 x2 y0 y2 x4 x6 y4 y6\n"#
"    int8 zw0246 = shuffle2(Arg0, Arg2, highLane8);  // z0 z2 w0 w2 z4 z6 w4 w6\n"#
"    int8 xy1357 = shuffle2(Arg1, Arg3, lowLane8);   // x1 x3 y1 y3 x5 x7 y5 y7\n"#
"    int8 zw1357 = shuffle2(Arg1, Arg3, highLane8);  // z1 z3 w1 w3 z5 z7 w5 w7\n"#
"    *Arg4 = shuffle2(xy0246, xy1357, lowLane8);   // x0 x1 x2 x3 x4 x5 x6 x7\n"#
"    *Arg5 = shuffle2(xy0246, xy1357, highLane8);  // y0 y1 y2 y3 y4 y5 y6 y7\n"#
"    *Arg6 = shuffle2(zw0246, zw1357, lowLane8);   // z0 z1 z2 z3 z4 z5 z6 z7\n"#
"    *Arg7 = shuffle2(zw0246, zw1357, highLane8);  // w0 w1 w2 w3 w4 w5 w6 w7">;

/// @brief Receives float4x4 matrix as 2 halfs (2 float2x4 matrixes) using 2 float8,
///        transposes it and outputs the rows of the transposed matrix.
/// @param Arg0  - float8  xyzw0In  - Part of the float2x4 matrix to be transposed
/// @param Arg1  - float8  xyzw1In  - Part of the float2x4 matrix to be transposed
/// @param Arg2  - float8* xzOut  - Part of the transposed float4x2 matrix.
///                 In case of isLoad is true xzOut will contain rows 0,2 of the transposed matrix,
///                 otherwise it will contain rows 0,1.
/// @param Arg3  - float8* ywOut  - Part of the transposed int4x2 matrix
///                 In case of isLoad is true ywOut will contain rows 1,3 of the transposed matrix,
///                 otherwise it will contain rows 2,3.
/// @param Arg4  - bool    isLoad   - indicates if the origen of the operation is load\store.

def __ocl_transpose_common4_float4x4_avx2: insert_code<"\n"#
"    if (Arg4) {\n"#
"      // in this case Arg0 = x0 y0 z0 w0 x1 y1 z1 w1\n"#
"      // in this case Arg0 = x2 y2 w2 z2 x3 y3 z3 w3\n"#
"      Arg0 = Arg0.s04152637;  // x0 x1 y0 y1 z0 z1 w0 w1\n"#
"      Arg1 = Arg1.s15043726;  // y2 y3 x2 x3 w2 w3 z2 z3\n"#
"    } else { // isStore\n"#
"      // in this case Arg0 = x0 x1 x2 x3 y0 y1 y2 y3\n"#
"      // in this case Arg0 = z0 z1 z2 z3 w0 w1 w2 w3\n"#
"      Arg0 = Arg0.s04261537;  // x0 y0 x2 y2 x1 y1 x3 y3\n"#
"      Arg1 = Arg1.s26043715;  // z2 w2 z0 w0 z3 w3 z1 w1\n"#
"    }\n"#
"    // TODO : Replace this blend built-in with ?: when clang bug will be fixed\n"#
"    //int8 cond = {0, 0, 0xFFFFFFFF, 0xFFFFFFFF, 0, 0, 0xFFFFFFFF, 0xFFFFFFFF};\n"#
"    //int8 xz = cond ? Arg0 : Arg1;\n"#
"    *Arg2 = as_float8(_mm256_blend_ps((__m256)Arg0,          // x0 x1 x2 x3 z0 z1 z2 z3 for isLoad\n"#
"                                      (__m256)Arg1, 0xCC));  // x0 y0 z0 w0 x1 y1 z1 w1 for isStore\n"#
"    uint8 ywShuffle = {2, 3, 8, 9, 6, 7, 12, 13};\n"#
"    *Arg3 = shuffle2(Arg0, Arg1, ywShuffle);  // y0 y1 y2 y3 w0 w1 w2 w3 for isLoad\n"#
"                                              // x2 y2 z2 w2 x3 y3 z3 w3 for isStore\n">;

/// @brief Receives float4x4 matrix as 2 halfs (2 float2x4 matrixes) using 2 float8,
///        transposes it and outputs the rows of the transposed matrix.
/// @param Arg0  - float8   xyzw01 - Upper part of the int2x4 matrix to be transposed
/// @param Arg1  - float8   xyzw23 - Lower part of the int2x4 matrix to be transposed
/// @param Arg2  - float8*  xz - Part of the transposed float4x2 matrix.
///                 In case of isLoad is true xzOut will contain rows 0,2 of the transposed matrix,
///                 otherwise it will contain rows 0,1.
/// @param Arg3  - float8*  yw - Part of the transposed int4x2 matrix
///                 In case of isLoad is true ywOut will contain rows 1,3 of the transposed matrix,
///                 otherwise it will contain rows 2,3.
/// @param Arg4  - bool     isLoad - indicates if the origen of the operation is load\store.
/// @param Arg5  - float4*  xOut - Row 0 of the transposed matrix
/// @param Arg6  - float4*  yOut - Row 1 of the transposed matrix
/// @param Arg7  - float4*  zOut - Row 2 of the transposed matrix
/// @param Arg8  - float4*  wOut - Row 3 of the transposed matrix

def __ocl_load_transpose_common_float4x4_avx2:  insert_code<"\n"#
"    float8 xz;\n"#
"    float8 yw;\n"#
"    float8* Arg2=&xz;\n"#
"    float8* Arg3=&yw;\n"#
"    bool Arg4 = true;\n\n"#
"    // __ocl_transpose_common4_float4x4_avx2(Arg0, Arg1,float8* Arg2,float8* Arg3, bool Arg4)\n"#
"    #{__ocl_transpose_common4_float4x4_avx2}#\n"#
"    *Arg5 = xz.lo;\n"#
"    *Arg6 = yw.lo;\n"#
"    *Arg7 = xz.hi;\n"#
"    *Arg8 = yw.hi;\n">;

/// @brief  Receives char8x4 matrix as char32, transposes it and outputs the rows
///        of the transposed matrix
/// @param Arg0  - my_char32  xyzwIn - char8x4 matrix to be transposed
/// @param Arg1  - char8*     xOut - Row 0 of the transposed matrix
/// @param Arg2  - char8*     yOut - Row 1 of the transposed matrix
/// @param Arg3  - char8*     zOut - Row 2 of the transposed matrix
/// @param Arg4  - char8*     wOut - Row 3 of the transposed matrix

def __ocl_load_transpose_char4x8_common_avx2 :  insert_code  <"\n"#
"    my_char32 xyzwIn = Arg0;  // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3 | x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6 x7 y7 z7 w7\n"#
"    my_char32 dummy;\n"#
"    // The reason we use here __builtin_shufflevector and not shuffle(), shuffle2() OpenCL functions\n"#
"    // is because the input typeses here are \"char32\", which are not legal OpenCL built-in types.\n"#
"    // We do not want to sue \"__builtin_shufflevector\" everywhere because it's not a legal OpenCL function,\n"#
"    // it's kind of a hack that makes Clang create LLVM shufflevector instructions.\n"#
"    xyzwIn = __builtin_shufflevector (xyzwIn, dummy,                          // x0 x1 x2 x3 y0 y1 y2 y3 z0 z1 z2 z3 w0 w1 w2 w3 | x4 x5 x6 x7 y4 y5 y6 y7 z4 z5 z6 z7 w4 w5 w6 w7\n"#
"          0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15,\n"#
"          16, 20, 24, 28, 17, 21, 25, 29, 18, 22, 26, 30, 19, 23, 27, 31);\n"#
"    xyzwIn = as_my_char32((as_int8(xyzwIn)).s04152637);                       // x0 x1 x2 x3 x4 x5 x6 x7 y0 y1 y2 y3 y4 y5 y6 y7 | z0 z1 z2 z3 z4 z5 z6 z7 w0 w1 w2 w3 w4 w5 w6 w7\n"#
"    my_char32 xz = __builtin_shufflevector (xyzwIn, dummy,                    // x0  D x1  D x2  D x3  D x4  D x5  D x6  D x7  D | z0  D z1  D z2  D z3  D z4  D z5  D z6  D z7  D\n"#
"          0, 32, 1, 33, 2, 34, 3, 35, 4, 36, 5, 37, 6, 38, 7, 39,\n"#
"          16, 48, 17, 49, 18, 50, 19, 51, 20, 52, 21, 53, 22, 54, 23, 55);\n"#
"    my_char32 yw = __builtin_shufflevector (xyzwIn, dummy,                    // y0  D y1  D y2  D y3  D y4  D y5  D y6  D y7  D | w0  D w1  D w2  D w3  D w4  D w5  D w6  D w7  D\n"#
"         8, 40, 9, 41, 10, 42, 11, 43, 12, 44, 13, 45, 14, 46, 15, 47,\n"#
"         24, 56, 25, 57, 26, 58, 27, 59, 28, 60, 29, 61, 30, 62, 31, 63);\n"#
"    char16 x = as_char16((as_int8(xz)).lo);                                   // x0  D x1  D x2  D x3  D x4  D x5  D x6  D x7  D\n"#
"    char16 y = as_char16((as_int8(yw)).lo);                                   // y0  D y1  D y2  D y3  D y4  D y5  D y6  D y7  D\n"#
"    char16 z = as_char16((as_int8(xz)).hi);                                   // z0  D z1  D z2  D z3  D z4  D z5  D z6  D z7  D\n"#
"    char16 w = as_char16((as_int8(yw)).hi);                                   // w0  D w1  D w2  D w3  D w4  D w5  D w6  D w7  D\n"#
"    *Arg1 = x.s02468ACE;\n"#
"    *Arg2 = y.s02468ACE;\n"#
"    *Arg3 = z.s02468ACE;\n"#
"    *Arg4 = w.s02468ACE;\n">;

/// @brief Receives char4x8 matrix as 4 matrix rows, transposes it and outputs the matrix
///        as a whole using char32
/// @param Arg0 - my_char32* - xyzwOut  - This parameter will contain the transposed char4x8 matrix
/// @param Arg1 - char8 - xIn      - Row 0 of the matrix to be transposed
/// @param Arg2 - char8 - yIn      - Row 1 of the matrix to be transposed
/// @param Arg3 - char8 - zIn      - Row 2 of the matrix to be transposed
/// @param Arg4 - char8 - wIn      - Row 3 of the matrix to be transposed

def __ocl_transpose_store_char4x8_common_avx2:  insert_code<"\n"#
"    char16 x;\n"#
"    char16 y;\n"#
"    char16 z;\n"#
"    char16 w;\n"#
"    x.s02468ACE = Arg1;  // x0  D x1  D x2  D x3  D x4  D x5  D x6  D x7  D\n"#
"    y.s02468ACE = Arg2;  // y0  D y1  D y2  D y3  D y4  D y5  D y6  D y7  D\n"#
"    z.s02468ACE = Arg3;  // z0  D z1  D z2  D z3  D z4  D z5  D z6  D z7  D\n"#
"    w.s02468ACE = Arg4;  // w0  D w1  D w2  D w3  D w4  D w5  D w6  D w7  D\n"#
"    my_char32 xz;\n"#
"    my_char32 yw;\n"#
"    (*(int8*)&xz).lo = as_int4(x);  // x0  D x1  D x2  D x3  D x4  D x5  D x6  D x7  D |  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D\n"#
"    (*(int8*)&yw).lo = as_int4(y);  // y0  D y1  D y2  D y3  D y4  D y5  D y6  D y7  D |  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D\n"#
"    (*(int8*)&xz).hi = as_int4(z);  // x0  D x1  D x2  D x3  D x4  D x5  D x6  D x7  D | z0  D z1  D z2  D z3  D z4  D z5  D z6  D z7  D\n"#
"    (*(int8*)&yw).hi = as_int4(w);  // y0  D y1  D y2  D y3  D y4  D y5  D y6  D y7  D | w0  D w1  D w2  D w3  D w4  D w5  D w6  D w7  D\n"#
"    // The reason we use here __builtin_shufflevector and not shuffle(), shuffle2() OpenCL functions\n"#
"    // is because the input typeses here are \"char32\", which are not legal OpenCL built-in types.\n"#
"    // We do not want to sue \"__builtin_shufflevector\" everywhere because it's not a legal OpenCL function,\n"#
"    // it's kind of a hack that makes Clang create LLVM shufflevector instructions.\n"#
"    my_char32 dummy;\n"#
"    xz = __builtin_shufflevector (xz, dummy,                                  // x0 x1 x2 x3 x4 x5 x6 x7  D  D  D  D  D  D  D  D | z0 z1 z2 z3 z4 z5 z6 z7  D  D  D  D  D  D  D  D\n"#
"          0, 2, 4, 6, 8, 10, 12, 14, 0, 0, 0, 0, 0, 0, 0, 0,\n"#
"          16, 18, 20, 22, 24, 26, 28, 30, 0, 0, 0, 0, 0, 0, 0, 0);\n"#
"    yw = __builtin_shufflevector (yw, dummy,                                  // y0 y1 y2 y3 y4 y5 y6 y7  D  D  D  D  D  D  D  D | w0 w1 w2 w3 w4 w5 w6 w7  D  D  D  D  D  D  D  D\n"#
"          0, 2, 4, 6, 8, 10, 12, 14, 0, 0, 0, 0, 0, 0, 0, 0,\n"#
"          16, 18, 20, 22, 24, 26, 28, 30, 0, 0, 0, 0, 0, 0, 0, 0);\n"#
"    my_char32 txyzw = __builtin_shufflevector (xz, yw,                        // x0 y0 x1 y1 x2 y2 x3 y3 x4 y4 x5 y5 x6 y6 x7 y7 | z0 w0 z1 w1 z2 w2 z3 w3 z4 w4 z5 w5 z6 w6 z7 w7\n"#
"          0, 32, 1, 33, 2, 34, 3, 35, 4, 36, 5, 37, 6, 38, 7, 39,\n"#
"          16, 48, 17, 49, 18, 50, 19, 51, 20, 52, 21, 53, 22, 54, 23, 55);\n"#
"    txyzw = as_my_char32((as_int8(txyzw)).s01452367);                         // x0 y0 x1 y1 x2 y2 x3 y3 z0 w0 z1 w1 z2 w2 z3 w3 | x4 y4 x5 y5 x6 y6 x7 y7 z4 w4 z5 w5 z6 w6 z7 w7\n"#
"    txyzw = __builtin_shufflevector (txyzw, dummy,                            // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3 | x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6 x7 y7 z7 w7\n"#
"          0, 1, 8, 9, 2, 3, 10, 11, 4, 5, 12, 13, 6, 7, 14, 15,\n"#
"          16, 17, 24, 25, 18, 19, 26, 27, 20, 21, 28, 29, 22, 23, 30, 31);\n"#
"    *Arg0 = txyzw;\n">;
