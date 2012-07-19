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

#pragma once

#include "cl_types2.h"
int __attribute__((overloadable)) movemask ( int4 mask );
int __attribute__((overloadable)) movemask ( long4 mask );
int __attribute__((overloadable)) movemask ( int8 mask );
int __attribute__((overloadable)) movemask ( long8 mask );
int __attribute__((overloadable)) movemask ( int16 mask );
int __attribute__((overloadable)) movemask ( long16 mask );

// Define function generic prototypes
#define DEF_GEOM_PROTO_F_F(FUNC)

#define DEF_GEOM_PROTO_D_D(FUNC)

#define DEF_GEOM_PROTO_F2_F(FUNC) \
	float __##FUNC##f2as4(float4 x);\
	float __attribute__((overloadable)) FUNC(float2 x)\
	{\
        float4	tempX = (float4)(0);\
		tempX.lo = x;\
		return __##FUNC##f2as4(tempX);\
	}

#define DEF_GEOM_PROTO_D2_D(FUNC)

#define DEF_GEOM_PROTO_F3_F(FUNC) \
	float __attribute__((overloadable)) FUNC(float3 x)\
	{\
		float4	tempX = (float4)(0);\
		tempX.s012 = x;\
		return FUNC(tempX);\
	}

#define DEF_GEOM_PROTO_D3_D(FUNC) \
	double __attribute__((overloadable)) FUNC(double3 x)\
	{\
		double4	tempX = (double4)(0);\
		tempX.s012 = x;\
		return FUNC(tempX);\
	}

#define DEF_GEOM_PROTO_F4_F(FUNC)

#define DEF_GEOM_PROTO_D4_D(FUNC)

#define DEF_GEOM_PROTO_F_F_F(FUNC)

#define DEF_GEOM_PROTO_D_D_D(FUNC)

#define DEF_GEOM_PROTO_F2_F2_F(FUNC) \
	float __##FUNC##f2as4(float4 x, float4 y);\
	float __attribute__((overloadable)) FUNC(float2 x, float2 y)\
	{\
		float4	tempX = (float4)(0);\
		tempX.lo = x;\
		float4	tempY = (float4)(0);\
		tempY.lo = y;\
		return __##FUNC##f2as4(tempX, tempY);\
	}

#define DEF_GEOM_PROTO_D2_D2_D(FUNC)

#define DEF_GEOM_PROTO_F3_F3_F(FUNC) \
	float __attribute__((overloadable)) FUNC(float3 x, float3 y)\
	{\
		float4	tempX = (float4)(0);\
		float4	tempY = (float4)(0);\
		tempX.s012 = x;\
		tempY.s012 = y;\
		return FUNC(tempX, tempY);\
	}

#define DEF_GEOM_PROTO_D3_D3_D(FUNC) \
	double __attribute__((overloadable)) FUNC(double3 x, double3 y)\
	{\
		double4	tempX = (double4)(0);\
		double4	tempY = (double4)(0);\
		tempX.s012 = x;\
		tempY.s012 = y;\
		return FUNC(tempX, tempY);\
	}

#define DEF_GEOM_PROTO_F4_F4_F(FUNC)

#define DEF_GEOM_PROTO_D4_D4_D(FUNC)

#define DEF_GEOM_PROTO_F2_F2(FUNC) \
	float4 __##FUNC##f2as4(float4 x);\
	float2 __attribute__((overloadable)) FUNC(float2 x)\
	{\
		float4	tempX = (float4)(0);\
		tempX.lo = x;\
		tempX = __##FUNC##f2as4(tempX);\
		return tempX.lo;\
	}

#define DEF_GEOM_PROTO_D2_D2(FUNC)

#define DEF_GEOM_PROTO_F3_F3(FUNC) \
	float3 __attribute__((overloadable)) FUNC(float3 x)\
	{\
		float4	tempX = (float4)(0);\
		float4	res;\
		tempX.s012 = x;\
		res = FUNC(tempX);\
		return res.s012;\
	}

#define DEF_GEOM_PROTO_D3_D3(FUNC) \
	double3 __attribute__((overloadable)) FUNC(double3 x)\
	{\
		double4	tempX = (double4)(0);\
		double4	res;\
		tempX.s012 = x;\
		res = FUNC(tempX);\
		return res.s012;\
	}

#define DEF_GEOM_PROTO_F4_F4(FUNC)

#define DEF_GEOM_PROTO_D4_D4(FUNC)

#define DEF_GEOM_PROTO_F3_F3_F3(FUNC)\
	float3 __attribute__((overloadable)) FUNC(float3 x, float3 y)\
	{\
		float4	tempX = (float4)(0);\
		float4	tempY = (float4)(0);\
		float4	res;\
		tempX.s012 = x;\
		tempY.s012 = y;\
		res = FUNC(tempX, tempY);\
		return res.s012;\
	}

#define DEF_GEOM_PROTO_D3_D3_D3(FUNC)\
	double3 __attribute__((overloadable)) FUNC(double3 x, double3 y)\
	{\
		double4	tempX = (double4)(0);\
		double4	tempY = (double4)(0);\
		double4	res;\
		tempX.s012 = x;\
		tempY.s012 = y;\
		res = FUNC(tempX, tempY);\
		return res.s012;\
	}

#define DEF_GEOM_PROTO_F4_F4_F4(FUNC)

#define DEF_GEOM_PROTO_D4_D4_D4(FUNC)


DEF_GEOM_PROTO_F_F(normalize)
DEF_GEOM_PROTO_F2_F2(normalize)
DEF_GEOM_PROTO_F3_F3(normalize)
DEF_GEOM_PROTO_F4_F4(normalize)

DEF_GEOM_PROTO_D_D(normalize)
DEF_GEOM_PROTO_D2_D2(normalize)
DEF_GEOM_PROTO_D3_D3(normalize)
DEF_GEOM_PROTO_D4_D4(normalize)

DEF_GEOM_PROTO_F_F(fast_normalize)
DEF_GEOM_PROTO_F2_F2(fast_normalize)
DEF_GEOM_PROTO_F3_F3(fast_normalize)
DEF_GEOM_PROTO_F4_F4(fast_normalize)

DEF_GEOM_PROTO_F_F(length)
DEF_GEOM_PROTO_F2_F(length)
DEF_GEOM_PROTO_F3_F(length)
DEF_GEOM_PROTO_F4_F(length)

DEF_GEOM_PROTO_D_D(length)
DEF_GEOM_PROTO_D2_D(length)
DEF_GEOM_PROTO_D3_D(length)
DEF_GEOM_PROTO_D4_D(length)

DEF_GEOM_PROTO_F_F(fast_length)
DEF_GEOM_PROTO_F2_F(fast_length)
DEF_GEOM_PROTO_F3_F(fast_length)
DEF_GEOM_PROTO_F4_F(fast_length)

DEF_GEOM_PROTO_F_F_F(distance)
DEF_GEOM_PROTO_F2_F2_F(distance)
DEF_GEOM_PROTO_F3_F3_F(distance)
DEF_GEOM_PROTO_F4_F4_F(distance)

DEF_GEOM_PROTO_D_D_D(distance)
DEF_GEOM_PROTO_D2_D2_D(distance)
DEF_GEOM_PROTO_D3_D3_D(distance)
DEF_GEOM_PROTO_D4_D4_D(distance)

DEF_GEOM_PROTO_F_F_F(fast_distance)
DEF_GEOM_PROTO_F2_F2_F(fast_distance)
DEF_GEOM_PROTO_F3_F3_F(fast_distance)
DEF_GEOM_PROTO_F4_F4_F(fast_distance)

DEF_GEOM_PROTO_F_F_F(dot)
DEF_GEOM_PROTO_F2_F2_F(dot)
DEF_GEOM_PROTO_F3_F3_F(dot)
DEF_GEOM_PROTO_F4_F4_F(dot)

DEF_GEOM_PROTO_D_D_D(dot)
DEF_GEOM_PROTO_D2_D2_D(dot)
DEF_GEOM_PROTO_D3_D3_D(dot)
DEF_GEOM_PROTO_D4_D4_D(dot)

DEF_GEOM_PROTO_F3_F3_F3(cross)
DEF_GEOM_PROTO_F4_F4_F4(cross)

DEF_GEOM_PROTO_D3_D3_D3(cross)
DEF_GEOM_PROTO_D4_D4_D4(cross)

/*
Dot generation macros
*/

#define DEF_GEOM_SOA_DOT1_PROTO( DATA_TYPE, VEC_WIDTH ) \
DATA_TYPE##VEC_WIDTH __attribute__((overloadable)) soa_dot1( DATA_TYPE##VEC_WIDTH p0, DATA_TYPE##VEC_WIDTH p1) \
{ \
	return (p0 * p1); \
}

#define DEF_GEOM_SOA_DOT2_PROTO( DATA_TYPE, VEC_WIDTH ) \
DATA_TYPE##VEC_WIDTH __attribute__((overloadable)) \
soa_dot2( DATA_TYPE##VEC_WIDTH p0_x, DATA_TYPE##VEC_WIDTH p0_y, \
		  DATA_TYPE##VEC_WIDTH p1_x, DATA_TYPE##VEC_WIDTH p1_y) \
{ \
	return (p0_x * p1_x) + (p0_y * p1_y); \
}

#define DEF_GEOM_SOA_DOT3_PROTO( DATA_TYPE, VEC_WIDTH ) \
DATA_TYPE##VEC_WIDTH __attribute__((overloadable)) \
soa_dot3( DATA_TYPE##VEC_WIDTH p0_x, DATA_TYPE##VEC_WIDTH p0_y, DATA_TYPE##VEC_WIDTH p0_z, \
		  DATA_TYPE##VEC_WIDTH p1_x, DATA_TYPE##VEC_WIDTH p1_y, DATA_TYPE##VEC_WIDTH p1_z) \
{ \
	return (p0_x * p1_x) + (p0_y * p1_y) + (p0_z * p1_z); \
}								
								
#define DEF_GEOM_SOA_DOT4_PROTO( DATA_TYPE, VEC_WIDTH ) \
DATA_TYPE##VEC_WIDTH __attribute__((overloadable)) \
soa_dot4( DATA_TYPE##VEC_WIDTH p0_x, DATA_TYPE##VEC_WIDTH p0_y, DATA_TYPE##VEC_WIDTH p0_z, DATA_TYPE##VEC_WIDTH p0_w, \
		  DATA_TYPE##VEC_WIDTH p1_x, DATA_TYPE##VEC_WIDTH p1_y, DATA_TYPE##VEC_WIDTH p1_z, DATA_TYPE##VEC_WIDTH p1_w) \
{ \
	return (p0_x * p1_x) + (p0_y * p1_y) + (p0_z * p1_z) + (p0_w * p1_w); \
}								

/*
Cross generation macros
*/							

#define DEF_GEOM_SOA_CROSS3_PROTO( DATA_TYPE, VEC_WIDTH ) \
void __attribute__((overloadable)) \
soa_cross3( DATA_TYPE##VEC_WIDTH p0_x, DATA_TYPE##VEC_WIDTH p0_y, DATA_TYPE##VEC_WIDTH p0_z, \
            DATA_TYPE##VEC_WIDTH p1_x, DATA_TYPE##VEC_WIDTH p1_y, DATA_TYPE##VEC_WIDTH p1_z, \
		    DATA_TYPE##VEC_WIDTH *res_x, DATA_TYPE##VEC_WIDTH *res_y, DATA_TYPE##VEC_WIDTH *res_z) \
{ \
	*res_x = (p0_y * p1_z) - ( p0_z * p1_y); \
	*res_y = (p0_z * p1_x) - ( p0_x * p1_z); \
	*res_z = (p0_x * p1_y) - ( p0_y * p1_x); \
}			

#define DEF_GEOM_SOA_CROSS4_PROTO( DATA_TYPE, VEC_WIDTH ) \
void __attribute__((overloadable)) \
soa_cross4( DATA_TYPE##VEC_WIDTH p0_x, DATA_TYPE##VEC_WIDTH p0_y, DATA_TYPE##VEC_WIDTH p0_z, DATA_TYPE##VEC_WIDTH p0_w, \
            DATA_TYPE##VEC_WIDTH p1_x, DATA_TYPE##VEC_WIDTH p1_y, DATA_TYPE##VEC_WIDTH p1_z, DATA_TYPE##VEC_WIDTH p1_w, \
			DATA_TYPE##VEC_WIDTH *res_x, DATA_TYPE##VEC_WIDTH *res_y, DATA_TYPE##VEC_WIDTH *res_z, DATA_TYPE##VEC_WIDTH *res_w) \
{ \
	*res_x = (p0_y * p1_z) - ( p0_z * p1_y); \
	*res_y = (p0_z * p1_x) - ( p0_x * p1_z); \
	*res_z = (p0_x * p1_y) - ( p0_y * p1_x); \
	*res_w = ( DATA_TYPE )0.0f; \
}

/*
Fast & Regular Length generation macros
*/

// length 1					
#define DEF_GEOM_SOA_LENGTH1_PROTO( FAST_PREFIX, HALF_PREFIX, DATA_TYPE, VEC_WIDTH ) \
DATA_TYPE##VEC_WIDTH __attribute__((overloadable)) \
soa_##FAST_PREFIX##length1( DATA_TYPE##VEC_WIDTH p0) \
{ \
	return fabs(p0); \
}

// length 2 for floats
#define DEF_GEOM_SOA_LENGTH2_FLOAT_PROTO( VEC_WIDTH ) \
float##VEC_WIDTH __attribute__((overloadable)) \
soa_length2( float##VEC_WIDTH p0_x, float##VEC_WIDTH p0_y ) \
{ \
	float##VEC_WIDTH sum = p0_x * p0_x + p0_y * p0_y; \
	int##VEC_WIDTH overflow_mask = sum == INFINITY; \
	int##VEC_WIDTH underflow_mask = sum < (2 * FLT_MIN / FLT_EPSILON); \
	if( movemask(overflow_mask) || movemask(underflow_mask) ) \
	{ \
		double##VEC_WIDTH dp_p0_x = convert_double##VEC_WIDTH(p0_x); \
		double##VEC_WIDTH dp_p0_y = convert_double##VEC_WIDTH(p0_y); \
		double##VEC_WIDTH dp_sum = (dp_p0_x * dp_p0_x) + (dp_p0_y * dp_p0_y); \
		dp_sum = sqrt( dp_sum ); \
		return convert_float##VEC_WIDTH( dp_sum ); \
	} \
	return sqrt( sum ); \
}	

// length 2 for double
#define DEF_GEOM_SOA_LENGTH2_DOUBLE_PROTO( VEC_WIDTH ) \
double##VEC_WIDTH __attribute__((overloadable)) \
soa_length2( double##VEC_WIDTH p0_x, double##VEC_WIDTH p0_y ) \
{ \
	double##VEC_WIDTH sum = p0_x * p0_x + p0_y * p0_y; \
	double##VEC_WIDTH sum_multiplier = 1.0; \
	long##VEC_WIDTH overflow_mask = (sum == INFINITY); \
	long##VEC_WIDTH underflow_mask = (sum  < (2 * DBL_MIN / DBL_EPSILON)); \
	if ( movemask(overflow_mask)  ) \
	{	\
		double##VEC_WIDTH overflow_p0_x = p0_x * 0x1.0p-600; \
		double##VEC_WIDTH overflow_p0_y = p0_y * 0x1.0p-600; \
		double##VEC_WIDTH overflow_sum = overflow_p0_x * overflow_p0_x + overflow_p0_y * overflow_p0_y; \
		sum_multiplier = select( sum_multiplier, 0x1.0p600, overflow_mask ); \
		sum = select( sum, overflow_sum, overflow_mask); \
	} \
	if ( movemask(underflow_mask) ) \
	{ \
		double##VEC_WIDTH underflow_p0_x = p0_x * 0x1.0p700; \
		double##VEC_WIDTH underflow_p0_y = p0_y * 0x1.0p700; \
		double##VEC_WIDTH underflow_sum = underflow_p0_x * underflow_p0_x + underflow_p0_y * underflow_p0_y; \
		sum_multiplier = select( sum_multiplier, 0x1.0p-700, underflow_mask ); \
		sum = select( sum, underflow_sum, underflow_mask); \
	} \
	return sqrt( sum ) * sum_multiplier; \
}	

// fast_length 2 for floats and doubles
#define DEF_GEOM_SOA_FAST_LENGTH2_PROTO( DATA_TYPE, VEC_WIDTH ) \
DATA_TYPE##VEC_WIDTH __attribute__((overloadable)) \
soa_fast_length2( DATA_TYPE##VEC_WIDTH p0_x, DATA_TYPE##VEC_WIDTH p0_y ) \
{ \
	DATA_TYPE##VEC_WIDTH sum = p0_x * p0_x + p0_y * p0_y; \
	return sqrt( sum ); \
}	

// length 3 for floats
#define DEF_GEOM_SOA_LENGTH3_FLOAT_PROTO( VEC_WIDTH ) \
float##VEC_WIDTH __attribute__((overloadable)) \
soa_length3( float##VEC_WIDTH p0_x, float##VEC_WIDTH p0_y, float##VEC_WIDTH p0_z ) \
{ \
	float##VEC_WIDTH sum = p0_x * p0_x + p0_y * p0_y + p0_z * p0_z; \
	int##VEC_WIDTH overflow_mask = sum == INFINITY; \
	int##VEC_WIDTH underflow_mask = sum < (2 * FLT_MIN / FLT_EPSILON); \
	if( movemask(overflow_mask) || movemask(underflow_mask) ) \
	{ \
		double##VEC_WIDTH dp_p0_x = convert_double##VEC_WIDTH(p0_x); \
		double##VEC_WIDTH dp_p0_y = convert_double##VEC_WIDTH(p0_y); \
		double##VEC_WIDTH dp_p0_z = convert_double##VEC_WIDTH(p0_z); \
		double##VEC_WIDTH dp_sum = (dp_p0_x * dp_p0_x) + (dp_p0_y * dp_p0_y) + (dp_p0_z * dp_p0_z); \
		dp_sum = sqrt( dp_sum ); \
		return convert_float##VEC_WIDTH( dp_sum ); \
	} \
	return sqrt( sum ); \
}	

// length 3 for double
#define DEF_GEOM_SOA_LENGTH3_DOUBLE_PROTO( VEC_WIDTH ) \
double##VEC_WIDTH __attribute__((overloadable)) \
soa_length3( double##VEC_WIDTH p0_x, double##VEC_WIDTH p0_y, double##VEC_WIDTH p0_z ) \
{ \
	double##VEC_WIDTH sum = p0_x * p0_x + p0_y * p0_y + p0_z * p0_z; \
	double##VEC_WIDTH sum_multiplier = 1.0; \
	long##VEC_WIDTH overflow_mask = (sum == INFINITY); \
	long##VEC_WIDTH underflow_mask = (sum  < (2 * DBL_MIN / DBL_EPSILON)); \
	if ( movemask(overflow_mask)  ) \
	{	\
		double##VEC_WIDTH overflow_p0_x = p0_x * 0x1.0p-600; \
		double##VEC_WIDTH overflow_p0_y = p0_y * 0x1.0p-600; \
		double##VEC_WIDTH overflow_p0_z = p0_z * 0x1.0p-600; \
		double##VEC_WIDTH overflow_sum = overflow_p0_x * overflow_p0_x + overflow_p0_y * overflow_p0_y + overflow_p0_z * overflow_p0_z; \
		sum_multiplier = select( sum_multiplier, 0x1.0p600, overflow_mask ); \
		sum = select( sum, overflow_sum, overflow_mask); \
	} \
	if ( movemask(underflow_mask) ) \
	{ \
		double##VEC_WIDTH underflow_p0_x = p0_x * 0x1.0p700; \
		double##VEC_WIDTH underflow_p0_y = p0_y * 0x1.0p700; \
		double##VEC_WIDTH underflow_p0_z = p0_z * 0x1.0p700; \
		double##VEC_WIDTH underflow_sum = underflow_p0_x * underflow_p0_x + underflow_p0_y * underflow_p0_y + underflow_p0_z * underflow_p0_z; \
		sum_multiplier = select( sum_multiplier, 0x1.0p-700, underflow_mask ); \
		sum = select( sum, underflow_sum, underflow_mask); \
	} \
	return sqrt( sum ) * sum_multiplier; \
}	

// fast_length 3 for floats and doubles
#define DEF_GEOM_SOA_FAST_LENGTH3_PROTO( DATA_TYPE, VEC_WIDTH ) \
DATA_TYPE##VEC_WIDTH __attribute__((overloadable)) \
soa_fast_length3( DATA_TYPE##VEC_WIDTH p0_x, DATA_TYPE##VEC_WIDTH p0_y, DATA_TYPE##VEC_WIDTH p0_z ) \
{ \
	DATA_TYPE##VEC_WIDTH sum = p0_x * p0_x + p0_y * p0_y + p0_z * p0_z; \
	return sqrt( sum ); \
}	

// length 4 for floats
#define DEF_GEOM_SOA_LENGTH4_FLOAT_PROTO( VEC_WIDTH ) \
float##VEC_WIDTH __attribute__((overloadable)) \
soa_length4( float##VEC_WIDTH p0_x, float##VEC_WIDTH p0_y, float##VEC_WIDTH p0_z, float##VEC_WIDTH p0_w ) \
{ \
	float##VEC_WIDTH sum = p0_x * p0_x + p0_y * p0_y + p0_z * p0_z + p0_w * p0_w; \
	int##VEC_WIDTH overflow_mask = sum == INFINITY; \
	int##VEC_WIDTH underflow_mask = sum < (2 * FLT_MIN / FLT_EPSILON); \
	if( movemask(overflow_mask) || movemask(underflow_mask) ) \
	{ \
		double##VEC_WIDTH dp_p0_x = convert_double##VEC_WIDTH(p0_x); \
		double##VEC_WIDTH dp_p0_y = convert_double##VEC_WIDTH(p0_y); \
		double##VEC_WIDTH dp_p0_z = convert_double##VEC_WIDTH(p0_z); \
		double##VEC_WIDTH dp_p0_w = convert_double##VEC_WIDTH(p0_w); \
		double##VEC_WIDTH dp_sum = (dp_p0_x * dp_p0_x) + (dp_p0_y * dp_p0_y) + (dp_p0_z * dp_p0_z) + (dp_p0_w * dp_p0_w); \
		dp_sum = sqrt( dp_sum ); \
		return convert_float##VEC_WIDTH( dp_sum ); \
	} \
	return sqrt( sum ); \
}	

// length 4 for double
#define DEF_GEOM_SOA_LENGTH4_DOUBLE_PROTO( VEC_WIDTH ) \
double##VEC_WIDTH __attribute__((overloadable)) \
soa_length4( double##VEC_WIDTH p0_x, double##VEC_WIDTH p0_y, double##VEC_WIDTH p0_z, double##VEC_WIDTH p0_w ) \
{ \
	double##VEC_WIDTH sum = p0_x * p0_x + p0_y * p0_y + p0_z * p0_z + p0_w * p0_w; \
	double##VEC_WIDTH sum_multiplier = 1.0; \
	long##VEC_WIDTH overflow_mask = (sum == INFINITY); \
	long##VEC_WIDTH underflow_mask = (sum  < (2 * DBL_MIN / DBL_EPSILON)); \
	if ( movemask(overflow_mask)  ) \
	{	\
		double##VEC_WIDTH overflow_p0_x = p0_x * 0x1.0p-600; \
		double##VEC_WIDTH overflow_p0_y = p0_y * 0x1.0p-600; \
		double##VEC_WIDTH overflow_p0_z = p0_z * 0x1.0p-600; \
		double##VEC_WIDTH overflow_p0_w = p0_w * 0x1.0p-600; \
		double##VEC_WIDTH overflow_sum = overflow_p0_x * overflow_p0_x + overflow_p0_y * overflow_p0_y + overflow_p0_z * overflow_p0_z + overflow_p0_w * overflow_p0_w; \
		sum_multiplier = select( sum_multiplier, 0x1.0p600, overflow_mask ); \
		sum = select( sum, overflow_sum, overflow_mask); \
	} \
	if ( movemask(underflow_mask) ) \
	{ \
		double##VEC_WIDTH underflow_p0_x = p0_x * 0x1.0p700; \
		double##VEC_WIDTH underflow_p0_y = p0_y * 0x1.0p700; \
		double##VEC_WIDTH underflow_p0_z = p0_z * 0x1.0p700; \
		double##VEC_WIDTH underflow_p0_w = p0_w * 0x1.0p700; \
		double##VEC_WIDTH underflow_sum = underflow_p0_x * underflow_p0_x + underflow_p0_y * underflow_p0_y + underflow_p0_z * underflow_p0_z + underflow_p0_w * underflow_p0_w; \
		sum_multiplier = select( sum_multiplier, 0x1.0p-700, underflow_mask ); \
		sum = select( sum, underflow_sum, underflow_mask); \
	} \
	return sqrt( sum ) * sum_multiplier; \
}	

// fast_length 4 for floats and doubles
#define DEF_GEOM_SOA_FAST_LENGTH4_PROTO( DATA_TYPE, VEC_WIDTH ) \
DATA_TYPE##VEC_WIDTH __attribute__((overloadable)) \
soa_fast_length4( DATA_TYPE##VEC_WIDTH p0_x, DATA_TYPE##VEC_WIDTH p0_y, DATA_TYPE##VEC_WIDTH p0_z, DATA_TYPE##VEC_WIDTH p0_w ) \
{ \
	DATA_TYPE##VEC_WIDTH sum = p0_x * p0_x + p0_y * p0_y + p0_z * p0_z + p0_w * p0_w; \
	return sqrt( sum ); \
}	
/*
Fast & Regular Distance generation macros
*/							
		 
#define DEF_GEOM_SOA_DISTANCE1_PROTO( FAST_PREFIX, DATA_TYPE, VEC_WIDTH ) \
DATA_TYPE##VEC_WIDTH __attribute__((overloadable)) \
soa_##FAST_PREFIX##distance1( DATA_TYPE##VEC_WIDTH p0, DATA_TYPE##VEC_WIDTH p1) \
{ \
	return soa_##FAST_PREFIX##length1(p0 - p1); \
}

#define DEF_GEOM_SOA_DISTANCE2_PROTO( FAST_PREFIX, DATA_TYPE, VEC_WIDTH ) \
DATA_TYPE##VEC_WIDTH __attribute__((overloadable)) \
soa_##FAST_PREFIX##distance2( DATA_TYPE##VEC_WIDTH p0_x, DATA_TYPE##VEC_WIDTH p0_y, \
								DATA_TYPE##VEC_WIDTH p1_x, DATA_TYPE##VEC_WIDTH p1_y ) \
{ \
	return soa_##FAST_PREFIX##length2(p0_x - p1_x, p0_y - p1_y); \
}

#define DEF_GEOM_SOA_DISTANCE3_PROTO( FAST_PREFIX, DATA_TYPE, VEC_WIDTH ) \
DATA_TYPE##VEC_WIDTH __attribute__((overloadable)) \
soa_##FAST_PREFIX##distance3( DATA_TYPE##VEC_WIDTH p0_x, DATA_TYPE##VEC_WIDTH p0_y, DATA_TYPE##VEC_WIDTH p0_z, \
							DATA_TYPE##VEC_WIDTH p1_x, DATA_TYPE##VEC_WIDTH p1_y, DATA_TYPE##VEC_WIDTH p1_z ) \
{ \
	return soa_##FAST_PREFIX##length3(p0_x - p1_x, p0_y - p1_y, p0_z - p1_z); \
}

#define DEF_GEOM_SOA_DISTANCE4_PROTO( FAST_PREFIX, DATA_TYPE, VEC_WIDTH ) \
DATA_TYPE##VEC_WIDTH __attribute__((overloadable)) \
soa_##FAST_PREFIX##distance4( DATA_TYPE##VEC_WIDTH p0_x, DATA_TYPE##VEC_WIDTH p0_y, DATA_TYPE##VEC_WIDTH p0_z, DATA_TYPE##VEC_WIDTH p0_w, \
      						  DATA_TYPE##VEC_WIDTH p1_x, DATA_TYPE##VEC_WIDTH p1_y, DATA_TYPE##VEC_WIDTH p1_z, DATA_TYPE##VEC_WIDTH p1_w ) \
{ \
	return soa_##FAST_PREFIX##length4(p0_x - p1_x, p0_y - p1_y, p0_z - p1_z, p0_w - p1_w); \
}
									
										  
/*
Fast & Regular Normalize generation macros
*/					
#define DEF_GEOM_SOA_NORMALIZE1_PROTO( DATA_TYPE, MASK_DATA_TYPE, VEC_WIDTH ) \
DATA_TYPE##VEC_WIDTH __attribute__((overloadable)) \
soa_normalize1( DATA_TYPE##VEC_WIDTH p0 ) \
{ \
	MASK_DATA_TYPE##VEC_WIDTH zero_mask = (p0 == 0.f);\
	MASK_DATA_TYPE##VEC_WIDTH NAN_mask = (p0 != p0);\
	MASK_DATA_TYPE##VEC_WIDTH gtz_mask = (p0 > 0.f);\
	DATA_TYPE##VEC_WIDTH res = select((DATA_TYPE##VEC_WIDTH)-1.f, (DATA_TYPE##VEC_WIDTH)1.f, gtz_mask);\
	res = select(res, p0, NAN_mask);\
	res = select(res, p0, zero_mask);\
	return res;\
}

#define DEF_GEOM_SOA_NORMALIZE1_FAST_PROTO( DATA_TYPE, MASK_DATA_TYPE, VEC_WIDTH ) \
DATA_TYPE##VEC_WIDTH __attribute__((overloadable)) \
soa_fast_normalize1( DATA_TYPE##VEC_WIDTH p0 ) \
{ \
	MASK_DATA_TYPE##VEC_WIDTH gtz_mask = (p0 > 0.f);\
	MASK_DATA_TYPE##VEC_WIDTH zero_mask = (p0 == 0.f);\
	DATA_TYPE##VEC_WIDTH res = select((DATA_TYPE##VEC_WIDTH)-1.f, (DATA_TYPE##VEC_WIDTH)1.f, gtz_mask);\
	return select(res, p0, zero_mask);\
}

#define DEF_GEOM_SOA_NORMALIZE2_DOUBLE_PROTO( VEC_WIDTH ) \
void __attribute__((overloadable)) \
soa_normalize2( double##VEC_WIDTH p0_x, double##VEC_WIDTH p0_y, \
				double##VEC_WIDTH *res_x, double##VEC_WIDTH *res_y) \
{ \
    double##VEC_WIDTH total = 0.0, value; \
    total = p0_x * p0_x + p0_y * p0_y; \
    long##VEC_WIDTH underflow_mask = (total < (2 * DBL_MIN / DBL_EPSILON)); \
	long##VEC_WIDTH underflow_zero_mask = 0; \
	long##VEC_WIDTH overflow_mask = (total == INFINITY); \
	\
    if( movemask(underflow_mask) ) \
    {  \
        double##VEC_WIDTH underflow_total = 0.0; \
		double##VEC_WIDTH t_x, t_y; \
		t_x = p0_x * 0x1.0p700; \
		t_y = p0_y * 0x1.0p700; \
        underflow_total = t_x * t_x + t_y * t_y; \
		underflow_zero_mask = (underflow_total == 0.0); \
		underflow_zero_mask = underflow_zero_mask; \
		p0_x = select(p0_x, t_x, underflow_mask); \
		p0_y = select(p0_y, t_y, underflow_mask); \
		total = select( total, underflow_total, underflow_mask); \
    } \
    \
	if( movemask(overflow_mask) ) \
    { \
        double##VEC_WIDTH scale = 0x1.0p-512 / 2; \
        double##VEC_WIDTH overflow_total = 0; \
		double##VEC_WIDTH t_x, t_y; \
        t_x = p0_x * scale; \
        t_y = p0_y * scale; \
		overflow_total = t_x * t_x + t_y * t_y; \
		long##VEC_WIDTH still_overflow_mask = (overflow_total == INFINITY); \
		\
        if( movemask(still_overflow_mask) ) \
        { \
            double##VEC_WIDTH stil_overflow_total = 0.0; \
			double##VEC_WIDTH t_of_x, t_of_y; \
			\
			long##VEC_WIDTH is_inf_mask = isinf( t_x ); \
			t_of_x = select( copysign(0.0, p0_x), copysign(1.0, p0_x), is_inf_mask); \
			stil_overflow_total = select( stil_overflow_total, stil_overflow_total + 1.0, is_inf_mask); \
\
			is_inf_mask = isinf( t_y ); \
			t_of_y = select( copysign(0.0, p0_y), copysign(1.0, p0_y), is_inf_mask); \
			stil_overflow_total = select( stil_overflow_total, stil_overflow_total + 1.0, is_inf_mask); \
			\
			t_x = select(t_x, t_of_x, still_overflow_mask); \
			t_y = select(t_y, t_of_y, still_overflow_mask); \
			overflow_total = select(overflow_total, stil_overflow_total, still_overflow_mask); \
        } \
\
		p0_x = select(p0_x, t_x, overflow_mask); \
		p0_y = select(p0_y, t_y, overflow_mask); \
		total = select(total, overflow_total, overflow_mask); \
    } \
    \
    value = sqrt( total ); \
	*res_x = select(p0_x / value, p0_x, underflow_zero_mask); \
	*res_y = select(p0_y / value, p0_y, underflow_zero_mask); \
}

#define DEF_GEOM_SOA_NORMALIZE2_FLOAT_PROTO( VEC_WIDTH ) \
void __attribute__((overloadable)) \
soa_normalize2( float##VEC_WIDTH p0_x, float##VEC_WIDTH p0_y, float##VEC_WIDTH *res_x, float##VEC_WIDTH *res_y) \
{\
	double##VEC_WIDTH p0_x_d = convert_double##VEC_WIDTH(p0_x);\
	double##VEC_WIDTH p0_y_d = convert_double##VEC_WIDTH(p0_y);\
	double##VEC_WIDTH length = p0_x_d * p0_x_d + p0_y_d * p0_y_d;\
	double##VEC_WIDTH length_inf;\
	long##VEC_WIDTH zero_mask = (length == 0.f);\
	long##VEC_WIDTH inf_mask = (length == INFINITY);\
	\
	if( movemask( inf_mask ) )\
	{\
		float##VEC_WIDTH temp_x = (fabs(p0_x) == INFINITY) ? copysign( (float##VEC_WIDTH)1.0f, p0_x ) : copysign( (float##VEC_WIDTH)0.0f, p0_x );\
		float##VEC_WIDTH temp_y = (fabs(p0_y) == INFINITY) ? copysign( (float##VEC_WIDTH)1.0f, p0_y ) : copysign( (float##VEC_WIDTH)0.0f, p0_y );\
		double##VEC_WIDTH temp_x_d = convert_double##VEC_WIDTH(temp_x);\
		double##VEC_WIDTH temp_y_d = convert_double##VEC_WIDTH(temp_y);\
		length_inf = temp_x_d * temp_x_d + temp_y_d * temp_y_d;\
		length_inf = sqrt(length_inf);\
	}\
\
	length = sqrt(length);\
	length = select( length, length_inf, inf_mask); \
	double##VEC_WIDTH norm_x = p0_x_d / length; \
	double##VEC_WIDTH norm_y = p0_y_d / length; \
	*res_x = convert_float##VEC_WIDTH( norm_x ); \
	*res_y = convert_float##VEC_WIDTH( norm_y ); \
	*res_x = convert_float##VEC_WIDTH( select( norm_x, p0_x_d, zero_mask) );\
	*res_y = convert_float##VEC_WIDTH( select( norm_y, p0_y_d, zero_mask) );\
}

#define DEF_GEOM_SOA_NORMALIZE2_FAST_PROTO( DATA_TYPE, VEC_WIDTH ) \
void __attribute__((overloadable)) \
soa_fast_normalize2( DATA_TYPE##VEC_WIDTH p0_x, DATA_TYPE##VEC_WIDTH p0_y, \
		             DATA_TYPE##VEC_WIDTH *res_x, DATA_TYPE##VEC_WIDTH *res_y) \
{ \
	DATA_TYPE##VEC_WIDTH length = p0_x * p0_x + p0_y * p0_y; \
	DATA_TYPE##VEC_WIDTH rnorm = half_rsqrt( length ); \
	rnorm = (length == (DATA_TYPE##VEC_WIDTH)0.0f) ? (DATA_TYPE##VEC_WIDTH)1.0f : rnorm; \
	*res_x = p0_x * rnorm; \
	*res_y = p0_y * rnorm; \
}
				 
#define DEF_GEOM_SOA_NORMALIZE3_DOUBLE_PROTO( VEC_WIDTH ) \
void __attribute__((overloadable)) \
soa_normalize3( double##VEC_WIDTH p0_x, double##VEC_WIDTH p0_y, double##VEC_WIDTH p0_z, \
		        double##VEC_WIDTH *res_x, double##VEC_WIDTH *res_y, double##VEC_WIDTH *res_z) \
{ \
    double##VEC_WIDTH total = 0.0, value; \
    total = p0_x * p0_x + p0_y * p0_y + p0_z * p0_z; \
    long##VEC_WIDTH underflow_mask = (total < (3 * DBL_MIN / DBL_EPSILON)); \
	long##VEC_WIDTH underflow_zero_mask = 0; \
	long##VEC_WIDTH overflow_mask = (total == INFINITY); \
	\
    if( movemask(underflow_mask) ) \
    {  \
        double##VEC_WIDTH underflow_total = 0.0; \
		double##VEC_WIDTH t_x, t_y, t_z; \
		t_x = p0_x * 0x1.0p700; \
		t_y = p0_y * 0x1.0p700; \
		t_z = p0_z * 0x1.0p700; \
        underflow_total = t_x * t_x + t_y * t_y + t_z * t_z; \
		underflow_zero_mask = (underflow_total == 0.0); \
		underflow_zero_mask = underflow_zero_mask; \
		p0_x = select(p0_x, t_x, underflow_mask); \
		p0_y = select(p0_y, t_y, underflow_mask); \
		p0_z = select(p0_z, t_z, underflow_mask); \
		total = select( total, underflow_total, underflow_mask); \
    } \
    \
	if( movemask(overflow_mask) ) \
    { \
        double##VEC_WIDTH scale = 0x1.0p-512 / 2; \
        double##VEC_WIDTH overflow_total = 0; \
		double##VEC_WIDTH t_x, t_y, t_z; \
        t_x = p0_x * scale; \
        t_y = p0_y * scale; \
        t_z = p0_z * scale; \
		overflow_total = t_x * t_x + t_y * t_y + t_z * t_z; \
		long##VEC_WIDTH still_overflow_mask = (overflow_total == INFINITY); \
		\
        if( movemask(still_overflow_mask) ) \
        { \
            double##VEC_WIDTH stil_overflow_total = 0.0; \
			double##VEC_WIDTH t_of_x, t_of_y, t_of_z; \
			\
			long##VEC_WIDTH is_inf_mask = isinf( t_x ); \
			t_of_x = select( copysign(0.0, p0_x), copysign(1.0, p0_x), is_inf_mask); \
			stil_overflow_total = select( stil_overflow_total, stil_overflow_total + 1.0, is_inf_mask); \
			\
			is_inf_mask = isinf( t_y ); \
			t_of_y = select( copysign(0.0, p0_y), copysign(1.0, p0_y), is_inf_mask); \
			stil_overflow_total = select( stil_overflow_total, stil_overflow_total + 1.0, is_inf_mask); \
			\
			is_inf_mask = isinf( t_z ); \
			t_of_z = select( copysign(0.0, p0_z), copysign(1.0, p0_z), is_inf_mask); \
			stil_overflow_total = select( stil_overflow_total, stil_overflow_total + 1.0, is_inf_mask); \
			\
			t_x = select(t_x, t_of_x, still_overflow_mask); \
			t_y = select(t_y, t_of_y, still_overflow_mask); \
			t_z = select(t_z, t_of_z, still_overflow_mask); \
			overflow_total = select(overflow_total, stil_overflow_total, still_overflow_mask); \
        } \
\
		p0_x = select(p0_x, t_x, overflow_mask); \
		p0_y = select(p0_y, t_y, overflow_mask); \
		p0_z = select(p0_z, t_z, overflow_mask); \
		total = select(total, overflow_total, overflow_mask); \
    } \
    \
    value = sqrt( total ); \
	*res_x = select(p0_x / value, p0_x, underflow_zero_mask); \
	*res_y = select(p0_y / value, p0_y, underflow_zero_mask); \
	*res_z = select(p0_z / value, p0_z, underflow_zero_mask); \
}

#define DEF_GEOM_SOA_NORMALIZE3_FLOAT_PROTO( VEC_WIDTH ) \
void __attribute__((overloadable)) \
soa_normalize3( float##VEC_WIDTH p0_x, float##VEC_WIDTH p0_y, float##VEC_WIDTH p0_z, \
                float##VEC_WIDTH *res_x, float##VEC_WIDTH *res_y, float##VEC_WIDTH *res_z ) \
{\
	double##VEC_WIDTH p0_x_d = convert_double##VEC_WIDTH(p0_x);\
	double##VEC_WIDTH p0_y_d = convert_double##VEC_WIDTH(p0_y);\
	double##VEC_WIDTH p0_z_d = convert_double##VEC_WIDTH(p0_z);\
	double##VEC_WIDTH length = p0_x_d * p0_x_d + p0_y_d * p0_y_d + p0_z_d * p0_z_d;\
	double##VEC_WIDTH length_inf;\
	long##VEC_WIDTH zero_mask = (length == 0.f);\
	long##VEC_WIDTH inf_mask = (length == INFINITY);\
	\
	if( movemask( inf_mask ) )\
	{\
		float##VEC_WIDTH temp_x = (fabs(p0_x) == INFINITY) ? copysign( (float##VEC_WIDTH)1.0f, p0_x ) : copysign( (float##VEC_WIDTH)0.0f, p0_x );\
		float##VEC_WIDTH temp_y = (fabs(p0_y) == INFINITY) ? copysign( (float##VEC_WIDTH)1.0f, p0_y ) : copysign( (float##VEC_WIDTH)0.0f, p0_y );\
		float##VEC_WIDTH temp_z = (fabs(p0_z) == INFINITY) ? copysign( (float##VEC_WIDTH)1.0f, p0_z ) : copysign( (float##VEC_WIDTH)0.0f, p0_z );\
		double##VEC_WIDTH temp_x_d = convert_double##VEC_WIDTH(temp_x);\
		double##VEC_WIDTH temp_y_d = convert_double##VEC_WIDTH(temp_y);\
		double##VEC_WIDTH temp_z_d = convert_double##VEC_WIDTH(temp_z);\
		length_inf = temp_x_d * temp_x_d + temp_y_d * temp_y_d + temp_z_d * temp_z_d;\
		length_inf = sqrt(length_inf);\
	}\
\
	length = sqrt(length);\
	length = select( length, length_inf, inf_mask); \
	double##VEC_WIDTH norm_x = p0_x_d / length; \
	double##VEC_WIDTH norm_y = p0_y_d / length; \
	double##VEC_WIDTH norm_z = p0_z_d / length; \
	*res_x = convert_float##VEC_WIDTH( norm_x ); \
	*res_y = convert_float##VEC_WIDTH( norm_y ); \
	*res_z = convert_float##VEC_WIDTH( norm_z ); \
	*res_x = convert_float##VEC_WIDTH( select( norm_x, p0_x_d, zero_mask) );\
	*res_y = convert_float##VEC_WIDTH( select( norm_y, p0_y_d, zero_mask) );\
	*res_z = convert_float##VEC_WIDTH( select( norm_z, p0_z_d, zero_mask) );\
}

#define DEF_GEOM_SOA_NORMALIZE3_FAST_PROTO( DATA_TYPE, VEC_WIDTH ) \
void __attribute__((overloadable)) \
soa_fast_normalize3( DATA_TYPE##VEC_WIDTH p0_x, DATA_TYPE##VEC_WIDTH p0_y, DATA_TYPE##VEC_WIDTH p0_z,\
		             DATA_TYPE##VEC_WIDTH *res_x, DATA_TYPE##VEC_WIDTH *res_y, DATA_TYPE##VEC_WIDTH *res_z) \
{ \
	DATA_TYPE##VEC_WIDTH length = p0_x * p0_x + p0_y * p0_y + p0_z * p0_z; \
	DATA_TYPE##VEC_WIDTH rnorm = half_rsqrt( length ); \
	rnorm = (length == (DATA_TYPE##VEC_WIDTH)0.0f) ? (DATA_TYPE##VEC_WIDTH)1.0f : rnorm; \
	*res_x = p0_x * rnorm; \
	*res_y = p0_y * rnorm; \
	*res_z = p0_z * rnorm; \
}

#define DEF_GEOM_SOA_NORMALIZE4_DOUBLE_PROTO( VEC_WIDTH ) \
void __attribute__((overloadable)) \
soa_normalize4( double##VEC_WIDTH p0_x, double##VEC_WIDTH p0_y, double##VEC_WIDTH p0_z, double##VEC_WIDTH p0_w, \
				double##VEC_WIDTH *res_x, double##VEC_WIDTH *res_y, double##VEC_WIDTH *res_z, double##VEC_WIDTH *res_w) \
{ \
    double##VEC_WIDTH total = 0.0, value; \
    total = p0_x * p0_x + p0_y * p0_y + p0_z * p0_z + p0_w * p0_w; \
    long##VEC_WIDTH underflow_mask = (total < (4 * DBL_MIN / DBL_EPSILON)); \
	long##VEC_WIDTH underflow_zero_mask = 0; \
	long##VEC_WIDTH overflow_mask = (total == INFINITY); \
	\
    if( movemask(underflow_mask) ) \
    {  \
        double##VEC_WIDTH underflow_total = 0.0; \
		double##VEC_WIDTH t_x, t_y, t_z, t_w; \
		t_x = p0_x * 0x1.0p700; \
		t_y = p0_y * 0x1.0p700; \
		t_z = p0_z * 0x1.0p700; \
		t_w = p0_w * 0x1.0p700; \
        underflow_total = t_x * t_x + t_y * t_y + t_z * t_z + t_w * t_w; \
		underflow_zero_mask = (underflow_total == 0.0); \
		underflow_zero_mask = underflow_zero_mask; \
		p0_x = select(p0_x, t_x, underflow_mask); \
		p0_y = select(p0_y, t_y, underflow_mask); \
		p0_z = select(p0_z, t_z, underflow_mask); \
		p0_w = select(p0_w, t_w, underflow_mask); \
		total = select( total, underflow_total, underflow_mask); \
    } \
    \
	if( movemask(overflow_mask) ) \
    { \
        double##VEC_WIDTH scale = 0x1.0p-512 / 2; \
        double##VEC_WIDTH overflow_total = 0; \
		double##VEC_WIDTH t_x, t_y, t_z, t_w; \
        t_x = p0_x * scale; \
        t_y = p0_y * scale; \
        t_z = p0_z * scale; \
        t_w = p0_w * scale; \
		overflow_total = t_x * t_x + t_y * t_y + t_z * t_z + t_w * t_w; \
		long##VEC_WIDTH still_overflow_mask = (overflow_total == INFINITY); \
		\
        if( movemask(still_overflow_mask) ) \
        { \
            double##VEC_WIDTH stil_overflow_total = 0.0; \
			double##VEC_WIDTH t_of_x, t_of_y, t_of_z, t_of_w; \
			\
			long##VEC_WIDTH is_inf_mask = isinf( t_x ); \
			t_of_x = select( copysign(0.0, p0_x), copysign(1.0, p0_x), is_inf_mask); \
			stil_overflow_total = select( stil_overflow_total, stil_overflow_total + 1.0, is_inf_mask); \
			\
			is_inf_mask = isinf( t_y ); \
			t_of_y = select( copysign(0.0, p0_y), copysign(1.0, p0_y), is_inf_mask); \
			stil_overflow_total = select( stil_overflow_total, stil_overflow_total + 1.0, is_inf_mask); \
			\
			is_inf_mask = isinf( t_z ); \
			t_of_z = select( copysign(0.0, p0_z), copysign(1.0, p0_z), is_inf_mask); \
			stil_overflow_total = select( stil_overflow_total, stil_overflow_total + 1.0, is_inf_mask); \
			\
			is_inf_mask = isinf( t_w ); \
			t_of_w = select( copysign(0.0, p0_w), copysign(1.0, p0_w), is_inf_mask); \
			stil_overflow_total = select( stil_overflow_total, stil_overflow_total + 1.0, is_inf_mask); \
			\
			t_x = select(t_x, t_of_x, still_overflow_mask); \
			t_y = select(t_y, t_of_y, still_overflow_mask); \
			t_z = select(t_z, t_of_z, still_overflow_mask); \
			t_w = select(t_w, t_of_w, still_overflow_mask); \
			overflow_total = select(overflow_total, stil_overflow_total, still_overflow_mask); \
        } \
\
		p0_x = select(p0_x, t_x, overflow_mask); \
		p0_y = select(p0_y, t_y, overflow_mask); \
		p0_z = select(p0_z, t_z, overflow_mask); \
		p0_w = select(p0_w, t_w, overflow_mask); \
		total = select(total, overflow_total, overflow_mask); \
    } \
    \
    value = sqrt( total ); \
	*res_x = select(p0_x / value, p0_x, underflow_zero_mask); \
	*res_y = select(p0_y / value, p0_y, underflow_zero_mask); \
	*res_z = select(p0_z / value, p0_z, underflow_zero_mask); \
	*res_w = select(p0_w / value, p0_w, underflow_zero_mask); \
}

#define DEF_GEOM_SOA_NORMALIZE4_FLOAT_PROTO( VEC_WIDTH ) \
void __attribute__((overloadable)) \
soa_normalize4( float##VEC_WIDTH p0_x, float##VEC_WIDTH p0_y, float##VEC_WIDTH p0_z, float##VEC_WIDTH p0_w, \
                float##VEC_WIDTH *res_x, float##VEC_WIDTH *res_y, float##VEC_WIDTH *res_z, float##VEC_WIDTH *res_w ) \
{\
	double##VEC_WIDTH p0_x_d = convert_double##VEC_WIDTH(p0_x);\
	double##VEC_WIDTH p0_y_d = convert_double##VEC_WIDTH(p0_y);\
	double##VEC_WIDTH p0_z_d = convert_double##VEC_WIDTH(p0_z);\
	double##VEC_WIDTH p0_w_d = convert_double##VEC_WIDTH(p0_w);\
	double##VEC_WIDTH length = p0_x_d * p0_x_d + p0_y_d * p0_y_d + p0_z_d * p0_z_d + p0_w_d * p0_w_d;\
	double##VEC_WIDTH length_inf;\
	long##VEC_WIDTH zero_mask = (length == 0.f);\
	long##VEC_WIDTH inf_mask = (length == INFINITY);\
	\
	if( movemask( inf_mask ) )\
	{\
		float##VEC_WIDTH temp_x = (fabs(p0_x) == INFINITY) ? copysign( (float##VEC_WIDTH)1.0f, p0_x ) : copysign( (float##VEC_WIDTH)0.0f, p0_x );\
		float##VEC_WIDTH temp_y = (fabs(p0_y) == INFINITY) ? copysign( (float##VEC_WIDTH)1.0f, p0_y ) : copysign( (float##VEC_WIDTH)0.0f, p0_y );\
		float##VEC_WIDTH temp_z = (fabs(p0_z) == INFINITY) ? copysign( (float##VEC_WIDTH)1.0f, p0_z ) : copysign( (float##VEC_WIDTH)0.0f, p0_z );\
		float##VEC_WIDTH temp_w = (fabs(p0_w) == INFINITY) ? copysign( (float##VEC_WIDTH)1.0f, p0_w ) : copysign( (float##VEC_WIDTH)0.0f, p0_w );\
		double##VEC_WIDTH temp_x_d = convert_double##VEC_WIDTH(temp_x);\
		double##VEC_WIDTH temp_y_d = convert_double##VEC_WIDTH(temp_y);\
		double##VEC_WIDTH temp_z_d = convert_double##VEC_WIDTH(temp_z);\
		double##VEC_WIDTH temp_w_d = convert_double##VEC_WIDTH(temp_w);\
		length_inf = temp_x_d * temp_x_d + temp_y_d * temp_y_d + temp_z_d * temp_z_d + temp_w_d * temp_w_d;\
		length_inf = sqrt(length_inf);\
	}\
\
	length = sqrt(length);\
	length = select( length, length_inf, inf_mask); \
	double##VEC_WIDTH norm_x = p0_x_d / length; \
	double##VEC_WIDTH norm_y = p0_y_d / length; \
	double##VEC_WIDTH norm_z = p0_z_d / length; \
	double##VEC_WIDTH norm_w = p0_w_d / length; \
	*res_x = convert_float##VEC_WIDTH( norm_x ); \
	*res_y = convert_float##VEC_WIDTH( norm_y ); \
	*res_z = convert_float##VEC_WIDTH( norm_z ); \
	*res_w = convert_float##VEC_WIDTH( norm_w ); \
	*res_x = convert_float##VEC_WIDTH( select( norm_x, p0_x_d, zero_mask) );\
	*res_y = convert_float##VEC_WIDTH( select( norm_y, p0_y_d, zero_mask) );\
	*res_z = convert_float##VEC_WIDTH( select( norm_z, p0_z_d, zero_mask) );\
	*res_w = convert_float##VEC_WIDTH( select( norm_w, p0_w_d, zero_mask) );\
}

#define DEF_GEOM_SOA_NORMALIZE4_FAST_PROTO( DATA_TYPE, VEC_WIDTH ) \
void __attribute__((overloadable)) \
soa_fast_normalize4( DATA_TYPE##VEC_WIDTH p0_x, DATA_TYPE##VEC_WIDTH p0_y, DATA_TYPE##VEC_WIDTH p0_z, DATA_TYPE##VEC_WIDTH p0_w, \
		             DATA_TYPE##VEC_WIDTH *res_x, DATA_TYPE##VEC_WIDTH *res_y, DATA_TYPE##VEC_WIDTH *res_z, DATA_TYPE##VEC_WIDTH *res_w) \
{ \
	DATA_TYPE##VEC_WIDTH length = p0_x * p0_x + p0_y * p0_y + p0_z * p0_z + p0_w * p0_w; \
	DATA_TYPE##VEC_WIDTH rnorm = half_rsqrt( length ); \
	rnorm = (length == (DATA_TYPE##VEC_WIDTH)0.0f) ? (DATA_TYPE##VEC_WIDTH)1.0f : rnorm; \
	*res_x = p0_x * rnorm; \
	*res_y = p0_y * rnorm; \
	*res_z = p0_z * rnorm; \
	*res_w = p0_w * rnorm; \
}

DEF_GEOM_SOA_DOT1_PROTO( float, 4 )
DEF_GEOM_SOA_DOT1_PROTO( float, 8 )
DEF_GEOM_SOA_DOT1_PROTO( float, 16 )
DEF_GEOM_SOA_DOT1_PROTO( double, 4 )
DEF_GEOM_SOA_DOT1_PROTO( double, 8 )
DEF_GEOM_SOA_DOT1_PROTO( double, 16 )

DEF_GEOM_SOA_DOT2_PROTO( float, 4 )
DEF_GEOM_SOA_DOT2_PROTO( float, 8 )
DEF_GEOM_SOA_DOT2_PROTO( float, 16 )
DEF_GEOM_SOA_DOT2_PROTO( double, 4 )
DEF_GEOM_SOA_DOT2_PROTO( double, 8 )
DEF_GEOM_SOA_DOT2_PROTO( double, 16 )

DEF_GEOM_SOA_DOT3_PROTO( float, 4 )
DEF_GEOM_SOA_DOT3_PROTO( float, 8 )
DEF_GEOM_SOA_DOT3_PROTO( float, 16 )
DEF_GEOM_SOA_DOT3_PROTO( double, 4 )
DEF_GEOM_SOA_DOT3_PROTO( double, 8 )
DEF_GEOM_SOA_DOT3_PROTO( double, 16 )

DEF_GEOM_SOA_DOT4_PROTO( float, 4 )
DEF_GEOM_SOA_DOT4_PROTO( float, 8 )
DEF_GEOM_SOA_DOT4_PROTO( float, 16 )
DEF_GEOM_SOA_DOT4_PROTO( double, 4 )
DEF_GEOM_SOA_DOT4_PROTO( double, 8 )
DEF_GEOM_SOA_DOT4_PROTO( double, 16 )

DEF_GEOM_SOA_CROSS3_PROTO( float, 4 )
DEF_GEOM_SOA_CROSS3_PROTO( float, 8 )
DEF_GEOM_SOA_CROSS3_PROTO( float, 16 )
DEF_GEOM_SOA_CROSS3_PROTO( double, 4 )
DEF_GEOM_SOA_CROSS3_PROTO( double, 8 )
DEF_GEOM_SOA_CROSS3_PROTO( double, 16 )

DEF_GEOM_SOA_CROSS4_PROTO( float, 4 )
DEF_GEOM_SOA_CROSS4_PROTO( float, 8 )
DEF_GEOM_SOA_CROSS4_PROTO( float, 16 )
DEF_GEOM_SOA_CROSS4_PROTO( double, 4 )
DEF_GEOM_SOA_CROSS4_PROTO( double, 8 )
DEF_GEOM_SOA_CROSS4_PROTO( double, 16 )

DEF_GEOM_SOA_LENGTH1_PROTO( fast_ , half_ , float, 4 )
DEF_GEOM_SOA_LENGTH1_PROTO( fast_ , half_ , float, 8 )
DEF_GEOM_SOA_LENGTH1_PROTO( fast_ , half_ , float, 16 )

DEF_GEOM_SOA_FAST_LENGTH2_PROTO( float, 4 )
DEF_GEOM_SOA_FAST_LENGTH2_PROTO( float, 8 )
DEF_GEOM_SOA_FAST_LENGTH2_PROTO( float, 16 )

DEF_GEOM_SOA_FAST_LENGTH3_PROTO( float, 4 )
DEF_GEOM_SOA_FAST_LENGTH3_PROTO( float, 8 )
DEF_GEOM_SOA_FAST_LENGTH3_PROTO( float, 16 )

DEF_GEOM_SOA_FAST_LENGTH4_PROTO( float, 4 )
DEF_GEOM_SOA_FAST_LENGTH4_PROTO( float, 8 )
DEF_GEOM_SOA_FAST_LENGTH4_PROTO( float, 16 )

DEF_GEOM_SOA_LENGTH1_PROTO( , , float, 4 )
DEF_GEOM_SOA_LENGTH1_PROTO( , , float, 8 )
DEF_GEOM_SOA_LENGTH1_PROTO( , , float, 16 )
DEF_GEOM_SOA_LENGTH1_PROTO( , , double, 4 )
DEF_GEOM_SOA_LENGTH1_PROTO( , , double, 8 )
DEF_GEOM_SOA_LENGTH1_PROTO( , , double, 16 )
                              
DEF_GEOM_SOA_LENGTH2_FLOAT_PROTO( 4 )
DEF_GEOM_SOA_LENGTH2_FLOAT_PROTO( 8 )
DEF_GEOM_SOA_LENGTH2_FLOAT_PROTO( 16 )
DEF_GEOM_SOA_LENGTH2_DOUBLE_PROTO( 4 )
DEF_GEOM_SOA_LENGTH2_DOUBLE_PROTO( 8 )
DEF_GEOM_SOA_LENGTH2_DOUBLE_PROTO( 16 )
                              
DEF_GEOM_SOA_LENGTH3_FLOAT_PROTO( 4 )
DEF_GEOM_SOA_LENGTH3_FLOAT_PROTO( 8 )
DEF_GEOM_SOA_LENGTH3_FLOAT_PROTO( 16 )
DEF_GEOM_SOA_LENGTH3_DOUBLE_PROTO( 4 )
DEF_GEOM_SOA_LENGTH3_DOUBLE_PROTO( 8 )
DEF_GEOM_SOA_LENGTH3_DOUBLE_PROTO( 16 )
                              
DEF_GEOM_SOA_LENGTH4_FLOAT_PROTO( 4 )
DEF_GEOM_SOA_LENGTH4_FLOAT_PROTO( 8 )
DEF_GEOM_SOA_LENGTH4_FLOAT_PROTO( 16 )
DEF_GEOM_SOA_LENGTH4_DOUBLE_PROTO( 4 )
DEF_GEOM_SOA_LENGTH4_DOUBLE_PROTO( 8 )
DEF_GEOM_SOA_LENGTH4_DOUBLE_PROTO( 16 )

DEF_GEOM_SOA_DISTANCE1_PROTO( fast_ , float, 4 )
DEF_GEOM_SOA_DISTANCE1_PROTO( fast_ , float, 8 )
DEF_GEOM_SOA_DISTANCE1_PROTO( fast_ , float, 16 )

DEF_GEOM_SOA_DISTANCE2_PROTO( fast_ , float, 4 )
DEF_GEOM_SOA_DISTANCE2_PROTO( fast_ , float, 8 )
DEF_GEOM_SOA_DISTANCE2_PROTO( fast_ , float, 16 )

DEF_GEOM_SOA_DISTANCE3_PROTO( fast_ , float, 4 )
DEF_GEOM_SOA_DISTANCE3_PROTO( fast_ , float, 8 )
DEF_GEOM_SOA_DISTANCE3_PROTO( fast_ , float, 16 )

DEF_GEOM_SOA_DISTANCE4_PROTO( fast_ , float, 4 )
DEF_GEOM_SOA_DISTANCE4_PROTO( fast_ , float, 8 )
DEF_GEOM_SOA_DISTANCE4_PROTO( fast_ , float, 16 )

DEF_GEOM_SOA_DISTANCE1_PROTO( , float, 4 )
DEF_GEOM_SOA_DISTANCE1_PROTO( , float, 8 )
DEF_GEOM_SOA_DISTANCE1_PROTO( , float, 16 )
DEF_GEOM_SOA_DISTANCE1_PROTO( , double, 4 )
DEF_GEOM_SOA_DISTANCE1_PROTO( , double, 8 )
DEF_GEOM_SOA_DISTANCE1_PROTO( , double, 16 )

DEF_GEOM_SOA_DISTANCE2_PROTO( , float, 4 )
DEF_GEOM_SOA_DISTANCE2_PROTO( , float, 8 )
DEF_GEOM_SOA_DISTANCE2_PROTO( , float, 16 )
DEF_GEOM_SOA_DISTANCE2_PROTO( , double, 4 )
DEF_GEOM_SOA_DISTANCE2_PROTO( , double, 8 )
DEF_GEOM_SOA_DISTANCE2_PROTO( , double, 16 )

DEF_GEOM_SOA_DISTANCE3_PROTO( , float, 4 )
DEF_GEOM_SOA_DISTANCE3_PROTO( , float, 8 )
DEF_GEOM_SOA_DISTANCE3_PROTO( , float, 16 )
DEF_GEOM_SOA_DISTANCE3_PROTO( , double, 4 )
DEF_GEOM_SOA_DISTANCE3_PROTO( , double, 8 )
DEF_GEOM_SOA_DISTANCE3_PROTO( , double, 16 )

DEF_GEOM_SOA_DISTANCE4_PROTO( , float, 4 )
DEF_GEOM_SOA_DISTANCE4_PROTO( , float, 8 )
DEF_GEOM_SOA_DISTANCE4_PROTO( , float, 16 )
DEF_GEOM_SOA_DISTANCE4_PROTO( , double, 4 )
DEF_GEOM_SOA_DISTANCE4_PROTO( , double, 8 )
DEF_GEOM_SOA_DISTANCE4_PROTO( , double, 16 )

DEF_GEOM_SOA_NORMALIZE1_FAST_PROTO( float, int, 4 )
DEF_GEOM_SOA_NORMALIZE1_FAST_PROTO( float, int, 8 )
DEF_GEOM_SOA_NORMALIZE1_FAST_PROTO( float, int, 16 )

DEF_GEOM_SOA_NORMALIZE2_FAST_PROTO( float, 4 )
DEF_GEOM_SOA_NORMALIZE2_FAST_PROTO( float, 8 )
DEF_GEOM_SOA_NORMALIZE2_FAST_PROTO( float, 16 )

DEF_GEOM_SOA_NORMALIZE3_FAST_PROTO( float, 4 )
DEF_GEOM_SOA_NORMALIZE3_FAST_PROTO( float, 8 )
DEF_GEOM_SOA_NORMALIZE3_FAST_PROTO( float, 16 )

DEF_GEOM_SOA_NORMALIZE4_FAST_PROTO( float, 4 )
DEF_GEOM_SOA_NORMALIZE4_FAST_PROTO( float, 8 )
DEF_GEOM_SOA_NORMALIZE4_FAST_PROTO( float, 16 )

DEF_GEOM_SOA_NORMALIZE1_PROTO( float, int, 4 )
DEF_GEOM_SOA_NORMALIZE1_PROTO( float, int, 8 )
DEF_GEOM_SOA_NORMALIZE1_PROTO( float, int, 16 )
DEF_GEOM_SOA_NORMALIZE1_PROTO( double, long, 4 )
DEF_GEOM_SOA_NORMALIZE1_PROTO( double, long, 8 )
DEF_GEOM_SOA_NORMALIZE1_PROTO( double, long, 16 )
                                 
DEF_GEOM_SOA_NORMALIZE2_FLOAT_PROTO( 4 )
DEF_GEOM_SOA_NORMALIZE2_FLOAT_PROTO( 8 )
DEF_GEOM_SOA_NORMALIZE2_FLOAT_PROTO( 16 )
DEF_GEOM_SOA_NORMALIZE2_DOUBLE_PROTO( 4 )
DEF_GEOM_SOA_NORMALIZE2_DOUBLE_PROTO( 8 )
DEF_GEOM_SOA_NORMALIZE2_DOUBLE_PROTO( 16 )
                                 
DEF_GEOM_SOA_NORMALIZE3_FLOAT_PROTO( 4 )
DEF_GEOM_SOA_NORMALIZE3_FLOAT_PROTO( 8 )
DEF_GEOM_SOA_NORMALIZE3_FLOAT_PROTO( 16 )
DEF_GEOM_SOA_NORMALIZE3_DOUBLE_PROTO( 4 )
DEF_GEOM_SOA_NORMALIZE3_DOUBLE_PROTO( 8 )
DEF_GEOM_SOA_NORMALIZE3_DOUBLE_PROTO( 16 )
                                 
DEF_GEOM_SOA_NORMALIZE4_FLOAT_PROTO( 4 )
DEF_GEOM_SOA_NORMALIZE4_FLOAT_PROTO( 8 )
DEF_GEOM_SOA_NORMALIZE4_FLOAT_PROTO( 16 )
DEF_GEOM_SOA_NORMALIZE4_DOUBLE_PROTO( 4 )
DEF_GEOM_SOA_NORMALIZE4_DOUBLE_PROTO( 8 )
DEF_GEOM_SOA_NORMALIZE4_DOUBLE_PROTO( 16 ) 
