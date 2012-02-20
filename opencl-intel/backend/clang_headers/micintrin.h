#ifndef __MICINTRIN_H
#define __MICINTRIN_H

#if !defined(__MIC__) && !defined(__MIC2__)
#error "MIC is not enabled!"
#endif

typedef double __v8df __attribute__((__vector_size__(64)));
typedef float __v16sf __attribute__((__vector_size__(64)));
typedef long __v8di __attribute__((__vector_size__(64)));
typedef int __v16si __attribute__((__vector_size__(64)));

typedef float __m512 __attribute__((__vector_size__(64)));
typedef double __m512d __attribute__((__vector_size__(64)));
typedef long __m512i __attribute__((__vector_size__(64)));

typedef unsigned char __mmask8;
typedef unsigned short __mmask16;

#ifdef __MASK8_TRANSITION
#define __mmaskt  __mmask8
#else
#define __mmaskt  __mmask16
#endif

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_castpd_ps(__m512d in)
{
  return (__m512)in;
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_castpd_si512(__m512d in)
{
  return (__m512i)in;
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_castps_pd(__m512 in)
{
  return (__m512d)in;
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_castps_si512(__m512 in)
{
  return (__m512i)in;
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_castsi512_ps(__m512i in)
{
  return (__m512)in;
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_castsi512_pd(__m512i in)
{
  return (__m512d)in;
}


/* Constant for special read-only mask register 'k0'. */
#define _MM_K0_REG (0xffff)


/* Constants for register swizzle primitives. */
typedef enum __MM_SWIZZLE_ENUM {
    _MM_SWIZ_REG_NONE,      /* hgfe dcba - Nop */
#define _MM_SWIZ_REG_DCBA _MM_SWIZ_REG_NONE
    _MM_SWIZ_REG_CDAB,      /* ghef cdab - Swap pairs */
    _MM_SWIZ_REG_BADC,      /* fehg badc - Swap with two-away */
    _MM_SWIZ_REG_AAAA,      /* eeee aaaa - broadcast a element */
    _MM_SWIZ_REG_BBBB,      /* ffff bbbb - broadcast b element */
    _MM_SWIZ_REG_CCCC,      /* gggg cccc - broadcast c element */
    _MM_SWIZ_REG_DDDD,      /* hhhh dddd - broadcast d element */
    _MM_SWIZ_REG_DACB       /* hegf dacb - cross-product */
} _MM_SWIZZLE_ENUM;

/* Constants for broadcasts to vectors with 32-bit elements. */
typedef enum __MM_BROADCAST32_ENUM {
    _MM_BROADCAST32_NONE,   /* identity swizzle/convert */
#define _MM_BROADCAST_16X16 _MM_BROADCAST32_NONE
    _MM_BROADCAST_1X16,     /* broadcast x 16 ( aaaa aaaa aaaa aaaa ) */
    _MM_BROADCAST_4X16      /* broadcast x 4  ( dcba dcba dcba dcba ) */
} _MM_BROADCAST32_ENUM;

/* Constants for broadcasts to vectors with 64-bit elements. */
typedef enum __MM_BROADCAST64_ENUM {
    _MM_BROADCAST64_NONE,   /* identity swizzle/convert */
#define _MM_BROADCAST_8X8 _MM_BROADCAST64_NONE
    _MM_BROADCAST_1X8,      /* broadcast x 8 ( aaaa aaaa ) */
    _MM_BROADCAST_4X8       /* broadcast x 2 ( dcba dcba ) */
} _MM_BROADCAST64_ENUM;

/* Constants for float32 SwizUpConv conversion primitives. */
typedef enum __MM_UPCONV_F32_ENUM {
    _MM_16X16_F32,            /* identity swizzle/convert */
    _MM_1X16_F32,             /* broadcast x 16 ( aaaa aaaa aaaa aaaa ) */
    _MM_4X16_F32,             /* broadcast x 4  ( dcba dcba dcba dcba ) */
    _MM_UPCONV_F32_UNUSED,
    _MM_UI8_TO_F32,           /* 16 x uint8   => 16 x fp32 */
    _MM_UN8_TO_F32,           /* 16 x unorm8  => 16 x fp32 */
    _MM_F16_TO_F32,           /* 16 x float16 => 16 x fp32 */
    _MM_SI16_TO_F32           /* 16 x sint16  => 16 x fp32 */
} _MM_UPCONV_F32_ENUM;

/* Constants for int32 SwizUpConv conversion primitives. */
typedef enum __MM_UPCONV_I32_ENUM {
    _MM_16X16_I32,            /* identity swizzle/convert */
    _MM_1X16_I32,             /* broadcast x 16 ( aaaa aaaa aaaa aaaa ) */
    _MM_4X16_I32,             /* broadcast x 4  ( dcba dcba dcba dcba ) */
    _MM_UPCONV_I32_UNUSED,
    _MM_UI8_TO_I32,           /* 16 x uint8   => 16 x uint32 */
    _MM_SI8_TO_I32,           /* 16 x sint8   => 16 x sint32 */
    _MM_UI16_TO_I32,          /* 16 x uint16  => 16 x uint32 */
    _MM_SI16_TO_I32           /* 16 x sint16  => 16 x sint32 */
} _MM_UPCONV_I32_ENUM;

/* Constants for float64 SwizUpConv conversion primitives. */
typedef enum __MM_UPCONV_F64_ENUM {
    _MM_8X8_F64,              /* identity swizzle/convert */
    _MM_1X8_F64,              /* broadcast x 8 ( aaaa aaaa ) */
    _MM_4X8_F64               /* broadcast x 4  ( dcba dcba ) */
} _MM_UPCONV_F64_ENUM;

/* Constants for int64 SwizUpConv conversion primitives. */
typedef enum __MM_UPCONV_I64_ENUM {
    _MM_8X8_I64,              /* identity swizzle/convert */
    _MM_1X8_I64,              /* broadcast x 8 ( aaaa aaaa ) */
    _MM_4X8_I64               /* broadcast x 4  ( dcba dcba ) */
} _MM_UPCONV_I64_ENUM;

/* Constants for 32-bit FullUpConv conversion vload primitives. */
typedef enum __MM_FULLUP32_ENUM {
    _MM_FULLUPC_NONE,              /* no conversion      */
    _MM_FULLUPC_FLOAT16,           /* float16 => float32 */
    _MM_FULLUPC_UNUSED,            /* float16 => float32 */
    _MM_FULLUPC_SRGB8,             /* srgb8   => float32 */
    _MM_FULLUPC_UINT8,             /* uint8   => float32 */
    _MM_FULLUPC_SINT8,             /* sint8   => float32 */
    _MM_FULLUPC_UNORM8,            /* unorm8  => float32 */
    _MM_FULLUPC_SNORM8,            /* snorm8  => float32 */
    _MM_FULLUPC_UINT16,            /* uint16  => float32 */
    _MM_FULLUPC_SINT16,            /* sint16  => float32 */
    _MM_FULLUPC_UNORM16,           /* unorm16 => float32 */
    _MM_FULLUPC_SNORM16,           /* snorm16 => float32 */
    _MM_FULLUPC_UINT8I,            /* uint8   => uint32  */
    _MM_FULLUPC_SINT8I,            /* sint8   => sint32  */
    _MM_FULLUPC_UINT16I,           /* uint16  => uint32  */
    _MM_FULLUPC_SINT16I,           /* sint16  => sint32  */
    _MM_FULLUPC_UNORM10A,          /* unorm10A10B10C2D field A => float32 */
    _MM_FULLUPC_UNORM10B,          /* unorm10A10B10C2D field B => float32 */
    _MM_FULLUPC_UNORM10C,          /* unorm10A10B10C2D field C => float32 */
    _MM_FULLUPC_UNORM2D,           /* unorm10A10B10C2D field D => float32 */
    _MM_FULLUPC_FLOAT11A,          /* float11A11B10C field A   => float32 */
    _MM_FULLUPC_FLOAT11B,          /* float11A11B10C field B   => float32 */
    _MM_FULLUPC_FLOAT10C           /* float11A11B10C field C   => float32 */
} _MM_FULLUP32_ENUM;

/* Constants for 64-bit FullUpConv conversion vload primitives. */
typedef enum __MM_FULLUP64_ENUM {
    _MM_FULLUPC64_NONE            /* no conversion      */
} _MM_FULLUP64_ENUM;

/* Constants for 32-bit DownConv conversion vstore primitives. */
typedef enum __MM_DOWNCONV32_ENUM {
    _MM_DOWNC_NONE,                /* no conversion      */
    _MM_DOWNC_FLOAT16,             /* float32 => float16 */
    _MM_DOWNC_FLOAT16RZ,           /* float32 => float16 (round to zero) */
    _MM_DOWNC_UNUSED,              /* float32 => float16 (round to zero) */
    _MM_DOWNC_UINT8,               /* float32 => uint8   */
    _MM_DOWNC_SINT8,               /* float32 => sint8   */
    _MM_DOWNC_UNORM8,              /* float32 => unorm8  */
    _MM_DOWNC_SNORM8,              /* float32 => snorm8  */
    _MM_DOWNC_UINT16,              /* float32 => uint16  */
    _MM_DOWNC_SINT16,              /* float32 => sint16  */
    _MM_DOWNC_UNORM16,             /* float32 => unorm16 */
    _MM_DOWNC_SNORM16,             /* float32 => snorm16 */
    _MM_DOWNC_UINT8I,              /* uint32  => uint8   */
    _MM_DOWNC_SINT8I,              /* sint32  => sint8   */
    _MM_DOWNC_UINT16I,             /* uint32  => uint16  */
    _MM_DOWNC_SINT16I              /* sint32  => sint16  */
} _MM_DOWNCONV32_ENUM;

/* Constants for 64-bit DownConv conversion vstore primitives. */
typedef enum __MM_DOWNCONV64_ENUM {
    _MM_DOWNC64_NONE               /* no conversion      */
} _MM_DOWNCONV64_ENUM;

/* Constants for vstore Subset primitives. */
typedef enum __MM_STORE_SUBSET32_ENUM {
    _MM_SUBSET32_16,              /* store 16 elements (ponm lkji hgfe dcba) */
    _MM_SUBSET32_1,               /* store  1 element  (a)    */
    _MM_SUBSET32_4                /* store  4 elements (dcba) */
} _MM_STORE_SUBSET32_ENUM;

typedef enum __MM_STORE_SUBSET64_ENUM {
    _MM_SUBSET64_8,               /* store 8 64-bit elements  (hgfe dcba) */
    _MM_SUBSET64_1,               /* store  1 element  (a)    */
    _MM_SUBSET64_4                /* store  4 elements (dcba) */
} _MM_STORE_SUBSET64_ENUM;


/*
 * Constants for rounding mode.
 * These names beginnig with "_MM_ROUND" are deprecated.
 * Use the names beginning with "_MM_FROUND" going forward.
 */
typedef enum __MM_ROUND_MOD_ENUM {
    _MM_ROUND_MODE_NEAREST,             /* round to nearest (even) */
    _MM_ROUND_MODE_DOWN,                /* round toward negative infinity */
    _MM_ROUND_MODE_UP,                  /* round toward positive infinity */
    _MM_ROUND_MODE_TOWARD_ZERO,         /* round toward zero */
    _MM_ROUND_MODE_DEFAULT              /* round mode from MXCSR */
} _MM_ROUND_MODE_ENUM;

/* Constants for exponent adjustment. */
typedef enum __MM_EXP_ADJ_ENUM {
    _MM_EXPADJ_NONE,               /* 2**0  (32.0 - no exp adjustment) */
    _MM_EXPADJ_4,                  /* 2**4  (28.4)  */
    _MM_EXPADJ_5,                  /* 2**5  (27.5)  */
    _MM_EXPADJ_8,                  /* 2**8  (24.8)  */
    _MM_EXPADJ_16,                 /* 2**16 (16.16) */
    _MM_EXPADJ_24,                 /* 2**24 (8.24)  */
    _MM_EXPADJ_31,                 /* 2**31 (1.31)  */
    _MM_EXPADJ_32                  /* 2**32 (0.32)  */
} _MM_EXP_ADJ_ENUM;

/* Constants for use with _mm_vprefetch. */
typedef enum __MM_PREFETCH_HINT_ENUM {
    _MM_PFHINT_NONE,
    _MM_PFHINT_EX,
    _MM_PFHINT_NT,
    _MM_PFHINT_EX_NT,
    _MM_PFHINT_MISS,
    _MM_PFHINT_EX_MISS,
    _MM_PFHINT_NT_MISS,
    _MM_PFHINT_EX_NT_MISS
} _MM_PREFETCH_HINT_ENUM;

/* Constants for index scale (vgather/vscatter). */
typedef enum __MM_INDEX_SCALE_ENUM {
    _MM_SCALE_1 = 1,
    _MM_SCALE_2 = 2,
    _MM_SCALE_4 = 4,
    _MM_SCALE_8 = 8
} _MM_INDEX_SCALE_ENUM;

/*
 * Constants for load/store temporal hints.
 *
 * This was originally defined as an enumeration, hence the suffix "_ENUM"
 * in the type name, but was changed to int when the prefetch requests
 * were added.
 */

typedef int _MM_MEM_HINT_ENUM;

#define _MM_HINT_NONE           0x0
#define _MM_HINT_NT             0x1     /* Load or store is non-temporal. */
#define _MM_HINT_PF_FULL        0x2     /* Request auto-inserted prefetches. */
#define _MM_HINT_PF_CLOSE       0x4     /* Request auto-inserted prefetch. */

/* Constants for permutation with shuf128x32. */
typedef enum __MM_PERM_ENUM {
    _MM_PERM_AAAA = 0x00, _MM_PERM_AAAB = 0x01, _MM_PERM_AAAC = 0x02,
    _MM_PERM_AAAD = 0x03, _MM_PERM_AABA = 0x04, _MM_PERM_AABB = 0x05,
    _MM_PERM_AABC = 0x06, _MM_PERM_AABD = 0x07, _MM_PERM_AACA = 0x08,
    _MM_PERM_AACB = 0x09, _MM_PERM_AACC = 0x0A, _MM_PERM_AACD = 0x0B,
    _MM_PERM_AADA = 0x0C, _MM_PERM_AADB = 0x0D, _MM_PERM_AADC = 0x0E,
    _MM_PERM_AADD = 0x0F, _MM_PERM_ABAA = 0x10, _MM_PERM_ABAB = 0x11,
    _MM_PERM_ABAC = 0x12, _MM_PERM_ABAD = 0x13, _MM_PERM_ABBA = 0x14,
    _MM_PERM_ABBB = 0x15, _MM_PERM_ABBC = 0x16, _MM_PERM_ABBD = 0x17,
    _MM_PERM_ABCA = 0x18, _MM_PERM_ABCB = 0x19, _MM_PERM_ABCC = 0x1A,
    _MM_PERM_ABCD = 0x1B, _MM_PERM_ABDA = 0x1C, _MM_PERM_ABDB = 0x1D,
    _MM_PERM_ABDC = 0x1E, _MM_PERM_ABDD = 0x1F, _MM_PERM_ACAA = 0x20,
    _MM_PERM_ACAB = 0x21, _MM_PERM_ACAC = 0x22, _MM_PERM_ACAD = 0x23,
    _MM_PERM_ACBA = 0x24, _MM_PERM_ACBB = 0x25, _MM_PERM_ACBC = 0x26,
    _MM_PERM_ACBD = 0x27, _MM_PERM_ACCA = 0x28, _MM_PERM_ACCB = 0x29,
    _MM_PERM_ACCC = 0x2A, _MM_PERM_ACCD = 0x2B, _MM_PERM_ACDA = 0x2C,
    _MM_PERM_ACDB = 0x2D, _MM_PERM_ACDC = 0x2E, _MM_PERM_ACDD = 0x2F,
    _MM_PERM_ADAA = 0x30, _MM_PERM_ADAB = 0x31, _MM_PERM_ADAC = 0x32,
    _MM_PERM_ADAD = 0x33, _MM_PERM_ADBA = 0x34, _MM_PERM_ADBB = 0x35,
    _MM_PERM_ADBC = 0x36, _MM_PERM_ADBD = 0x37, _MM_PERM_ADCA = 0x38,
    _MM_PERM_ADCB = 0x39, _MM_PERM_ADCC = 0x3A, _MM_PERM_ADCD = 0x3B,
    _MM_PERM_ADDA = 0x3C, _MM_PERM_ADDB = 0x3D, _MM_PERM_ADDC = 0x3E,
    _MM_PERM_ADDD = 0x3F, _MM_PERM_BAAA = 0x40, _MM_PERM_BAAB = 0x41,
    _MM_PERM_BAAC = 0x42, _MM_PERM_BAAD = 0x43, _MM_PERM_BABA = 0x44,
    _MM_PERM_BABB = 0x45, _MM_PERM_BABC = 0x46, _MM_PERM_BABD = 0x47,
    _MM_PERM_BACA = 0x48, _MM_PERM_BACB = 0x49, _MM_PERM_BACC = 0x4A,
    _MM_PERM_BACD = 0x4B, _MM_PERM_BADA = 0x4C, _MM_PERM_BADB = 0x4D,
    _MM_PERM_BADC = 0x4E, _MM_PERM_BADD = 0x4F, _MM_PERM_BBAA = 0x50,
    _MM_PERM_BBAB = 0x51, _MM_PERM_BBAC = 0x52, _MM_PERM_BBAD = 0x53,
    _MM_PERM_BBBA = 0x54, _MM_PERM_BBBB = 0x55, _MM_PERM_BBBC = 0x56,
    _MM_PERM_BBBD = 0x57, _MM_PERM_BBCA = 0x58, _MM_PERM_BBCB = 0x59,
    _MM_PERM_BBCC = 0x5A, _MM_PERM_BBCD = 0x5B, _MM_PERM_BBDA = 0x5C,
    _MM_PERM_BBDB = 0x5D, _MM_PERM_BBDC = 0x5E, _MM_PERM_BBDD = 0x5F,
    _MM_PERM_BCAA = 0x60, _MM_PERM_BCAB = 0x61, _MM_PERM_BCAC = 0x62,
    _MM_PERM_BCAD = 0x63, _MM_PERM_BCBA = 0x64, _MM_PERM_BCBB = 0x65,
    _MM_PERM_BCBC = 0x66, _MM_PERM_BCBD = 0x67, _MM_PERM_BCCA = 0x68,
    _MM_PERM_BCCB = 0x69, _MM_PERM_BCCC = 0x6A, _MM_PERM_BCCD = 0x6B,
    _MM_PERM_BCDA = 0x6C, _MM_PERM_BCDB = 0x6D, _MM_PERM_BCDC = 0x6E,
    _MM_PERM_BCDD = 0x6F, _MM_PERM_BDAA = 0x70, _MM_PERM_BDAB = 0x71,
    _MM_PERM_BDAC = 0x72, _MM_PERM_BDAD = 0x73, _MM_PERM_BDBA = 0x74,
    _MM_PERM_BDBB = 0x75, _MM_PERM_BDBC = 0x76, _MM_PERM_BDBD = 0x77,
    _MM_PERM_BDCA = 0x78, _MM_PERM_BDCB = 0x79, _MM_PERM_BDCC = 0x7A,
    _MM_PERM_BDCD = 0x7B, _MM_PERM_BDDA = 0x7C, _MM_PERM_BDDB = 0x7D,
    _MM_PERM_BDDC = 0x7E, _MM_PERM_BDDD = 0x7F, _MM_PERM_CAAA = 0x80,
    _MM_PERM_CAAB = 0x81, _MM_PERM_CAAC = 0x82, _MM_PERM_CAAD = 0x83,
    _MM_PERM_CABA = 0x84, _MM_PERM_CABB = 0x85, _MM_PERM_CABC = 0x86,
    _MM_PERM_CABD = 0x87, _MM_PERM_CACA = 0x88, _MM_PERM_CACB = 0x89,
    _MM_PERM_CACC = 0x8A, _MM_PERM_CACD = 0x8B, _MM_PERM_CADA = 0x8C,
    _MM_PERM_CADB = 0x8D, _MM_PERM_CADC = 0x8E, _MM_PERM_CADD = 0x8F,
    _MM_PERM_CBAA = 0x90, _MM_PERM_CBAB = 0x91, _MM_PERM_CBAC = 0x92,
    _MM_PERM_CBAD = 0x93, _MM_PERM_CBBA = 0x94, _MM_PERM_CBBB = 0x95,
    _MM_PERM_CBBC = 0x96, _MM_PERM_CBBD = 0x97, _MM_PERM_CBCA = 0x98,
    _MM_PERM_CBCB = 0x99, _MM_PERM_CBCC = 0x9A, _MM_PERM_CBCD = 0x9B,
    _MM_PERM_CBDA = 0x9C, _MM_PERM_CBDB = 0x9D, _MM_PERM_CBDC = 0x9E,
    _MM_PERM_CBDD = 0x9F, _MM_PERM_CCAA = 0xA0, _MM_PERM_CCAB = 0xA1,
    _MM_PERM_CCAC = 0xA2, _MM_PERM_CCAD = 0xA3, _MM_PERM_CCBA = 0xA4,
    _MM_PERM_CCBB = 0xA5, _MM_PERM_CCBC = 0xA6, _MM_PERM_CCBD = 0xA7,
    _MM_PERM_CCCA = 0xA8, _MM_PERM_CCCB = 0xA9, _MM_PERM_CCCC = 0xAA,
    _MM_PERM_CCCD = 0xAB, _MM_PERM_CCDA = 0xAC, _MM_PERM_CCDB = 0xAD,
    _MM_PERM_CCDC = 0xAE, _MM_PERM_CCDD = 0xAF, _MM_PERM_CDAA = 0xB0,
    _MM_PERM_CDAB = 0xB1, _MM_PERM_CDAC = 0xB2, _MM_PERM_CDAD = 0xB3,
    _MM_PERM_CDBA = 0xB4, _MM_PERM_CDBB = 0xB5, _MM_PERM_CDBC = 0xB6,
    _MM_PERM_CDBD = 0xB7, _MM_PERM_CDCA = 0xB8, _MM_PERM_CDCB = 0xB9,
    _MM_PERM_CDCC = 0xBA, _MM_PERM_CDCD = 0xBB, _MM_PERM_CDDA = 0xBC,
    _MM_PERM_CDDB = 0xBD, _MM_PERM_CDDC = 0xBE, _MM_PERM_CDDD = 0xBF,
    _MM_PERM_DAAA = 0xC0, _MM_PERM_DAAB = 0xC1, _MM_PERM_DAAC = 0xC2,
    _MM_PERM_DAAD = 0xC3, _MM_PERM_DABA = 0xC4, _MM_PERM_DABB = 0xC5,
    _MM_PERM_DABC = 0xC6, _MM_PERM_DABD = 0xC7, _MM_PERM_DACA = 0xC8,
    _MM_PERM_DACB = 0xC9, _MM_PERM_DACC = 0xCA, _MM_PERM_DACD = 0xCB,
    _MM_PERM_DADA = 0xCC, _MM_PERM_DADB = 0xCD, _MM_PERM_DADC = 0xCE,
    _MM_PERM_DADD = 0xCF, _MM_PERM_DBAA = 0xD0, _MM_PERM_DBAB = 0xD1,
    _MM_PERM_DBAC = 0xD2, _MM_PERM_DBAD = 0xD3, _MM_PERM_DBBA = 0xD4,
    _MM_PERM_DBBB = 0xD5, _MM_PERM_DBBC = 0xD6, _MM_PERM_DBBD = 0xD7,
    _MM_PERM_DBCA = 0xD8, _MM_PERM_DBCB = 0xD9, _MM_PERM_DBCC = 0xDA,
    _MM_PERM_DBCD = 0xDB, _MM_PERM_DBDA = 0xDC, _MM_PERM_DBDB = 0xDD,
    _MM_PERM_DBDC = 0xDE, _MM_PERM_DBDD = 0xDF, _MM_PERM_DCAA = 0xE0,
    _MM_PERM_DCAB = 0xE1, _MM_PERM_DCAC = 0xE2, _MM_PERM_DCAD = 0xE3,
    _MM_PERM_DCBA = 0xE4, _MM_PERM_DCBB = 0xE5, _MM_PERM_DCBC = 0xE6,
    _MM_PERM_DCBD = 0xE7, _MM_PERM_DCCA = 0xE8, _MM_PERM_DCCB = 0xE9,
    _MM_PERM_DCCC = 0xEA, _MM_PERM_DCCD = 0xEB, _MM_PERM_DCDA = 0xEC,
    _MM_PERM_DCDB = 0xED, _MM_PERM_DCDC = 0xEE, _MM_PERM_DCDD = 0xEF,
    _MM_PERM_DDAA = 0xF0, _MM_PERM_DDAB = 0xF1, _MM_PERM_DDAC = 0xF2,
    _MM_PERM_DDAD = 0xF3, _MM_PERM_DDBA = 0xF4, _MM_PERM_DDBB = 0xF5,
    _MM_PERM_DDBC = 0xF6, _MM_PERM_DDBD = 0xF7, _MM_PERM_DDCA = 0xF8,
    _MM_PERM_DDCB = 0xF9, _MM_PERM_DDCC = 0xFA, _MM_PERM_DDCD = 0xFB,
    _MM_PERM_DDDA = 0xFC, _MM_PERM_DDDB = 0xFD, _MM_PERM_DDDC = 0xFE,
    _MM_PERM_DDDD = 0xFF
} _MM_PERM_ENUM;

/* Constants for cvtins_ps2u10. */
typedef enum __MM_UNORM10_FIELD_ENUM {
    _MM_UNORM10A,
    _MM_UNORM10B,
    _MM_UNORM10C,
    _MM_UNORM2D
} _MM_UNORM10_FIELD_ENUM;

/* Constants for cvtins_ps2f11. */
typedef enum __MM_FLOAT11_FIELD_ENUM {
    _MM_FLOAT11A,
    _MM_FLOAT11B,
    _MM_FLOAT10C,
    _MM_FLOAT11NONE
} _MM_FLOAT11_FIELD_ENUM;


/*
 * Helper type and macro for computing the values of the immediate
 * used in mm512_fixup_ps.
 */
typedef enum __MM_FIXUPRESULT_ENUM {
    _MM_FIXUP_NO_CHANGE,
    _MM_FIXUP_NEG_INF,
    _MM_FIXUP_NEG_ZERO,
    _MM_FIXUP_POS_ZERO,
    _MM_FIXUP_POS_INF,
    _MM_FIXUP_NAN,
    _MM_FIXUP_MAX_FLOAT,
    _MM_FIXUP_MIN_FLOAT
} _MM_FIXUPRESULT_ENUM;

#define _MM_FIXUP(_NegInf, \
                  _Neg, \
                  _NegZero, \
                  _PosZero, \
                  _Pos, \
                  _PosInf, \
                  _Nan) \
   ((int) (_NegInf) | \
   ((int) (_Neg) << 3) | \
   ((int) (_NegZero) << 6) | \
   ((int) (_PosZero) << 9) | \
   ((int) (_Pos) << 12) | \
   ((int) (_PosInf) << 15) | \
   ((int) (_Nan) << 18))

/* MACRO functions for setting and reading the VXCSR. */
#define _MM_VXCSR_EXCEPT_MASK       0x003f
#define _MM_VXCSR_MASK_MASK         0x1f80
#define _MM_VXCSR_ROUND_MASK        0x6000
#define _MM_VXCSR_FLUSH_ZERO_MASK   0x8000

#define _MM_SET_VXCSR_EXCEPTION_STATE(mask)                              \
            _mm_setvxcsr((_mm_getvxcsr() & ~_MM_VXCSR_EXCEPT_MASK) |     \
                         (mask & _MM_VXCSR_EXCEPT_MASK))
#define _MM_GET_VXCSR_EXCEPTION_STATE()                                  \
            (_mm_getvxcsr() & _MM_VXCSR_EXCEPT_MASK)

#define _MM_SET_VXCSR_EXCEPTION_MASK(mask)                               \
            _mm_setvxcsr((_mm_getvxcsr() & ~_MM_VXCSR_MASK_MASK) |       \
                         (mask & _MM_VXCSR_MASK_MASK))
#define _MM_GET_VXCSR_EXCEPTION_MASK()                                   \
            (_mm_getvxcsr() & _MM_VXCSR_MASK_MASK)

#define _MM_SET_VXCSR_ROUNDING_MODE(mode)                                \
            _mm_setvxcsr((_mm_getvxcsr() & ~_MM_VXCSR_ROUND_MASK) |      \
                         (mode & _MM_VXCSR_ROUND_MASK))
#define _MM_GET_VXCSR_ROUNDING_MODE()                                    \
            (_mm_getvxcsr() & _MM_VXCSR_ROUND_MASK)

#define _MM_SET_VXCSR_FLUSH_ZERO_MODE(mode)                              \
            _mm_setvxcsr((_mm_getvxcsr() & ~_MM_VXCSR_FLUSH_ZERO_MASK) | \
                         (mode & _MM_VXCSR_FLUSH_ZERO_MASK))
#define _MM_GET_VXCSR_FLUSH_ZERO_MODE(mode)                              \
            (_mm_getvxcsr() & _MM_VXCSR_FLUSH_ZERO_MASK)

/*
 * Vector move with mask.  Returns the vector with elements
 * from v_old and 'src' depending on bits set in mask 'k1'.
 */

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_mov_ps(__m512 v_old, __mmask16 k1, __m512 src)
{
  return __builtin_ia32_maskmovps512(v_old, k1, src);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_mov_pd(__m512d v_old, __mmaskt k1, __m512d src)
{
  return __builtin_ia32_maskmovpd512(v_old, k1, src);
}

#define _mm512_mask_movd(v_old, k1, src) \
  _mm512_mask_mov_ps((v_old), (k1), (src))

#define _mm512_mask_movq(v_old, k1, src) \
  _mm512_mask_mov_pd((v_old), (k1), (src))

#define _mm512_mask_mov_epi32(v_old, k1, src) \
  _mm512_mask_mov_ps((v_old), (k1), (src))

#define _mm512_mask_mov_epi64(v_old, k1, src) \
  _mm512_mask_mov_pd((v_old), (k1), (src))

/*
 * Load doubleword vector.
 *    The 1, 2, 4, 8, 16, 32, or 64 bytes (depending on the conversion and
 *    broadcast in effect) at memory address 'mt' are broadcast and/or
 *    converted to a doubleword vector.
 */

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_loadd(void const *mt,
             const _MM_FULLUP32_ENUM full_up,
             const _MM_BROADCAST32_ENUM broadcast,
             const _MM_MEM_HINT_ENUM nt)
{
  return __builtin_ia32_loadps512(mt, full_up, broadcast, nt);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_loadd(__m512 v1_old, __mmask16 k1, void const *mt,
                  const _MM_FULLUP32_ENUM full_up,
                  const _MM_BROADCAST32_ENUM broadcast,
                  const _MM_MEM_HINT_ENUM nt)
{
  return __builtin_ia32_mask_loadps512(v1_old, k1, mt, full_up, broadcast, nt);
}

/*
 * Load quadword vector.
 *    The 8, 32, or 64 bytes (depending on the broadcast in effect) at memory
 *    address 'mt' are broadcast and/or converted to a quadword vector.
 *
 * The 'full_up' parameter sets the conversion to be used and in current
 * model it has to be _MM_FULLUPC64_NONE.
 */

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_loadq(void const *mt, const _MM_FULLUP64_ENUM full_up,
             const _MM_BROADCAST64_ENUM broadcast, const _MM_MEM_HINT_ENUM nt)
{
  return __builtin_ia32_loadpd512(mt, full_up, broadcast, nt);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_loadq(__m512d v1_old, __mmaskt k1, void const *mt,
                  const _MM_FULLUP64_ENUM full_up, const _MM_BROADCAST64_ENUM broadcast,
                  const _MM_MEM_HINT_ENUM nt)
{
  return __builtin_ia32_mask_loadpd512(v1_old, k1, mt, full_up, broadcast, nt);
}

/*
 * Swizzle/broadcast/upconversion operations.
 *
 * Almost all vector instructions have the form:
 *     opcode  reg1{k1}, reg2, S(reg3/m)
 * where S(reg3/m) is a function that returns the vector result of the
 * swizzle/broadcast/upconversion process on memory or register reg3.
 * Rather than supplying an extra swizzle/broadcast/upconversion argument
 * to all intrinsic functions, this functionality is provided as a set of
 * intrinsics whose results can be used as parameters to other intrinsics.
 * The compiler may optimize such sequences by combining the swizzle,
 * broadcast or upconversion with the subsequent vector operation.
 */

/*
 * 32-bit Register Swizzle
 * Returns the result of the swizzle process on float32 vector 'v'.
 *
 * Typical usage:
 *
 *       v1 = _mm512_add_ps(v2, _mm512_swizzle_r32(v3, _MM_SWIZ_REG_DACB));
 */
#define _mm512_swizzle_r32(v, s) \
  _mm512_swizzle_ps((v), (s))

/*
 * 64-bit Register Swizzle
 * Returns the result of the swizzle process on float64 vector 'v'.
 *
 * Typical usage:
 *
 *       _mm512_add_pd(v2, _mm512_swizzle_r64(v3, _MM_SWIZ_REG_DCBA))
 */
#define _mm512_swizzle_r64(v, s) \
  _mm512_swizzle_pd((v), (s))

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_swizzle_ps(__m512 v, const _MM_SWIZZLE_ENUM s)
{
  return __builtin_ia32_swizzleps512(v, s);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_swizzle_pd(__m512d v, const _MM_SWIZZLE_ENUM s)
{
  return __builtin_ia32_swizzlepd512(v, s);
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_swizzle_epi32(__m512i v, const _MM_SWIZZLE_ENUM s)
{
  return __builtin_ia32_swizzlepi512(v, s);
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_swizzle_epi64(__m512i v, const _MM_SWIZZLE_ENUM s)
{
  return __builtin_ia32_swizzlepq512(v, s);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_swizzle_ps(__m512 old, __mmask16 k, __m512 v, const _MM_SWIZZLE_ENUM s)
{
  return __builtin_ia32_mask_swizzleps512(old, k, v, s);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_swizzle_pd(__m512d old, __mmaskt k, __m512d v, const _MM_SWIZZLE_ENUM s)
{
  return __builtin_ia32_mask_swizzlepd512(old, k, v, s);
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_swizzle_epi32(__m512i old, __mmask16 k, __m512i v, const _MM_SWIZZLE_ENUM s)
{
  return __builtin_ia32_mask_swizzlepi512(old, k, v, s);
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_swizzle_epi64(__m512i old, __mmaskt k, __m512i v, const _MM_SWIZZLE_ENUM s)
{
  return __builtin_ia32_mask_swizzlepq512(old, k, v, s);
}

/*
 * Memory upconvert or broadcast to 32-bit or 64-bit integer,
 * or to float32 or float64.
 * Returns the result of the broadcast or upconversion process
 * on the given memory value.
 *
 * Typical usage:
 *
 *       _mm512_add_pi(v2,
 *           _mm512_upconv_int32(&uint8_mem, _MM_UI8_TO_I32, _MM_HINT_NONE))
 *       _mm512_add_ps(v2,
 *           _mm512_upconv_float32(&uint8_mem, _MM_4X16_F32, _MM_HINT_NT))
 */
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_upconv_int32(void const *mt, const _MM_UPCONV_I32_ENUM s, const _MM_MEM_HINT_ENUM nt)
{
  return __builtin_ia32_upconvi512(mt, s, nt);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_upconv_float32(void const *mt, const _MM_UPCONV_F32_ENUM s, const _MM_MEM_HINT_ENUM nt)
{
  return __builtin_ia32_upconvs512(mt, s, nt);
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_upconv_int64(void const *mt, const _MM_UPCONV_I64_ENUM s, const _MM_MEM_HINT_ENUM nt)
{
  return __builtin_ia32_upconvq512(mt, s, nt);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_upconv_float64(void const *mt, const _MM_UPCONV_F64_ENUM s, const _MM_MEM_HINT_ENUM nt)
{
  return __builtin_ia32_upconvd512(mt, s, nt);
}

/*
 * Store doubleword vector.
 *    Downconverts and stores all 16 elements, the 4 lower elements,
 *    or the lowest element of doubleword vector 'v2' to the memory location
 *    'mt', depending on the value of 'subset' which can be one of the
 *    following constants:
 *     _MM_SUBSET32_16   - store 16 elements (ponm lkji hgfe dcba)
 *     _MM_SUBSET32_1    - store  1 element  (a)
 *     _MM_SUBSET32_4    - store  4 elements (dcba)
 */
__inline__ void __attribute__((__always_inline__, __nodebug__))
_mm512_stored(void *m, __m512 v1,
              const _MM_DOWNCONV32_ENUM d,
              const _MM_STORE_SUBSET32_ENUM subset,
              const _MM_MEM_HINT_ENUM nt)
{
  return __builtin_ia32_storeps512(m, v1, d, subset, nt);
}

__inline__ void __attribute__((__always_inline__, __nodebug__))
_mm512_mask_stored(void *m, __mmask16 k1, __m512 v1,
                   const _MM_DOWNCONV32_ENUM d,
                   const _MM_STORE_SUBSET32_ENUM subset,
                   const _MM_MEM_HINT_ENUM nt)
{
  return __builtin_ia32_mask_storeps512(m, k1, v1, d, subset, nt);
}

/*
 * Store quadword vector.
 *    Downconverts and stores all 8 elements, the 4 lower elements, or the
 *    lowest element of quadword vector 'v2' to the memory location 'mt',
 *    depending on the value of 'subset' which can be one of the following
 *    constants:
 *     _MM_SUBSET64_8    - store all 8 elements (hgfe dcba)
 *     _MM_SUBSET64_1    - store  1 element  (a)
 *     _MM_SUBSET64_4    - store  4 elements (dcba)
 */
__inline__ void __attribute__((__always_inline__, __nodebug__))
_mm512_storeq(void *mt, __m512d v1,
              const _MM_DOWNCONV64_ENUM d,
              const _MM_STORE_SUBSET64_ENUM subset,
              const _MM_MEM_HINT_ENUM nt)
{
  return __builtin_ia32_storepd512(mt, v1, d, subset, nt);
}

__inline__ void __attribute__((__always_inline__, __nodebug__))
_mm512_mask_storeq(void *mt, __mmaskt k1, __m512d v1,
                   const _MM_DOWNCONV64_ENUM d,
                   const _MM_STORE_SUBSET64_ENUM subset,
                   const _MM_MEM_HINT_ENUM nt)
{
  return __builtin_ia32_mask_storepd512(mt, k1, v1, d, subset, nt);
}

/*
 * Vector operations.
 *
 * Most vector instructions have the form:
 *     opcode reg1{k1}, reg2, S(reg3/m)
 * where reg1 is a destination operand.  The instructions are writemasked,
 * so only those elements with the corresponding bit set in vector mask
 * register k1 are computed and stored into reg1.  Elements in reg1 with
 * the corresponding bit clear in k1 retain their previous values.
 * This means that the destination vector reg1 is also a source vector
 * and it should be passed to the intrinsic function as an additional
 * parameter.  To simplify usage and to enable compiler optimizations,
 * pairs of intrinsics are provided for each vector instruction -- the fully
 * general form that takes the original reg1 and k1 arguments, and the
 * non-writemasked form where all destination elements are updated:
 *
 *          _mm512_opcode(reg2, reg3)
 *     _mm512_mask_opcode(reg1_old, k1, reg2, reg3)
 */

/*
 * Add int32 vectors with carry.
 *
 *    Adds int32 vector 'v1', the "carry" represented in vector mask
 *    register 'k2' and int32 vector 'v3'.
 *    The carry of the sum is returned via '*k2_res'.
 */
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_adc_epi32(__m512i v1, __mmask16 k2, __m512i v3, __mmask16 *k2_res)
{
  return __builtin_ia32_adcpi512(v1, k2, v3, k2_res);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_adc_epi32(__m512i v1, __mmask16 k1, __mmask16 k2, __m512i v3, __mmask16 *k2_res)
{
  return __builtin_ia32_mask_adcpi512(v1, k1, k2, v3, k2_res);
}

#define _mm512_adc_pi(v1, k2, v3, k2_res) \
  _mm512_adc_epi32((v1), (k2), (v3), (k2_res))
#define _mm512_mask_adc_pi(v1, k1, k2, v3, k2_res) \
  _mm512_mask_adc_epi32((v1), (k1), (k2), (v3), (k2_res))

/*
 * Add float32 or float64 vectors and negate the sum.
 */
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_addn_pd(__m512d v2, __m512d v3)
{
  return __builtin_ia32_addnpd512(v2, v3);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_addn_pd(__m512d v1_old, __mmaskt k1,
                    __m512d v2, __m512d v3)
{
  return __builtin_ia32_mask_addnpd512(v1_old, k1, v2, v3);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_addn_ps(__m512 v2, __m512 v3)
{
  return __builtin_ia32_addnps512(v2, v3);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_addn_ps(__m512 v1_old, __mmask16 k1,
                    __m512 v2, __m512 v3)
{
  return __builtin_ia32_mask_addnps512(v1_old, k1, v2, v3);
}

/*
 * Add, subtract or multiply float64, float32, int64 or int32 vectors.
 *
 *  add         v2 + v3
 *  mul         v2 * v3
 *  sub         v2 - v3
 *  subr        v3 - v2
 */
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_add_pd(__m512d v2, __m512d v3)
{
  return __builtin_ia32_addpd512(v2, v3);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_add_pd(__m512d v1_old, __mmaskt k1, __m512d v2, __m512d v3)
{
  return __builtin_ia32_mask_addpd512(v1_old, k1, v2, v3);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_add_ps(__m512 v2, __m512 v3)
{
  return __builtin_ia32_addps512(v2, v3);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_add_ps(__m512 v1_old, __mmask16 k1, __m512 v2, __m512 v3)
{
  return __builtin_ia32_mask_addps512(v1_old, k1, v2, v3);
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_add_epi32(__m512i v2, __m512i v3)
{
  return __builtin_ia32_addpi512(v2, v3);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_add_epi32(__m512i v1_old, __mmask16 k1, __m512i v2, __m512i v3)
{
  return __builtin_ia32_mask_addpi512(v1_old, k1, v2, v3);
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_add_epi64(__m512i v2, __m512i v3)
{
  return __builtin_ia32_addpq512(v2, v3);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_add_epi64(__m512i v1_old, __mmaskt k1, __m512i v2, __m512i v3)
{
  return __builtin_ia32_mask_addpq512(v1_old, k1, v2, v3);
}

#define _mm512_add_pi(v2, v3) \
  _mm512_add_epi32((v2), (v3))
#define _mm512_mask_add_pi(v1_old, k1, v2, v3) \
  _mm512_mask_add_epi32((v1_old), (k1), (v2, v3)

#define _mm512_add_pq(v2, v3) \
  _mm512_add_epi64((v2), (v3))
#define _mm512_mask_add_pq(v1_old, k1, v2, v3) \
  _mm512_mask_add_epi64((v1_old), (k1), (v2), (v3))

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mul_pd(__m512d v2, __m512d v3)
{
  return __builtin_ia32_mulpd512(v2, v3);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_mul_pd(__m512d v1_old, __mmaskt k1, __m512d v2, __m512d v3)
{
  return __builtin_ia32_mask_mulpd512(v1_old, k1, v2, v3);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mul_ps(__m512 v2, __m512 v3)
{
  return __builtin_ia32_mulps512(v2, v3);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_mul_ps(__m512 v1_old, __mmask16 k1, __m512 v2, __m512 v3)
{
  return __builtin_ia32_mask_mulps512(v1_old, k1, v2, v3);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_sub_pd(__m512d v2, __m512d v3)
{
  return __builtin_ia32_mulpd512(v2, v3);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_sub_pd(__m512d v1_old, __mmaskt k1, __m512d v2, __m512d v3)
{
  return __builtin_ia32_mask_mulpd512(v1_old, k1, v2, v3);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_sub_ps(__m512 v2, __m512 v3)
{
  return __builtin_ia32_subps512(v2, v3);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_sub_ps(__m512 v1_old, __mmask16 k1, __m512 v2, __m512 v3)
{
  return __builtin_ia32_mask_subps512(v1_old, k1, v2, v3);
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_sub_epi32(__m512i v2, __m512i v3)
{
  return __builtin_ia32_subpi512(v2, v3);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_sub_epi32(__m512i v1_old, __mmask16 k1, __m512i v2, __m512i v3)
{
  return __builtin_ia32_mask_subpi512(v1_old, k1, v2, v3);
}

#define _mm512_sub_pi(v2, v3) \
  _mm512_sub_epi32((v2), (v3))
#define _mm512_mask_sub_pi(v1_old, k1, v2, v3) \
  _mm512_mask_sub_epi32((v1_old), (k1), (v2), (v3))

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_subr_pd(__m512d v2, __m512d v3)
{
  return __builtin_ia32_subpd512(v2, v3);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_subr_pd(__m512d v1_old, __mmaskt k1, __m512d v2, __m512d v3)
{
  return __builtin_ia32_mask_subpd512(v1_old, k1, v2, v3);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_subr_ps(__m512 v2,__m512 v3)
{
  return __builtin_ia32_subrps512(v2, v3);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_subr_ps(__m512 v1_old, __mmask16 k1, __m512 v2, __m512 v3)
{
  return __builtin_ia32_mask_subrps512(v1_old, k1, v2, v3);
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_subr_epi32(__m512i v2, __m512i v3)
{
  return __builtin_ia32_subrpi512(v2, v3);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_subr_epi32(__m512i v1_old, __mmask16 k1, __m512i v2, __m512i v3)
{
  return __builtin_ia32_mask_subrpi512(v1_old, k1, v2, v3);
}

#define _mm512_subr_pi(v2, v3) \
  _mm512_subr_epi32((v2), (v3))
#define _mm512_mask_subr_pi(v1_old, k1, v2, v3) \
  _mm512_mask_subr_epi32((v1_old), (k1), (v2), (v3))

/*
 * Add int32 vectors and set carry.
 *    Add int32 vector 'v1' and int32 vector 'v3'.
 *    The carry from the sum is returned in 'k2_res'.
 */
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_addsetc_epi32(__m512i v1, __m512i v3, __mmask16 *k2_res)
{
  return __builtin_ia32_addsetcpi512(v1, v3, k2_res);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_addsetc_epi32(__m512i v1, __mmask16 k1, __mmask16 k2_old, __m512i v3, __mmask16 *k2_res)
{
  return __builtin_ia32_mask_addsetcpi512(v1, k1, k2_old, v3, k2_res);
}

#define _mm512_addsetc_pi(v1, v3, k2_res) \
  _mm512_addsetc_epi32((v1), (v3), (k2_res))
#define _mm512_mask_addsetc_pi(v1, k1, k2_old, v3, k2_res) \
  _mm512_mask_addsetc_epi32((v1), (k1), (k2_old), (v3), (k2_res))

/*
 * Add int32 or float32 Vectors and Set Mask to Sign.
 *    Performs an element-by-element addition of vectors 'v2' and 'v3'.
 *    The sign of the result for the n-th element is returned in "k1_res".
 */
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_addsets_epi32(__m512i v2, __m512i v3, __mmask16 *k1_res)
{
  return __builtin_ia32_addsetspi512(v2, v3, k1_res);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_addsets_epi32(__m512i v1_old, __mmask16 k1, __m512i v2, __m512i v3, __mmask16 *k1_res)
{
  return __builtin_ia32_mask_addsetspi512(v1_old, k1, v2, v3, k1_res);
}

#define _mm512_addsets_pi(v2, v3, k1_res) \
  _mm512_addsets_epi32((v2), (v3), (k1_res))
#define _mm512_mask_addsets_pi(v1_old, k1, v2, v3, k1_res) \
  _mm512_mask_addsets_epi32((v1_old), (k1), (v2), (v3), (k1_res))

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_addsets_ps(__m512 v2, __m512 v3, __mmask16 *k1_res)
{
  return __builtin_ia32_addsetsps512(v2, v3, k1_res);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_addsets_ps(__m512 v1_old, __mmask16 k1, __m512 v2, __m512 v3, __mmask16 *k1_res)
{
  return __builtin_ia32_mask_addsetsps512(v1_old, k1, v2, v3, k1_res);
}

/*
 * Subtract int32 vectors and set borrow.
 *    Performs an element-by-element subtraction of the int32 vector 'v3'
 *    from int32 vector 'v1'.
 *    The borrow from the subtraction for the n-th element
 *    is written into the n-th bit of vector mask 'k2_res'.
 */
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_subsetb_epi32(__m512i v1, __m512i v3, __mmask16 *k2_res)
{
  return __builtin_ia32_subsetbpi512(v1, v3, k2_res);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_subsetb_epi32(__m512i v1, __mmask16 k1, __mmask16 k2_old, __m512i v3, __mmask16 *k2_res)
{
  return __builtin_ia32_mask_subsetbpi512(v1, k1, k2_old, v3, k2_res);
}

#define _mm512_subsetb_pi(v1, v3, k2_res) \
  _mm512_subsetb_epi32((v1), (v3), (k2_res))
#define _mm512_mask_subsetb_pi(v1, k1, k2_old, v3, k2_res) \
  _mm512_mask_subsetb_epi32((v1), (k1), (k2_old), (v3), (k2_res))

/*
 * Reverse subtract int32 vectors and set borrow.
 *    Performs an element-by-element subtraction of int32 vector 'v1' from
 *    the int32 vector 'v3'.
 *    The borrow from the subtraction for the n-th element
 *    is written into the n-th bit of vector mask 'k2_res'.
 */
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_subrsetb_epi32(__m512i v1, __m512i v3, __mmask16 *k2_res)
{
  return __builtin_ia32_subrsetbpi512(v1, v3, k2_res);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_subrsetb_epi32(__m512i v1, __mmask16 k1, __mmask16 k2_old, __m512i v3, __mmask16 *k2_res)
{
  return __builtin_ia32_mask_subrsetbpi512(v1, k1, k2_old, v3, k2_res);
}

#define _mm512_subrsetb_pi(v1, v3, k2_res) \
  _mm512_subrsetb_epi32((v1), (v3), (k2_res))
#define _mm512_mask_subrsetb_pi(v1, k1, k2_old, v3, k2_res) \
  _mm512_mask_subrsetb_epi32((v1), (k1), (k2_old), (v3), (k2_res))

/*
 * Subtract int32 vectors with borrow.
 *    Performs an element-by-element three-input subtraction of the int32
 *    vector 'v3', as well as the corresponding bit of 'k2', from int32
 *    vector 'v1'.
 *
 *    In addition, the borrow from the subtraction difference for the n-th
 *    element is written into the n-th bit of vector mask 'k2_res'.
 */
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_sbb_epi32(__m512i v1, __mmask16 k2, __m512i v3, __mmask16 *k2_res)
{
  return __builtin_ia32_sbbpi512(v1, k2, v3, k2_res);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_sbb_epi32(__m512i v1, __mmask16 k1, __mmask16 k2, __m512i v3, __mmask16 *k2_res)
{
  return __builtin_ia32_mask_sbbpi512(v1, k1, k2, v3, k2_res);
}

#define _mm512_sbb_pi(v1, k2, v3, k2_res) \
  _mm512_sbb_epi32((v1), (k2), (v3), (k2_res))
#define _mm512_mask_sbb_pi(v1, k1, k2, v3, k2_res) \
  _mm512_mask_sbb_epi32((v1), (k1), (k2), (v3), (k2_res))

/*
 * Reverse subtract int32 vectors with borrow.
 *    Performs an element-by-element three-input subtraction of the int32
 *    vector 'v1', as well as the corresponding bit of 'k2', from int32
 *    vector 'v3'.
 *
 *    In addition, the borrow from the subtraction difference for the n-th
 *    element is written into the n-th bit of vector mask 'k2_res'.
 */
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_sbbr_epi32(__m512i v1, __mmask16 k2, __m512i v3, __mmask16 *k2_res)
{
  return __builtin_ia32_sbbrpi512(v1, k2, v3, k2_res);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_sbbr_epi32(__m512i v1, __mmask16 k1, __mmask16 k2, __m512i v3, __mmask16 *k2_res)
{
  return __builtin_ia32_mask_sbbrpi512(v1, k1, k2, v3, k2_res);
}

#define _mm512_sbbr_pi(v1, k2, v3, k2_res) \
  _mm512_sbbr_epi32((v1), (k2), (v3), (k2_res))
#define _mm512_mask_sbbr_pi(v1, k1, k2, v3, k2_res) \
  _mm512_mask_sbbr_epi32((v1), (k1), (k2), (v3), (k2_res))

/*
 * Bitwise and, and not, or, and xor of int32 or int64 vectors.
 * "and not" ands the ones complement of the first vector operand
 * with the second.
 */

#define _mm512_and_pi(v2, v3) \
  _mm512_and_epi32((v2), (v3))
#define _mm512_mask_and_pi(v1_old, k1, v2, v3) \
  _mm512_mask_and_epi32((v1_old), (k1), (v2), (v3))

#define _mm512_and_pq(v2, v3) \
  _mm512_and_epi64((v2), (v3))
#define _mm512_mask_and_pq(v1_old, k1, v2, v3) \
  _mm512_mask_and_epi64((v1_old), (k1), (v2), (v3))

#define _mm512_andn_pi(v2, v3) \
  _mm512_andnot_epi32((v2), (v3))
#define _mm512_mask_andn_pi(v1_old, k1, v2, v3) \
  _mm512_mask_andnot_epi32((v1_old), (k1), (v2), (v3))

#define _mm512_andn_pq(v2, v3) \
  _mm512_andnot_epi64((v2), (v3))
#define _mm512_mask_andn_pq(v1_old, k1, v2, v3) \
  _mm512_mask_andnot_epi64((v1_old), (k1), (v2), (v3))

#define _mm512_or_pi(v2, v3) \
  _mm512_or_epi32((v2), (v3))
#define _mm512_mask_or_pi(v1_old, k1, v2, v3) \
  _mm512_mask_or_epi32((v1_old), (k1), (v2), (v3))

#define _mm512_or_pq(v2, v3) \
  _mm512_or_epi64((v2), (v3))
#define _mm512_mask_or_pq(v1_old, k1, v2, v3) \
  _mm512_mask_or_epi64((v1_old), (k1), (v2), (v3))

#define _mm512_xor_pi(v2, v3) \
  _mm512_xor_epi32((v2), (v3))
#define _mm512_mask_xor_pi(v1_old, k1, v2, v3) \
  _mm512_mask_xor_epi32((v1_old), (k1), (v2), (v3))

#define _mm512_xor_pq(v2, v3) \
  _mm512_xor_epi64((v2), (v3))
#define _mm512_mask_xor_pq(v1_old, k1, v2, v3) \
  _mm512_mask_xor_epi64((v1_old), (k1), (v2), (v3))

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_and_epi32(__m512i v2, __m512i v3)
{
  return __builtin_ia32_andpi512(v2, v3);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_and_epi32(__m512i v1_old, __mmask16 k1, __m512i v2, __m512i v3)
{
  return __builtin_ia32_mask_andpi512(v1_old, k1, v2, v3);
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_and_epi64(__m512i v2, __m512i v3)
{
  return __builtin_ia32_andpq512(v2, v3);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_and_epi64(__m512i v1_old, __mmaskt k1, __m512i v2, __m512i v3)
{
  return __builtin_ia32_mask_andpq512(v1_old, k1, v2, v3);
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_andnot_epi32(__m512i v2, __m512i v3)
{
  return __builtin_ia32_andnpi512(v2, v3);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_andnot_epi32(__m512i v1_old, __mmask16 k1, __m512i v2, __m512i v3)
{
  return __builtin_ia32_mask_andnpi512(v1_old, k1, v2, v3);
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_andnot_epi64(__m512i v2, __m512i v3)
{
  return __builtin_ia32_andnpq512(v2, v3);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_andnot_epi64(__m512i v1_old, __mmaskt k1, __m512i v2, __m512i v3)
{
  return __builtin_ia32_mask_andnpq512(v1_old, k1, v2, v3);
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_or_epi32(__m512i v2, __m512i v3)
{
  return __builtin_ia32_orpi512(v2, v3);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_or_epi32(__m512i v1_old, __mmask16 k1, __m512i v2, __m512i v3)
{
  return __builtin_ia32_mask_orpi512(v1_old, k1, v2, v3);
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_or_epi64(__m512i v2, __m512i v3)
{
  return __builtin_ia32_orpq512(v2, v3);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_or_epi64(__m512i v1_old, __mmaskt k1, __m512i v2, __m512i v3)
{
  return __builtin_ia32_mask_orpq512(v1_old, k1, v2, v3);
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_xor_epi32(__m512i v2, __m512i v3)
{
  return __builtin_ia32_xorpi512(v2, v3);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_xor_epi32(__m512i v1_old, __mmask16 k1, __m512i v2, __m512i v3)
{
  return __builtin_ia32_mask_xorpi512(v1_old, k1, v2, v3);
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_xor_epi64(__m512i v2, __m512i v3)
{
  return __builtin_ia32_xorpq512(v2, v3);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_xor_epi64(__m512i v1_old, __mmaskt k1, __m512i v2, __m512i v3)
{
  return __builtin_ia32_mask_xorpq512(v1_old, k1, v2, v3);
}

/*
 * bit-interleave int32 vectors.
 *    Performs an element-by-element bitwise interleave, using a 1:1 or 2:1
 *    pattern, between int32 vector 'v2' and int32 vector 'v3'.
 */
#define _mm512_bitinterleave11_pi(v2, v3) \
  _mm512_bitinterleave11_epi32((v2), (v3))
#define _mm512_mask_bitinterleave11_pi(v1_old, k1, v2, v3) \
  _mm512_mask_interleave11_epi32((v1_old), (k1), (v2), (v3))

#define _mm512_bitinterleave21_pi(v2, v3) \
  _mm512_bitinterleave21_epi32((v2), (v3))
#define _mm512_mask_bitinterleave21_pi(v1_old, k1, v2, v3) \
  _mm512_mask_interleave21_epi32((v1_old), (k1), (v2), (v3))

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_bitinterleave11_epi32(__m512i v2, __m512i v3)
{
  return __builtin_ia32_bitinterleave11pi512(v2, v3);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_bitinterleave11_epi32(__m512i v1_old, __mmask16 k1, __m512i v2, __m512i v3)
{
  return __builtin_ia32_mask_bitinterleave11pi512(v1_old, k1, v2, v3);
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_bitinterleave21_epi32(__m512i v2, __m512i v3)
{
  return __builtin_ia32_bitinterleave21pi512(v2, v3);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_bitinterleave21_epi32(__m512i v1_old, __mmask16 k1, __m512i v2, __m512i v3)
{
  return __builtin_ia32_mask_bitinterleave21pi512(v1_old, k1, v2, v3);
}

/*
 * Clamp to zero int32 vector.
 *    Performs an element-by-element clamp of int32 vector 'v2' to the range
 *    between zero and int32 vector 'v3'.
 *
 *    A negative value for either source will generate a zero result.
 *    Another way to view this intrinsic is as returning for each element
 *    the lesser of the two sources, but never less than 0.
 */
#define _mm512_clampz_pi(v2, v3) \
  _mm512_clampz_epi32((v2), (v3))
#define _mm512_mask_clampz_pi(v1_old, k1, v2, v3) \
  _mm512_mask_clampz_epi32((v1_old), (k1), (v2), (v3))

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_clampz_epi32(__m512i v2, __m512i v3)
{
  return __builtin_ia32_clampzpi512(v2, v3);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_clampz_epi32(__m512i v1_old, __mmask16 k1, __m512i v2, __m512i v3)
{
  return __builtin_ia32_mask_clampzpi512(v1_old, k1, v2, v3);
}

/*
 * Clamp to zero float32 vector.
 *    Performs an element-by-element clamp of float32 vector 'v2' to the range
 *    between zero and float32 vector 'v3'.
 *
 *    When a negative clamping value is provided, then it behaves
 *    as if the user has reversed the instruction.
 *    The intrinsic has the following property:
 *        clampz(x,y) == -clampz(-x,-y)
 *    If the clamping limit (v3) is a NaN, the instruction returns
 *    a zero with the sign of the clamped value (second operand).
 *    If both are NaNs, the instruction returns a positive zero.
 */
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_clampz_ps(__m512 v2, __m512 v3)
{
  return __builtin_ia32_clampzps512(v2, v3);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_clampz_ps(__m512 v1_old, __mmask16 k1, __m512 v2, __m512 v3)
{
  return __builtin_ia32_mask_clampzps512(v1_old, k1, v2, v3);
}

/*
 * Compare float32, float64 or int32 vectors and set mask.
 *    Performs an element-by-element comparison between vectors
 *    'v1' and 'v2'.  The resulting truth value is returned
 *    in the corresponding element bit of the result mask.
 *
 *    The writemask in the masked flavors of these intrinsics does not
 *    perform the normal writemasking function.  While it does enable/disable
 *    element comparisons, it does not block updating of the destination.
 *    Instead, if a writemask bit is 0, the corresponding destination mask
 *    bit is set to 0.
 */

__inline__ __mmaskt __attribute__((__always_inline__, __nodebug__))
_mm512_cmpeq_pd(__m512d v1, __m512d v2)
{
  return __builtin_ia32_cmpeqpd512(v1, v2);
}
__inline__ __mmaskt __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cmpeq_pd(__mmaskt k1, __m512d v1, __m512d v2)
{
  return __builtin_ia32_mask_cmpeqpd512(k1, v1, v2);
}

__inline__ __mmaskt __attribute__((__always_inline__, __nodebug__))
_mm512_cmplt_pd(__m512d v1, __m512d v2)
{
  return __builtin_ia32_cmpltpd512(v1, v2);
}
__inline__ __mmaskt __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cmplt_pd(__mmaskt k1, __m512d v1, __m512d v2)
{
  return __builtin_ia32_mask_cmpltpd512(k1, v1, v2);
}

__inline__ __mmaskt __attribute__((__always_inline__, __nodebug__))
_mm512_cmple_pd(__m512d v1, __m512d v2)
{
  return __builtin_ia32_cmplepd512(v1, v2);
}
__inline__ __mmaskt __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cmple_pd(__mmaskt k1, __m512d v1, __m512d v2)
{
  return __builtin_ia32_mask_cmplepd512(k1, v1, v2);
}

__inline__ __mmaskt __attribute__((__always_inline__, __nodebug__))
_mm512_cmpunord_pd(__m512d v1, __m512d v2)
{
  return __builtin_ia32_cmpunordpd512(v1, v2);
}
__inline__ __mmaskt __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cmpunord_pd(__mmaskt k1, __m512d v1, __m512d v2)
{
  return __builtin_ia32_mask_cmpunordpd512(k1, v1, v2);
}

__inline__ __mmaskt __attribute__((__always_inline__, __nodebug__))
_mm512_cmpneq_pd(__m512d v1, __m512d v2)
{
  return __builtin_ia32_cmpneqpd512(v1, v2);
}
__inline__ __mmaskt __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cmpneq_pd(__mmaskt k1, __m512d v1, __m512d v2)
{
  return __builtin_ia32_mask_cmpneqpd512(k1, v1, v2);
}

__inline__ __mmaskt __attribute__((__always_inline__, __nodebug__))
_mm512_cmpnlt_pd(__m512d v1, __m512d v2)
{
  return __builtin_ia32_cmpnltpd512(v1, v2);
}
__inline__ __mmaskt __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cmpnlt_pd(__mmaskt k1, __m512d v1, __m512d v2)
{
  return __builtin_ia32_mask_cmpnltpd512(k1, v1, v2);
}

__inline__ __mmaskt __attribute__((__always_inline__, __nodebug__))
_mm512_cmpnle_pd(__m512d v1, __m512d v2)
{
  return __builtin_ia32_cmpnlepd512(v1, v2);
}
__inline__ __mmaskt __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cmpnle_pd(__mmaskt k1, __m512d v1, __m512d v2)
{
  return __builtin_ia32_mask_cmpnlepd512(k1, v1, v2);
}

__inline__ __mmaskt __attribute__((__always_inline__, __nodebug__))
_mm512_cmpord_pd(__m512d v1, __m512d v2)
{
  return __builtin_ia32_cmpordpd512(v1, v2);
}
__inline__ __mmaskt __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cmpord_pd(__mmaskt k1, __m512d v1, __m512d v2)
{
  return __builtin_ia32_mask_cmpordpd512(k1, v1, v2);
}

__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_cmpeq_ps(__m512 v1, __m512 v2)
{
  return __builtin_ia32_cmpeqps512(v1, v2);
}
__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cmpeq_ps(__mmask16 k1, __m512 v1, __m512 v2)
{
  return __builtin_ia32_mask_cmpeqps512(k1, v1, v2);
}

__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_cmplt_ps(__m512 v1, __m512 v2)
{
  return __builtin_ia32_cmpltps512(v1, v2);
}
__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cmplt_ps(__mmask16 k1, __m512 v1, __m512 v2)
{
  return __builtin_ia32_mask_cmpltps512(k1, v1, v2);
}

__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_cmple_ps(__m512 v1, __m512 v2)
{
  return __builtin_ia32_cmpleps512(v1, v2);
}
__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cmple_ps(__mmask16 k1, __m512 v1, __m512 v2)
{
  return __builtin_ia32_mask_cmpleps512(k1, v1, v2);
}

__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_cmpunord_ps(__m512 v1, __m512 v2)
{
  return __builtin_ia32_cmpunordps512(v1, v2);
}
__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cmpunord_ps(__mmask16 k1, __m512 v1, __m512 v2)
{
  return __builtin_ia32_mask_cmpunordps512(k1, v1, v2);
}

__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_cmpneq_ps(__m512 v1, __m512 v2)
{
  return __builtin_ia32_cmpneqps512(v1, v2);
}
__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cmpneq_ps(__mmask16 k1, __m512 v1, __m512 v2)
{
  return __builtin_ia32_mask_cmpneqps512(k1, v1, v2);
}

__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_cmpnlt_ps(__m512 v1, __m512 v2)
{
  return __builtin_ia32_cmpnltps512(v1, v2);
}
__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cmpnlt_ps(__mmask16 k1, __m512 v1, __m512 v2)
{
  return __builtin_ia32_mask_cmpnltps512(k1, v1, v2);
}

__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_cmpnle_ps(__m512 v1, __m512 v2)
{
  return __builtin_ia32_cmpnleps512(v1, v2);
}
__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cmpnle_ps(__mmask16 k1, __m512 v1, __m512 v2)
{
  return __builtin_ia32_mask_cmpnleps512(k1, v1, v2);
}

__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_cmpord_ps(__m512 v1, __m512 v2)
{
  return __builtin_ia32_cmpordps512(v1, v2);
}
__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cmpord_ps(__mmask16 k1, __m512 v1, __m512 v2)
{
  return __builtin_ia32_mask_cmpordps512(k1, v1, v2);
}

/* 32-bit signed integer compares. */

__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_cmpeq_pi(__m512i v1, __m512i v2)
{
  return __builtin_ia32_cmpeqpi512(v1, v2);
}
__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cmpeq_pi(__mmask16 k1, __m512i v1, __m512i v2)
{
  return __builtin_ia32_mask_cmpeqpi512(k1, v1, v2);
}

__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_cmplt_pi(__m512i v1, __m512i v2)
{
  return __builtin_ia32_cmpltpi512(v1, v2);
}
__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cmplt_pi(__mmask16 k1, __m512i v1, __m512i v2)
{
  return __builtin_ia32_mask_cmpltpi512(k1, v1, v2);
}

__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_cmple_pi(__m512i v1, __m512i v2)
{
  return __builtin_ia32_cmplepi512(v1, v2);
}
__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cmple_pi(__mmask16 k1, __m512i v1, __m512i v2)
{
  return __builtin_ia32_mask_cmplepi512(k1, v1, v2);
}

__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_cmpneq_pi(__m512i v1, __m512i v2)
{
  return __builtin_ia32_cmpneqpi512(v1, v2);
}
__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cmpneq_pi(__mmask16 k1, __m512i v1, __m512i v2)
{
  return __builtin_ia32_mask_cmpneqpi512(k1, v1, v2);
}

__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_cmpnlt_pi(__m512i v1, __m512i v2)
{
  return __builtin_ia32_cmpnltpi512(v1, v2);
}
__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cmpnlt_pi(__mmask16 k1, __m512i v1, __m512i v2)
{
  return __builtin_ia32_mask_cmpnltpi512(k1, v1, v2);
}

__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_cmpnle_pi(__m512i v1, __m512i v2)
{
  return __builtin_ia32_cmpnlepi512(v1, v2);
}
__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cmpnle_pi(__mmask16 k1, __m512i v1, __m512i v2)
{
  return __builtin_ia32_mask_cmpnlepi512(k1, v1, v2);
}

/* 32-bit unsigned integer compares. */

__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_cmpeq_pu(__m512i v1, __m512i v2)
{
  return __builtin_ia32_cmpeqpu512(v1, v2);
}
__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cmpeq_pu(__mmask16 k1, __m512i v1, __m512i v2)
{
  return __builtin_ia32_mask_cmpeqpu512(k1, v1, v2);
}

__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_cmplt_pu(__m512i v1, __m512i v2)
{
  return __builtin_ia32_cmpltpu512(v1, v2);
}
__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cmplt_pu(__mmask16 k1, __m512i v1, __m512i v2)
{
  return __builtin_ia32_mask_cmpltpu512(k1, v1, v2);
}

__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_cmple_pu(__m512i v1, __m512i v2)
{
  return __builtin_ia32_cmplepu512(v1, v2);
}
__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cmple_pu(__mmask16 k1, __m512i v1, __m512i v2)
{
  return __builtin_ia32_mask_cmplepu512(k1, v1, v2);
}

__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_cmpneq_pu(__m512i v1, __m512i v2)
{
  return __builtin_ia32_cmpneqpu512(v1, v2);
}
__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cmpneq_pu(__mmask16 k1, __m512i v1, __m512i v2)
{
  return __builtin_ia32_mask_cmpneqpu512(k1, v1, v2);
}

__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_cmpnlt_pu(__m512i v1, __m512i v2)
{
  return __builtin_ia32_cmpnltpu512(v1, v2);
}
__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cmpnlt_pu(__mmask16 k1, __m512i v1, __m512i v2)
{
  return __builtin_ia32_mask_cmpnltpu512(k1, v1, v2);
}

__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_cmpnle_pu(__m512i v1, __m512i v2)
{
  return __builtin_ia32_cmpnlepu512(v1, v2);
}
__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cmpnle_pu(__mmask16 k1, __m512i v1, __m512i v2)
{
  return __builtin_ia32_mask_cmpnlepu512(k1, v1, v2);
}

/*
 * Convert and insert float32 vector to float11:11:10 vector
 *    Performs an element-by-element conversion and rounding from the float32
 *    vector v2 to a float11 or float10 vector, depending on the 'field' being
 *    inserted.  v1 is input because of insertion mask and writemasking.
 *
 *    Rounding control values (rounding) can be one of the following:
 *         _MM_FROUND_TO_NEAREST_INT,
 *         _MM_FROUND_TO_ZERO
 */
#define _mm512_cvtins_ps2f11(v1, v2, rounding, field) \
  _mm512_cvtins_roundps_f11((v1), (v2), (rounding), (field))
#define _mm512_mask_cvtins_ps2f11(v1, k1, v2, rounding, field) \
  _mm512_mask_cvtins_roundps_f11((v1), (k1), (v2), (rounding), (field))

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_cvtins_roundps_f11(__m512 v1, __m512 v2,
                          const int rounding,
                          const _MM_FLOAT11_FIELD_ENUM field)
{
  return __builtin_ia32_cvtinsps2f11512(v1, v2, rounding, field);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cvtins_roundps_f11(__m512 v1, __mmask16 k1, __m512 v2,
                               const int rounding,
                               const _MM_FLOAT11_FIELD_ENUM field)
{
  return __builtin_ia32_mask_cvtinsps2f11512(v1, k1, v2, rounding, field);
}

/*
 * Convert and insert float32 vector to unorm10:10:10:2 vector
 *    Performs an element-by-element conversion from the float32 vector 'v2'
 *    to a unorm10 or unorm2 vector, depending on the 'field' being inserted.
 *    The result is in the form unorm10:10:10:2.
 *    v1 is input because of insertion mask and writemasking.
 */
#define _mm512_cvtins_ps2u10(v1, v2, field) \
  _mm512_cvtinsps_u10((v1), (v2), (field))
#define _mm512_mask_cvtins_ps2u10(v1, k1, v2, field) \
  _mm512_mask_cvtinsps_u10((v1), (k1), (v2), (field))

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_cvtinsps_u10(__m512 v1, __m512 v2, const _MM_UNORM10_FIELD_ENUM field)
{
  return __builtin_ia32_cvtinsps2u10512(v1, v2, field);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cvtinsps_u10(__m512 v1, __mmask16 k1, __m512 v2, const _MM_UNORM10_FIELD_ENUM field)
{
  return __builtin_ia32_mask_cvtinsps2u10512(v1, k1, v2, field);
}

/*
 * Convert float64 vector to float32, int32 or unsigned int32 vector.
 *    Performs an element-by-element conversion from the float64 vector 'v2'
 *    to a float32, int32 or unsigned int32 vector.
 *
 *    The "cvtl" intrinsics put the converted values in the low half of the
 *    result while the "cvth" intrinsics put them in the high half; the other
 *    half retains the value from v1_old.
 *
 *    Unconventional write mask note:
 *    For these write-masked "cvth" intrinsics, the *low* eight bits of the
 *    write mask control which of the high eight elements are written, and
 *    the high eight bits of the write mask are ignored.
 */

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_cvtl_pd2ps(__m512 v1_old, __m512d v2, const int rounding)
{
  return __builtin_ia32_cvtlpd2ps512(v1_old, v2, rounding);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_cvth_pd2ps(__m512 v1_old, __m512d v2, const int rounding)
{
  return __builtin_ia32_cvthpd2ps512(v1_old, v2, rounding);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cvtl_pd2ps(__m512 v1_old, __mmaskt k1, __m512d v2, const int rounding)
{
  return __builtin_ia32_mask_cvtlpd2ps512(v1_old, k1, v2, rounding);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cvth_pd2ps(__m512 v1_old, __mmaskt k1, __m512d v2, const int rounding)
{
  return __builtin_ia32_mask_cvthpd2ps512(v1_old, k1, v2, rounding);
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_cvtl_pd2pi(__m512i v1_old, __m512d v2, const int rounding)
{
  return __builtin_ia32_cvtlpd2pi512(v1_old, v2, rounding);
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_cvth_pd2pi(__m512i v1_old, __m512d v2, const int rounding)
{
  return __builtin_ia32_cvthpd2pi512(v1_old, v2, rounding);
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cvtl_pd2pi(__m512i v1_old, __mmaskt k1, __m512d v2, const int rounding)
{
  return __builtin_ia32_mask_cvtlpd2pi512(v1_old, k1, v2, rounding);
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cvth_pd2pi(__m512i v1_old, __mmaskt k1, __m512d v2, const int rounding)
{
  return __builtin_ia32_mask_cvthpd2pi512(v1_old, k1, v2, rounding);
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_cvtl_pd2pu(__m512i v1_old, __m512d v2, const int rounding)
{
  return __builtin_ia32_cvtlpd2pu512(v1_old, v2, rounding);
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_cvth_pd2pu(__m512i v1_old, __m512d v2, const int rounding)
{
  return __builtin_ia32_cvthpd2pu512(v1_old, v2, rounding);
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cvtl_pd2pu(__m512i v1_old, __mmaskt k1, __m512d v2, const int rounding)
{
  return __builtin_ia32_mask_cvtlpd2pu512(v1_old, k1, v2, rounding);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cvth_pd2pu(__m512i v1_old, __mmaskt k1, __m512d v2, const int rounding)
{
  return __builtin_ia32_mask_cvthpd2pu512(v1_old, k1, v2, rounding);
}

/*
 * Convert float32 vector to float64, int32, unsigned int32 or srgb8 vector.
 * Conversions to int32 or unsigned int32 require a rounding mode and
 * an exponent adjustment.  When converting to float64, the "cvtl" and
 * "cvtps" intrinsics convert the lower 8 elements of the source, while
 * the "cvth" intrinsics convert the upper 8 elements.
 */
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_cvtl_ps2pd(__m512 v2)
{
  return __builtin_ia32_cvtlps2pd512(v2);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_cvth_ps2pd(__m512 v2)
{
  return __builtin_ia32_cvthps2pd512(v2);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cvtl_ps2pd(__m512d v1_old, __mmaskt k1, __m512 v2)
{
  return __builtin_ia32_mask_cvtlps2pd512(v1_old, k1, v2);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cvth_ps2pd(__m512d v1_old, __mmaskt k1, __m512 v2)
{
  return __builtin_ia32_mask_cvthps2pd512(v1_old, k1, v2);
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_cvt_ps2pi(__m512 v2, const int rounding, const _MM_EXP_ADJ_ENUM expadj)
{
  return __builtin_ia32_cvtps2pi512(v2, rounding, expadj);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cvt_ps2pi(__m512i v1_old, __mmask16 k1, __m512 v2, const int rounding, const _MM_EXP_ADJ_ENUM expadj)
{
  return __builtin_ia32_mask_cvtps2pi512(v1_old, k1, v2, rounding, expadj);
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_cvt_ps2pu(__m512 v2, int rounding, _MM_EXP_ADJ_ENUM expadj)
{
  return __builtin_ia32_cvtps2pu512(v2, rounding, expadj);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cvt_ps2pu(__m512i v1_old, __mmask16 k1, __m512 v2, int rounding, _MM_EXP_ADJ_ENUM expadj)
{
  return __builtin_ia32_mask_cvtps2pu512(v1_old, k1, v2, rounding, expadj);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_cvt_ps2srgb8(__m512 v2)
{
  return __builtin_ia32_cvtps2srgb8512(v2);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cvt_ps2srgb8(__m512 v1_old, __mmask16 k1, __m512 v2)
{
  return __builtin_ia32_mask_cvtps2srgb8512(v1_old, k1, v2);
}

#define _mm512_cvtps_srgb8(v2) _mm512_cvt_ps2srgb8((v2))
#define _mm512_mask_cvtps_srgb8(v1_old, k1, v2) \
    _mm512_mask_cvt_ps2srgb8((v1_old), (k1), (v2))

/*
 * Convert int32 or unsigned int32 vector to float32 or float64 vector.
 *
 * Conversions to float32 require an an exponent adjustment.
 * When converting to float64, the "cvtl", "epi32" and "epu32" intrinsics
 * intrinsics convert the lower 8 elements of the source, while
 * the "cvth" intrinsics convert the upper 8 elements.
 */

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_cvtl_pi2pd(__m512i v2)
{
  return __builtin_ia32_cvtlpi2pd512(v2);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_cvth_pi2pd(__m512i v2)
{
  return __builtin_ia32_cvthpi2pd512(v2);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cvtl_pi2pd(__m512d v1_old, __mmaskt k1, __m512i v2)
{
  return __builtin_ia32_mask_cvtlpi2pd512(v1_old, k1, v2);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cvth_pi2pd(__m512d v1_old, __mmaskt k1, __m512i v2)
{
  return __builtin_ia32_mask_cvthpi2pd512(v1_old, k1, v2);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_cvt_pi2ps(__m512i v2, const _MM_EXP_ADJ_ENUM expadj)
{
  return __builtin_ia32_cvtpi2ps512(v2, expadj);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cvt_pi2ps(__m512 v1_old, __mmask16 k1, __m512i v2, const _MM_EXP_ADJ_ENUM expadj)
{
  return __builtin_ia32_mask_cvtpi2ps512(v1_old, k1, v2, expadj);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_cvtl_pu2pd(__m512i v2)
{
  return __builtin_ia32_cvtlpu2pd512(v2);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_cvth_pu2pd(__m512i v2)
{
  return __builtin_ia32_cvthpu2pd512(v2);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cvtl_pu2pd(__m512d v1_old, __mmaskt k1, __m512i v2)
{
  return __builtin_ia32_mask_cvtlpu2pd512(v1_old, k1, v2);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cvth_pu2pd(__m512d v1_old, __mmaskt k1, __m512i v2)
{
  return __builtin_ia32_mask_cvthpu2pd512(v1_old, k1, v2);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_cvt_pu2ps(__m512i v2, const _MM_EXP_ADJ_ENUM expadj)
{
  return __builtin_ia32_cvtpu2ps512(v2, expadj);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cvt_pu2ps(__m512 v1_old, __mmask16 k1, __m512i v2, const _MM_EXP_ADJ_ENUM expadj)
{
  return __builtin_ia32_mask_cvtpu2ps512(v1_old, k1, v2, expadj);
}

/*
 * Table look up assist for float32 vectors base-2 exponential calculation.
 *    The 5 least-significant bits of each element (which we'll denote as m)
 *    of vector 'v2' are used as an index into a 32-entry table in order to
 *    look up the corresponding float32 value 2**(m/32).
 */
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_exp2lut_ps(__m512i v2)
{
  return __builtin_ia32_exp2lutps512(v2);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_exp2lut_ps(__m512 v1_old, __mmask16 k1, __m512i v2)
{
  return __builtin_ia32_mask_exp2lutps512(v1_old, k1, v2);
}

/*
 * Fix up special float32 vector numbers.
 *    Performs an element-by-element fix-up of various real and special number
 *    types in the float32 vector 'v2', as specified by a 21-bit immediate
 *    table.
 */
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_fixup_ps(__m512 v1, __m512 v2, int imm21)
{
  return __builtin_ia32_fixupps512(v1, v2, imm21);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_fixup_ps(__m512 v1, __mmask16 k1, __m512 v2, int imm21)
{
  return __builtin_ia32_mask_fixupps512(v1, k1, v2, imm21);
}

/*
 * Gather element vector.
 *
 *    A set of memory locations pointed by base address 'mv' and
 *    index vector 'index' with scale 'scale' are converted to a doubleword
 *    vector.
 *
 *    Note the special mask behavior as only a subset of the active elements
 *    of write mask 'k1' are actually operated on.
 *    This instruction does not have broadcast support.
 *    Note also the special mask behavior as the corresponding bits in
 *    write mask 'k1' are reset with each destination element being updated
 *    according to the subset of write mask 'k1'. This is useful to allow
 *    conditional re-trigger of the instruction until all the elements
 *    from a given write mask have been successfully loaded.
 */
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_gatherd_step(__m512 v1_old, __mmask16 k1,
                    __m512i index,
                    void const *mv,
                    _MM_FULLUP32_ENUM up_conv,
                    _MM_INDEX_SCALE_ENUM scale,
                    _MM_MEM_HINT_ENUM nt,
                    __mmask16 *res_mask)
{
  return __builtin_ia32_gatherstepps512(v1_old, k1, index, mv, up_conv, scale, nt, res_mask);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_gatherd(__m512i index, void const *mv,
               _MM_FULLUP32_ENUM up_conv,
               _MM_INDEX_SCALE_ENUM scale,
               _MM_MEM_HINT_ENUM nt)
{
  return __builtin_ia32_gatherps512(index, mv, up_conv, scale, nt);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_gatherd(__m512 v1_old, __mmask16 k1,
                    __m512i index, void const *mv,
                    _MM_FULLUP32_ENUM up_conv,
                    _MM_INDEX_SCALE_ENUM scale,
                    _MM_MEM_HINT_ENUM nt)
{
  return __builtin_ia32_mask_gatherps512(v1_old, k1, index, mv, up_conv, scale, nt);
}

/*
 * Gather Prefetch Doubleword Vector.
 */
__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_gatherpfd_step(__m512i index, __mmask16 k1,
                      void const *mv,
                      _MM_FULLUP32_ENUM up_conv,
                      _MM_INDEX_SCALE_ENUM scale,
                      _MM_MEM_HINT_ENUM nt)
{
  return __builtin_ia32_gatherpfstepps512(index, k1, mv, up_conv, scale, nt);
}

__inline__ void __attribute__((__always_inline__, __nodebug__))
_mm512_gatherpfd(__m512i index, void const *mv,
                 _MM_FULLUP32_ENUM up_conv,
                 _MM_INDEX_SCALE_ENUM scale,
                 _MM_MEM_HINT_ENUM nt)
{
  __builtin_ia32_gatherpfps512(index, mv, up_conv, scale, nt);
}

__inline__ void __attribute__((__always_inline__, __nodebug__))
_mm512_mask_gatherpfd(__m512i index, __mmask16 k1,
                      void const *mv,
                      _MM_FULLUP32_ENUM up_conv,
                      _MM_INDEX_SCALE_ENUM scale,
                      _MM_MEM_HINT_ENUM nt)
{
  __builtin_ia32_mask_gatherpfps512(index, k1, mv, up_conv, scale, nt);
}

/*
 * Scatter element vector.
 *
 *    Downconverts and stores elements in doubleword vector 'v1' to
 *    the memory locations pointed by base address 'mv' and index vector
 *    'index', with scale 'scale'.
 *    Only a subset of the active elements of write mask 'k1' are actually
 *    operated on.  There are only two guarantees about the function:
 *      (a) the result mask is a subset of the source mask (identity is
 *          included)
 *      (b) on a given invocation of the function, at least one element
 *          will be selected from the source mask
 *
 * Parameters:
 *    mv        - base address of memory locations
 *    index     - index vector
 *    scale     - scaling factor
 *    k1        - source mask
 *    v1        - data vector
 *    down_conv - type of down-conversion (one of _MM_DOWNC constants)
 *
 * Return:
 *    The result mask with the corresponding bits in write mask 'k1' reset
 *    with each destination element being updated according to the subset
 *    of write mask 'k1'.  This is useful to allow conditional re-trigger of
 *    the instruction until all the elements from a given write mask have been
 *    successfully stored.
 */
__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_scatterd_step(void *mv, __mmask16 k1,
                     __m512i index, __m512 v1,
                     _MM_DOWNCONV32_ENUM down_conv,
                     _MM_INDEX_SCALE_ENUM scale,
                     _MM_MEM_HINT_ENUM nt)
{
  return __builtin_ia32_scatterstepps512(mv, k1, index, v1, down_conv, scale, nt);
}

/*
 * Scatter all element vector.
 *
 *    Downconverts and stores all elements in doubleword vector 'v1' to
 *    the memory locations pointed by base address 'mv' and index vector
 *    'index', with scale 'scale'.
 *    This function performs several iterations of vscatter until all
 *    the elements from a given write mask have been successfully stored:
 *
 *     __mmask16 res = k1;
 *     do {
 *        res = _mm512_vscatterd(mv, res, index, v1, down_conv, scale);
 *     } while (res != 0)
 *
 * Parameters:
 *    mv        - base address of memory locations
 *    index     - index vector
 *    scale     - scaling factor
 *    k1        - source mask
 *    v1        - data vector
 *    down_conv - type of down-conversion (one of _MM_DOWNC constants)
 */
__inline__ void __attribute__((__always_inline__, __nodebug__))
_mm512_scatterd(void *mv, __m512i index, __m512 v1,
                _MM_DOWNCONV32_ENUM down_conv,
                _MM_INDEX_SCALE_ENUM scale,
                _MM_MEM_HINT_ENUM nt)
{
  __builtin_ia32_scatterps512(mv, index, v1, down_conv, scale, nt);
}

__inline__ void __attribute__((__always_inline__, __nodebug__))
_mm512_mask_scatterd(void *mv, __mmask16 k1, __m512i index,
                     __m512 v1,
                     _MM_DOWNCONV32_ENUM down_conv,
                     _MM_INDEX_SCALE_ENUM scale,
                     _MM_MEM_HINT_ENUM nt)
{
  __builtin_ia32_mask_scatterps512(mv, k1, index, v1, down_conv, scale, nt);
}

/*
 * Scatter prefetch element vector.
 *
 *    Prefetches into the L1 level of cache the memory locations pointed by
 *    base address 'mv' and index vector 'index', with scale 'scale', with
 *    request for ownership (exclusive).  Downconversion operand specifies the
 *    granularity used by compilers to better encode the instruction if
 *    a displacement, using disp8*N feature, is provided when specifying the
 *    address.  If any memory access causes any type of memory exception,
 *    the memory access will be considered as completed (destination mask
 *    updated) and the exception ignored.  Downconversion parameter is
 *    optional, and it is used to correctly encode disp8*N.
 *
 *    Note the special mask behavior as the corresponding bits in
 *    write mask 'k1' are reset with each destination element being updated
 *    according to the subset of write mask 'k1'.  This is useful to allow
 *    conditional re-trigger of the instruction until all the elements from
 *    a given write mask have been successfully stored.
 */
__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_scatterpfd_step(void *mv, __mmask16 k1,
                       __m512i index,
                       _MM_DOWNCONV32_ENUM down_conv,
                       _MM_INDEX_SCALE_ENUM scale,
                       _MM_MEM_HINT_ENUM nt)
{
  return __builtin_ia32_scatterpfstepps512(mv, k1, index, down_conv, scale, nt);
}

__inline__ void __attribute__((__always_inline__, __nodebug__))
_mm512_scatterpfd(void *mv, __m512i index,
                  _MM_DOWNCONV32_ENUM down_conv,
                  _MM_INDEX_SCALE_ENUM scale,
                  _MM_MEM_HINT_ENUM nt)
{
  __builtin_ia32_scatterpfps512(mv, index, down_conv, scale, nt);
}

__inline__ void __attribute__((__always_inline__, __nodebug__))
_mm512_mask_scatterpfd(void *mv, __mmask16 k1,
                       __m512i index,
                       _MM_DOWNCONV32_ENUM down_conv,
                       _MM_INDEX_SCALE_ENUM scale,
                       _MM_MEM_HINT_ENUM nt)
{
  __builtin_ia32_mask_scatterpfps512(mv, k1, index, down_conv, scale, nt);
}

/*
 * Extract float32 vector of exponents.
 *    Perform an element-by-element exponent extraction from the float32
 *    vector 'v2'.
 */
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_getexp_ps(__m512 v2)
{
  return __builtin_ia32_getexpps512(v2);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_getexp_ps(__m512 v1_old, __mmask16 k1, __m512 v2)
{
  return __builtin_ia32_mask_getexpps512(v1_old, k1, v2);
}

/*
 * Rotate int32 vector and bitfield-insert into int32 vector.
 *    Performs an element-by-element rotation and bitfield insertion from
 *    the int32 vector 'v3' into int32 vector 'v2'.
 */
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_insertfield_pi(__m512i v2, __m512i v3,
                      int rotation,
                      int bit_idx_low,
                      int bit_idx_high)
{
  return __builtin_ia32_insertfieldpi512(v2, v3, rotation, bit_idx_low, bit_idx_high);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_insertfield_pi(__m512i v1_old, __mmask16 k1,
                           __m512i v2, __m512i v3,
                           int rotation,
                           int bit_idx_low,
                           int bit_idx_high)
{
  return __builtin_ia32_mask_insertfieldpi512(v1_old, k1, v2, v3, rotation, bit_idx_low, bit_idx_high);
}

/*
 * Load unaligned high and unpack to doubleword vector.
 *    The high-64-byte portion of the byte/word/doubleword stream starting
 *    at the element-aligned address 'mt' is loaded, converted and expanded
 *    into the writemask-enabled elements of doubleword vector.
 *    Doubleword vector is returned.
 *
 *    The number of set bits in the writemask determines the length of the
 *    converted doubleword stream, as each converted doubleword is mapped
 *    to exactly one of the doubleword elements in returned vector, skipping
 *    over writemasked elements of 'v1_old'.
 */
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_loadunpackhd(__m512 v1_old, void const *mt,
                    _MM_FULLUP32_ENUM full_up,
                    _MM_MEM_HINT_ENUM nt)
{
  return __builtin_ia32_loadunpackhps512(v1_old, mt, full_up, nt);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_loadunpackhd(__m512 v1_old, __mmask16 k1,
                         void const *mt,
                         _MM_FULLUP32_ENUM full_up,
                         _MM_MEM_HINT_ENUM nt)
{
  return __builtin_ia32_mask_loadunpackhps512(v1_old, k1, mt, full_up, nt);
}

/*
 * Load unaligned high and unpack to quadword vector.
 *    The high-64-byte portion of the quadword stream starting
 *    at the element-aligned address 'mt' is loaded, converted and expanded
 *    into the writemask-enabled elements of quadword vector.
 *    Quadword vector is returned.
 *
 *    The number of set bits in the writemask determines the length of the
 *    converted doubleword stream, as each converted doubleword is mapped
 *    to exactly one of the doubleword elements in returned vector,
 *    skipping over writemasked elements of 'v1_old'.
 */
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_loadunpackhq(__m512d v1_old, void const *mt,
                    _MM_FULLUP64_ENUM full_up,
                    _MM_MEM_HINT_ENUM nt)
{
  return __builtin_ia32_loadunpackhpd512(v1_old, mt, full_up, nt);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_loadunpackhq(__m512d v1_old, __mmaskt k1,
                         void const *mt,
                         _MM_FULLUP64_ENUM full_up,
                         _MM_MEM_HINT_ENUM nt)
{
  return __builtin_ia32_mask_loadunpackhpd512(v1_old, k1, mt, full_up, nt);
}

/*
 * Load unaligned low and unpack to doubleword vector.
 *    The low-64-byte portion of the byte/word/doubleword stream starting
 *    at the element-aligned address 'mt' is loaded, converted and expanded
 *    into the writemask-enabled elements of doubleword resulting vector.
 *
 *    The number of set bits in the writemask determines the length of the
 *    converted doubleword stream, as each converted doubleword is mapped
 *    to exactly one of the doubleword elements in returned vector,
 *    skipping over writemasked elements of 'v1_old'.
 */
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_loadunpackld(__m512 v1_old, void const *mt,
                    _MM_FULLUP32_ENUM full_up,
                    _MM_MEM_HINT_ENUM nt)
{
  return __builtin_ia32_loadunpacklps512(v1_old, mt, full_up, nt);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_loadunpackld(__m512 v1_old, __mmask16 k1,
                         void const *mt,
                         _MM_FULLUP32_ENUM full_up,
                         _MM_MEM_HINT_ENUM nt)
{
  return __builtin_ia32_mask_loadunpacklps512(v1_old, k1, mt, full_up, nt);
}

/*
 * Load unaligned low and unpack to quadword vector.
 *    The low-64-byte portion of the quadword stream starting at the
 *    element-aligned address 'mt' is loaded, converted and expanded
 *    into the writemask-enabled elements of quadword resulting vector.
 *    The number of set bits in the writemask determines the length of
 *    the converted quadword stream, as each converted quadword is
 *    mapped to exactly one of the quadword elements in returned vector,
 *    skipping over writemasked elements of 'v1_old'.
 */
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_loadunpacklq(__m512d v1_old, void const *m,
                    _MM_FULLUP64_ENUM full_up,
                    _MM_MEM_HINT_ENUM nt)
{
  return __builtin_ia32_loadunpacklpd512(v1_old, m, full_up, nt);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_loadunpacklq(__m512d v1_old, __mmaskt k1,
                         void const *m,
                         _MM_FULLUP64_ENUM full_up,
                         _MM_MEM_HINT_ENUM nt)
{
  return __builtin_ia32_mask_loadunpacklpd512(v1_old, k1, m, full_up, nt);
}

/*
 * Pack and store dword or qword vector elements to high
 * 64-byte aligned stream.
 *
 *    Downconvert and pack mask-enabled elements of v1 to form a stream,
 *    logically map the stream starting at potentially unaligned address mt,
 *    and store that portion of the stream that maps at or after the first
 *    64-byte aligned address at or above mt.
 *
 *    The mask is not used as a writemask for this instruction.  Instead,
 *    the mask is used as an element selector, choosing which elements are
 *    added to the stream.  The no-writemask option is available to select
 *    a mask of 0xFFFF for this instruction.
 */
__inline__ void __attribute__((__always_inline__, __nodebug__))
_mm512_packstorehd(void *mt, __m512 v1,
                   _MM_DOWNCONV32_ENUM d,
                   _MM_MEM_HINT_ENUM nt)
{
  __builtin_ia32_packstorehps512(mt, v1, d, nt);
}

__inline__ void __attribute__((__always_inline__, __nodebug__))
_mm512_mask_packstorehd(void *mt, __mmask16 k1, __m512 v1,
                        _MM_DOWNCONV32_ENUM d,
                        _MM_MEM_HINT_ENUM nt)
{
  __builtin_ia32_mask_packstorehps512(mt, k1, v1, d, nt);
}

__inline__ void __attribute__((__always_inline__, __nodebug__))
_mm512_packstorehq(void *mt, __m512d v1,
                   _MM_DOWNCONV64_ENUM d,
                   _MM_MEM_HINT_ENUM nt)
{
  __builtin_ia32_packstorehpd512(mt, v1, d, nt);
}

__inline__ void __attribute__((__always_inline__, __nodebug__))
_mm512_mask_packstorehq(void *mt, __mmaskt k1, __m512d v1,
                        _MM_DOWNCONV64_ENUM d,
                        _MM_MEM_HINT_ENUM nt)
{
  __builtin_ia32_mask_packstorehpd512(mt, k1, v1, d, nt);
}

/*
 * Pack and store dword or qword vector elements to low 64-byte aligned stream.
 *
 *    Downconvert and pack mask-enabled elements of v1 to form a stream,
 *    logically map the stream starting at potentially unaligned address mt,
 *    and store that portion of the stream that maps below the first 64-byte
 *    aligned address at or above mt.
 *
 *    The mask is not used as a writemask for this instruction.  Instead,
 *    the mask is used as an element selector, choosing which elements are
 *    added to the stream.  The no-writemask option is available to select
 *    a mask of 0xFFFF for this instruction.
 */

__inline__ void __attribute__((__always_inline__, __nodebug__))
_mm512_packstoreld(void *mt, __m512 v1,
                   _MM_DOWNCONV32_ENUM d,
                   _MM_MEM_HINT_ENUM nt)
{
  __builtin_ia32_packstorelps512(mt, v1, d, nt);
}

__inline__ void __attribute__((__always_inline__, __nodebug__))
_mm512_mask_packstoreld(void *mt, __mmask16 k1, __m512 v1,
                        _MM_DOWNCONV32_ENUM d,
                        _MM_MEM_HINT_ENUM nt)
{
  __builtin_ia32_mask_packstorelps512(mt, k1, v1, d, nt);
}

__inline__ void __attribute__((__always_inline__, __nodebug__))
_mm512_packstorelq(void *mt, __m512d v1,
                   _MM_DOWNCONV64_ENUM d,
                   _MM_MEM_HINT_ENUM nt)
{
  __builtin_ia32_packstorelpd512(mt, v1, d, nt);
}

__inline__ void __attribute__((__always_inline__, __nodebug__))
_mm512_mask_packstorelq(void *mt, __mmaskt k1, __m512d v1,
                        _MM_DOWNCONV64_ENUM d,
                        _MM_MEM_HINT_ENUM nt)
{
  __builtin_ia32_mask_packstorelpd512(mt, k1, v1, d, nt);
}

/*
 * Look up logarithm base-2 of float32 vector.
 *    The 6 most-significant mantissa bits of each element of the float32
 *    vector 'v2' are used as an index into a 64-entry table in order to
 *    look up the corresponding float32 value Log2(m).
 */
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_log2lut_ps(__m512 v2)
{
  return __builtin_ia32_log2lut_ps512(v2);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_log2lut_ps(__m512 v1_old, __mmask16 k1, __m512 v2)
{
  return __builtin_ia32_mask_log2lut_ps512(v1_old, k1, v2);
}

/*
 * Fused multiply and add of float32, float64 or int32 vectors, with order.
 *
 * Given vector parameters in the order v1, v2, v3, this group
 * of FMA instructions computes the following
 *
 *  madd132     (v1 * v3) + v2
 *  madd213     (v2 * v1) + v3
 *  madd231     (v2 * v3) + v1
 *
 * The "maddn" family negates the result:
 *
 *  maddn132    -((v1 * v3) + v2)
 *
 * The "msub" family performs a subtract instead of an add:
 *
 *  msub132     (v1 * v3) - v2
 *
 * The "msubr" family reverses the operands of the subtract:
 *
 *  msubr132    v2 - (v1 * v3)
 *
 * The "msubr23c1" family uses the constant 1.0 in place of v1:
 *
 *  msubr231c1  1.0 - (v2 * v3)
 */

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_madd132_pd(__m512d v1, __m512d v2, __m512d v3)
{
  return __builtin_ia32_madd132pd512(v1, v2, v3);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_madd132_pd(__m512d v1, __mmaskt k1, __m512d v2, __m512d v3)
{
  return __builtin_ia32_mask_madd132pd512(v1, k1, v2, v3);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_madd132_ps(__m512 v1, __m512 v2, __m512 v3)
{
  return __builtin_ia32_madd132ps512(v1, v2, v3);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_madd132_ps(__m512 v1, __mmask16 k1, __m512 v2, __m512 v3)
{
  return __builtin_ia32_mask_madd132ps512(v1, k1, v2, v3);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_madd213_pd(__m512d v1, __m512d v2, __m512d v3)
{
  return __builtin_ia32_madd213pd512(v1, v2, v3);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_madd213_pd(__m512d v1, __mmaskt k1, __m512d v2, __m512d v3)
{
  return __builtin_ia32_mask_madd213pd512(v1, k1, v2, v3);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_madd213_ps(__m512 v1, __m512 v2, __m512 v3)
{
  return __builtin_ia32_madd213ps512(v1, v2, v3);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_madd213_ps(__m512 v1, __mmask16 k1, __m512 v2, __m512 v3)
{
  return __builtin_ia32_mask_madd213ps512(v1, k1, v2, v3);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_madd231_pd(__m512d v1, __m512d v2, __m512d v3)
{
  return __builtin_ia32_madd231pd512(v1, v2, v3);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_madd231_pd(__m512d v1, __mmaskt k1, __m512d v2, __m512d v3)
{
  return __builtin_ia32_mask_madd231pd512(v1, k1, v2, v3);
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_madd231_pi(__m512i v1, __m512i v2, __m512i v3)
{
  return __builtin_ia32_madd231pi512(v1, v2, v3);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_madd231_pi(__m512i v1, __mmask16 k1, __m512i v2, __m512i v3)
{
  return __builtin_ia32_mask_madd231pi512(v1, k1, v2, v3);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_madd231_ps(__m512 v1, __m512 v2, __m512 v3)
{
  return __builtin_ia32_madd231ps512(v1, v2, v3);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_madd231_ps(__m512 v1, __mmask16 k1, __m512 v2, __m512 v3)
{
  return __builtin_ia32_mask_madd231ps512(v1, k1, v2, v3);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_maddn132_pd(__m512d v1, __m512d v2, __m512d v3)
{
  return __builtin_ia32_maddn132pd512(v1, v2, v3);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_maddn132_pd(__m512d v1, __mmaskt k1, __m512d v2, __m512d v3)
{
  return __builtin_ia32_mask_maddn132pd512(v1, k1, v2, v3);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_maddn132_ps(__m512 v1, __m512 v2, __m512 v3)
{
  return __builtin_ia32_maddn132ps512(v1, v2, v3);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_maddn132_ps(__m512 v1, __mmask16 k1, __m512 v2, __m512 v3)
{
  return __builtin_ia32_mask_maddn132ps512(v1, k1, v2, v3);
}


__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_maddn213_pd(__m512d v1, __m512d v2, __m512d v3)
{
  return __builtin_ia32_maddn213pd512(v1, v2, v3);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_maddn213_pd(__m512d v1, __mmaskt k1, __m512d v2, __m512d v3)
{
  return __builtin_ia32_mask_maddn213pd512(v1, k1, v2, v3);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_maddn213_ps(__m512 v1, __m512 v2, __m512 v3)
{
  return __builtin_ia32_maddn213ps512(v1, v2, v3);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_maddn213_ps(__m512 v1, __mmask16 k1, __m512 v2, __m512 v3)
{
  return __builtin_ia32_mask_maddn213ps512(v1, k1, v2, v3);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_maddn231_pd(__m512d v1, __m512d v2, __m512d v3)
{
  return __builtin_ia32_maddn231pd512(v1, v2, v3);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_maddn231_pd(__m512d v1, __mmaskt k1, __m512d v2, __m512d v3)
{
  return __builtin_ia32_mask_maddn231pd512(v1, k1, v2, v3);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_maddn231_ps(__m512 v1, __m512 v2, __m512 v3)
{
  return __builtin_ia32_maddn231ps512(v1, v2, v3);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_maddn231_ps(__m512 v1, __mmask16 k1, __m512 v2, __m512 v3)
{
  return __builtin_ia32_mask_maddn231ps512(v1, k1, v2, v3);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_msub132_pd(__m512d v1, __m512d v2, __m512d v3)
{
  return __builtin_ia32_msub132pd512(v1, v2, v3);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_msub132_pd(__m512d v1, __mmaskt k1, __m512d v2, __m512d v3)
{
  return __builtin_ia32_mask_msub132pd512(v1, k1, v2, v3);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_msub132_ps(__m512 v1, __m512 v2, __m512 v3)
{
  return __builtin_ia32_msub132ps512(v1, v2, v3);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_msub132_ps(__m512 v1, __mmask16 k1, __m512 v2, __m512 v3)
{
  return __builtin_ia32_mask_msub132ps512(v1, k1, v2, v3);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_msub213_pd(__m512d v1, __m512d v2, __m512d v3)
{
  return __builtin_ia32_msub213pd512(v1, v2, v3);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_msub213_pd(__m512d v1, __mmaskt k1, __m512d v2, __m512d v3)
{
  return __builtin_ia32_mask_msub213pd512(v1, k1, v2, v3);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_msub213_ps(__m512 v1, __m512 v2, __m512 v3)
{
  return __builtin_ia32_msub213ps512(v1, v2, v3);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_msub213_ps(__m512 v1, __mmask16 k1, __m512 v2, __m512 v3)
{
  return __builtin_ia32_mask_msub213ps512(v1, k1, v2, v3);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_msub231_pd(__m512d v1, __m512d v2, __m512d v3)
{
  return __builtin_ia32_msub231pd512(v1, v2, v3);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_msub231_pd(__m512d v1, __mmaskt k1, __m512d v2, __m512d v3)
{
  return __builtin_ia32_mask_msub231pd512(v1, k1, v2, v3);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_msub231_ps(__m512 v1, __m512 v2, __m512 v3)
{
  return __builtin_ia32_msub231ps512(v1, v2, v3);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_msub231_ps(__m512 v1, __mmask16 k1, __m512 v2, __m512 v3)
{
  return __builtin_ia32_mask_msub231ps512(v1, k1, v2, v3);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_msubr132_pd(__m512d v1, __m512d v2, __m512d v3)
{
  return __builtin_ia32_msubr132pd512(v1, v2, v3);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_msubr132_pd(__m512d v1, __mmaskt k1, __m512d v2, __m512d v3)
{
  return __builtin_ia32_mask_msubr132pd512(v1, k1, v2, v3);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_msubr132_ps(__m512 v1, __m512 v2, __m512 v3)
{
  return __builtin_ia32_msubr132ps512(v1, v2, v3);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_msubr132_ps(__m512 v1, __mmask16 k1, __m512 v2, __m512 v3)
{
  return __builtin_ia32_mask_msubr132ps512(v1, k1, v2, v3);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_msubr213_pd(__m512d v1, __m512d v2, __m512d v3)
{
  return __builtin_ia32_msubr213pd512(v1, v2, v3);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_msubr213_pd(__m512d v1, __mmaskt k1, __m512d v2, __m512d v3)
{
  return __builtin_ia32_mask_msubr213pd512(v1, k1, v2, v3);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_msubr213_ps(__m512 v1, __m512 v2, __m512 v3)
{
  return __builtin_ia32_msubr213ps512(v1, v2, v3);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_msubr213_ps(__m512 v1, __mmask16 k1, __m512 v2, __m512 v3)
{
  return __builtin_ia32_mask_msubr213ps512(v1, k1, v2, v3);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_msubr231_pd(__m512d v1, __m512d v2, __m512d v3)
{
  return __builtin_ia32_msubr231pd512(v1, v2, v3);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_msubr231_pd(__m512d v1, __mmaskt k1, __m512d v2, __m512d v3)
{
  return __builtin_ia32_mask_msubr231pd512(v1, k1, v2, v3);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_msubr231_ps(__m512 v1, __m512 v2, __m512 v3)
{
  return __builtin_ia32_msubr231ps512(v1, v2, v3);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_msubr231_ps(__m512 v1, __mmask16 k1, __m512 v2, __m512 v3)
{
  return __builtin_ia32_mask_msubr231ps512(v1, k1, v2, v3);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_msubr23c1_pd(__m512d v2, __m512d v3)
{
  return __builtin_ia32_msubr23c1pd512(v2, v3);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_msubr23c1_pd(__m512d v1_old, __mmaskt k1, __m512d v2, __m512d v3)
{
  return __builtin_ia32_mask_msubr23c1pd512(v1_old, k1, v2, v3);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_msubr23c1_ps(__m512 v2, __m512 v3)
{
  return __builtin_ia32_msubr23c1ps512(v2, v3);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_msubr23c1_ps(__m512 v1_old, __mmask16 k1, __m512 v2, __m512 v3)
{
  return __builtin_ia32_mask_msubr23c1ps512(v1_old, k1, v2, v3);
}

/*
 * Multiply and add int32 or float32 vectors with alternating elements.
 *
 *    Multiply vector v2 by certain elements of vector v3, and add that
 *    result to certain other elements of v3.
 *
 *    This intrinsic is built around the concept of 4-element sets, of which
 *    there are four elements 0-3, 4-7, 8-11, and 12-15.
 *    Each element 0-3 of vector v2 is multiplied by element 1 of v3,
 *    the result is added to element 0 of v3, and the final sum is written
 *    into the corresponding element 0-3 of the result vector.
 *    Similarly each element 4-7 of v2 is multiplied by element 5 of v3,
 *    and added to element 4 of v3.
 *    Each element 8-11 of v2 is multiplied by element 9 of v3,
 *    and added to element 8 of v3.
 *    Each element 12-15 of vector v2 is multiplied by element 13 of v3,
 *    and added to element 12 of v3.
 */
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_madd233_pi(__m512i v2, __m512i v3)
{
  return __builtin_ia32_madd233pi512(v2, v3);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_madd233_pi(__m512i v1_old, __mmask16 k1, __m512i v2, __m512i v3)
{
  return __builtin_ia32_mask_madd233pi512(v1_old, k1, v2, v3);
}
#define _mm512_fmadd233_epi32(v2, v3) \
  _mm512_madd233_pi((v2), (v3))
#define _mm512_mask_fmadd233_epi32(v1_old, k1, v2, v3) \
  _mm512_mask_madd233_pi((v1_old), (k1), (v2), (v3))

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_madd233_ps(__m512 v2, __m512 v3)
{
  return __builtin_ia32_madd233ps512(v2, v3);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_madd233_ps(__m512 v1_old, __mmask16 k1, __m512 v2, __m512 v3)
{
  return __builtin_ia32_mask_madd233ps512(v1_old, k1, v2, v3);
}

/*
 * Minimum or maximum of float32, float64, int32 or unsigned int32 vectors.
 *
 * gmaxabs returns maximum of absolute values of source operands.
 * gmax, gmaxabs and gmin have DX10 and IEEE 754R semantics:
 *
 * gmin     dest = src0 < src1 ? src0 : src1
 * gmax:    dest = src0 >= src1 ? src0 : src1
 *          >= is used instead of > so that
 *          if gmin(x,y) = x then gmax(x,y) = y.
 *
 *    NaN has special handling: If one source operand is NaN, then the other
 *    source operand is returned (choice made per-component).  If both are NaN,
 *    then the quietized NaN from the first source is returned.
 */

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_max_ps(__m512 v2, __m512 v3)
{
  return __builtin_ia32_maxps512(v2, v3);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_max_ps(__m512 v1_old, __mmask16 k1, __m512 v2, __m512 v3)
{
  return __builtin_ia32_mask_maxps512(v1_old, k1, v2, v3);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_maxabs_ps(__m512 v2, __m512 v3)
{
  return __builtin_ia32_maxabsps512(v2, v3);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_maxabs_ps(__m512 v1_old, __mmask16 k1, __m512 v2, __m512 v3)
{
  return __builtin_ia32_mask_maxabsps512(v1_old, k1, v2, v3);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_max_pd(__m512d v2, __m512d v3)
{
  return __builtin_ia32_maxpd512(v2, v3);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_max_pd(__m512d v1_old, __mmaskt k1, __m512d v2, __m512d v3)
{
  return __builtin_ia32_mask_maxpd512(v1_old, k1, v2, v3);
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_max_epi32(__m512i v2, __m512i v3)
{
  return __builtin_ia32_maxpi512(v2, v3);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_max_epi32(__m512i v1_old, __mmask16 k1, __m512i v2, __m512i v3)
{
  return __builtin_ia32_mask_maxpi512(v1_old, k1, v2, v3);
}
#define _mm512_max_pi(v2, v3) \
  _mm512_max_epi32((v2), (v3))
#define _mm512_mask_max_pi(v1_old, k1, v2, v3) \
  _mm512_mask_max_epi32((v1_old), (k1), (v2), (v3))

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_max_epu32(__m512i v2,__m512i v3)
{
  return __builtin_ia32_maxpu512(v2, v3);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_max_epu32(__m512i v1_old, __mmask16 k1, __m512i v2, __m512i v3)
{
  return __builtin_ia32_mask_maxpu512(v1_old, k1, v2, v3);
}
#define _mm512_max_pu(v2, v3) \
  _mm512_max_epu32((v2), (v3))
#define _mm512_mask_max_pu(v1_old, k1, v2, v3) \
  _mm512_mask_max_epu32((v1_old), (k1), (v2), (v3))

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_min_ps(__m512 v2, __m512 v3)
{
  return __builtin_ia32_minps512(v2, v3);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_min_ps(__m512 v1_old, __mmask16 k1, __m512 v2, __m512 v3)
{
  return __builtin_ia32_mask_minps512(v1_old, k1, v2, v3);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_min_pd(__m512d v2,__m512d v3)
{
  return __builtin_ia32_minpd512(v2, v3);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_min_pd(__m512d v1_old, __mmaskt k1, __m512d v2,__m512d v3)
{
  return __builtin_ia32_mask_minpd512(v1_old, k1, v2, v3);
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_min_epi32(__m512i v2, __m512i v3)
{
  return __builtin_ia32_minpi512(v2, v3);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_min_epi32(__m512i v1_old, __mmask16 k1, __m512i v2, __m512i v3)
{
  return __builtin_ia32_mask_minpi512(v1_old, k1, v2, v3);
}
#define _mm512_min_pi(v2, v3) \
  _mm512_min_epi32((v2), (v3))
#define _mm512_mask_min_pi(v1_old, k1, v2, v3) \
  _mm512_mask_min_epi32((v1_old), (k1), (v2), (v3))

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_min_epu32(__m512i v2, __m512i v3)
{
  return __builtin_ia32_minpu512(v2, v3);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_min_epu32(__m512i v1_old, __mmask16 k1, __m512i v2, __m512i v3)
{
  return __builtin_ia32_mask_minpu512(v1_old, k1, v2, v3);
}
#define _mm512_min_pu(v2, v3) \
  _mm512_min_epu32((v2), (v3))
#define _mm512_mask_min_pu(v1_old, k1, v2, v3) \
  _mm512_mask_min_epu32((v1_old), (k1), (v2), (v3))

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_gmax_ps(__m512 v2, __m512 v3)
{
  return __builtin_ia32_gmaxps512(v2, v3);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_gmax_ps(__m512 v1_old, __mmask16 k1, __m512 v2, __m512 v3)
{
  return __builtin_ia32_mask_gmaxps512(v1_old, k1, v2, v3);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_gmaxabs_ps(__m512 v2, __m512 v3)
{
  return __builtin_ia32_gmaxabsps512(v2, v3);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_gmaxabs_ps(__m512 v1_old, __mmask16 k1, __m512 v2, __m512 v3)
{
  return __builtin_ia32_mask_gmaxabsps512(v1_old, k1, v2, v3);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_gmax_pd(__m512d v2, __m512d v3)
{
  return __builtin_ia32_gmaxpd512(v2, v3);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_gmax_pd(__m512d v1_old, __mmaskt k1, __m512d v2, __m512d v3)
{
  return __builtin_ia32_mask_gmaxpd512(v1_old, k1, v2, v3);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_gmin_ps(__m512 v2, __m512 v3)
{
  return __builtin_ia32_gminps512(v2, v3);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_gmin_ps(__m512 v1_old, __mmask16 k1, __m512 v2, __m512 v3)
{
  return __builtin_ia32_mask_gminps512(v1_old, k1, v2, v3);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_gmin_pd(__m512d v2, __m512d v3)
{
  return __builtin_ia32_gminpd512(v2, v3);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_gmin_pd(__m512d v1_old, __mmaskt k1, __m512d v2, __m512d v3)
{
  return __builtin_ia32_mask_gminpd512(v1_old, k1, v2, v3);
}

/*
 * Multiply int32 or unsigned int32 vectors, and select the high or low
 * half of the 64-bit result.
 */

#define _mm512_mulh_pi(v2, v3) \
  _mm512_mulhi_epi32((v2), (v3))
#define _mm512_mask_mulh_pi(v1_old, k1, v2, v3) \
  _mm512_mask_mulhi_epi32((v1_old), (k1), (v2), (v3))
#define _mm512_mulh_pu(v2, v3) \
  _mm512_mulhi_epu32((v2), (v3))
#define _mm512_mask_mulh_pu(v1_old, k1, v2, v3) \
  _mm512_mask_mulhi_epu32((v1_old), (k1), (v2), (v3))
#define _mm512_mull_pi(v2, v3) \
  _mm512_mullo_epi32((v2), (v3))
#define _mm512_mask_mull_pi(v1_old, k1, v2, v3) \
  _mm512_mask_mullo_epi32((v1_old), (k1), (v2), (v3))

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mulhi_epi32(__m512i v2, __m512i v3)
{
  return __builtin_ia32_mulhpi512(v2, v3);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_mulhi_epi32(__m512i v1_old, __mmask16 k1, __m512i v2, __m512i v3)
{
  return __builtin_ia32_mask_mulhpi512(v1_old, k1, v2, v3);
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mulhi_epu32(__m512i v2, __m512i v3)
{
  return __builtin_ia32_mulhpu512(v2, v3);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_mulhi_epu32(__m512i v1_old, __mmask16 k1, __m512i v2, __m512i v3)
{
  return __builtin_ia32_mask_mulhpu512(v1_old, k1, v2, v3);
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mullo_epi32(__m512i v2, __m512i v3)
{
  return __builtin_ia32_mullpi512(v2, v3);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_mullo_epi32(__m512i v1_old, __mmask16 k1, __m512i v2, __m512i v3)
{
  return __builtin_ia32_mask_mullpi512(v1_old, k1, v2, v3);
}

/*
 * Refine reciprocal of float32 vector.
 *    Performs an element-by-element refinement of the reciprocal approximation
 *    of the float32 vector 'v3', given the error (generally produced by
 *    VRCPRESPS) in float32 vector 'v2'.
 *
 *    If both element sources are NaN, then the first source quietized NaN
 *    is returned as result for that element.  If any element of 'v3' is NaN,
 *    NaN is returned for that element.
 */
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_rcprefine_ps(__m512 v2, __m512 v3)
{
  return __builtin_ia32_rcprefineps512(v2, v3);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_rcprefine_ps(__m512 v1_old, __mmask16 k1, __m512 v2, __m512 v3)
{
  return __builtin_ia32_mask_rcprefineps512(v1_old, k1, v2, v3);
}

/*
 * Reciprocal residual of float32 vector.
 *    Computes the element-by-element approximate-reciprocal residual
 *    of the float32 vector 'v2'.  The residual is a measure of the error
 *    in the reciprocal approximation, in the form of the difference between
 *    1.0 and the product of the approximate reciprocal times the original
 *    value (which would be 1.0 given an exact reciprocal).
 */
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_rcpres_ps(__m512 v2)
{
  return __builtin_ia32_rcpresps512(v2);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_rcpres_ps(__m512 v1_old, __mmask16 k1, __m512 v2)
{
  return __builtin_ia32_mask_rcpresps512(v1_old, k1, v2);
}

/*
 * Rotate and bitfield-mask int32 vector.
 *    Performs an element-by-element rotation and bitfield masking of the
 *    int32 vector 'v2'.
 */
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_rotatefield_pi(__m512i v2, const int rotation,
                      const int bit_idx_low,
                      const int bit_idx_high)
{
  return __builtin_ia32_rotatefieldpi512(v2, rotation, bit_idx_low, bit_idx_high);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_rotatefield_pi(__m512i v1_old, __mmask16 k1,
                           __m512i v2, const int rotation,
                           const int bit_idx_low,
                           const int bit_idx_high)
{
  return __builtin_ia32_mask_rotatefiledpi512(v1_old, k1, v2, rotation, bit_idx_low, bit_idx_high);
}

/*
 * Round float32 or float64 vector.
 *    Performs an element-by-element rounding of the float32 or float64
 *    vector 'v2'.  The rounding result for each element is a float32 or
 *    float64 containing an integer or fixed-point value, depending on the
 *    value of expadj; the direction of rounding depends on the value of 'rc'.
 */
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_round_ps(__m512 v2, const int rounding,
                const _MM_EXP_ADJ_ENUM expadj)
{
  return __builtin_ia32_roundps512(v2, rounding, expadj);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_round_ps(__m512 v1_old, __mmask16 k1,
                     __m512 v2, const int rounding,
                     const _MM_EXP_ADJ_ENUM expadj)
{
  return __builtin_ia32_mask_roundps512(v1_old, k1, v2, rounding, expadj);
}

/*
 * Table look up assist for float32 vector reciprocal square root calculation.
 *    The 6 most-significant mantissa bits and 1 least-significant exponent bit
 *    of each element (which we'll denote as m) of the float32 vector 'v2'
 *    are used as an index into a 128-entry table in order to look up
 *    the corresponding float32 value 1.0/(m**0.5).
 */
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_rsqrtlut_ps(__m512 v2)
{
  return __builtin_ia32_rsqrtlutps512(v2);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_rsqrtlut_ps(__m512 v1_old, __mmask16 k1, __m512 v2)
{
  return __builtin_ia32_mask_rsqrtlutps512(v1_old, k1, v2);
}

/*
 * Scale float32 vectors.
 *    Performs an element-by-element scale of float32 vector 'v2' by
 *    multiplying it by 2**exp, where exp is the vector 'v3'.
 *    The result is returned.
 *
 *    This instruction is needed for scaling u and v coordinates according to
 *    the mipmap size, which is 2**mipmap level, and for the evaluation of
 *    Exp2. Handling of cases where the exponent would go out of range are
 *    handled as if vmulps by 2**exp had been performed.
 */
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_scale_ps(__m512 v2, __m512i v3)
{
  return __builtin_ia32_scaleps512(v2, v3);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_scale_ps(__m512 v1_old, __mmask16 k1, __m512 v2, __m512i v3)
{
  return __builtin_ia32_mask_scaleps512(v1_old, k1, v2, v3);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_scale_round_ps(__m512 v2, __m512i v3, const int rounding)
{
  return __builtin_ia32_round_scaleps512(v2, v3, rounding);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_scale_round_ps(__m512 v1_old, __mmask16 k1,
                           __m512 v2, __m512i v3,
                           const int rounding)
{
  return __builtin_ia32_mask_round_scaleps512(v1_old, k1, v2, v3, rounding);
}

/*
 * Shuffle vector dqwords then doublewords.
 *    Shuffles 128-bit blocks of the vector read from vector 'v2',
 *    and then 32-bit blocks of the result.  The result of the second shuffle
 *    is returned.
 */
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_shuf128x32(__m512 v2, const _MM_PERM_ENUM perm128,
                  const _MM_PERM_ENUM perm32)
{
  return __builtin_ia32_shuf128x32(v2, perm128, perm32);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_shuf128x32(__m512 v1_old, __mmask16 k1,
                       __m512 v2, const _MM_PERM_ENUM perm128,
                       const _MM_PERM_ENUM perm32)
{
  return __builtin_ia32_mask_shuf128x32(v1_old, k1, v2, perm128, perm32);
}
/*
 * Shuffle vector dqwords then doublewords from memory.
 *    Shuffles 128-bit blocks of the vector read from memory 'mt',
 *    and then 32-bit blocks of the result.  The result of the second shuffle
 *    is returned.
 */
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_shuf128x32_m(void const* mt,
                    const _MM_PERM_ENUM perm128,
                    const _MM_PERM_ENUM perm32,
                    const _MM_MEM_HINT_ENUM nt)
{
  return __builtin_ia32_shuf128x32m(mt, perm128, perm32, nt);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_shuf128x32_m(__m512 v1_old, __mmask16 k1,
                         void const* mt,
                         const _MM_PERM_ENUM perm128,
                         const _MM_PERM_ENUM perm32,
                         const _MM_MEM_HINT_ENUM nt)
{
  return __builtin_ia32_mask_shuf128x32m(v1_old, k1, mt, perm128, perm32, nt);
}

/* FIXME: need builtins to unify the interface for KNC */
#define _mm512_shuffle_epi32(v2, perm) \
  _mm512_shuf128x32((v2), _MM_PERM_DCBA, (perm))
#define _mm512_mask_shuffle_epi32(v1_old, k1, v2, perm) \
  _mm512_mask_shuf128x32((v1_old), (k1), (v2), _MM_PERM_DCBA, (perm))

/*
 * Shift int32 vector by count modulo 32.
 *
 *    Performs an element-by-element shift of int32 vector 'v2', shifting
 *    by the number of bits, modulo 32, specified by the int32 vector 'v3'.
 *
 *    sll    logical shift left
 *    srl    logical shift right
 *    sra    arithmetic shift right
 */
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_sll_pi(__m512i v2, __m512i v3)
{
  return __builtin_ia32_sllpi512(v2, v3);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_sll_pi(__m512i v1_old,  __mmask16 k1, __m512i v2,__m512i v3)
{
  return __builtin_ia32_mask_sllpi512(v1_old, k1, v2, v3);
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_sra_pi(__m512i v2, __m512i v3)
{
  return __builtin_ia32_srapi512(v2, v3);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_sra_pi(__m512i v1_old,  __mmask16 k1, __m512i v2, __m512i v3)
{
  return __builtin_ia32_mask_srapi512(v1_old, k1, v2, v3);
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_srl_pi(__m512i v2, __m512i v3)
{
  return __builtin_ia32_srlpi512(v2, v3);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_srl_pi(__m512i v1_old,  __mmask16 k1, __m512i v2, __m512i v3)
{
  return __builtin_ia32_mask_srlpi512(v1_old, k1, v2, v3);
}

/*
 * Logical AND and set vector mask.
 *
 *    Performs an element-by-element bitwise AND between int32 vector 'v1'
 *    and the int32 vector 'v2', and uses the result to construct a 16-bit
 *    vector mask, with a 0-bit for each element for which the result of
 *    the AND was 0, and a 1-bit where the result of the AND was not zero.
 *    Vector mask is returned.
 *
 *    The writemask does not perform the normal writemasking function
 *    for this instruction.  While it does enable/disable comparisons,
 *    it does not block updating of the result; instead, if a writemask
 *    bit is 0, the corresponding destination bit is set to 0.
 */

__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_test_epi32(__m512i v2, __m512i v3)
{
  return __builtin_ia32_testpi512(v2, v3);
}
__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_test_epi32(__mmask16 k1, __m512i v2, __m512i v3)
{
  return __builtin_ia32_mask_testpi512(k1, v2, v3);
}
#define _mm512_test_pi(v2, v3) \
  _mm512_test_epi32((v2), (v3))
#define _mm512_mask_test_pi(k1, v2, v3) \
  _mm512_mask_test_epi32((k1), (v2), (v3))

/*
 * Return 512 vector with undefined elements.  It is recommended to use the
 * result of this intrinsic as the old value for masked versions of intrinsics
 * when the old values will never be meaningfully used.
 */
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_undefined(void)
{
  return __builtin_ia32_undef512();
}
#define _mm512_undefined_pd() _mm512_castps_pd(_mm512_undefined())
#define _mm512_undefined_ps() _mm512_undefined()
#define _mm512_undefined_pi() _mm512_castps_si512(_mm512_undefined())
#define _mm512_undefined_pq() _mm512_castps_si512(_mm512_undefined())
#define _mm512_undefined_epi32() _mm512_castps_si512(_mm512_undefined())
#define _mm512_undefined_epi64() _mm512_castps_si512(_mm512_undefined())

/*
 * Return 512 vector with all elements 0.
 */
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_setzero(void)
{
  return __builtin_ia32_zero512();
}
#define _mm512_setzero_pd() _mm512_castps_pd(_mm512_setzero())
#define _mm512_setzero_ps() _mm512_setzero()
#define _mm512_setzero_pi() _mm512_castps_si512(_mm512_setzero())
#define _mm512_setzero_pq() _mm512_castps_si512(_mm512_setzero())
#define _mm512_setzero_epi32() _mm512_castps_si512(_mm512_setzero())
#define _mm512_setzero_epi64() _mm512_castps_si512(_mm512_setzero())

/*
 * Return float64 vector initialized with 8 elements of a.
 */
/* FIXME: the prototype is changing in ICC from array to multiple params. */
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_set_pd(const double a[8])
{
  return _mm512_loadq(a, _MM_FULLUPC64_NONE, _MM_BROADCAST64_NONE, _MM_HINT_NONE);
}

/*
 * Return float32 vector initialized with 16 elements of a.
 */
/* FIXME: the prototype is changing in ICC from array to multiple params. */
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_set_ps(const float a[16])
{
  return _mm512_loadd(a, _MM_FULLUPC_NONE, _MM_BROADCAST32_NONE, _MM_HINT_NONE);
}

/*
 * Return int32 vector initialized with 16 elements of a.
 */
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_set_pi(const int a[16])
{
  return _mm512_castps_si512(_mm512_loadd(a,
                                          _MM_FULLUPC_NONE,
                                          _MM_BROADCAST32_NONE,
                                          _MM_HINT_NONE));
}

/*
 * Return float64 vector with all 8 elements equal to double a.
 */
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_set_1to8_pd(double a)
{
  return __builtin_ia32_brdcstpd512(a);
}

/*
 * Return int64 vector with all 8 elements equal to a.
 */
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_set_1to8_pq(long a)
{
  return __builtin_ia32_brdcstpq512(a);
}

#define _mm512_set_1to8_epi64(a) _mm512_set_1to8_pq((a))

/*
 * Return float32 vector with all 16 elements equal to float a.
 */
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_set_1to16_ps(float a)
{
  return __builtin_ia32_brdcstps512(a);
}

/*
 * Return int32 vector with all 16 elements equal to int a.
 */
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_set_1to16_pi(int a)
{
  return __builtin_ia32_brdcstpi512(a);
}

#define _mm512_set_1to16_epi32(a) _mm512_set_1to16_pi((a))

/*
 * Return float64 vector dcbadcba.
 * (v4, v0 = a; v5, v1 = b; v6, v2 = c; v7, v3 = d).
 */
/* FIXME: need refining for MIC2 */
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_set_4to8_pd(double a, double b,
                   double c, double d)
{
  double _L[4] = { a, b, c, d };
  return _mm512_loadq(_L, _MM_FULLUPC64_NONE, _MM_BROADCAST_4X8, _MM_HINT_NONE);
}

/*
 * Return int64 vector dcbadcba.
 * (v4, v0 = a; v5, v1 = b; v6, v2 = c; v7, v3 = d).
 */
/* FIXME: need refining for MIC2 */
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_set_4to8_pq(long a, long b,
                   long c, long d)
{
  long _L[4] = { a, b, c, d };
  return _mm512_castpd_si512(_mm512_loadq(_L,
                                          _MM_FULLUPC_NONE,
                                          _MM_BROADCAST_4X8,
                                          _MM_HINT_NONE));
}
#define _mm512_set_4to8_epi64(a, b, c, d) \
    _mm512_set_4to8_pq((a), (b), (c), (d))

/*
 * Return float32 vector dcbadcbadcbadcba.
 * (v12, v8, v4, v0 = a; v13, v9, v5, v1 = b; v14, v10, v6, v2 = c;
 *  v15, v11, v7, v3 = d).
 */
/* FIXME: need refining for MIC2 */
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_set_4to16_ps(float a, float b, float c, float d)
{
  float _L[4] = { a, b, c, d };
  return _mm512_loadd(_L, _MM_FULLUPC_NONE, _MM_BROADCAST_4X16, _MM_HINT_NONE);
}

/*
 * Return int32 vector dcbadcbadcbadcba.
 * (v12, v8, v4, v0 = a; v13, v9, v5, v1 = b; v14, v10, v6, v2 = c;
 *  v15, v11, v7, v3 = d).
 */
/* FIXME: need refining for MIC2 */
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_set_4to16_pi(int a, int b, int c, int d)
{
  int _L[4] = { a, b, c, d };
  return _mm512_castps_si512(_mm512_loadd(_L,
                                          _MM_FULLUPC_NONE,
                                          _MM_BROADCAST_4X16,
                                          _MM_HINT_NONE));
}

#define _mm512_set_4to16_epi32(a, b, c, d) \
    _mm512_set_4to16_pi((a), (b), (c), (d))

/*
 * Return float32 vector e15 e14 e13 ... e1 e0 (v15=e15, v14=e14, ..., v0=e0).
 */
/* FIXME: need refining for MIC2 */
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_set_16to16_ps(float e15, float e14, float e13,
                     float e12, float e11, float e10,
                     float  e9, float  e8, float  e7,
                     float  e6, float  e5, float  e4,
                     float  e3, float  e2, float  e1,
                     float e0)
{
  float _L[16] = { e0, e1, e2, e3, e4, e5, e6, e7, e8, e9,
                   e10, e11, e12, e13, e14, e15 };
  return _mm512_loadd(_L, _MM_FULLUPC_NONE, _MM_BROADCAST32_NONE, _MM_HINT_NONE);
}

/*
 * Return int32 vector e15 e14 e13 ... e1 e0 (v15=e15, v14=e14, ..., v0=e0).
 */
/* FIXME: need refining for MIC2 */
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_set_16to16_pi(int e15, int e14, int e13, int e12,
                     int e11, int e10, int e9, int e8,
                     int e7, int e6, int e5, int e4,
                     int e3, int e2, int e1, int e0)
{
  int _L[16] = { e0, e1, e2, e3, e4, e5, e6, e7, e8, e9,
                 e10, e11, e12, e13, e14, e15 };
  return _mm512_castps_si512(_mm512_loadd(_L,
                                          _MM_FULLUPC_NONE,
                                          _MM_BROADCAST32_NONE,
                                          _MM_HINT_NONE));
}

#define _mm512_set_16to16_epi32(e15, e14, e13, e12, e11, e10, e9, e8, \
                                 e7,  e6,  e5,  e4,  e3,  e2, e1, e0) \
    _mm512_set_16to16_pi((e15), (e14), (e13), (e12), \
                         (e11), (e10),  (e9),  (e8), \
                          (e7),  (e6),  (e5),  (e4), \
                          (e3),  (e2),  (e1),  (e0))

/*
 * Return float64 vector e7 e6 e5 ... e1 e0 (v7=e7, v6=e6, ..., v0=e0).
 */
/* FIXME: need refining for MIC2 */
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_set_8to8_pd(double e7, double e6, double e5,
                   double e4, double e3, double e2,
                   double e1, double e0)
{
  double _L[8] = { e0, e1, e2, e3, e4, e5, e6, e7 };
  return _mm512_loadq(_L, _MM_FULLUPC_NONE, _MM_BROADCAST64_NONE, _MM_HINT_NONE);
}

/*
 * Return int64 vector e7 e6 e5 ... e1 e0 (v7=e7, v6=e6, ..., v0=e0).
 */
/* FIXME: need refining for MIC2 */
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_set_8to8_pq(long e7, long e6,
                   long e5, long e4,
                   long e3, long e2,
                   long e1, long e0)
{
  long _L[8] = { e0, e1, e2, e3, e4, e5, e6, e7 };
  return _mm512_castpd_si512(_mm512_loadq(_L,
                                          _MM_FULLUPC_NONE,
                                          _MM_BROADCAST64_NONE,
                                          _MM_HINT_NONE));
}

#define _mm512_set_8to8_epi64(e7, e6, e5, e4, e3, e2, e1, e0) \
    _mm512_set_8to8_pq((e7), (e6), (e5), (e4), (e3), (e2), (e1), (e0))

/*
 * Math intrinsics.
 */

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_acos_pd(__m512d v2)
{
  return __builtin_ia32_acospd512(v2);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_acos_pd(__m512d v1_old, __mmaskt k1, __m512d v2)
{
  return __builtin_ia32_mask_acospd512(v1_old, k1, v2);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_acos_ps(__m512 v2)
{
  return __builtin_ia32_acosps512(v2);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_acos_ps(__m512 v1_old, __mmask16 k1, __m512 v2)
{
  return __builtin_ia32_mask_acosps512(v1_old, k1, v2);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_acosh_pd(__m512d v2)
{
  return __builtin_ia32_acoshpd512(v2);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_acosh_pd(__m512d v1_old, __mmaskt k1, __m512d v2)
{
  return __builtin_ia32_mask_acoshpd512(v1_old, k1, v2);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_acosh_ps(__m512 v2)
{
  return __builtin_ia32_acoshps512(v2);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_acosh_ps(__m512 v1_old, __mmask16 k1, __m512 v2)
{
  return __builtin_ia32_mask_acoshps512(v1_old, k1, v2);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_asin_pd(__m512d v2)
{
  return __builtin_ia32_asinpd512(v2);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_asin_pd(__m512d v1_old, __mmaskt k1, __m512d v2)
{
  return __builtin_ia32_mask_asinpd512(v1_old, k1, v2);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_asin_ps(__m512 v2)
{
  return __builtin_ia32_asinps512(v2);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_asin_ps(__m512 v1_old, __mmask16 k1, __m512 v2)
{
  return __builtin_ia32_mask_asinps512(v1_old, k1, v2);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_asinh_pd(__m512d v2)
{
  return __builtin_ia32_asinhpd512(v2);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_asinh_pd(__m512d v1_old, __mmaskt k1, __m512d v2)
{
  return __builtin_ia32_mask_asinhpd512(v1_old, k1, v2);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_asinh_ps(__m512 v2)
{
  return __builtin_ia32_asinhps512(v2);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_asinh_ps(__m512 v1_old, __mmask16 k1, __m512 v2)
{
  return __builtin_ia32_mask_asinhps512(v1_old, k1, v2);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_atan2_pd(__m512d v2, __m512d v3)
{
  return __builtin_ia32_atan2pd512(v2, v3);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_atan2_pd(__m512d v1_old, __mmaskt k1, __m512d v2, __m512d v3)
{
  return __builtin_ia32_mask_atan2pd512(v1_old, k1, v2, v3);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_atan2_ps(__m512 v2, __m512 v3)
{
  return __builtin_ia32_atan2ps512(v2, v3);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_atan2_ps(__m512 v1_old, __mmask16 k1, __m512 v2, __m512 v3)
{
  return __builtin_ia32_mask_atan2ps512(v1_old, k1, v2, v3);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_atan_pd(__m512d v2)
{
  return __builtin_ia32_atanpd512(v2);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_atan_pd(__m512d v1_old, __mmaskt k1, __m512d v2)
{
  return __builtin_ia32_mask_atanpd512(v1_old, k1, v2);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_atan_ps(__m512 v2)
{
  return __builtin_ia32_atanps512(v2);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_atan_ps(__m512 v1_old, __mmask16 k1, __m512 v2)
{
  return __builtin_ia32_mask_atanps512(v1_old, k1, v2);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_atanh_pd(__m512d v2)
{
  return __builtin_ia32_atanhpd512(v2);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_atanh_pd(__m512d v1_old, __mmaskt k1, __m512d v2)
{
  return __builtin_ia32_mask_atanhpd512(v1_old, k1, v2);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_atanh_ps(__m512 v2)
{
  return __builtin_ia32_atanhps512(v2);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_atanh_ps(__m512 v1_old, __mmask16 k1, __m512 v2)
{
  return __builtin_ia32_mask_atanhps512(v1_old, k1, v2);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_cbrt_pd(__m512d v2)
{
  return __builtin_ia32_cbrtpd512(v2);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cbrt_pd(__m512d v1_old, __mmaskt k1, __m512d v2)
{
  return __builtin_ia32_mask_cbrtpd512(v1_old, k1, v2);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_cbrt_ps(__m512 v2)
{
  return __builtin_ia32_cbrtps512(v2);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cbrt_ps(__m512 v1_old, __mmask16 k1, __m512 v2)
{
  return __builtin_ia32_mask_cbrtps512(v1_old, k1, v2);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_cdfnorminv_pd(__m512d v2)
{
  return __builtin_ia32_cdfnorminvpd512(v2);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cdfnorminv_pd(__m512d v1_old, __mmaskt k1, __m512d v2)
{
  return __builtin_ia32_mask_cdfnorminvpd512(v1_old, k1, v2);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_cdfnorminv_ps(__m512  v2)
{
  return __builtin_ia32_cdfnorminvps512(v2);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cdfnorminv_ps(__m512 v1_old, __mmask16 k1, __m512 v2)
{
  return __builtin_ia32_mask_cdfnorminvps512(v1_old, k1, v2);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_ceil_pd(__m512d v2)
{
  return __builtin_ia32_ceilpd512(v2);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_ceil_pd(__m512d v1_old, __mmaskt k1, __m512d v2)
{
  return __builtin_ia32_mask_ceilpd512(v1_old, k1, v2);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_ceil_ps(__m512 v2)
{
  return __builtin_ia32_ceilps512(v2);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_ceil_ps(__m512 v1_old, __mmask16 k1, __m512 v2)
{
  return __builtin_ia32_mask_ceilps512(v1_old, k1, v2);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_cos_pd(__m512d v2)
{
  return __builtin_ia32_cospd512(v2);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cos_pd(__m512d v1_old, __mmaskt k1, __m512d v2)
{
  return __builtin_ia32_mask_cospd512(v1_old, k1, v2);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_cos_ps(__m512 v2)
{
  return __builtin_ia32_cosps512(v2);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cos_ps(__m512 v1_old, __mmask16 k1, __m512 v2)
{
  return __builtin_ia32_mask_cosps512(v1_old, k1, v2);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_cosd_pd(__m512d v2)
{
  return __builtin_ia32_cosdpd512(v2);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cosd_pd(__m512d v1_old, __mmaskt k1, __m512d v2)
{
  return __builtin_ia32_mask_cosdpd512(v1_old, k1, v2);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_cosd_ps(__m512  v2)
{
  return __builtin_ia32_cosdps512(v2);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cosd_ps(__m512  v1_old, __mmask16 k1, __m512 v2)
{
  return __builtin_ia32_mask_cosdps512(v1_old, k1, v2);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_cosh_pd(__m512d v2)
{
  return __builtin_ia32_coshpd512(v2);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cosh_pd(__m512d v1_old, __mmaskt k1, __m512d v2)
{
  return __builtin_ia32_mask_coshpd512(v2);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_cosh_ps(__m512 v2)
{
  return __builtin_ia32_coshps512(v2);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cosh_ps(__m512 v1_old, __mmask16 k1, __m512 v2)
{
  return __builtin_ia32_mask_coshps512(v1_old, k1, v2);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_erf_pd(__m512d v2)
{
  return __builtin_ia32_erfpd512(v2);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_erf_pd(__m512d v1_old, __mmaskt k1, __m512d v2)
{
  return __builtin_ia32_mask_erfpd512(v1_old, k1, v2);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_erfc_pd(__m512d v2)
{
  return __builtin_ia32_erfcpd512(v2);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_erfc_pd(__m512d v1_old, __mmaskt k1, __m512d v2)
{
  return __builtin_ia32_mask_erfcpd512(v1_old, k1, v2);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_erf_ps(__m512 v2)
{
  return __builtin_ia32_erfps512(v2);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_erf_ps(__m512 v1_old, __mmask16 k1, __m512 v2)
{
  return __builtin_ia32_mask_erfps512(v1_old, k1, v2);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_erfc_ps(__m512 v2)
{
  return __builtin_ia32_erfcps512(v2);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_erfc_ps(__m512 v1_old, __mmask16 k1, __m512 v2)
{
  return __builtin_ia32_mask_erfcps512(v1_old, k1, v2);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_erfinv_pd(__m512d v2)
{
  return __builtin_ia32_erfinvpd512(v2);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_erfinv_pd(__m512d v1_old, __mmaskt k1, __m512d v2)
{
  return __builtin_ia32_mask_erfinvpd512(v1_old, k1, v2);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_erfinv_ps(__m512 v2)
{
  return __builtin_ia32_erfinvps512(v2);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_erfinv_ps(__m512 v1_old, __mmask16 k1, __m512 v2)
{
  return __builtin_ia32_mask_erfinvps512(v1_old, k1, v2);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_exp10_pd(__m512d v2)
{
  return __builtin_ia32_exp10pd512(v2);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_exp10_pd(__m512d v1_old, __mmaskt k1, __m512d v2)
{
  return __builtin_ia32_mask_exp10pd512(v1_old, k1, v2);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_exp10_ps(__m512  v2)
{
  return __builtin_ia32_exp10ps512(v2);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_exp10_ps(__m512  v1_old, __mmask16 k1, __m512 v2)
{
  return __builtin_ia32_mask_exp10ps512(v1_old, k1, v2);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_exp2_pd(__m512d v2)
{
  return __builtin_ia32_exp2pd512(v2);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_exp2_pd(__m512d v1_old, __mmaskt k1, __m512d v2)
{
  return __builtin_ia32_mask_exp2pd512(v1_old, k1, v2);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_exp2_ps(__m512 v2)
{
  return __builtin_ia32_exp2ps512(v2);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_exp2_ps(__m512 v1_old, __mmask16 k1, __m512 v2)
{
  return __builtin_ia32_mask_exp2ps512(v1_old, k1, v2);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_exp_pd(__m512d v2)
{
  return __builtin_ia32_exppd512(v2);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_exp_pd(__m512d v1_old, __mmaskt k1, __m512d v2)
{
  return __builtin_ia32_mask_exppd512(v1_old, k1, v2);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_exp_ps(__m512 v2)
{
  return __builtin_ia32_expps512(v2);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_exp_ps(__m512 v1_old, __mmask16 k1, __m512 v2)
{
  return __builtin_ia32_mask_expps512(v1_old, k1, v2);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_expm1_pd(__m512d v2)
{
  return __builtin_ia32_expm1pd512(v2);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_expm1_pd(__m512d v1_old, __mmaskt k1, __m512d v2)
{
  return __builtin_ia32_mask_expm1pd512(v1_old, k1, v2);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_expm1_ps(__m512  v2)
{
  return __builtin_ia32_expm1ps512(v2);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_expm1_ps(__m512  v1_old, __mmask16 k1, __m512 v2)
{
  return __builtin_ia32_mask_expm1ps512(v1_old, k1, v2);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_floor_pd(__m512d v2)
{
  return __builtin_ia32_floorpd512(v2);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_floor_pd(__m512d v1_old, __mmaskt k1, __m512d v2)
{
  return __builtin_ia32_mask_floorpd512(v1_old, k1, v2);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_floor_ps(__m512 v2)
{
  return __builtin_ia32_floorps512(v2);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_floor_ps(__m512 v1_old, __mmask16 k1, __m512 v2)
{
  return __builtin_ia32_mask_floorps512(v1_old, k1, v2);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_hypot_pd(__m512d v2, __m512d v3)
{
  return __builtin_ia32_hypotpd512(v2, v3);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_hypot_pd(__m512d v1_old, __mmaskt k1, __m512d v2, __m512d v3)
{
  return __builtin_ia32_mask_hypotpd512(v1_old, k1, v2, v3);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_hypot_ps(__m512 v2, __m512 v3)
{
  return __builtin_ia32_hypotps512(v2, v3);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_hypot_ps(__m512 v1_old, __mmask16 k1, __m512 v2, __m512 v3)
{
  return __builtin_ia32_mask_hypotps512(v1_old, k1, v2, v3);
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_div_pi(__m512i v2, __m512i v3)
{
  return __builtin_ia32_divpi512(v2, v3);
}
#define _mm512_div_epi32(v2, v3) _mm512_div_pi((v2), (v3))
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_div_pi(__m512i v1_old, __mmask16 k1, __m512i v2, __m512i v3)
{
  return __builtin_ia32_mask_divpi512(v1_old, k1, v2, v3);
}
#define _mm512_mask_div_epi32(v1_old, k1, v2, v3) \
    _mm512_mask_div_pi((v1_old), (k1), (v2), (v3))

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_div_ps(__m512 v2, __m512 v3)
{
  return __builtin_ia32_divps512(v2, v3);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_div_ps(__m512 v1_old, __mmask16 k1, __m512 v2, __m512 v3)
{
  return __builtin_ia32_mask_divps512(v1_old, k1, v2, v3);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_div_pd(__m512d v2, __m512d v3)
{
  return __builtin_ia32_divpd512(v2, v3);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_div_pd(__m512d v1_old, __mmaskt k1, __m512d v2, __m512d v3)
{
  return __builtin_ia32_mask_divpd512(v1_old, k1, v2, v3);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_invsqrt_pd(__m512d v2)
{
  return __builtin_ia32_invsqrtpd512(v2);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_invsqrt_pd(__m512d v1_old, __mmaskt k1, __m512d v2)
{
  return __builtin_ia32_mask_invsqrtpd512(v1_old, k1, v2);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_invsqrt_ps(__m512 v2)
{
  return __builtin_ia32_invsqrtps512(v2);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_invsqrt_ps(__m512 v1_old, __mmask16 k1, __m512 v2)
{
  return __builtin_ia32_mask_invsqrtps512(v1_old, k1, v2);
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_rem_pi(__m512i v2, __m512i v3)
{
  return __builtin_ia32_rempi512(v2, v3);
}
#define _mm512_rem_epi32(v2, v3) _mm512_rem_pi((v2), (v3))
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_rem_pi(__m512i v1_old, __mmask16 k1, __m512i v2, __m512i v3)
{
  return __builtin_ia32_mask_rempi512(v1_old, k1, v2, v3);
}
#define _mm512_mask_rem_epi32(v1_old, k1, v2, v3) \
    _mm512_mask_rem_pi((v1_old), (k1), (v2), (v3))

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_log10_pd(__m512d v2)
{
  return __builtin_ia32_log10pd512(v2);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_log10_pd(__m512d v1_old, __mmaskt k1, __m512d v2)
{
  return __builtin_ia32_mask_log10pd512(v1_old, k1, v2);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_log10_ps(__m512 v2)
{
  return __builtin_ia32_log10ps512(v2);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_log10_ps(__m512 v1_old, __mmask16 k1, __m512 v2)
{
  return __builtin_ia32_mask_log10ps512(v1_old, k1, v2);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_log1p_pd(__m512d v2)
{
  return __builtin_ia32_log1ppd512(v2);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_log1p_pd(__m512d v1_old, __mmaskt k1, __m512d v2)
{
  return __builtin_ia32_mask_log1ppd512(v1_old, k1, v2);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_log1p_ps(__m512  v2)
{
  return __builtin_ia32_log1pps512(v2);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_log1p_ps(__m512 v1_old, __mmask16 k1, __m512 v2)
{
  return __builtin_ia32_mask_log1pps512(v1_old, k1, v2);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_log2_pd(__m512d v2)
{
  return __builtin_ia32_log2pd512(v2);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_log2_pd(__m512d v1_old, __mmaskt k1, __m512d v2)
{
  return __builtin_ia32_mask_log2pd512(v1_old, k1, v2);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_log2_ps(__m512 v2)
{
  return __builtin_ia32_log2ps512(v2);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_log2_ps(__m512 v1_old, __mmask16 k1, __m512 v2)
{
  return __builtin_ia32_mask_log2ps512(v1_old, k1, v2);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_log_pd(__m512d v2)
{
  return __builtin_ia32_logpd512(v2);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_log_pd(__m512d v1_old, __mmaskt k1, __m512d v2)
{
  return __builtin_ia32_mask_logpd512(v1_old, k1, v2);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_log_ps(__m512 v2)
{
  return __builtin_ia32_logps512(v2);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_log_ps(__m512 v1_old, __mmask16 k1, __m512 v2)
{
  return __builtin_ia32_mask_logps512(v1_old, k1, v2);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_logb_pd(__m512d v2)
{
  return __builtin_ia32_logbpd512(v2);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_logb_pd(__m512d v1_old, __mmaskt k1, __m512d v2)
{
  return __builtin_ia32_mask_logbpd512(v1_old, k1, v2);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_logb_ps(__m512  v2)
{
  return __builtin_ia32_logbps512(v2);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_logb_ps(__m512  v1_old, __mmask16 k1, __m512 v2)
{
  return __builtin_ia32_mask_logbps512(v1_old, k1, v2);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_nearbyint_pd(__m512d v2)
{
  return __builtin_ia32_nearbyintpd512(v2);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_nearbyint_pd(__m512d v1_old, __mmaskt k1, __m512d v2)
{
  return __builtin_ia32_mask_nearbyintpd512(v1_old, k1, v2);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_nearbyint_ps(__m512 v2)
{
  return __builtin_ia32_nearbyintps512(v2);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_nearbyint_ps(__m512 v1_old, __mmask16 k1, __m512 v2)
{
  return __builtin_ia32_mask_nearbyintps512(v1_old, k1, v2);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_pow_pd(__m512d v2, __m512d v3)
{
  return __builtin_ia32_powpd512(v2, v3);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_pow_pd(__m512d v1_old, __mmaskt k1, __m512d v2, __m512d v3)
{
  return __builtin_ia32_mask_powpd512(v1_old, k1, v2, v3);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_pow_ps(__m512 v2, __m512 v3)
{
  return __builtin_ia32_powps512(v2, v3);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_pow_ps(__m512 v1_old, __mmask16 k1, __m512 v2, __m512 v3)
{
  return __builtin_ia32_mask_powps512(v1_old, k1, v2, v3);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_recip_pd(__m512d v2)
{
  return __builtin_ia32_recippd512(v2);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_recip_pd(__m512d v1_old, __mmaskt k1, __m512d v2)
{
  return __builtin_ia32_mask_recippd512(v1_old, k1, v2);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_recip_ps(__m512 v2)
{
  return __builtin_ia32_recipps512(v2);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_recip_ps(__m512 v1_old, __mmask16 k1, __m512 v2)
{
  return __builtin_ia32_mask_recipps512(v1_old, k1, v2);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_rint_pd(__m512d v2)
{
  return __builtin_ia32_rintpd512(v2);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_rint_pd(__m512d v1_old, __mmaskt k1, __m512d v2)
{
  return __builtin_ia32_mask_rintpd512(v1_old, k1, v2);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_rint_ps(__m512 v2)
{
  return __builtin_ia32_rintps512(v2);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_rint_ps(__m512 v1_old, __mmask16 k1, __m512 v2)
{
  return __builtin_ia32_mask_rintps512(v1_old, k1, v2);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_svml_round_pd(__m512d v2)
{
  return __builtin_ia32_svml_roundpd512(v2);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_svml_round_pd(__m512d v1_old, __mmaskt k1, __m512d v2)
{
  return __builtin_ia32_mask_svml_roundpd512(v1_old, k1, v2);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_sin_pd(__m512d v2)
{
  return __builtin_ia32_sinpd512(v2);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_sin_pd(__m512d v1_old, __mmaskt k1, __m512d v2)
{
  return __builtin_ia32_mask_sinpd512(v1_old, k1, v2);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_sin_ps(__m512 v2)
{
  return __builtin_ia32_sinps512(v2);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_sin_ps(__m512 v1_old, __mmask16 k1, __m512 v2)
{
  return __builtin_ia32_mask_sinps512(v1_old, k1, v2);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_sinh_pd(__m512d v2)
{
  return __builtin_ia32_sinhpd512(v2);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_sinh_pd(__m512d v1_old, __mmaskt k1, __m512d v2)
{
  return __builtin_ia32_mask_sinhpd512(v1_old, k1, v2);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_sinh_ps(__m512 v2)
{
  return __builtin_ia32_sinhps512(v2);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_sinh_ps(__m512 v1_old, __mmask16 k1, __m512 v2)
{
  return __builtin_ia32_mask_sinhps512(v1_old, k1, v2);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_sind_pd(__m512d v2)
{
  return __builtin_ia32_sindpd512(v2);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_sind_pd(__m512d v1_old, __mmaskt k1, __m512d v2)
{
  return __builtin_ia32_mask_sindpd512(v1_old, k1, v2);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_sind_ps(__m512  v2)
{
  return __builtin_ia32_sindps512(v2);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_sind_ps(__m512  v1_old, __mmask16 k1, __m512 v2)
{
  return __builtin_ia32_mask_sindps512(v1_old, k1, v2);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_sqrt_pd(__m512d v2)
{
  return __builtin_ia32_sqrtpd512(v2);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_sqrt_pd(__m512d v1_old, __mmaskt k1, __m512d v2)
{
  return __builtin_ia32_mask_sqrtpd512(v1_old, k1, v2);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_sqrt_ps(__m512 v2)
{
  return __builtin_ia32_sqrtps512(v2);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_sqrt_ps(__m512 v1_old, __mmask16 k1, __m512 v2)
{
  return __builtin_ia32_mask_sqrtps512(v1_old, k1, v2);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_tan_pd(__m512d v2)
{
  return __builtin_ia32_tanpd512(v2);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_tan_pd(__m512d v1_old, __mmaskt k1, __m512d v2)
{
  return __builtin_ia32_mask_tanpd512(v1_old, k1, v2);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_tan_ps(__m512 v2)
{
  return __builtin_ia32_tanps512(v2);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_tan_ps(__m512 v1_old, __mmask16 k1, __m512 v2)
{
  return __builtin_ia32_mask_tanps512(v1_old, k1, v2);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_tand_pd(__m512d v2)
{
  return __builtin_ia32_tandpd512(v2);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_tand_pd(__m512d v1_old, __mmaskt k1, __m512d v2)
{
  return __builtin_ia32_mask_tandpd512(v1_old, k1, v2);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_tand_ps(__m512  v2)
{
  return __builtin_ia32_tandps512(v2);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_tand_ps(__m512  v1_old, __mmask16 k1, __m512 v2)
{
  return __builtin_ia32_mask_tandps512(v1_old, k1, v2);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_tanh_pd(__m512d v2)
{
  return __builtin_ia32_tanhpd512(v2);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_tanh_pd(__m512d v1_old, __mmaskt k1, __m512d v2)
{
  return __builtin_ia32_mask_tanhpd512(v1_old, k1, v2);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_tanh_ps(__m512 v2)
{
  return __builtin_ia32_tanhps512(v2);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_tanh_ps(__m512 v1_old, __mmask16 k1, __m512 v2)
{
  return __builtin_ia32_mask_tanhps512(v1_old, k1, v2);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_trunc_pd(__m512d v2)
{
  return __builtin_ia32_truncpd512(v2);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_trunc_pd(__m512d v1_old, __mmaskt k1, __m512d v2)
{
  return __builtin_ia32_mask_truncpd512(v1_old, k1, v2);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_trunc_ps(__m512 v2)
{
  return __builtin_ia32_truncps512(v2);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_trunc_ps(__m512 v1_old, __mmask16 k1, __m512 v2)
{
  return __builtin_ia32_mask_truncps512(v1_old, k1, v2);
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_div_pu(__m512i v2, __m512i v3)
{
  return __builtin_ia32_divpu512(v2, v3);
}
#define _mm512_div_epu32(v2, v3) _mm512_div_pu((v2), (v3))
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_div_pu(__m512i v1_old, __mmask16 k1, __m512i v2, __m512i v3)
{
  return __builtin_ia32_mask_divpu512(v1_old, k1, v2, v3);
}
#define _mm512_mask_div_epu32(v1_old, k1, v2, v3) \
    _mm512_mask_div_pu((v1_old), (k1), (v2), (v3))

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_rem_pu(__m512i v2, __m512i v3)
{
  return __builtin_ia32_rempu512(v2, v3);
}
#define _mm512_rem_epu32(v2, v3) _mm512_rem_pu((v2), (v3))
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_rem_pu(__m512i v1_old, __mmask16 k1, __m512i v2, __m512i v3)
{
  return __builtin_ia32_mask_rempu512(v1_old, k1, v2, v3);
}
#define _mm512_mask_rem_epu32(v1_old, k1, v2, v3) \
    _mm512_mask_rem_pu((v1_old), (k1), (v2), (v3))

/*
 * Reduction intrinsics - perform corresponding operation on all elements
 * of source vector and return scalar value.
 * For example, _mm512_reduce_add_ps returns float32 value
 * calculated as v1[0] + v1[1] + ... + v1[15].
 */
__inline__ float __attribute__((__always_inline__, __nodebug__))
_mm512_reduce_add_ps(__m512 v1)
{
  return __builtin_ia32_reduce_addps512(v1);
}
__inline__ float __attribute__((__always_inline__, __nodebug__))
_mm512_mask_reduce_add_ps(__mmask16 k1, __m512 v1)
{
  return __builtin_ia32_mask_reduce_addps512(k1, v1);
}

__inline__ double __attribute__((__always_inline__, __nodebug__))
_mm512_reduce_add_pd(__m512d v1)
{
  return __builtin_ia32_reduce_addpd512(v1);
}
__inline__ double __attribute__((__always_inline__, __nodebug__))
_mm512_mask_reduce_add_pd(__mmaskt k1, __m512d v1)
{
  return __builtin_ia32_mask_reduce_addpd512(k1, v1);
}

__inline__ int __attribute__((__always_inline__, __nodebug__))
_mm512_reduce_add_epi32(__m512i v1)
{
  return __builtin_ia32_reduce_addpi512(v1);
}
__inline__ int __attribute__((__always_inline__, __nodebug__))
_mm512_mask_reduce_add_epi32(__mmask16 k1, __m512i v1)
{
  return __builtin_ia32_mask_reduce_addpi512(k1, v1);
}

__inline__ float __attribute__((__always_inline__, __nodebug__))
_mm512_reduce_mul_ps(__m512 v1)
{
  return __builtin_ia32_reduce_mulps512(v1);
}
__inline__ float __attribute__((__always_inline__, __nodebug__))
_mm512_mask_reduce_mul_ps(__mmask16 k1, __m512 v1)
{
  return __builtin_ia32_mask_reduce_mulps512(k1, v1);
}

__inline__ double __attribute__((__always_inline__, __nodebug__))
_mm512_reduce_mul_pd(__m512d v1)
{
  return __builtin_ia32_reduce_mulpd512(v1);
}
__inline__ double __attribute__((__always_inline__, __nodebug__))
_mm512_mask_reduce_mul_pd(__mmaskt k1, __m512d v1)
{
  return __builtin_ia32_mask_reduce_mulpd512(k1, v1);
}

__inline__ int __attribute__((__always_inline__, __nodebug__))
_mm512_reduce_mul_epi32(__m512i v1)
{
  return __builtin_ia32_reduce_mulpi512(v1);
}
__inline__ int __attribute__((__always_inline__, __nodebug__))
_mm512_mask_reduce_mul_epi32(__mmask16 k1, __m512i v1)
{
  return __builtin_ia32_mask_reduce_mulpi512(k1, v1);
}

__inline__ float __attribute__((__always_inline__, __nodebug__))
_mm512_reduce_min_ps(__m512 v1)
{
  return __builtin_ia32_reduce_minps512(v1);
}
__inline__ float __attribute__((__always_inline__, __nodebug__))
_mm512_mask_reduce_min_ps(__mmask16 k1, __m512 v1)
{
  return __builtin_ia32_mask_reduce_minps512(k1, v1);
}

__inline__ double __attribute__((__always_inline__, __nodebug__))
_mm512_reduce_min_pd(__m512d v1)
{
  return __builtin_ia32_reduce_minpd512(v1);
}
__inline__ double __attribute__((__always_inline__, __nodebug__))
_mm512_mask_reduce_min_pd(__mmaskt k1, __m512d v1)
{
  return __builtin_ia32_mask_reduce_minpd512(k1, v1);
}

__inline__ int __attribute__((__always_inline__, __nodebug__))
_mm512_reduce_min_epi32(__m512i v1)
{
  return __builtin_ia32_reduce_minpi512(v1);
}
__inline__ int __attribute__((__always_inline__, __nodebug__))
_mm512_mask_reduce_min_epi32(__mmask16 k1, __m512i v1)
{
  return __builtin_ia32_mask_reduce_minpi512(k1, v1);
}

__inline__ unsigned int __attribute__((__always_inline__, __nodebug__))
_mm512_reduce_min_epu32(__m512i v1)
{
  return __builtin_ia32_reduce_minpu512(v1);
}
__inline__ unsigned int __attribute__((__always_inline__, __nodebug__))
_mm512_mask_reduce_min_epu32(__mmask16 k1, __m512i v1)
{
  return __builtin_ia32_mask_reduce_minpu512(k1, v1);
}

__inline__ float __attribute__((__always_inline__, __nodebug__))
_mm512_reduce_max_ps(__m512 v1)
{
  return __builtin_ia32_reduce_maxps512(v1);
}
__inline__ float __attribute__((__always_inline__, __nodebug__))
_mm512_mask_reduce_max_ps(__mmask16 k1, __m512 v1)
{
  return __builtin_ia32_mask_reduce_maxps512(k1, v1);
}

__inline__ double __attribute__((__always_inline__, __nodebug__))
_mm512_reduce_max_pd(__m512d v1)
{
  return __builtin_ia32_reduce_maxpd512(v1);
}
__inline__ double __attribute__((__always_inline__, __nodebug__))
_mm512_mask_reduce_max_pd(__mmaskt k1, __m512d v1)
{
  return __builtin_ia32_mask_reduce_maxpd512(k1, v1);
}

__inline__ int __attribute__((__always_inline__, __nodebug__))
_mm512_reduce_max_epi32(__m512i v1)
{
  return __builtin_ia32_reduce_maxpi512(v1);
}
__inline__ int __attribute__((__always_inline__, __nodebug__))
_mm512_mask_reduce_max_epi32(__mmask16 k1, __m512i v1)
{
  return __builtin_ia32_mask_reduce_maxpi512(k1, v1);
}

__inline__ unsigned int __attribute__((__always_inline__, __nodebug__))
_mm512_reduce_max_epu32(__m512i v1)
{
  return __builtin_ia32_reduce_maxpu512(v1);
}
__inline__ unsigned int __attribute__((__always_inline__, __nodebug__))
_mm512_mask_reduce_max_epu32(__mmask16 k1, __m512i v1)
{
  return __builtin_ia32_mask_reduce_maxpu512(k1, v1);
}

__inline__ int __attribute__((__always_inline__, __nodebug__))
_mm512_reduce_or_epi32(__m512i v1)
{
  return __builtin_ia32_reduce_orpi512(v1);
}
__inline__ int __attribute__((__always_inline__, __nodebug__))
_mm512_mask_reduce_or_epi32(__mmask16 k1, __m512i v1)
{
  return __builtin_ia32_mask_reduce_orpi512(k1, v1);
}

__inline__ int __attribute__((__always_inline__, __nodebug__))
_mm512_reduce_and_epi32(__m512i v1)
{
  return __builtin_ia32_reduce_andpi512(v1);
}
__inline__ int __attribute__((__always_inline__, __nodebug__))
_mm512_mask_reduce_and_epi32(__mmask16 k1, __m512i v1)
{
  return __builtin_ia32_mask_reduce_andpi512(k1, v1);
}

__inline__ float __attribute__((__always_inline__, __nodebug__))
_mm512_reduce_gmin_ps(__m512 v1)
{
  return __builtin_ia32_reduce_gminps512(v1);
}
__inline__ float __attribute__((__always_inline__, __nodebug__))
_mm512_mask_reduce_gmin_ps(__mmask16 k1, __m512 v1)
{
  return __builtin_ia32_mask_reduce_gminps512(k1, v1);
}

__inline__ double __attribute__((__always_inline__, __nodebug__))
_mm512_reduce_gmin_pd(__m512d v1)
{
  return __builtin_ia32_reduce_gminpd512(v1);
}
__inline__ double __attribute__((__always_inline__, __nodebug__))
_mm512_mask_reduce_gmin_pd(__mmaskt k1, __m512d v1)
{
  return __builtin_ia32_mask_reduce_gminpd512(k1, v1);
}

__inline__ float __attribute__((__always_inline__, __nodebug__))
_mm512_reduce_gmax_ps(__m512 v1)
{
  return __builtin_ia32_reduce_gmaxps512(v1);
}
__inline__ float __attribute__((__always_inline__, __nodebug__))
_mm512_mask_reduce_gmax_ps(__mmask16 k1, __m512 v1)
{
  return __builtin_ia32_mask_reduce_gmaxps512(k1, v1);
}

__inline__ double __attribute__((__always_inline__, __nodebug__))
_mm512_reduce_gmax_pd(__m512d v1)
{
  return __builtin_ia32_reduce_gmaxpd512(v1);
}
__inline__ double __attribute__((__always_inline__, __nodebug__))
_mm512_mask_reduce_gmax_pd(__mmaskt k1, __m512d v1)
{
  return __builtin_ia32_mask_reduce_gmaxpd512(k1, v1);
}

/*
 * Scalar intrinsics.
 */

/*
 * 1:1 bit interleave.
 *
 *    Performs a 1:1 bit interleave of the operand (r1) and operand (r2).
 */
__inline__ unsigned short __attribute__((__always_inline__, __nodebug__))
_mm_bitinterleave11_16(unsigned short r1, unsigned short r2)
{
  return __builtin_ia32_bitinterleave11w(r1, r2);
}
__inline__ unsigned int __attribute__((__always_inline__, __nodebug__))
_mm_bitinterleave11_32(unsigned int r1, unsigned int r2)
{
  return __builtin_ia32_bitinterleave11d(r1, r2);
}
__inline__ unsigned long __attribute__((__always_inline__, __nodebug__))
_mm_bitinterleave11_64(unsigned long r1, unsigned long r2)
{
  return __builtin_ia32_bitinterleave11q(r1, r2);
}

/*
 * 2:1 bit interleave.
 *
 *    Performs a 2:1 bit interleave of the operand (r1) and operand (r2).
 */
__inline__ unsigned short __attribute__((__always_inline__, __nodebug__))
_mm_bitinterleave21_16(unsigned short r1, unsigned short r2)
{
  return __builtin_ia32_bitinterleave21w(r1, r2);
}
__inline__ unsigned int __attribute__((__always_inline__, __nodebug__))
_mm_bitinterleave21_32(unsigned int r1, unsigned int r2)
{
  return __builtin_ia32_bitinterleave21d(r1, r2);
}
__inline__ unsigned long __attribute__((__always_inline__, __nodebug__))
_mm_bitinterleave21_64(unsigned long r1, unsigned long r2)
{
  return __builtin_ia32_bitinterleave21q(r1, r2);
}

/*
 * Fast Bit Scan Forward.
 *
 *    Searches the source operand (r2) for the least significant set bit
 *    (1 bit).  If a least significant 1 bit is found, its bit index is
 *    returned, otherwise the result is -1.
 */
__inline__ short __attribute__((__always_inline__, __nodebug__))
_mm_bsff_16(unsigned short r2)
{
  return __builtin_ia32_bsffw(r2);
}
__inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_bsff_32(unsigned int r2)
{
  return __builtin_ia32_bsffd(r2);
}
__inline__ long __attribute__((__always_inline__, __nodebug__))
_mm_bsff_64(unsigned long r2)
{
  return __builtin_ia32_bsffq(r2);
}

/*
 * Bit Scan Forward Initialized.
 *
 *    Searches the operand r2 for the least significant set bit (1 bit)
 *    at a position greater than the bit position specified by r1.
 *    If a least significant 1 bit is found, its bit index is returned,
 *    otherwise the result is -1.
 *
 *    The value of r1 is a signed offset from bit 0 of the operand r2.
 *    Any negative r1 value will produce a search starting from bit 0,
 *    like _mm_bsff_*.
 *    Any r1 value equal to or greater than (OPERAND SIZE-1) will cause
 *    the result to be set to -1.
 */
__inline__ short __attribute__((__always_inline__, __nodebug__))
_mm_bsfi_16(short r1, unsigned short r2)
{
  return __builtin_ia32_bsfiw(r1, r2);
}
__inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_bsfi_32(int r1, unsigned int r2)
{
  return __builtin_ia32_bsfid(r1, r2);
}
__inline__ long __attribute__((__always_inline__, __nodebug__))
_mm_bsfi_64(long r1, unsigned long r2)
{
  return __builtin_ia32_bsfiq(r1, r2);
}

/*
 * Fast Bit Scan Reverse.
 *
 *    Searches the operand r2 for the most significant set bit (1 bit)
 *    If a most significant 1 bit is found, its bit index is returned.
 *    Otherwise the result is -1.
 */
__inline__ short __attribute__((__always_inline__, __nodebug__))
_mm_bsrf_16(unsigned short r2)
{
  return __builtin_ia32_bsrfw(r2);
}
__inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_bsrf_32(unsigned int r2)
{
  return __builtin_ia32_bsrfd(r2);
}
__inline__ long __attribute__((__always_inline__, __nodebug__))
_mm_bsrf_64(unsigned long r2)
{
  return __builtin_ia32_bsrfq(r2);
}

/*
 * Bit Scan Reverse Initialized.
 *
 *    Searches the  operand r2 for the most significant set bit (1 bit)
 *    less than bit of r1. If a most significant 1 bit is found, its bit index
 *    is stored in the result; otherwise, the result is set to -1.
 *
 *    The value of r1 is a signed offset from bit 0 of the operand r2.
 *    Any r1 value greater than or equal to OPERAND SIZE will produce a search
 *    starting from bit OPERAND SIZE-1, like BSR. Any r1 value less than or
 *    equal to 0 will cause the result to be set to -1. This instruction
 *    allows continuation of searches through bit vectors without having to
 *    mask off each most-significant 1-bit before restarting, as is required
 *    with BSRF.
 */
__inline__ short __attribute__((__always_inline__, __nodebug__))
_mm_bsri_16(short r1, unsigned short r2)
{
  return __builtin_ia32_bsriw(r1, r2);
}
__inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_bsri_32(int r1, unsigned int r2)
{
  return __builtin_ia32_bsrid(r1, r2);
}
__inline__ long __attribute__((__always_inline__, __nodebug__))
_mm_bsri_64(long r1, unsigned long r2)
{
  return __builtin_ia32_bsriq(r1, r2);
}

/*
 * Bit population count.
 *
 * Performs a population count of the 1-bits in the source operand (r2).
 */
__inline__ unsigned short __attribute__((__always_inline__, __nodebug__))
_mm_countbits_16(unsigned short r2)
{
  return __builtin_ia32_countbitsw(r2);
}
__inline__ unsigned int __attribute__((__always_inline__, __nodebug__))
_mm_countbits_32(unsigned int r2)
{
  return __builtin_ia32_countbitsd(r2);
}
__inline__ unsigned long __attribute__((__always_inline__, __nodebug__))
_mm_countbits_64(unsigned long r2)
{
  return __builtin_ia32_countbitsq(r2);
}


/*
 * Stall thread.
 *
 *    Stall thread for specified clock without blocking other threads.
 *    Hints that the processor should not fetch/issue instructions for the
 *    current thread for the specified number of clock cycles.
 *    Any of the following events will cause the processor to start fetching
 *    instructions for the delayed thread again: the counter counting down
 *    to zero, an interrupt, an NMI or SMI, a debug exception, a machine check
 *    exception, the BINIT# signal, the INIT# signal, or the RESET# signal.
 *    Note that an interrupt will cause the processor to start fetching
 *    instructions for that thread only if the state was entered with
 *    interrupts enabled.
 */
__inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_delay_16(unsigned short r1)
{
  return __builtin_ia32_delayw(r1);
}
__inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_delay_32(unsigned int r1)
{
  return __builtin_ia32_delayd(r1);
}
__inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_delay_64(unsigned long r1)
{
  return __builtin_ia32_delayq(r1);
}

/*
 * Rotate and Bitfield-insert.
 *
 *    Performs a rotation and bitfield insertion from the operand r2 to
 *    the result. Operand r1 is also input.
 */
__inline__ unsigned short __attribute__((__always_inline__, __nodebug__))
_mm_insertfield_16(unsigned short r1, unsigned short r2,
                   const int rotation,
                   const int bit_idx_low,
                   const int bit_idx_high)
{
  return __builtin_ia32_insertfieldw(r1, r2, rotation, bit_idx_low, bit_idx_high);
}

__inline__ unsigned int __attribute__((__always_inline__, __nodebug__))
_mm_insertfield_32(unsigned int r1, unsigned int r2,
                   const int rotation,
                   const int bit_idx_low,
                   const int bit_idx_high)
{
  return __builtin_ia32_insertfieldd(r1, r2, rotation, bit_idx_low, bit_idx_high);
}

__inline__ unsigned long __attribute__((__always_inline__, __nodebug__))
_mm_insertfield_64(unsigned long r1, unsigned long r2,
                   const int rotation,
                   const int bit_idx_low,
                   const int bit_idx_high)
{
  return __builtin_ia32_insertfieldq(r1, r2, rotation, bit_idx_low, bit_idx_high);
}

/*
 * Load VXCSR Register.
 *
 *    Loads the source operand into the VXCSR control/status register.
 *    The source operand is a 32-bit memory location.
 *    This function sets the value of VXCSR register to 'val'.
 */
__inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_setvxcsr(unsigned int val)
{
  __builtin_ia32_setvxcsr(val);
}

/*
 * Store VXCSR Register.
 *
 *    Stores the contents of the VXCSR control and status register
 *    to the destination operand..
 *    This function returns the value of VXCSR register.
 */
__inline__ unsigned int __attribute__((__always_inline__, __nodebug__))
_mm_getvxcsr(void)
{
  return __builtin_ia32_getvxcsr();
}


/*
 * Set per-quad mask.
 *
 *    For each quad (that is, each set of four 4-bit-aligned bits, such as
 *    bits 0-3 or 4-7) within the first qquad (16-bits) in the source operand
 *    (r2), the 4 bits are ORed together, and the corresponding bit of the
 *    result is set to the result of the OR.
 *
 *    This instruction is used to determine how many and which quads are
 *    mask-enabled in a qquad, where a quad is 2x2 pixels and a qquad is
 *    4x4 pixels (four quads), stored in a single vector register.
 */
__inline__ unsigned short __attribute__((__always_inline__, __nodebug__))
_mm_quadmask16_16(unsigned short r2)
{
  return __builtin_ia32_quadmask16w(r2);
}
__inline__ unsigned int __attribute__((__always_inline__, __nodebug__))
_mm_quadmask16_32(unsigned int r2)
{
  return __builtin_ia32_quadmask16d(r2);
}
__inline__ unsigned long __attribute__((__always_inline__, __nodebug__))
_mm_quadmask16_64(unsigned long r2)
{
  return __builtin_ia32_quadmask16q(r2);
}

/*
 * Rotate and mask.
 *
 *    Performs a rotation and mask from the source operand (r1) into the
 *    result.
 */
__inline__ unsigned short __attribute__((__always_inline__, __nodebug__))
_mm_rotatefield_16(unsigned short r1,
                   const int rotation,
                   const int bit_idx_low,
                   const int bit_idx_high)
{
  return __builtin_ia32_rotatefieldw(r1, rotation, bit_idx_low, bit_idx_high);
}
__inline__ unsigned int __attribute__((__always_inline__, __nodebug__))
_mm_rotatefield_32(unsigned int r1,
                   const int rotation,
                   const int bit_idx_low,
                   const int bit_idx_high)
{
  return __builtin_ia32_rotatefieldd(r1, rotation, bit_idx_low, bit_idx_high);
}
__inline__ unsigned long __attribute__((__always_inline__, __nodebug__))
_mm_rotatefield_64(unsigned long r1,
                   const int rotation,
                   const int bit_idx_low,
                   const int bit_idx_high)
{
  return __builtin_ia32_rotatefieldq(r1, rotation, bit_idx_low, bit_idx_high);
}

/*
 * Set performance monitor filtering mask for current thread.
 */
__inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_spflt_16(unsigned short r1)
{
  return __builtin_ia32_spfltw(r1);
}
__inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_spflt_32(unsigned int r1)
{
  return __builtin_ia32_spfltd(r1);
}
__inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_spflt_64(unsigned long r1)
{
  return __builtin_ia32_spfltq(r1);
}

/*
 * Prefetch an L1 cache line.
 *
 *    This is very similar to the existing IA-32 prefetch instruction,
 *    VPREFETCHh, as described in IA-32 Intel r. Architecture Software
 *    Developer's Manual: Volume 2. If the line selected is already present
 *    in the cache hierarchy at a level closer to the processor, no data
 *    movement occurs. Prefetches from uncacheable or WC memory are ignored.
 *    In contrast with the existing prefetch instruction, this instruction
 *    uses disp8*64 addressing. Displacements that would normally be 8 bits
 *    according to the ModR/M byte are still 8 bits but scaled by 64 so that
 *    they have cache-line granularity. This instruction is a hint and may be
 *    speculative, and may be dropped or specify invalid addresses without
 *    causing problems. The 'hint' contains a set of hint bits that modify
 *    the prefetching behavior:
 *  _MM_PFHINT_EX: make line Exclusive in the L1 cache (unless it's already
 *               Exclusive or Modified in the L1 cache).
 *  _MM_PFHINT_MISS: stop picking instructions on this thread until the memory
 *               operand is known to be in the L1 cache. If, on any given
 *               clock, all threads are non-runnable, then any that are
 *               non-runnable due to the execution of prefetch with misshint
 *               will be treated as runnable threads.
 *  _MM_PFHINT_NT: load data into the L1 nontemporal cache rather than
 *               the L1 temporal cache. For Larrabee, this will be implemented
 *               by loading the data in the #TIDth way and making the data
 *               MRU. Data should still be cached normally in the L2 and
 *               higher caches.
 *    Any kind of interruption will turn off the misshint state; see DELAY
 *    for a detailed discussion of this mechanism.
 */
__inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_vprefetch1(const void *m, const _MM_PREFETCH_HINT_ENUM hint)
{
  __builtin_ia32_vprefetch1(m, hint);
}

/*
 * Prefetch an L2 cache line.
 *
 *    This is very similar to the existing IA-32 prefetch instruction,
 *    VPREFETCHh, as described in IA-32 Intel r. Architecture Software
 *    Developer's Manual: Volume 2. If the line selected is already present
 *    in the cache hierarchy at a level closer to the processor, no data
 *    movement occurs. Prefetches from uncacheable or WC memory are ignored.
 *    In contrast with the existing prefetch instruction, this instruction
 *    uses disp8*64 addressing. Displacements that would normally be 8 bits
 *    according to the ModR/M byte are still 8 bits but are scaled by 64 so
 *    that they have cache-line granularity. This instruction is a hint and
 *    may be speculative, and may be dropped or specify invalid addresses
 *    without causing problems.
 *    The 'hint' contains a pair of hint bits that modify the prefetching
 *    behavior:
 *  _MM_PFHINT_EX: make line Exclusive in the L2 cache (unless it's already
 *               Exclusive or Modified in the L2 cache).
 *  _MM_PFHINT_NT: load data into the L2 nontemporal cache rather than the L2
 *               temporal cache. For Larrabee, this will be implemented by
 *               loading the data in the #TIDth way and making the data MRU.
 *               Data should still be cached normally in the L3 and higher
 *               caches.
 */
__inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_vprefetch2(const void *m, const _MM_PREFETCH_HINT_ENUM hint)
{
  __builtin_ia32_vprefetch2(m, hint);
}

/*
 * Evict L1 Cache Line containing m8.
 *
 *    Invalidates from the first-level cache the cache line containing
 *    the specified linear address (updating accordingly the cache hierarchy
 *    if the line is dirty). Note that, unlike CLFLUSH, the invalidation is not
 *    broadcasted throughout the cache coherence domain.
 */
__inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_clevict1(const void* p)
{
  __builtin_ia32_clevict1(p);
}

/*
 * Evict L2 Cache Line containing m8.
 *
 *    Invalidates from the second-level cache the cache line containing
 *    the specified linear address (updating accordingly the cache hierarchy
 *    if the line is dirty). Note that, unlike CLFLUSH, the invalidation is not
 *    broadcasted throughout the cache coherence domain.
 */
__inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_clevict2(const void* p)
{
  __builtin_ia32_clevict2(p);
}

/*
 * Evict cache line from specified cache level:
 * _MM_HINT_T0 -- first level
 * _MM_HINT_T1 -- second level
 */

__inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_clevict(const void* p, const int level)
{
  __builtin_ia32_clevict(p, level);
}

/*
 * Mask arithmetic operations.
 */
__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_kand(__mmask16 k1, __mmask16 k2)
{
  return __builtin_ia32_kand(k1, k2);
}
__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_kandn(__mmask16 k1, __mmask16 k2)
{
  return __builtin_ia32_kandn(k1, k2);
}
__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_kandnr(__mmask16 k1, __mmask16 k2)
{
  return __builtin_ia32_kandnr(k1, k2);
}
__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_kmovlhb(__mmask16 k1, __mmask16 k2)
{
  return __builtin_ia32_kmovlhb(k1, k2);
}
__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_knot(__mmask16 k1)
{
  return __builtin_ia32_knot(k1);
}
__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_kor(__mmask16 k1, __mmask16 k2)
{
  return __builtin_ia32_kor(k1, k2);
}
__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_kxnor(__mmask16 k1, __mmask16 k2)
{
  return __builtin_ia32_kxnor(k1, k2);
}
__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_kxor(__mmask16 k1, __mmask16 k2)
{
  return __builtin_ia32_kxor(k1, k2);
}
__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_kswapb(__mmask16 k1, __mmask16 k2)
{
  return __builtin_ia32_kswapb(k1, k2);
}
__inline__ int __attribute__((__always_inline__, __nodebug__))
_mm512_kortestz(__mmask16 k1, __mmask16 k2)
{
  return __builtin_ia32_kortestz(k1, k2);
}
__inline__ int __attribute__((__always_inline__, __nodebug__))
_mm512_kortestc(__mmask16 k1, __mmask16 k2)
{
  return __builtin_ia32_kortestc(k1, k2);
}
__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_kmov(__mmask16 k1)
{
  return __builtin_ia32_kmov(k1);
}
__inline__ int __attribute__((__always_inline__, __nodebug__))
_mm512_mask2int(__mmask16 k1)
{
  return __builtin_ia32_mask2int(k1);
}
__inline__ __mmask16 __attribute__((__always_inline__, __nodebug__))
_mm512_int2mask(int mask)
{
  return __builtin_ia32_int2mask(mask);
}

#endif /* __MICINTRIN_H */
