#define __ENDIAN_LITTLE__                         0x001
#define __ROUNDING_MODE__                           rte
#define CL_VERSION_1_0                              100
#define CL_VERSION_1_1                              110
#define CL_VERSION_1_2                              120
#define CL_VERSION_2_0                              200

#define MAXFLOAT    0x1.fffffep127f

#define const_func __attribute__((const))
/**
 * A positive float constant expression. HUGE_VALF evaluates
 * to +infinity. Used as an error value returned by the built-in
 * math functions.
 */
#define HUGE_VALF (__builtin_huge_valf())

/**
 * A positive double constant expression. HUGE_VAL evaluates
 * to +infinity. Used as an error value returned by the built-in
 * math functions.
 */
#define HUGE_VAL (__builtin_huge_val())

/**
 * A constant expression of type float representing positive or
 * unsigned infinity.
 */
#define INFINITY (__builtin_inff())

/**
 * A constant expression of type float representing a quiet NaN.
 */
#define NAN as_float(__builtin_nanf((__private char const *)""))

#define FP_ILOGB0        INT_MIN
#define FP_ILOGBNAN      INT_MAX

#define FLT_DIG 6
#define FLT_MANT_DIG 24
#define FLT_MAX_10_EXP +38
#define FLT_MAX_EXP +128
#define FLT_MIN_10_EXP -37
#define FLT_MIN_EXP -125
#define FLT_RADIX 2
#define FLT_MAX 0x1.fffffep127f
#define FLT_MIN 0x1.0p-126f
#define FLT_EPSILON 0x1.0p-23f

#define M_E_F         2.71828182845904523536028747135266250f
#define M_LOG2E_F     1.44269504088896340735992468100189214f
#define M_LOG10E_F    0.434294481903251827651128918916605082f
#define M_LN2_F       0.693147180559945309417232121458176568f
#define M_LN10_F      2.30258509299404568401799145468436421f
#define M_PI_F        3.14159265358979323846264338327950288f
#define M_PI_2_F      1.57079632679489661923132169163975144f
#define M_PI_4_F      0.785398163397448309615660845819875721f
#define M_1_PI_F      0.318309886183790671537767526745028724f
#define M_2_PI_F      0.636619772367581343075535053490057448f
#define M_2_SQRTPI_F  1.12837916709551257389615890312154517f
#define M_SQRT2_F     1.41421356237309504880168872420969808f
#define M_SQRT1_2_F   0.707106781186547524400844362104849039f

#define CHAR_BIT   8
#define SCHAR_MAX  127
#define SCHAR_MIN  (-128)
#define UCHAR_MAX  255
#define CHAR_MAX   SCHAR_MAX
#define CHAR_MIN   SCHAR_MIN
#define USHRT_MAX  65535
#define SHRT_MAX   32767
#define SHRT_MIN   (-32768)
#define UINT_MAX   0xffffffff
#define INT_MAX    2147483647
#define INT_MIN    (-2147483647-1)
#define ULONG_MAX  0xffffffffffffffffUL
#define LONG_MAX   ((long)0x7fffffffffffffffL)
#define LONG_MIN   ((long)(-0x7fffffffffffffffL-1))

/**
 * Return the channel data type. Valid values are:
 * CLK_SNORM_INT8
 * CLK_SNORM_INT16
 * CLK_UNORM_INT8
 * CLK_UNORM_INT16
 * CLK_UNORM_SHORT_565
 * CLK_UNORM_SHORT_555
 * CLK_UNORM_SHORT_101010
 * CLK_SIGNED_INT8
 * CLK_SIGNED_INT16
 * CLK_SIGNED_INT32
 * CLK_UNSIGNED_INT8
 * CLK_UNSIGNED_INT16
 * CLK_UNSIGNED_INT32
 * CLK_HALF_FLOAT
 * CLK_FLOAT
 */

// Channel order
#define CLK_R         0x10B0
#define CLK_A         0x10B1
#define CLK_RG        0x10B2
#define CLK_RA        0x10B3
#define CLK_RGB       0x10B4
#define CLK_RGBA      0x10B5
#define CLK_BGRA      0x10B6
#define CLK_ARGB      0x10B7
#define CLK_INTENSITY 0x10B8
#define CLK_LUMINANCE 0x10B9
#define CLK_Rx                0x10BA
#define CLK_RGx               0x10BB
#define CLK_RGBx              0x10BC
#define CLK_DEPTH             0x10BD
#define CLK_DEPTH_STENCIL     0x10BE

// Channel Type.
#define CLK_SNORM_INT8        0x10D0
#define CLK_SNORM_INT16       0x10D1
#define CLK_UNORM_INT8        0x10D2
#define CLK_UNORM_INT16       0x10D3
#define CLK_UNORM_SHORT_565   0x10D4
#define CLK_UNORM_SHORT_555   0x10D5
#define CLK_UNORM_INT_101010  0x10D6
#define CLK_SIGNED_INT8       0x10D7
#define CLK_SIGNED_INT16      0x10D8
#define CLK_SIGNED_INT32      0x10D9
#define CLK_UNSIGNED_INT8     0x10DA
#define CLK_UNSIGNED_INT16    0x10DB
#define CLK_UNSIGNED_INT32    0x10DC
#define CLK_HALF_FLOAT        0x10DD
#define CLK_FLOAT             0x10DE
#define CLK_UNORM_INT24       0x10DF

/**
 * OpenCL as_typen operators
 * Reinterprets a data type as another data type of the same size
 */
#define as_char(x) __builtin_astype((x), char)
#define as_char2(x) __builtin_astype((x), char2)
#define as_char3(x) __builtin_astype((x), char3)
#define as_char4(x) __builtin_astype((x), char4)
#define as_char8(x) __builtin_astype((x), char8)
#define as_char16(x) __builtin_astype((x), char16)

#define as_uchar(x) __builtin_astype((x), uchar)
#define as_uchar2(x) __builtin_astype((x), uchar2)
#define as_uchar3(x) __builtin_astype((x), uchar3)
#define as_uchar4(x) __builtin_astype((x), uchar4)
#define as_uchar8(x) __builtin_astype((x), uchar8)
#define as_uchar16(x) __builtin_astype((x), uchar16)

#define as_short(x) __builtin_astype((x), short)
#define as_short2(x) __builtin_astype((x), short2)
#define as_short3(x) __builtin_astype((x), short3)
#define as_short4(x) __builtin_astype((x), short4)
#define as_short8(x) __builtin_astype((x), short8)
#define as_short16(x) __builtin_astype((x), short16)

#define as_ushort(x) __builtin_astype((x), ushort)
#define as_ushort2(x) __builtin_astype((x), ushort2)
#define as_ushort3(x) __builtin_astype((x), ushort3)
#define as_ushort4(x) __builtin_astype((x), ushort4)
#define as_ushort8(x) __builtin_astype((x), ushort8)
#define as_ushort16(x) __builtin_astype((x), ushort16)

#define as_int(x) __builtin_astype((x), int)
#define as_int2(x) __builtin_astype((x), int2)
#define as_int3(x) __builtin_astype((x), int3)
#define as_int4(x) __builtin_astype((x), int4)
#define as_int8(x) __builtin_astype((x), int8)
#define as_int16(x) __builtin_astype((x), int16)

#define as_uint(x) __builtin_astype((x), uint)
#define as_uint2(x) __builtin_astype((x), uint2)
#define as_uint3(x) __builtin_astype((x), uint3)
#define as_uint4(x) __builtin_astype((x), uint4)
#define as_uint8(x) __builtin_astype((x), uint8)
#define as_uint16(x) __builtin_astype((x), uint16)

#define as_long(x) __builtin_astype((x), long)
#define as_long2(x) __builtin_astype((x), long2)
#define as_long3(x) __builtin_astype((x), long3)
#define as_long4(x) __builtin_astype((x), long4)
#define as_long8(x) __builtin_astype((x), long8)
#define as_long16(x) __builtin_astype((x), long16)

#define as_ulong(x) __builtin_astype((x), ulong)
#define as_ulong2(x) __builtin_astype((x), ulong2)
#define as_ulong3(x) __builtin_astype((x), ulong3)
#define as_ulong4(x) __builtin_astype((x), ulong4)
#define as_ulong8(x) __builtin_astype((x), ulong8)
#define as_ulong16(x) __builtin_astype((x), ulong16)

#define as_half(x) __builtin_astype((x), half)
#define as_half2(x) __builtin_astype((x), half2)
#define as_half3(x) __builtin_astype((x), half3)
#define as_half4(x) __builtin_astype((x), half4)
#define as_half8(x) __builtin_astype((x), half8)
#define as_half16(x) __builtin_astype((x), half16)

#define as_float(x) __builtin_astype((x), float)
#define as_float2(x) __builtin_astype((x), float2)
#define as_float3(x) __builtin_astype((x), float3)
#define as_float4(x) __builtin_astype((x), float4)
#define as_float8(x) __builtin_astype((x), float8)
#define as_float16(x) __builtin_astype((x), float16)

#define readonly __attribute__((pure))
#define CL_DEVICE_MAX_GLOBAL_VARIABLE_SIZE          0x10000


/**
 * Value of maximum non-infinite single-precision floating-point
 * number.
 */

#define DBL_DIG 15
#define DBL_MANT_DIG 53
#define DBL_MAX_10_EXP +308
#define DBL_MAX_EXP +1024
#define DBL_MIN_10_EXP -307
#define DBL_MIN_EXP -1021
#define DBL_RADIX 2
#define DBL_MAX 0x1.fffffffffffffp1023
#define DBL_MIN 0x1.0p-1022
#define DBL_EPSILON 0x1.0p-52


#define M_E				0x1.5bf0a8b145769p+1
#define M_LOG2E			0x1.71547652b82fep+0
#define M_LOG10E		0x1.bcb7b1526e50ep-2
#define M_LN2			0x1.62e42fefa39efp-1
#define M_LN10			0x1.26bb1bbb55516p+1
#define M_PI			0x1.921fb54442d18p+1
#define M_PI_2			0x1.921fb54442d18p+0
#define M_PI_4			0x1.921fb54442d18p-1
#define M_1_PI			0x1.45f306dc9c883p-2
#define M_2_PI			0x1.45f306dc9c883p-1
#define M_2_SQRTPI		0x1.20dd750429b6dp+0
#define M_SQRT2			0x1.6a09e667f3bcdp+0
#define M_SQRT1_2		0x1.6a09e667f3bcdp-1

#define as_half(x) __builtin_astype((x), half)
#define as_half2(x) __builtin_astype((x), half2)
#define as_half3(x) __builtin_astype((x), half3)
#define as_half4(x) __builtin_astype((x), half4)
#define as_half8(x) __builtin_astype((x), half8)
#define as_half16(x) __builtin_astype((x), half16)

#define as_double(x) __builtin_astype((x), double)
#define as_double2(x) __builtin_astype((x), double2)
#define as_double3(x) __builtin_astype((x), double3)
#define as_double4(x) __builtin_astype((x), double4)
#define as_double8(x) __builtin_astype((x), double8)
#define as_double16(x) __builtin_astype((x), double16)

#define isless(X,Y)             ((X) <  (Y))
#define islessequal(X,Y)        ((X) <= (Y))
#define isgreater(X,Y)          ((X) >  (Y))
#define isgreaterequal(X,Y)     ((X) >= (Y))
#define isequal(X,Y)            ((X) == (Y))
#define isnotequal(X,Y)         ((X) != (Y))

#define vstorea_half_rte vstore_half_rte
#define vstorea_half_rtz vstore_half_rtz
#define vstorea_half_rtp vstore_half_rtp
#define vstorea_half_rtn vstore_half_rtn

/**
 * Queue a memory fence to ensure correct
 * ordering of memory operations to local memory
 */
#define CLK_LOCAL_MEM_FENCE    0x1

/**
 * Queue a memory fence to ensure correct
 * ordering of memory operations to global memory
 */
#define CLK_GLOBAL_MEM_FENCE   0x2

/**
 * The work_group_barrier function will ensure that all
 * image memory accesses become visible to all workitems
 * in the work-group
 */
#define CLK_IMAGE_MEM_FENCE    0x4

#define __kernel_exec(X, typen) __kernel \
	__attribute__((work_group_size_hint(X, 1, 1))) \
	__attribute__((vec_type_hint(typen)))

#define kernel_exec(X, typen) __kernel \
	__attribute__((work_group_size_hint(X, 1, 1))) \
	__attribute__((vec_type_hint(typen)))

#define CLK_ADDRESS_NONE                0
#define CLK_ADDRESS_CLAMP_TO_EDGE       2
#define CLK_ADDRESS_CLAMP               4
#define CLK_ADDRESS_REPEAT              6
#define CLK_ADDRESS_MIRRORED_REPEAT     8

#define __ADDRESS_MASK                  0xE

//
// Coordination Normalization
//
#define CLK_NORMALIZED_COORDS_FALSE     0
#define CLK_NORMALIZED_COORDS_TRUE      1

#define __NORMALIZED_MASK               1

//
// Filtering Mode.
//
#define CLK_FILTER_NEAREST              0x10
#define CLK_FILTER_LINEAR               0x20

#define __FILTER_MASK                   0x30

#define CLK_sRGB                                     0x10BF
#define CLK_sRGBx                                    0x10C0
#define CLK_sRGBA                                    0x10C1
#define CLK_sBGRA                                    0x10C2
#define CLK_ABGR                                     0x10C3


#if __OPENCL_C_VERSION__ >= 200

#define memory_scope_work_item       0x0 /* this must be used only with atomic_work_item_fence and CLK_IMAGE_MEM_FENCE */
#define memory_scope_work_group      0x1
#define memory_scope_device          0x2
#define memory_scope_all_svm_devices 0x3
#define memory_scope_sub_group       0x4

#endif /* __OPENCL_C_VERSION__ >= 200 */


#if !defined ( __MIC__ ) && !defined ( __MIC2__ )

#if __OPENCL_C_VERSION__ >= 200

#define ATOMIC_VAR_INIT(C)  (C)
#define ATOMIC_FLAG_INIT    ATOMIC_VAR_INIT(0)

#define memory_order_relaxed 0x0
#define memory_order_acquire 0x1
#define memory_order_release 0x2
#define memory_order_acq_rel 0x3
#define memory_order_seq_cst 0x4

#endif /* __OPENCL_C_VERSION__ >= 200 */


#define NULL                              0
#define CLK_ENQUEUE_FLAGS_WAIT_KERNEL     0
#define CLK_ENQUEUE_FLAGS_NO_WAIT         1
#define CLK_ENQUEUE_FLAGS_WAIT_WORK_GROUP  2

        // Enqueuing Kernels  6.13.17

#define CLK_SUCCESS                    0
#define CLK_OUT_OF_RESOURCES          -5
#define CLK_INVALID_ARG_SIZE          -51
#define CLK_INVALID_EVENT_WAIT_LIST   -57
#define CLK_EVENT_ALLOCATION_FAILURE  -100
#define CLK_ENQUEUE_FAILURE           -101
#define CLK_INVALID_QUEUE             -102
#define CLK_INVALID_NDRANGE           -160
#define CLK_DEVICE_QUEUE_FULL         -161

#define CL_COMPLETE                   0x0
#define CL_SUBMITTED                  0x2


        // Profiling info name (see capture_event_profiling_info)
#define CLK_PROFILING_COMMAND_EXEC_TIME 0x1

#define MAX_WORK_DIM        3

#define CLK_NULL_RESERVE_ID __builtin_astype((void*)NULL, reserve_id_t)


#endif   // !defined (__MIC__) && !defined(__MIC2__)
