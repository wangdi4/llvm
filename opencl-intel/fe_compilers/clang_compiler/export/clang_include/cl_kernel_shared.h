

#ifndef __CL_KERNEL_SHARED_H__
#define __CL_KERNEL_SHARED_H__

#ifndef __OPENCL_TYPES_DEFINED__
    #include <stddef.h>
    #include <stdint.h>

    typedef char            char16      __attribute__ ((__vector_size__(16)));
    typedef unsigned char   uchar16     __attribute__ ((__vector_size__(16)));
    typedef short           short8      __attribute__ ((__vector_size__(16)));
    typedef unsigned short  ushort8     __attribute__ ((__vector_size__(16)));
    typedef float           float4      __attribute__ ((__vector_size__(16)));
    typedef int             int4        __attribute__ ((__vector_size__(16)));
    typedef unsigned int    uint4       __attribute__ ((__vector_size__(16)));
    typedef long long       long2       __attribute__ ((__vector_size__(16)));
    typedef unsigned long long  ulong2  __attribute__ ((__vector_size__(16)));
    typedef double          double2     __attribute__ ((__vector_size__(16)));
    typedef int             sampler_t;  
    typedef size_t          event_t;
    typedef unsigned char   uchar;
    typedef unsigned short  ushort;
    typedef unsigned int    uint;

    typedef struct short16{ short8 lo, hi;  }short16;
    typedef struct ushort16{ ushort8 lo, hi;  }ushort16;
    typedef struct int8{ int4 lo, hi; }int8;
    typedef struct int16{ struct{ int4 lo, hi; }lo; struct{ int4 lo, hi; }hi; }int16;
    typedef struct uint16{ struct{ uint4 lo, hi; }lo; struct{ uint4 lo, hi; }hi; }uint16;
    typedef struct float16{ struct{ float4 lo, hi; }lo; struct{ float4 lo, hi; }hi; }float16;

    #define __OPENCL_TYPES_DEFINED__
#endif /* __OPENCL_TYPES_DEFINED__ */

// Channel order, must match cl.h
enum {
  CLK_R,
  CLK_A,
  CLK_RG,
  CLK_RA,
  CLK_RGB,
  CLK_RGBA,
  CLK_BGRA,
  CLK_ARGB,
  CLK_INTENSITY,
  CLK_LUMINANCE
};

typedef enum clk_channel_type{
  CLK_SNORM_INT8,
  CLK_SNORM_INT16,
  CLK_UNORM_INT8,
  CLK_UNORM_INT16,
  CLK_UNORM_SHORT_565,
  CLK_UNORM_SHORT_555,
  CLK_UNORM_INT_101010,

  CLK_SIGNED_INT8,
  CLK_SIGNED_INT16,
  CLK_SIGNED_INT32,
  CLK_UNSIGNED_INT8,
  CLK_UNSIGNED_INT16,
  CLK_UNSIGNED_INT32,

  CLK_HALF_FLOAT,            // four channel RGBA half
  CLK_FLOAT,                 // four channel RGBA float

  __CLK_VALID_IMAGE_TYPE_COUNT,
  __CLK_INVALID_IMAGE_TYPE = __CLK_VALID_IMAGE_TYPE_COUNT,
  __CLK_VALID_IMAGE_TYPE_MASK_BITS = 4,         // number of bits required to represent any image type
  __CLK_VALID_IMAGE_TYPE_MASK = ( 1 << __CLK_VALID_IMAGE_TYPE_MASK_BITS ) - 1
}clk_channel_type;

typedef enum clk_sampler_type
{
    __CLK_ADDRESS_BASE             = 0,
    CLK_ADDRESS_NONE               = 0 << __CLK_ADDRESS_BASE,
    CLK_ADDRESS_CLAMP              = 1 << __CLK_ADDRESS_BASE,
    CLK_ADDRESS_CLAMP_TO_EDGE      = 2 << __CLK_ADDRESS_BASE,
    CLK_ADDRESS_REPEAT             = 3 << __CLK_ADDRESS_BASE,
    CLK_ADDRESS_MIRROR             = 4 << __CLK_ADDRESS_BASE,
    __CLK_ADDRESS_MASK             = CLK_ADDRESS_NONE | CLK_ADDRESS_CLAMP | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_ADDRESS_REPEAT | CLK_ADDRESS_MIRROR,
    __CLK_ADDRESS_BITS             = 3,        // number of bits required to represent address info

    __CLK_NORMALIZED_BASE          = __CLK_ADDRESS_BITS,
    CLK_NORMALIZED_COORDS_FALSE    = 0,
    CLK_NORMALIZED_COORDS_TRUE     = 1 << __CLK_NORMALIZED_BASE,
    __CLK_NORMALIZED_MASK          = CLK_NORMALIZED_COORDS_FALSE | CLK_NORMALIZED_COORDS_TRUE,
    __CLK_NORMALIZED_BITS          = 1,        // number of bits required to represent normalization 

    __CLK_FILTER_BASE              = __CLK_NORMALIZED_BASE + __CLK_NORMALIZED_BITS,
    CLK_FILTER_NEAREST             = 0 << __CLK_FILTER_BASE,
    CLK_FILTER_LINEAR              = 1 << __CLK_FILTER_BASE,
    CLK_FILTER_ANISOTROPIC         = 2 << __CLK_FILTER_BASE,
    __CLK_FILTER_MASK              = CLK_FILTER_NEAREST | CLK_FILTER_LINEAR | CLK_FILTER_ANISOTROPIC,
    __CLK_FILTER_BITS              = 2,        // number of bits required to represent address info

    __CLK_MIP_BASE                 = __CLK_FILTER_BASE + __CLK_FILTER_BITS,
    CLK_MIP_NEAREST                = 0 << __CLK_MIP_BASE,
    CLK_MIP_LINEAR                 = 1 << __CLK_MIP_BASE,
    CLK_MIP_ANISOTROPIC            = 2 << __CLK_MIP_BASE,
    __CLK_MIP_MASK                 = CLK_MIP_NEAREST | CLK_MIP_LINEAR | CLK_MIP_ANISOTROPIC,
    __CLK_MIP_BITS                 = 2,
  
    __CLK_SAMPLER_BITS             = __CLK_MIP_BASE + __CLK_MIP_BITS,
    __CLK_SAMPLER_MASK             = __CLK_MIP_MASK | __CLK_FILTER_MASK | __CLK_NORMALIZED_MASK | __CLK_ADDRESS_MASK,
    
    __CLK_ANISOTROPIC_RATIO_BITS   = 5,
    __CLK_ANISOTROPIC_RATIO_MASK   = (int) 0x80000000 >> (__CLK_ANISOTROPIC_RATIO_BITS-1)
}clk_sampler_type;


#define __ALWAYS_INLINE       __attribute__ ((__always_inline__))
#if defined( __clang__ )
    //FIXME: commented out per <rdar://problem/6307429> ABI bustage in read_image
    #define __FAST_CALL           /* __attribute__ ((fastcall)) */
#else
    #define __FAST_CALL
#endif

#endif /* __CL_KERNEL_SHARED_H__ */
