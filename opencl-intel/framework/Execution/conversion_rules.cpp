/**
 * This file implements image channel conversions as described in chapter 8.3 of the spec.
 * See also 5.3.1.1 (Image Format Descriptor) for channel data type, and order.
 */


#include <assert.h>
#include <math.h>
#include <string.h>

#ifdef WIN32
    #include <intrin.h>
    #include <limits.h>
#else
	#ifdef __AVX__
		// immintrin is a meta header that includes Intel headers, but it is new
		// to AVX.
		#include <immintin.h>
	#else
		// copied from immintrin.h
		#ifdef __SSE3__
		#include <pmmintrin.h>
		#endif

		#ifdef __SSSE3__
		#include <tmmintrin.h>
		#endif

		#if defined (__SSE4_2__) || defined (__SSE4_1__)
		#include <smmintrin.h>
		#endif

		#if defined (__AES__) || defined (__PCLMUL__)
		#include <wmmintrin.h>
		#endif
	#endif
#endif

#include <cl_types.h>

#include "cl_sys_defines.h"

#include "conversion_rules.h"

// to overcome WinDef.h problem of min/max macro:
#define OCL_MIN(x, y) (x < y) ? x : y

#define CLAMP(val, low, high) (val < low) ? low : ((val > high) ? high : val)


#define TEN_BIT_MAX  0x3ff
#define SIX_BIT_MAX  0x3f
#define FIVE_BIT_MAX 0x1f

#ifdef WIN32
	#define AS_INTRIN_128 __declspec(intrin_type) _CRT_ALIGN(16)
#else
	#define AS_INTRIN_128 __attribute__ ((__vector_size__ (16), __may_alias__))
#endif

#define SHUFFLE_EPI8(x, mask) \
	_mm_shuffle_epi8((__m128i)x, (__m128i)mask)

// Due to a bug in gcc 4.4.X (on RHEL 6) we must not optimize several functions.
#if __GNUC__ == 4
#if __GNUC_MINOR__ == 4
    #define NOOPTIMIZE __attribute__((optimize(0)))
#endif
#endif

#ifndef NOOPTIMIZE
    #define NOOPTIMIZE
#endif


/**
 * The SIMD HALF conversion function DOES NOT WORK properly at the moment. Will fix it in the future.
 */
//#define FLOAT2HALF_SIMD 1
#undef FLOAT2HALF_SIMD

ALIGN16 int ones[] = {1, 1, 1, 1};

#ifdef WIN32
	__m128i _4x32to4x16 = {0, 1, 4, 5, 8, 9, 12, 13, 0, 0, 0, 0, 0,0,0,0};
	__m128i allzero     = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#else
	char AS_INTRIN_128 _4x32to4x16 = {0, 1, 4, 5, 8, 9, 12, 13, 0, 0, 0, 0, 0,0,0,0};
	char AS_INTRIN_128 allzero     = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#endif

 
static __m128i convert_to_int(const __m128 &val)
{
    int old_csr = _MM_GET_ROUNDING_MODE();
    _MM_SET_ROUNDING_MODE(0); // set rounding mode to RTE, which is 0 mask.
    
	__m128i ret128i = _mm_cvtps_epi32( val );
    _MM_SET_ROUNDING_MODE(old_csr);
	return ret128i;
}


/**
 * Saturate - set integer in boundary.
 * @param val
 * @return the value in boundary of the relevant integer.
 */
template <typename TargetIntVector>
TargetIntVector NOOPTIMIZE sat(const __m128i &intVal)
{
	TargetIntVector *t = (TargetIntVector*)&intVal;
	return *t;
}

// short/ushort 16 bits
template<> cl_short4 NOOPTIMIZE sat<cl_short4>(const __m128i &intVal)
{
    cl_short4 target;

	__m128i shortVal = _mm_packs_epi32(intVal, (__m128i)allzero);
    _mm_storel_epi64((__m128i*)&target, shortVal);

	return target;
}

template<> cl_ushort4 NOOPTIMIZE sat<cl_ushort4>(const __m128i &intVal)
{
    cl_ushort4 trgt;
    cl_int4 *cl_intVal = (cl_int4 *)&intVal;

    /*
	//if AVX: __m128i ushortVal = _mm_packus_epi32(intVal, (__m128i)allzero);
    // no single command to convert to unsigned, so value is signed.
    */
    for (int i = 0 ; i < 4 ; ++i)
    {
        trgt.s[i] = CLAMP(cl_intVal->s[i], 0, USHRT_MAX);
    }
	return trgt;
}


// char/uchar 8 bits
template<> cl_char4 NOOPTIMIZE sat<cl_char4>(const __m128i &intVal)
{
	__m128i shortVal = _mm_packs_epi32(intVal, (__m128i)allzero);
	__m128i charVal = _mm_packs_epi16(shortVal, (__m128i)allzero);

	cl_char4 trgt;
	cl_char16 *t = (cl_char16*)&charVal;
	for (int i = 0 ; i < 4 ; ++i) trgt.s[i] = t->s[i];

	return trgt;
}

template<> cl_uchar4 NOOPTIMIZE sat<cl_uchar4>(const __m128i &intVal)
{
	__m128i shortVal = _mm_packs_epi32(intVal, (__m128i)allzero);
	__m128i ucharVal = _mm_packus_epi16(shortVal, (__m128i)allzero);

	cl_uchar4 trgt;
	cl_uchar16 *t = (cl_uchar16*)&ucharVal;
	for (int i = 0 ; i < 4 ; ++i) trgt.s[i] = t->s[i];

	return trgt;

}


/**
 * Convert float to int
 * @param val
 * @param multiplier value to multiply each of the float4 elements.
 * @return
 */
template <typename TargetIntVect>
TargetIntVect floatVec2IntVec(const cl_float4 val, const float multiplier)
{
	__m128 orig = _mm_loadu_ps(val.s);
	__m128 mul_by = {multiplier, multiplier, multiplier, multiplier};
	__m128 mul = _mm_mul_ps(orig, mul_by);
	TargetIntVect ret = sat<TargetIntVect>(convert_to_int(mul) );
	return ret;
}


#define MASK_AS_128i(val) _mm_set1_epi32(val)
#define MASK_AS_128(val) _mm_set1_ps(val)

__m128 float2half_rte_simd(__m128 param)
{
    // _mm_cvtps_epi32 _m128 --> _m128i
    // _mm_cvtepi32_ps _m128i --> _m128

    int rm = _MM_GET_ROUNDING_MODE();
    _MM_SET_ROUNDING_MODE(_MM_ROUND_NEAREST);

    //cl_uint sign = (u.u >> 16) & 0x8000;
    __m128i temp = (__m128i)_mm_srli_epi32(_mm_cvtps_epi32(param), 0x10);
    __m128i signs = (__m128i)_mm_and_si128((__m128i)temp, MASK_AS_128i(0x8000));
    
    __m128 absParam = _mm_cvtepi32_ps(_mm_and_si128(_mm_cvtps_epi32(param), MASK_AS_128i(0x7fffffff)));

    //Nan
    //if( x != x )
    __m128i eq0 = _mm_cvtps_epi32(_mm_cmpneq_ps(absParam, absParam));
    __m128i eq = _mm_and_si128(_mm_cvtps_epi32(absParam),(__m128i) eq0);
    //u.u >>= (24-11);
    eq = (__m128i) _mm_srli_epi32((__m128i)eq, 0x0d);
    //u.u &= 0x7fff;
    eq = (__m128i) _mm_and_si128((__m128i)eq, MASK_AS_128i(0x7fff));
    //u.u |= 0x0200;   -- silence the NaN
    eq = (__m128i) _mm_or_si128((__m128i)eq, MASK_AS_128i(0x0200));
    //return u.u | sign;
    eq = (__m128i) _mm_or_si128((__m128i)eq,(__m128i) signs);
    eq = (__m128i) _mm_and_si128((__m128i)eq,(__m128i) eq0);
    __m128i dflt = eq0;

    // overflow
    //if( x >= MAKE_HEX_FLOAT(0x1.ffcp15f, 0x1ffcL, 3) )
    //return 0x7c00 | sign;
    __m128 eq1 = _mm_cmpge_ps(absParam, MASK_AS_128(0x477ff000));
    eq0 = _mm_and_si128(_mm_cvtps_epi32(eq1), MASK_AS_128i(0x7c00));
    eq0 = (__m128i) _mm_or_si128((__m128i)signs,(__m128i) eq0);
    eq0 = (__m128i) _mm_andnot_si128((__m128i)dflt,(__m128i) eq0);
    eq = (__m128i) _mm_or_si128((__m128i)eq,(__m128i) eq0);
    dflt = (__m128i) _mm_or_si128(_mm_cvtps_epi32(eq1),(__m128i) dflt);

    // underflow
    //  if( x <= MAKE_HEX_FLOAT(0x1.0p-25f, 0x1L, -25) )
    // return sign
    
    eq1 = _mm_cmple_ps(absParam, MASK_AS_128(0x33000000));
    eq0 = (__m128i) _mm_and_si128(_mm_cvtps_epi32(eq1),(__m128i) signs);
    eq0 = (__m128i) _mm_andnot_si128((__m128i)dflt,(__m128i) eq0);
    eq = (__m128i) _mm_or_si128((__m128i)eq,(__m128i) eq0);
    dflt = (__m128i) _mm_or_si128(_mm_cvtps_epi32(eq1),(__m128i) dflt);


    // very small
    //  if( x < MAKE_HEX_FLOAT(0x1.8p-24f, 0x18L, -28) )
    // return sign | 1;
    eq1 = _mm_cmplt_ps(absParam, MASK_AS_128(0x33c00000));
    eq0 = (__m128i) _mm_and_si128(_mm_cvtps_epi32(eq1), _mm_or_si128((__m128i)signs,(__m128i) *((__m128i *)ones)));
    eq0 = (__m128i) _mm_andnot_si128((__m128i)dflt,(__m128i) eq0);
    eq = (__m128i) _mm_or_si128((__m128i)eq,(__m128i) eq0);
    dflt = (__m128i) _mm_or_si128(_mm_cvtps_epi32(eq1),(__m128i) dflt);

    // half denormal
    //  if( x < MAKE_HEX_FLOAT(0x1.0p-14f, 0x1L, -14) )
    //  x *= MAKE_HEX_FLOAT(0x1.0p-125f, 0x1L, -125);
    //  return sign | x;
    eq1 = _mm_cmplt_ps(absParam, MASK_AS_128(0x38800000));
    __m128 eq2 = _mm_mul_ps(absParam, MASK_AS_128(0x01000000));  //x
    eq0 = (__m128i) _mm_and_si128(_mm_cvtps_epi32(eq1), _mm_or_si128((__m128i)signs,(__m128i) _mm_cvtps_epi32(eq2)));
    eq0 = (__m128i) _mm_andnot_si128((__m128i)dflt,(__m128i) eq0);
    eq = (__m128i) _mm_or_si128((__m128i)eq,(__m128i) eq0);
    dflt = (__m128i) _mm_or_si128(_mm_cvtps_epi32(eq1),(__m128i) dflt);

    // u.f *= MAKE_HEX_FLOAT(0x1.0p13f, 0x1L, 13);
    // u.u &= 0x7f800000;
    // x += u.f;
    // u.f = x - u.f;
    // u.f *= MAKE_HEX_FLOAT(0x1.0p-112f, 0x1L, -112);
    // return (u.u >> (24-11)) | sign;
    //int rm = _MM_GET_ROUNDING_MODE();
    //_MM_SET_ROUNDING_MODE(0x6000);

    eq1 = _mm_mul_ps(param, MASK_AS_128(0x46000000));
    eq0 = (__m128i) _mm_and_si128(_mm_cvtps_epi32(eq1), MASK_AS_128i(0x7f800000)); //u
    eq1 = _mm_add_ps( _mm_cvtepi32_ps(eq0) ,absParam); //x
    eq1 = _mm_sub_ps(eq1, _mm_cvtepi32_ps(eq0)); //u
    eq1 = _mm_mul_ps(eq1, MASK_AS_128(0x07800000));
    eq0 = (__m128i) _mm_srli_epi32(_mm_cvtps_epi32(eq1), 0x0d);
    eq0 = (__m128i) _mm_or_si128((__m128i)eq0,(__m128i) signs);
    eq0 = (__m128i) _mm_andnot_si128((__m128i)dflt,(__m128i) eq0);
    eq = (__m128i) _mm_or_si128((__m128i)eq,(__m128i) eq0);

    eq1 = _mm_castsi128_ps( SHUFFLE_EPI8(eq, _4x32to4x16) );

    _MM_SET_ROUNDING_MODE(rm);

    return eq1;
}


#if defined(_WIN32) && defined (_MSC_VER)
    #define MAKE_HEX_FLOAT(x,y,z)  ((float)ldexp( (float)(y), z))
    #define MAKE_HEX_DOUBLE(x,y,z) ldexp( (double)(y), z)
    #define MAKE_HEX_LONG(x,y,z)   ((long double) ldexp( (long double)(y), z))
#else
    #define MAKE_HEX_FLOAT(x,y,z)  ((float)ldexp( (float)(y), z))
    #define MAKE_HEX_DOUBLE(x,y,z) ldexp( (double)(y), z)
    #define MAKE_HEX_LONG(x,y,z)   ((long double) ldexp( (long double)(y), z))
#endif
cl_ushort float2half_rte( float f )
{
    union{ float f; cl_uint u; } u = {f};
    cl_uint sign = (u.u >> 16) & 0x8000;
    float x = fabsf(f);
    
    //Nan
    if( x != x )
    {
        u.u >>= (24-11);
        u.u &= 0x7fff;
        u.u |= 0x0200;      //silence the NaN
        return u.u | sign;
    }
        
    // overflow
    if( x >= MAKE_HEX_FLOAT(0x1.ffep15f, 0x1ffeL, 3) )
        return 0x7c00 | sign;
    
    // underflow
    if( x <= MAKE_HEX_FLOAT(0x1.0p-25f, 0x1L, -25) )
        return sign;    // The halfway case can return 0x0001 or 0. 0 is even.
    
    // very small
    if( x < MAKE_HEX_FLOAT(0x1.8p-24f, 0x18L, -28) )
        return sign | 1;

    // half denormal
    if( x < MAKE_HEX_FLOAT(0x1.0p-14f, 0x1L, -14) )
    {
        u.f = x * MAKE_HEX_FLOAT(0x1.0p-125f, 0x1L, -125);
        return sign | u.u;
    }

    u.f *= MAKE_HEX_FLOAT(0x1.0p13f, 0x1L, 13);
    u.u &= 0x7f800000;
    x += u.f;
    u.f = x - u.f;
    u.f *= MAKE_HEX_FLOAT(0x1.0p-112f, 0x1L, -112);
    
    return (u.u >> (24-11)) | sign;
}


template <typename VecType>
cl_int __arrange_by_channel_order(VecType *trgtColor, const VecType *srcColor, const cl_channel_order channelOrder)
{
    // start by cleaning target:
    //memset(trgtColor, 0, sizeof(VecType));
    MEMCPY_S(trgtColor, sizeof(VecType), srcColor, sizeof(VecType));
    
    #define MAP_CHANNEL(dst, src) trgtColor->s[dst] = srcColor->s[src];
    
    switch(channelOrder)
    {
        case CL_RGBA:
        case CL_RGBx:
            MAP_CHANNEL(3, 3);
        case CL_RGB:
        case CL_RGx:
            MAP_CHANNEL(2, 2);
        case CL_RG:
        case CL_Rx:
            MAP_CHANNEL(1, 1);
        case CL_R:
            MAP_CHANNEL(0, 0);
            break;
            
        case CL_RA:
            MAP_CHANNEL(0, 0);
            MAP_CHANNEL(1, 3);
            break;
            
        case CL_A:
            MAP_CHANNEL(0, 3);
            break;
            
        case CL_BGRA:
            MAP_CHANNEL(0, 2);
            MAP_CHANNEL(1, 1);
            MAP_CHANNEL(2, 0);
            MAP_CHANNEL(3, 3);
            break;
            
        case CL_ARGB:
            MAP_CHANNEL(0, 3);
            MAP_CHANNEL(1, 0);
            MAP_CHANNEL(2, 1);
            MAP_CHANNEL(3, 2);
            break;
            
        case CL_INTENSITY:
        case CL_LUMINANCE:
            MAP_CHANNEL(0, 0);
            break;
            
        default:
            return CL_IMAGE_FORMAT_NOT_SUPPORTED;
    }
    return CL_SUCCESS;
}


template <typename VecType>
cl_int __RGBA_fp_to_NORM(const cl_float4 *color,
                         const cl_channel_order channelOrder, VecType* trgt,
                         const cl_float multiplier)
{
    VecType modcolor = floatVec2IntVec<VecType>(*color, multiplier);
    return __arrange_by_channel_order<VecType>(trgt, &modcolor, channelOrder);
}


/**
 * Convert RGBA floating point color to image format requested.
 *
 * @param color RGBA float format
 * @param channelOrder target channel order
 * @param channelType target channel type
 * @param trgtColor allocated space for target color
 * @param trgtLength size (bytes) of target color
 *
 * @return
 * - CL_SUCCESS if OK
 * - CL_IMAGE_FORMAT_NOT_SUPPORTED if cannot convert to the specified format.
 * - CL_INVALID_ARG_SIZE if trgtLength is inappropriate for channel type.
 * 	@
 */
cl_int Intel::OpenCL::Framework::norm_float_to_image(const cl_float4 *color, const cl_channel_order channelOrder,
                           const cl_channel_type channelType, void* trgtColor, const size_t trgtLength)
{
    assert(trgtColor);

    switch (channelType)
    {
        case CL_SNORM_INT8:
        	return __RGBA_fp_to_NORM<cl_char4>(color, channelOrder, (cl_char4*)trgtColor, 127.0);
            break;
        case CL_SNORM_INT16:
        	return __RGBA_fp_to_NORM<cl_short4>(color, channelOrder, (cl_short4*)trgtColor, 65535.0);
            break;
        case CL_UNORM_INT8:
        	return __RGBA_fp_to_NORM<cl_uchar4>(color, channelOrder, (cl_uchar4*)trgtColor, 255.0);
            break;
        case CL_UNORM_INT16:
        	return __RGBA_fp_to_NORM<cl_ushort4>(color, channelOrder, (cl_ushort4*)trgtColor, 65535.0);
            break;
        case CL_UNORM_INT_101010:
        	{
        		// convert to cl_ushort, and from there round down.
        		cl_ushort4 tempColor;
    			cl_uint &trgt = *(cl_uint*)trgtColor; // output is 32 bit

    			__RGBA_fp_to_NORM<cl_ushort4>(color, channelOrder, &tempColor, 1023.0);
    			trgt = 0;
				trgt |= OCL_MIN(tempColor.s[0], TEN_BIT_MAX);
				trgt <<= 10;
        		trgt |= OCL_MIN(tempColor.s[1], TEN_BIT_MAX);
				trgt <<= 10;
        		trgt |= OCL_MIN(tempColor.s[2], TEN_BIT_MAX);
        	}
            break;
        case CL_FLOAT:
        	{
				// same as input.
				cl_float4 *trgt = (cl_float4 *)trgtColor;
                __arrange_by_channel_order<cl_float4>(trgt, color, channelOrder);
        	}
        	break;
        case CL_HALF_FLOAT:
			{
				cl_ushort4 *trgt = (cl_ushort4 *)trgtColor;
                
                cl_float4 orderedFloat; // start by rearranging channel order.
                __arrange_by_channel_order<cl_float4>(&orderedFloat, color, channelOrder);

#ifdef FLOAT2HALF_SIMD
                // using SIMD intrinsics:
			    __m128 tmpColor;
                cl_float4 tmpFloat;
				tmpColor = float2half_rte_simd(_mm_loadu_ps(orderedFloat.s));
				_mm_storeu_ps(tmpFloat.s, tmpColor);
                memcpy(trgt, &tmpFloat, sizeof(cl_ushort4));
#else // FLOAT2HALF_SIMD                
                // using function taken from 1.2 conformance:
                cl_ushort* dest = (cl_ushort*)trgt->s;
                for (size_t i=0 ; i < 4 ; ++i)
                {
                    cl_ushort res = float2half_rte(orderedFloat.s[i]);
                    dest[i] = res;
                    //memcpy(dest + (i*halfSize), &res, halfSize);
                }
#endif// FLOAT2HALF_SIMD
                /*
                fprintf( stderr, "Float to Half (order: %d  type: %d), original float: %f,%f,%f,%f swizelled: %f,%f,%f,%f "
                            "Converted to half: %02x,%02x,%02x,%02x\n",
                            channelOrder, channelType,
                            color->s[0], color->s[1], color->s[2], color->s[3],
                            orderedFloat.s[0], orderedFloat.s[1], orderedFloat.s[2], orderedFloat.s[3],
                            trgt->s[0], trgt->s[1], trgt->s[2], trgt->s[3]
                        );
                */

			}
        	break;
        case CL_UNORM_SHORT_565:
			{
				// convert to uint8, and from there round down.
				cl_uchar4 tempColor;
				cl_ushort &trgt = *(static_cast<cl_ushort*>(trgtColor)); // output is 16 bit

				__RGBA_fp_to_NORM<cl_uchar4>(color, channelOrder, &tempColor, 255.0);
				trgt = 0;
				trgt |= OCL_MIN(tempColor.s[0], FIVE_BIT_MAX);
				trgt <<=6;
				trgt |= OCL_MIN(tempColor.s[1], SIX_BIT_MAX);
				trgt <<=5;
				trgt |= OCL_MIN(tempColor.s[2], FIVE_BIT_MAX);
			}
			break;
        case CL_UNORM_SHORT_555:
			{
				// convert to uint8, and from there round down.
				cl_uchar4 tempColor;
				cl_ushort &trgt = *(static_cast<cl_ushort*>(trgtColor)); // output is 16 bit

				__RGBA_fp_to_NORM<cl_uchar4>(color, channelOrder, &tempColor, 255.0);
				trgt = 0;
				trgt |= OCL_MIN(tempColor.s[0], FIVE_BIT_MAX);
				trgt <<= 5;
				trgt |= OCL_MIN(tempColor.s[1], FIVE_BIT_MAX);
				trgt <<= 5;
				trgt |= OCL_MIN(tempColor.s[2], FIVE_BIT_MAX);
			}
			break;
        default:
            return CL_IMAGE_FORMAT_NOT_SUPPORTED;
    }

    return CL_SUCCESS;
}


template <typename OrigVecType, typename VecType>
cl_int __ANYINT_to_ANYINT(const OrigVecType *color, void *trgtPtr, const cl_channel_order order)
{
    __m128i color128;
	memcpy(&color128, color->s, sizeof(OrigVecType));
    VecType tmpColor = sat<VecType>(color128);
    
    VecType *trgt = (VecType *)trgtPtr;
    return __arrange_by_channel_order<VecType>(trgt, &tmpColor, order);
}

/**
 * Convert non normalized signed int point color to image format requested.
 *
 * @param color non normalized signed int.
 * @param channelOrder target channel order
 * @param channelType target channel type
 * @param trgtColor allocated space for target color
 * @param trgtLength size (bytes) of target color
 *
 * @return
 * - CL_SUCCESS if OK
 * - CL_IMAGE_FORMAT_NOT_SUPPORTED if cannot convert to the specified format.
 * - CL_INVALID_ARG_SIZE if trgtLength is inappropriate for channel type.
 * 	@
 */
cl_int Intel::OpenCL::Framework::non_norm_signed_to_image(const cl_int4 *color, const cl_channel_order channelOrder,
                           const cl_channel_type channelType, void* trgtColor, const size_t trgtLength)
{
    assert(trgtColor);

    switch (channelType)
    {
    case CL_SIGNED_INT8:
    	{
    		return __ANYINT_to_ANYINT<cl_int4, cl_char4>(color, trgtColor, channelOrder);
    	}
    	break;
    case CL_SIGNED_INT16:
		{
	    	return __ANYINT_to_ANYINT<cl_int4, cl_short4>(color, trgtColor, channelOrder);
		}
		break;
    case CL_SIGNED_INT32:
		{
			// same as input
    		return __ANYINT_to_ANYINT<cl_int4, cl_int4>(color, trgtColor, channelOrder);
		}
		break;
    default:
    	return CL_IMAGE_FORMAT_NOT_SUPPORTED;
	}

	return CL_SUCCESS;
}


/**
 * Convert non normalized unsigned int point color to image format requested.
 *
 * @param color non normalized unsigned int.
 * @param channelOrder target channel order
 * @param channelType target channel type
 * @param trgtColor allocated space for target color
 * @param trgtLength size (bytes) of target color
 *
 * @return
 * - CL_SUCCESS if OK
 * - CL_IMAGE_FORMAT_NOT_SUPPORTED if cannot convert to the specified format.
 * - CL_INVALID_ARG_SIZE if trgtLength is inappropriate for channel type.
 * 	@
 */
cl_int Intel::OpenCL::Framework::non_norm_unsigned_to_image(const cl_uint4 *color, const cl_channel_order channelOrder,
                           const cl_channel_type channelType, void* trgtColor, const size_t trgtLength)
{
    assert(trgtColor);

    switch (channelType)
    {
    case CL_UNSIGNED_INT8:
    	{
    		return __ANYINT_to_ANYINT<cl_uint4, cl_uchar4>(color, trgtColor, channelOrder);
    	}
    	break;
    case CL_UNSIGNED_INT16:
		{
	    	return __ANYINT_to_ANYINT<cl_uint4, cl_ushort4>(color, trgtColor, channelOrder);
		}
		break;
    case CL_UNSIGNED_INT32:
		{
			// same as input
    		return __ANYINT_to_ANYINT<cl_uint4, cl_uint4>(color, trgtColor, channelOrder);
		}
		break;
    default:
    	return CL_IMAGE_FORMAT_NOT_SUPPORTED;
	}

	return CL_SUCCESS;
}

