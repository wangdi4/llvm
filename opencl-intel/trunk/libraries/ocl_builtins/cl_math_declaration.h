// Copyright (c) 2006-2009 Intel Corporation
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

#ifdef __cplusplus
extern "C" {
#endif

#include "cl_types2.h"

/////////// Comment about the svmlcc_p2_f8_f8_f8 calling convention /////////////////////////////////////
// We use this CC for cases where the SVML function accepts it second parameter in XMM2 or YMM2 (instead of
// XMM1 or YMM1). This happens only in the some of "g9" flavors, and was probably employed as a workaround for a 
// bug in Intel Compiler.
// Here are examples for when we use this CC :
// #pragma linkage     __ocl_svml_g9_divf8_native_linkage   = ( result((ymm0)) parameters((ymm0),(ymm2))  )
// #pragma linkage     __ocl_svml_g9_ldexpf8_linkage   = ( result((ymm0)) parameters((ymm0),(xmm2 xmm3))  )
/////////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined ( __AVX__ ) /// Platform supports SNB

 /*
  * AVX requires svml g9 for 32-bit dll
  * and               e9 for 64-bit dll
  *
  */
  
#ifdef __x86_64__
#define OCL_SVML_FUNCTION(oclfunc)	__ocl_svml_e9##oclfunc
#else
#define OCL_SVML_FUNCTION(oclfunc)	__ocl_svml_g9##oclfunc
#endif

#elif defined( __SSE4_2__ ) /// Platform supports Nehalem

#ifdef __x86_64__
#define OCL_SVML_FUNCTION(oclfunc)	__ocl_svml_h8##oclfunc
#else
#define OCL_SVML_FUNCTION(oclfunc)	__ocl_svml_n8##oclfunc
#endif

#elif defined ( __SSE4_1__ ) /// Platform supports Penryn

#ifdef __x86_64__
#define OCL_SVML_FUNCTION(oclfunc)	__ocl_svml_y8##oclfunc
#else
#define OCL_SVML_FUNCTION(oclfunc)	__ocl_svml_p8##oclfunc
#endif

#elif defined ( __SSSE3__ ) /// Platform supports Merom

#ifdef __x86_64__
#define OCL_SVML_FUNCTION(oclfunc)	__ocl_svml_u8##oclfunc
#else
#define OCL_SVML_FUNCTION(oclfunc)	__ocl_svml_v8##oclfunc
#endif

#elif defined ( __SSE3__ ) /// Platform supports AMD

#ifdef __x86_64__
#define OCL_SVML_FUNCTION(oclfunc)	__ocl_svml_e7##oclfunc
#else
#define OCL_SVML_FUNCTION(oclfunc)	__ocl_svml_t7##oclfunc
#endif

#endif

#define __global    __attribute__((address_space(1)))
#define __constant  __attribute__((address_space(2)))
#define __local     __attribute__((address_space(3)))


#define OCL_SVML_P1_vFvF_ALL_MANUAL(func,svmlfunc)	\
	OCL_SVML_P1_F1_F1(func,svmlfunc)				\
	OCL_SVML_P1_F2_F2(func,svmlfunc)				\
	OCL_SVML_P1_F3_F3(func,svmlfunc)				\
	OCL_SVML_P1_F4_F4(func,svmlfunc)				\
	OCL_SVML_P1_F8_F8(func,svmlfunc)				\
	OCL_SVML_P1_F16_F16(func,svmlfunc)

#define OCL_SVML_P1_vFvF_ALL(func)		\
	OCL_SVML_P1_F1_F1(func,func)		\
	OCL_SVML_P1_F2_F2(func,func)		\
	OCL_SVML_P1_F3_F3(func,func)		\
	OCL_SVML_P1_F4_F4(func,func)		\
	OCL_SVML_P1_F8_F8(func,func)		\
	OCL_SVML_P1_F16_F16(func,func)		\
	OCL_SVML_P1_D1_D1(func,func)		\
	OCL_SVML_P1_D2_D2(func,func)		\
	OCL_SVML_P1_D3_D3(func,func)		\
	OCL_SVML_P1_D4_D4(func,func)		\
	OCL_SVML_P1_D8_D8(func,func)		\
	OCL_SVML_P1_D16_D16(func,func)

#define OCL_SP_INTR_DP_SVML_P1_vFvF_ALL(func)		\
	OCL_INTR_P1_F1_F1(func)		\
	OCL_INTR_P1_F2_F2_AS_F4(func)		\
	OCL_INTR_P1_F3_F3(func)		\
	OCL_INTR_P1_F4_F4(func)		\
	OCL_INTR_P1_F8_F8(func)		\
	OCL_INTR_P1_F16_F16(func)		\
	OCL_SVML_P1_D1_D1(func,func)		\
	OCL_SVML_P1_D2_D2(func,func)		\
	OCL_SVML_P1_D3_D3(func,func)		\
	OCL_SVML_P1_D4_D4(func,func)		\
	OCL_SVML_P1_D8_D8(func,func)		\
	OCL_SVML_P1_D16_D16(func,func)

#define OCL_INTR_P1_vFvF_ALL_AS_F1(func)		\
	OCL_INTR_P1_F1_F1(func)						\
	OCL_INTR_P1_F2_F2_AS_F1(func)				\
	OCL_INTR_P1_F3_F3(func)						\
	OCL_INTR_P1_F4_F4(func)						\
	OCL_INTR_P1_F8_F8(func)						\
	OCL_INTR_P1_F16_F16(func)	

#define OCL_INTR_P1_vFvF_F816_AS_F8(func)		\
	OCL_INTR_P1_F1_F1(func)						\
	OCL_INTR_P1_F2_F2_AS_F1(func)				\
	OCL_INTR_P1_F3_F3(func)						\
	OCL_INTR_P1_F4_F4(func)						\
	OCL_INTR_P1_F8_F8_AS_F8(func)				\
	OCL_INTR_P1_F16_F16_AS_F8(func)


#define OCL_INTR_P1_vFvF_INTR_vDvD_ALL_AS_F1(func)	\
	OCL_INTR_P1_vFvF_ALL_AS_F1(func)				\
	OCL_INTR_P1_D1_D1(func)							\
	OCL_INTR_P1_D2_D2(func)							\
	OCL_INTR_P1_D3_D3(func)							\
	OCL_INTR_P1_D4_D4(func)							\
	OCL_INTR_P1_D8_D8(func)							\
	OCL_INTR_P1_D16_D16(func)						

#define OCL_INTR_P1_vFvF_INTR_vDvD_ALL_AS_F8_D4(func)	\
    OCL_INTR_P1_vFvF_F816_AS_F8(func)   				\
	OCL_INTR_P1_D1_D1(func)							\
	OCL_INTR_P1_D2_D2(func)							\
	OCL_INTR_P1_D3_D3(func)							\
    OCL_INTR_P1_D4_D4_AS_D4(func)					\
	OCL_INTR_P1_D8_D8_AS_D4(func)					\
	OCL_INTR_P1_D16_D16_AS_D4(func)

#define OCL_INTR_P1_vFvF_vDvD_ALL_AS_F1(func)		\
	OCL_INTR_P1_vFvF_ALL_AS_F1(func)				\
	OCL_SVML_P1_D1_D1(func,func)					\
	OCL_SVML_P1_D2_D2(func,func)					\
	OCL_SVML_P1_D3_D3(func,func)					\
	OCL_SVML_P1_D4_D4(func,func)					\
	OCL_SVML_P1_D8_D8(func,func)					\
	OCL_SVML_P1_D16_D16(func,func)

#define OCL_INTR_P1_vFvF_ALL_AS_F4(func)		\
	OCL_INTR_P1_F1_F1(func)						\
	OCL_INTR_P1_F4_F4(func)						\
	OCL_INTR_P1_F3_F3(func)						\
	OCL_INTR_P1_F2_F2_AS_F4(func)				\
	OCL_INTR_P1_F8_F8(func)						\
	OCL_INTR_P1_F16_F16(func)					
	
#define OCL_INTR_P1_vFvF_INTR_vDvD_ALL_AS_F4(func)	\
	OCL_INTR_P1_vFvF_ALL_AS_F4(func)				\
	OCL_INTR_P1_D1_D1(func)							\
	OCL_INTR_P1_D2_D2(func)							\
	OCL_INTR_P1_D3_D3(func)							\
	OCL_INTR_P1_D4_D4(func)							\
	OCL_INTR_P1_D8_D8(func)							\
	OCL_INTR_P1_D16_D16(func)						\

#define OCL_INTR_P1_vFvF_INTR_vDvD_ALL_AS_F8D4(func)	\
	OCL_INTR_P1_F1_F1(func)						    \
	OCL_INTR_P1_F4_F4(func)						    \
	OCL_INTR_P1_F3_F3(func)						    \
	OCL_INTR_P1_F2_F2_AS_F4(func)			    	\
	OCL_INTR_P1_F8_F8_AS_F8(func)		    		\
	OCL_INTR_P1_F16_F16_AS_F8(func)	    			\
	OCL_INTR_P1_D1_D1(func)							\
	OCL_INTR_P1_D2_D2(func)							\
	OCL_INTR_P1_D3_D3(func)							\
	OCL_INTR_P1_D4_D4_AS_D4(func)					\
	OCL_INTR_P1_D8_D8_AS_D4(func)					\
	OCL_INTR_P1_D16_D16_AS_D4(func)					

#define OCL_INTR_P1_vFvF_vDvD_ALL_AS_F4(func)	\
	OCL_INTR_P1_vFvF_ALL_AS_F4(func)			\
	OCL_SVML_P1_D1_D1(func,func)				\
	OCL_SVML_P1_D2_D2(func,func)				\
	OCL_SVML_P1_D3_D3(func,func)				\
	OCL_SVML_P1_D4_D4(func,func)				\
	OCL_SVML_P1_D8_D8(func,func)				\
	OCL_SVML_P1_D16_D16(func,func)

#define OCL_SVML_P1_vFvI_ALL(func, sign)		\
	OCL_SVML_P1_F1_I1(func, sign)				\
	OCL_SVML_P1_F2_I2(func, sign)				\
	OCL_SVML_P1_F4_I4(func, sign)				\
	OCL_SVML_P1_F8_I8(func, sign)				\
	OCL_SVML_P1_F16_I16(func, sign)				\
	OCL_SVML_P1_D1_I1(func, sign)				\
	OCL_SVML_P1_D2_I2(func, sign)				\
	OCL_SVML_P1_D4_I4(func, sign)				\
	OCL_SVML_P1_D8_I8(func, sign)				\
	OCL_SVML_P1_D16_I16(func, sign)

#define OCL_INTR_P1_vFvI_ALL(func, sign)		\
	OCL_INTR_P1_F1_I1(func, sign)				\
	OCL_INTR_P1_F2_I2_AS_F1(func, sign)			\
	OCL_INTR_P1_F4_I4(func, sign)				\
	OCL_INTR_P1_F3_I3(func, sign)				\
	OCL_INTR_P1_F8_I8(func, sign)				\
	OCL_INTR_P1_F16_I16(func, sign)				\
	OCL_SVML_P1_D1_I1(func, sign)				\
	OCL_SVML_P1_D2_I2(func, sign)				\
	OCL_SVML_P1_D3_I3(func, sign)				\
	OCL_SVML_P1_D4_I4(func, sign)				\
	OCL_SVML_P1_D8_I8(func, sign)				\
	OCL_SVML_P1_D16_I16(func, sign)

#define OCL_INTR_P1_vFvI_F816_AS_F8(func, sign) \
	OCL_INTR_P1_F1_I1(func, sign)				\
	OCL_INTR_P1_F2_I2_AS_F1(func, sign)			\
	OCL_INTR_P1_F4_I4(func, sign)				\
	OCL_INTR_P1_F3_I3(func, sign)				\
	OCL_INTR_P1_F8_I8_AS_F8(func, sign)	    	\
	OCL_INTR_P1_F16_I16_AS_F8(func, sign)		\
	OCL_SVML_P1_D1_I1(func, sign)				\
	OCL_SVML_P1_D2_I2(func, sign)				\
	OCL_SVML_P1_D3_I3(func, sign)				\
	OCL_SVML_P1_D4_I4(func, sign)				\
	OCL_SVML_P1_D8_I8(func, sign)				\
	OCL_SVML_P1_D16_I16(func, sign)


	//
	//

#define OCL_SVML_P2_vFvFF_ALL(func)			\
	OCL_SVML_P2_F2_F2_F1(func)				\
	OCL_SVML_P2_F4_F4_F1(func)				\
	OCL_SVML_P2_F8_F8_F1(func)				\
	OCL_SVML_P2_F16_F16_F1(func)

#define OCL_INTR_P2_vFvFF_ALL_AS_F1(func)		\
	OCL_INTR_P2_F1_F1_F1(func)					\
	OCL_INTR_P2_F4_F4_F4(func)					\
	OCL_INTR_P2_F2_F2_F1_AS_F1(func)			\
	OCL_INTR_P2_F3_F3_F1(func)					\
	OCL_INTR_P2_F4_F4_F1(func)					\
	OCL_INTR_P2_F8_F8_F1(func)					\
	OCL_INTR_P2_F16_F16_F1(func)

#define OCL_INTR_P2_vFvFF_ALL_AS_F4(func)		\
	OCL_INTR_P2_F4_F4_F1(func)					\
	OCL_INTR_P2_F2_F2_F1_AS_F4(func)			\
	OCL_INTR_P2_F3_F3_F1(func)					\
	OCL_INTR_P2_F8_F8_F1(func)					\
	OCL_INTR_P2_F16_F16_F1(func)

#define OCL_INTR_P2_vFvFF_ALL_AS_F8(func)		\
	OCL_INTR_P2_F4_F4_F1(func)					\
	OCL_INTR_P2_F2_F2_F1_AS_F4(func)			\
	OCL_INTR_P2_F3_F3_F1(func)					\
	OCL_INTR_P2_F8_F8_F1_AS_F8(func)			\
	OCL_INTR_P2_F16_F16_F1_AS_F8(func)

#define OCL_SVML_P2_vFvFvF_ALL(func)			\
	OCL_SVML_P2_F1_F1_F1(func,func)				\
	OCL_SVML_P2_F2_F2_F2(func,func)				\
	OCL_SVML_P2_F4_F4_F4(func,func)				\
	OCL_SVML_P2_F3_F3_F3(func,func)				\
	OCL_SVML_P2_F8_F8_F8(func,func)				\
	OCL_SVML_P2_F16_F16_F16(func,func)			\
	OCL_SVML_P2_D1_D1_D1(func,func)				\
	OCL_SVML_P2_D2_D2_D2(func,func)				\
	OCL_SVML_P2_D3_D3_D3(func,func)				\
	OCL_SVML_P2_D4_D4_D4(func,func)				\
	OCL_SVML_P2_D8_D8_D8(func,func)				\
	OCL_SVML_P2_D16_D16_D16(func,func)

#define OCL_INTR_P2_vFvFvF_ALL_AS_F1(func)		\
	OCL_INTR_P2_F1_F1_F1(func)					\
	OCL_INTR_P2_F2_F2_F2_AS_F1(func)			\
	OCL_INTR_P2_F3_F3_F3(func)					\
	OCL_INTR_P2_F4_F4_F4(func)					\
	OCL_INTR_P2_F8_F8_F8_AS_F4(func)			\
	OCL_INTR_P2_F16_F16_F16_AS_F4(func)			\
	OCL_INTR_P2_D1_D1_D1(func)					\
	OCL_INTR_P2_D2_D2_D2_AS_D1(func)			\
	OCL_INTR_P2_D3_D3_D3_AS_D4(func)			\
	OCL_INTR_P2_D4_D4_D4(func)					\
	OCL_INTR_P2_D8_D8_D8_AS_D4(func)			\
	OCL_INTR_P2_D16_D16_D16_AS_D4(func)

#define OCL_INTR_P2_vFvFvF_ALL_AS_F4(func)		\
	OCL_INTR_P2_F1_F1_F1(func)					\
	OCL_INTR_P2_F4_F4_F4(func)					\
	OCL_INTR_P2_F3_F3_F3(func)					\
	OCL_INTR_P2_F2_F2_F2_AS_F4(func)			\
	OCL_INTR_P2_F8_F8_F8_AS_F4(func)			\
	OCL_INTR_P2_F16_F16_F16_AS_F4(func)			
	
#define OCL_INTR_P2_vFvFvF_ALL_AS_F8(func)		\
	OCL_INTR_P2_F1_F1_F1(func)					\
	OCL_INTR_P2_F4_F4_F4(func)					\
	OCL_INTR_P2_F3_F3_F3(func)      			\
	OCL_INTR_P2_F2_F2_F2_AS_F4(func)			\
	OCL_INTR_P2_F8_F8_F8_AS_F8(func)			\
	OCL_INTR_P2_F16_F16_F16_AS_F8(func)				

#define OCL_INTR_P2_vFvFvF_INTR_vDvDvD_ALL_AS_F4(func)	\
	OCL_INTR_P2_vFvFvF_ALL_AS_F4(func)					\
	OCL_INTR_P2_D1_D1_D1(func)							\
	OCL_INTR_P2_D2_D2_D2(func)							\
	OCL_INTR_P2_D3_D3_D3_AS_D2(func)					\
	OCL_INTR_P2_D4_D4_D4_AS_D2(func)					\
	OCL_INTR_P2_D8_D8_D8_AS_D2(func)					\
	OCL_INTR_P2_D16_D16_D16_AS_D2(func)

#define OCL_INTR_P2_vFvFvF_INTR_vDvDvD_ALL_AS_F8D4(func)	\
	OCL_INTR_P2_vFvFvF_ALL_AS_F8(func)					\
	OCL_INTR_P2_D1_D1_D1(func)							\
	OCL_INTR_P2_D2_D2_D2(func)							\
	OCL_INTR_P2_D3_D3_D3_AS_D2(func)					\
	OCL_INTR_P2_D4_D4_D4_AS_D4(func)					\
	OCL_INTR_P2_D8_D8_D8_AS_D4(func)					\
	OCL_INTR_P2_D16_D16_D16_AS_D4(func)

#define OCL_INTR_P2_vFvFvF_F234816_AS_F4(func)          \
    OCL_INTR_P2_F1_F1_F1(func)                          \
    OCL_INTR_P2_F2_F2_F2_AS_F4(func)                    \
    OCL_INTR_P2_F3_F3_F3_AS_F4(func)                    \
    OCL_INTR_P2_F4_F4_F4(func)                          \
    OCL_INTR_P2_F8_F8_F8_AS_F4(func)                    \
    OCL_INTR_P2_F16_F16_F16_AS_F4(func)

#define OCL_INTR_P2_vFvFvF_F234_AS_F4_F816_AS_F8(func)  \
    OCL_INTR_P2_F1_F1_F1(func)                          \
    OCL_INTR_P2_F2_F2_F2_AS_F4(func)                    \
    OCL_INTR_P2_F3_F3_F3_AS_F4(func)                    \
    OCL_INTR_P2_F4_F4_F4(func)                          \
    OCL_INTR_P2_F8_F8_F8_AS_F8(func)                    \
    OCL_INTR_P2_F16_F16_F16_AS_F8(func)

#define OCL_INTR_P2_vDvDvD_ALL_AS_D1(func)              \
	OCL_INTR_P2_D1_D1_D1(func)					\
	OCL_INTR_P2_D2_D2_D2_AS_D1(func)					\
	OCL_INTR_P2_D3_D3_D3_AS_D2(func)					\
	OCL_INTR_P2_D4_D4_D4_AS_D2(func)					\
	OCL_INTR_P2_D8_D8_D8_AS_D2(func)					\
	OCL_INTR_P2_D16_D16_D16_AS_D2(func)

#define OCL_INTR_P2_vFvFvF_AS_F4_INTR_vDvDvD_ALL_AS_D1(func)    \
    OCL_INTR_P2_vFvFvF_F234816_AS_F4(func)                      \
    OCL_INTR_P2_vDvDvD_ALL_AS_D1(func)

#define OCL_INTR_P2_vFvFvF_AS_F8_INTR_vDvDvD_ALL_AS_D1(func)	\
    OCL_INTR_P2_vFvFvF_F234_AS_F4_F816_AS_F8(func) 				\
    OCL_INTR_P2_vDvDvD_ALL_AS_D1(func)

#define OCL_INTR_P2_vFvFvF_vDvDvD_ALL_AS_F4(func)	\
	OCL_INTR_P2_vFvFvF_ALL_AS_F4(func)				\
	OCL_SVML_P2_D1_D1_D1(func,func)					\
	OCL_SVML_P2_D2_D2_D2(func,func)					\
	OCL_SVML_P2_D3_D3_D3(func,func)					\
	OCL_SVML_P2_D4_D4_D4(func,func)					\
	OCL_SVML_P2_D8_D8_D8(func,func)					\
	OCL_SVML_P2_D16_D16_D16(func,func)
	
#define OCL_INTR_P2_vFvFvF_vDvDvD_ALL_AS_F8(func)	\
	OCL_INTR_P2_vFvFvF_ALL_AS_F8(func)				\
	OCL_SVML_P2_D1_D1_D1(func,func)					\
	OCL_SVML_P2_D2_D2_D2(func,func)					\
	OCL_SVML_P2_D3_D3_D3(func,func)					\
	OCL_SVML_P2_D4_D4_D4(func,func)					\
	OCL_SVML_P2_D8_D8_D8(func,func)					\
	OCL_SVML_P2_D16_D16_D16(func,func)

#define OCL_SVML_P2_vFvFvF_ALL_MANUAL(func,svmlfunc)\
	OCL_SVML_P2_F1_F1_F1(func,svmlfunc)				\
	OCL_SVML_P2_F2_F2_F2(func,svmlfunc)				\
	OCL_SVML_P2_F4_F4_F4(func,svmlfunc)				\
	OCL_SVML_P2_F3_F3_F3(func,svmlfunc)				\
	OCL_SVML_P2_F8_F8_F8(func,svmlfunc)				\
	OCL_SVML_P2_F16_F16_F16(func,svmlfunc)

#define OCL_SVML_P2_vFvFvI_ALL(func, sign)			\
	OCL_SVML_P2_F1_F1_I1(func, sign)				\
	OCL_SVML_P2_F2_F2_I2(func, sign)				\
	OCL_SVML_P2_F3_F3_I3(func, sign)				\
	OCL_SVML_P2_F4_F4_I4(func, sign)				\
	OCL_SVML_P2_F8_F8_I8(func, sign)				\
	OCL_SVML_P2_F16_F16_I16(func, sign)				\
	OCL_SVML_P2_D1_D1_I1(func,sign)					\
	OCL_SVML_P2_D2_D2_I2(func,sign)					\
	OCL_SVML_P2_D3_D3_I3(func,sign)					\
	OCL_SVML_P2_D4_D4_I4(func,sign)					\
	OCL_SVML_P2_D8_D8_I8(func,sign)					\
	OCL_SVML_P2_D16_D16_I16(func,sign)

#define OCL_SVML_P2_vFvFI_ALL(func, sign)			\
	OCL_SVML_P2_F2_F2_I(func, sign)				\
	OCL_SVML_P2_F3_F3_I(func, sign)				\
	OCL_SVML_P2_F4_F4_I(func, sign)				\
	OCL_SVML_P2_F8_F8_I(func, sign)				\
	OCL_SVML_P2_F16_F16_I(func, sign)				\
	OCL_SVML_P2_D2_D2_I(func,sign)					\
	OCL_SVML_P2_D3_D3_I(func,sign)					\
	OCL_SVML_P2_D4_D4_I(func,sign)					\
	OCL_SVML_P2_D8_D8_I(func,sign)					\
	OCL_SVML_P2_D16_D16_I(func,sign)
		

#define OCL_SVML_P2_vFvFpvF(func)				\
	OCL_SVML_P2_F1_F1_pF1(func)					\
	OCL_SVML_P2_F1_F1_pF1_LOCAL(func)			\
	OCL_SVML_P2_F1_F1_pF1_GLOBAL(func)			\
	OCL_SVML_P2_F2_F2_pF2(func)					\
	OCL_SVML_P2_F2_F2_pF2_LOCAL(func)			\
	OCL_SVML_P2_F2_F2_pF2_GLOBAL(func)			\
	OCL_SVML_P2_F3_F3_pF3(func)					\
	OCL_SVML_P2_F3_F3_pF3_LOCAL(func)			\
	OCL_SVML_P2_F3_F3_pF3_GLOBAL(func)			\
	OCL_SVML_P2_F4_F4_pF4(func)					\
	OCL_SVML_P2_F4_F4_pF4_LOCAL(func)			\
	OCL_SVML_P2_F4_F4_pF4_GLOBAL(func)			\
	OCL_SVML_P2_F8_F8_pF8(func)					\
	OCL_SVML_P2_F8_F8_pF8_LOCAL(func)			\
	OCL_SVML_P2_F8_F8_pF8_GLOBAL(func)			\
	OCL_SVML_P2_F16_F16_pF16(func)				\
	OCL_SVML_P2_F16_F16_pF16_LOCAL(func)		\
	OCL_SVML_P2_F16_F16_pF16_GLOBAL(func)		\
	OCL_SVML_P2_D1_D1_pD1(func)					\
	OCL_SVML_P2_D1_D1_pD1_GLOBAL(func)			\
	OCL_SVML_P2_D1_D1_pD1_LOCAL(func)			\
	OCL_SVML_P2_D2_D2_pD2(func)					\
	OCL_SVML_P2_D2_D2_pD2_GLOBAL(func)			\
	OCL_SVML_P2_D2_D2_pD2_LOCAL(func)			\
	OCL_SVML_P2_D3_D3_pD3(func)					\
	OCL_SVML_P2_D3_D3_pD3_GLOBAL(func)			\
	OCL_SVML_P2_D3_D3_pD3_LOCAL(func)			\
	OCL_SVML_P2_D4_D4_pD4(func)					\
	OCL_SVML_P2_D4_D4_pD4_GLOBAL(func)			\
	OCL_SVML_P2_D4_D4_pD4_LOCAL(func)			\
	OCL_SVML_P2_D8_D8_pD8(func)					\
	OCL_SVML_P2_D8_D8_pD8_GLOBAL(func)			\
	OCL_SVML_P2_D8_D8_pD8_LOCAL(func)			\
	OCL_SVML_P2_D16_D16_pD16(func)				\
	OCL_SVML_P2_D16_D16_pD16_GLOBAL(func)		\
	OCL_SVML_P2_D16_D16_pD16_LOCAL(func)		

// f1 scalar, f2, f3, f4 as f4
#define OCL_INTR_P2_vFvFpvF_F23_AS_F4(func)     \
	OCL_INTR_P2_F1_F1_pF1(func)					\
	OCL_INTR_P2_F1_F1_pF1_LOCAL(func)			\
	OCL_INTR_P2_F1_F1_pF1_GLOBAL(func)			\
	OCL_INTR_P2_F4_F4_pF4(func)					\
	OCL_INTR_P2_F4_F4_pF4_LOCAL(func)			\
	OCL_INTR_P2_F4_F4_pF4_GLOBAL(func)			\
	OCL_INTR_P2_F3_F3_pF3(func)					\
	OCL_INTR_P2_F3_F3_pF3_LOCAL(func)			\
	OCL_INTR_P2_F3_F3_pF3_GLOBAL(func)			\
	OCL_INTR_P2_F2_F2_pF2(func)					\
	OCL_INTR_P2_F2_F2_pF2_LOCAL(func)			\
	OCL_INTR_P2_F2_F2_pF2_GLOBAL(func)

#define OCL_INTR_P2_vFvFpvF(func)				\
	OCL_INTR_P2_vFvFpvF_F23_AS_F4(func)         \
    OCL_INTR_P2_F8_F8_pF8(func)					\
	OCL_INTR_P2_F8_F8_pF8_LOCAL(func)			\
	OCL_INTR_P2_F8_F8_pF8_GLOBAL(func)			\
	OCL_INTR_P2_F16_F16_pF16(func)				\
	OCL_INTR_P2_F16_F16_pF16_LOCAL(func)		\
	OCL_INTR_P2_F16_F16_pF16_GLOBAL(func)		
	
#define OCL_INTR_P2_vFvFpvF_AS_F8(func)			\
	OCL_INTR_P2_vFvFpvF_F23_AS_F4(func)         \
    OCL_INTR_P2_F8_F8_pF8_AS_F8(func)			\
	OCL_INTR_P2_F8_F8_pF8_LOCAL(func)			\
	OCL_INTR_P2_F8_F8_pF8_GLOBAL(func)			\
	OCL_INTR_P2_F16_F16_pF16_AS_F8(func)		\
	OCL_INTR_P2_F16_F16_pF16_LOCAL(func)		\
	OCL_INTR_P2_F16_F16_pF16_GLOBAL(func)		

#define OCL_SVML_P2_vDvDpvD(func)				\
	OCL_SVML_P2_D1_D1_pD1(func)					\
	OCL_SVML_P2_D1_D1_pD1_GLOBAL(func)			\
	OCL_SVML_P2_D1_D1_pD1_LOCAL(func)			\
	OCL_SVML_P2_D2_D2_pD2(func)					\
	OCL_SVML_P2_D2_D2_pD2_GLOBAL(func)			\
	OCL_SVML_P2_D2_D2_pD2_LOCAL(func)			\
	OCL_SVML_P2_D3_D3_pD3(func)					\
	OCL_SVML_P2_D3_D3_pD3_GLOBAL(func)			\
	OCL_SVML_P2_D3_D3_pD3_LOCAL(func)			\
	OCL_SVML_P2_D4_D4_pD4(func)					\
	OCL_SVML_P2_D4_D4_pD4_GLOBAL(func)			\
	OCL_SVML_P2_D4_D4_pD4_LOCAL(func)			\
	OCL_SVML_P2_D8_D8_pD8(func)					\
	OCL_SVML_P2_D8_D8_pD8_GLOBAL(func)			\
	OCL_SVML_P2_D8_D8_pD8_LOCAL(func)			\
	OCL_SVML_P2_D16_D16_pD16(func)				\
	OCL_SVML_P2_D16_D16_pD16_GLOBAL(func)		\
	OCL_SVML_P2_D16_D16_pD16_LOCAL(func)

#define OCL_INTR_P2_vFvFpvF_vDvDpvD(func)		\
    OCL_INTR_P2_vFvFpvF(func)					\
    OCL_SVML_P2_vDvDpvD(func)

#define OCL_INTR_P2_vFvFpvF_vDvDpvD_F816_AS_F8(func)	\
    OCL_INTR_P2_vFvFpvF_AS_F8(func)                     \
    OCL_SVML_P2_vDvDpvD(func)

#define OCL_SVML_P2_vFvFPI_ALL(func,svmlfunc)			\
	OCL_SVML_P2_F1_F1_PI1(func,svmlfunc)				\
	OCL_SVML_P2_F1_F1_PI1_LOCAL(func)					\
	OCL_SVML_P2_F1_F1_PI1_GLOBAL(func)					\
	OCL_SVML_P2_F2_F2_PI2(func,svmlfunc)				\
	OCL_SVML_P2_F2_F2_PI2_LOCAL(func,svmlfunc)			\
	OCL_SVML_P2_F2_F2_PI2_GLOBAL(func,svmlfunc)			\
	OCL_SVML_P2_F3_F3_PI3(func,svmlfunc)				\
	OCL_SVML_P2_F3_F3_PI3_LOCAL(func,svmlfunc)			\
	OCL_SVML_P2_F3_F3_PI3_GLOBAL(func,svmlfunc)			\
	OCL_SVML_P2_F4_F4_PI4(func,svmlfunc)				\
	OCL_SVML_P2_F4_F4_PI4_LOCAL(func,svmlfunc)			\
	OCL_SVML_P2_F4_F4_PI4_GLOBAL(func,svmlfunc)			\
	OCL_SVML_P2_F8_F8_PI8(func,svmlfunc)				\
	OCL_SVML_P2_F8_F8_PI8_LOCAL(func,svmlfunc)			\
	OCL_SVML_P2_F8_F8_PI8_GLOBAL(func,svmlfunc)			\
	OCL_SVML_P2_F16_F16_PI16(func,svmlfunc)				\
	OCL_SVML_P2_F16_F16_PI16_LOCAL(func,svmlfunc)		\
	OCL_SVML_P2_F16_F16_PI16_GLOBAL(func,svmlfunc)		\
	OCL_SVML_P2_D1_D1_PI1(func,svmlfunc)				\
	OCL_SVML_P2_D1_D1_PI1_LOCAL(func)					\
	OCL_SVML_P2_D1_D1_PI1_GLOBAL(func)					\
	OCL_SVML_P2_D2_D2_PI2(func,svmlfunc)				\
	OCL_SVML_P2_D2_D2_PI2_LOCAL(func)					\
	OCL_SVML_P2_D2_D2_PI2_GLOBAL(func)					\
	OCL_SVML_P2_D3_D3_PI3(func,svmlfunc)				\
	OCL_SVML_P2_D3_D3_PI3_LOCAL(func)					\
	OCL_SVML_P2_D3_D3_PI3_GLOBAL(func)					\
	OCL_SVML_P2_D4_D4_PI4(func,svmlfunc)				\
	OCL_SVML_P2_D4_D4_PI4_LOCAL(func)					\
	OCL_SVML_P2_D4_D4_PI4_GLOBAL(func)					\
	OCL_SVML_P2_D8_D8_PI8(func,svmlfunc)				\
	OCL_SVML_P2_D8_D8_PI8_LOCAL(func)					\
	OCL_SVML_P2_D8_D8_PI8_GLOBAL(func)					\
	OCL_SVML_P2_D16_D16_PI16(func,svmlfunc)				\
	OCL_SVML_P2_D16_D16_PI16_LOCAL(func)				\
	OCL_SVML_P2_D16_D16_PI16_GLOBAL(func)	

#define OCL_SVML_P3_vFvFvFvF_ALL(func)		\
	OCL_SVML_P3_F1_F1_F1_F1(func)			\
	OCL_SVML_P3_F2_F2_F2_F2(func)			\
	OCL_SVML_P3_F3_F3_F3_F3(func)			\
	OCL_SVML_P3_F4_F4_F4_F4(func)			\
	OCL_SVML_P3_F8_F8_F8_F8(func)			\
	OCL_SVML_P3_F16_F16_F16_F16(func)		\
	OCL_SVML_P3_D1_D1_D1_D1(func)			\
	OCL_SVML_P3_D2_D2_D2_D2(func)			\
	OCL_SVML_P3_D3_D3_D3_D3(func)			\
	OCL_SVML_P3_D4_D4_D4_D4(func)			\
	OCL_SVML_P3_D8_D8_D8_D8(func)			\
	OCL_SVML_P3_D16_D16_D16_D16(func)
	
#define OCL_SVML_P3_vFvFvFpvI(func)				\
	OCL_SVML_P3_F1_F1_F1_PI1(func)				\
	OCL_SVML_P3_F1_F1_F1_PI1_LOCAL(func)		\
	OCL_SVML_P3_F1_F1_F1_PI1_GLOBAL(func)		\
	OCL_SVML_P3_F2_F2_F2_PI2(func)				\
	OCL_SVML_P3_F2_F2_F2_PI2_LOCAL(func)		\
	OCL_SVML_P3_F2_F2_F2_PI2_GLOBAL(func)		\
	OCL_SVML_P3_F3_F3_F3_PI3(func)				\
	OCL_SVML_P3_F3_F3_F3_PI3_LOCAL(func)		\
	OCL_SVML_P3_F3_F3_F3_PI3_GLOBAL(func)		\
	OCL_SVML_P3_F4_F4_F4_PI4(func)				\
	OCL_SVML_P3_F4_F4_F4_PI4_LOCAL(func)		\
	OCL_SVML_P3_F4_F4_F4_PI4_GLOBAL(func)		\
	OCL_SVML_P3_F8_F8_F8_PI8(func)				\
	OCL_SVML_P3_F8_F8_F8_PI8_LOCAL(func)		\
	OCL_SVML_P3_F8_F8_F8_PI8_GLOBAL(func)		\
	OCL_SVML_P3_F16_F16_F16_PI16(func)			\
	OCL_SVML_P3_F16_F16_F16_PI16_LOCAL(func)	\
	OCL_SVML_P3_F16_F16_F16_PI16_GLOBAL(func)

#define  OCL_INTR_P1_vIvF_ALL_AS_F4(func)		\
	OCL_INTR_P1_I1_F1(func)						\
	OCL_INTR_P1_I4_F4(func)						\
    OCL_INTR_P1_I2_F2_AS_F4(func)               \
	OCL_INTR_P1_I3_F3(func)				        \
	OCL_INTR_P1_I8_F8(func)						\
	OCL_INTR_P1_I16_F16(func)					\

//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//  1 - Parameter 
#define OCL_SVML_P1_F1_F1(func,svmlfunc)											\
	__attribute__((svmlcc)) __attribute__((const)) float OCL_SVML_FUNCTION(_##svmlfunc##f1)(float);		\
	__attribute__((overloadable)) float func(float x)								\
	{																				\
		return OCL_SVML_FUNCTION(_##svmlfunc##f1)(x);								\
	}																				

#define OCL_SVML_P1_F2_F2TT(func,svmlfunc)											\
	__attribute__((svmlcc)) float4 OCL_SVML_FUNCTION(_##svmlfunc##f2)(float4);		\
	__attribute__((overloadable)) float2 func(float2 x)								\
	{																				\
		float4 val;																	\
		val = _mm_castsi128_ps(_mm_loadl_epi64((__m128i*)&x));						\
		val = OCL_SVML_FUNCTION(_##svmlfunc##f2)(val);								\
		float2 res;																	\
		_mm_storel_epi64((__m128i*)&res, (__m128i)val);								\
		return res;																	\
	}																				

#define OCL_SVML_P1_F2_F2(func,svmlfunc)											\
	__attribute__((svmlcc)) __attribute__((const)) float4 OCL_SVML_FUNCTION(_##svmlfunc##f2)(float4);		\
	__attribute__((overloadable)) float2 func(float2 x)								\
	{																				\
		float4 val;																	\
		val.s01 = x;\
		float4 res = OCL_SVML_FUNCTION(_##svmlfunc##f2)(val);						\
		return res.s01;																\
	}

#define OCL_SVML_P1_F3_F3(func,svmlfunc)											\
	__attribute__((svmlcc)) __attribute__((const)) float4 OCL_SVML_FUNCTION(_##svmlfunc##f4)(float4);		\
	float3 __attribute__((overloadable)) func(float3 x)								\
	{																				\
		float4 val;																	\
		val.s012 = x;																\
		val = OCL_SVML_FUNCTION(_##svmlfunc##f4)(val);								\
		return val.s012;\
	}																				
#define OCL_SVML_P1_F4_F4(func,svmlfunc)											\
	__attribute__((svmlcc)) __attribute__((const)) float4 OCL_SVML_FUNCTION(_##svmlfunc##f4)(float4);		\
	__attribute__((overloadable)) float4 func(float4 x)								\
	{																				\
		return OCL_SVML_FUNCTION(_##svmlfunc##f4)(x);								\
	}																				

#define OCL_SVML_P1_F8_F8(func,svmlfunc)											\
	__attribute__((svmlcc)) __attribute__((const)) float8 OCL_SVML_FUNCTION(_##svmlfunc##f8)(float8);		\
	__attribute__((overloadable)) float8 func(float8 x)								\
	{																				\
		return OCL_SVML_FUNCTION(_##svmlfunc##f8)(x);								\
	}																				

#define OCL_SVML_P1_F16_F16(func,svmlfunc)											\
	__attribute__((svmlcc)) __attribute__((const)) float16 OCL_SVML_FUNCTION(_##svmlfunc##f16)(float16);	\
	__attribute__((overloadable)) float16 func(float16 x)							\
	{																				\
		return OCL_SVML_FUNCTION(_##svmlfunc##f16)(x);								\
	}																				

#define OCL_SVML_P1_D1_D1(func,svmlfunc)											\
	__attribute__((svmlcc)) __attribute__((const)) double OCL_SVML_FUNCTION(_##svmlfunc##1)(double);								\
	__attribute__((overloadable)) double func(double x)								\
	{																				\
		return OCL_SVML_FUNCTION(_##svmlfunc##1)(x);								\
	}																				

#define OCL_SVML_P1_D2_D2(func,svmlfunc)											\
	__attribute__((svmlcc)) __attribute__((const)) double2 OCL_SVML_FUNCTION(_##svmlfunc##2)(double2);								\
	__attribute__((overloadable)) double2 func(double2 x)							\
	{																				\
		return OCL_SVML_FUNCTION(_##svmlfunc##2)(x);								\
	}																				

#define OCL_SVML_P1_D3_D3(func,svmlfunc)											\
	__attribute__((svmlcc)) __attribute__((const)) double4 OCL_SVML_FUNCTION(_##svmlfunc##4)(double4);								\
	__attribute__((overloadable)) double3 func(double3 x)							\
	{																				\
		double4 valx;																\
		valx.s012 = x;																\
		double4 res = OCL_SVML_FUNCTION(_##svmlfunc##4)(valx);						\
		return res.s012;															\
	}	


#define OCL_SVML_P1_D4_D4(func,svmlfunc)											\
	__attribute__((svmlcc)) __attribute__((const)) double4 OCL_SVML_FUNCTION(_##svmlfunc##4)(double4);								\
	__attribute__((overloadable)) double4 func(double4 x)							\
	{																				\
		return OCL_SVML_FUNCTION(_##svmlfunc##4)(x);								\
	}																				

#define OCL_SVML_P1_D8_D8(func,svmlfunc)											\
	__attribute__((svmlcc)) __attribute__((const)) double8 OCL_SVML_FUNCTION(_##svmlfunc##8)(double8);								\
	__attribute__((overloadable)) double8 func(double8 x)							\
	{																				\
		return OCL_SVML_FUNCTION(_##svmlfunc##8)(x);								\
	}																				

#define OCL_SVML_P1_D16_D16(func,svmlfunc)											\
	__attribute__((svmlcc)) __attribute__((const)) double8 OCL_SVML_FUNCTION(_##svmlfunc##8)(double8);							\
	__attribute__((overloadable)) double16 func(double16 x)							\
	{																				\
		double16 res;																\
		res.lo = OCL_SVML_FUNCTION(_##svmlfunc##8)(x.lo);							\
		res.hi = OCL_SVML_FUNCTION(_##svmlfunc##8)(x.hi);							\
		return res;																	\
	}					


//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//  1 - Parameter 
#define OCL_INTR_P1_F1_F1(func)														\
	__attribute__((overloadable)) float func(float);					

#define OCL_INTR_P1_F2_F2_AS_F4(func)												\
	__attribute__((overloadable)) float2 func(float2 x)								\
	{																				\
		float4 val;																	\
		float2 res;																	\
		val = _mm_castsi128_ps(_mm_loadl_epi64((__m128i*)&x));						\
		val = func(val);															\
		_mm_storel_epi64((__m128i*)&res, (__m128i)val);								\
		return res;																	\
	}																				

#define OCL_INTR_P1_F2_F2_AS_F1(func)												\
	__attribute__((overloadable)) float2 func(float2 x)								\
	{																				\
		float2 res;																	\
		res.lo = func(x.lo);														\
		res.hi = func(x.hi);														\
		return res;																	\
	}																				

#define OCL_INTR_P1_F4_F4(func)														\
	__attribute__((overloadable)) float4 func(float4 x);									

#define OCL_INTR_P1_F3_F3(func)														\
	__attribute__((overloadable)) float3 func(float3 x)				\
	{																				\
		float4 res,valx;															\
		valx.s012 = x;																\
		res = func(valx);															\
		return res.s012;															\
	}																				

#define OCL_INTR_P1_F8_F8_PROXY(func)											\
	__attribute__((overloadable)) float8 func(float8 x)								\
	{																				\
		float8 res;																	\
		res.lo = func(x.lo);														\
		res.hi = func(x.hi);														\
		return res;																	\
	}																				

#if defined(__AVX__)
#define OCL_INTR_P1_F8_F8(func)														\
    __attribute__((overloadable)) float8 func(float8 x);

#define OCL_INTR_P1_F16_F16(func)													\
	__attribute__((overloadable)) float16 func(float16 x)							\
	{																				\
		float16 res;																\
		res.s01234567 =  func(x.s01234567);											\
		res.s89abcdef =  func(x.s89abcdef);											\
		return res;																	\
	}

#else // defined(__AVX__)
#define OCL_INTR_P1_F8_F8(func)	OCL_INTR_P1_F8_F8_PROXY(func)
#define OCL_INTR_P1_F16_F16(func)													\
	__attribute__((overloadable)) float16 func(float16 x)							\
    {																				\
		float16 res;																\
		res.s0123 =  func(x.s0123);													\
		res.s4567 =  func(x.s4567);													\
		res.s89ab =  func(x.s89ab);													\
		res.scdef =  func(x.scdef);													\
		return res;																	\
	}

#endif //defined(__AVX__)

#define OCL_INTR_P1_F8_F8_AS_F8(func)                                               \
    __attribute__((overloadable)) float8 func(float8 x);

#define OCL_INTR_P1_F16_F16_AS_F8(func)                                             \
	__attribute__((overloadable)) float16 func(float16 x)							\
	{																				\
		float16 res;																\
		res.s01234567 =  func(x.s01234567);											\
		res.s89abcdef =  func(x.s89abcdef);											\
		return res;																	\
	}


//doubles
#define OCL_INTR_P1_D1_D1(func)												\
	 __attribute__((overloadable)) double func(double);										


#define OCL_INTR_P1_D2_D2(func)												\
	 __attribute__((overloadable)) double2 func(double2 x);

#define OCL_INTR_P1_D3_D3(func)														\
	__attribute__((overloadable)) double3 func(double3 x)				\
	{																	\
		double3 res;													\
		res.s01 = func(x.s01);											\
		res.s2 = func(x.s2);											\
	return res;															\
	}	

#define OCL_INTR_P1_D4_D4(func)												\
	 __attribute__((overloadable)) double4 func(double4 x){	\
		double4 res;														\
		res.lo = func(x.lo);												\
		res.hi = func(x.hi);												\
		return res;															\
			}	

#define OCL_INTR_P1_D4_D4_AS_D4(func)										\
	 __attribute__((overloadable)) double4 func(double4 x);

#define OCL_INTR_P1_D8_D8(func)												\
	 __attribute__((overloadable)) double8 func(double8 x){	\
	double8 res;															\
	res.s01 = func(x.s01);													\
	res.s23 = func(x.s23);													\
	res.s45 = func(x.s45);													\
	res.s67 = func(x.s67);													\
	return res;																\
	}

#define OCL_INTR_P1_D8_D8_AS_D4(func)										\
	 __attribute__((overloadable)) double8 func(double8 x){	                \
	double8 res;															\
	res.s0123 = func(x.s0123);												\
	res.s4567 = func(x.s4567);												\
	return res;																\
	}

#define OCL_INTR_P1_D16_D16(func)											\
	 __attribute__((overloadable)) double16 func(double16 x){	\
	double16 res;															\
	res.s01 = func(x.s01);													\
	res.s23 = func(x.s23);													\
	res.s45 = func(x.s45);													\
	res.s67 = func(x.s67);													\
	res.s89 = func(x.s89);													\
	res.sab = func(x.sab);													\
	res.scd = func(x.scd);													\
	res.sef = func(x.sef);													\
	return res;																\
			}

#define OCL_INTR_P1_D16_D16_AS_D4(func)										\
	 __attribute__((overloadable)) double16 func(double16 x){	\
	double16 res;															\
	res.s0123 = func(x.s0123);												\
	res.s4567 = func(x.s4567);												\
	res.s89ab = func(x.s89ab);												\
	res.scdef = func(x.scdef);												\
	return res;																\
    }


//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
#define OCL_SVML_P2_F2_F2_F1(func)												\
	__attribute__((overloadable)) float2 func(float2 x,float y)					\
	{																			\
		float4 valx;															\
		valx = _mm_castsi128_ps(_mm_loadl_epi64((__m128i*)&x));					\
		float4 valy;															\
		valy = _mm_load_ss(&y);													\
		valy = _mm_shuffle_ps(valy, valy, 0);									\
		valx = OCL_SVML_FUNCTION(_##func##f4)(valx,valy);						\
		float2 res;																\
		_mm_storel_epi64((__m128i*)&res, (__m128i)valx);						\
		return res;																\
	}																			

#define OCL_SVML_P2_F4_F4_F1(func)												\
	__attribute__((overloadable)) float4 func(float4 x,float y)					\
	{																			\
		float4 valy;															\
		valy = _mm_load_ss(&y);													\
		valy = _mm_shuffle_ps(valy, valy, 0);									\
		return OCL_SVML_FUNCTION(_##func##f4)(x,valy);							\
	}																			

#define OCL_SVML_P2_F8_F8_F1(func)												\
	__attribute__((overloadable)) float8 func(float8 x,float y)					\
	{																			\
		float8 valy = y;														\
		return OCL_SVML_FUNCTION(_##func##f8)(x,valy);							\
	}																			
	
#define OCL_SVML_P2_F16_F16_F1(func)											\
	__attribute__((overloadable)) float16 func(float16 x,float y)				\
	{																			\
		float16 valy = y;														\
        float16 res;                                                            \
		res.lo = OCL_SVML_FUNCTION(_##func##f8)(x.lo,valy.lo);					\
        res.hi = OCL_SVML_FUNCTION(_##func##f8)(x.hi,valy.lo);                  \
        return res;                                                             \
	}																				


//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
#define OCL_INTR_P2_F2_F2_F1_AS_F1(func)										\
	__attribute__((overloadable)) float2 func(float2 x,float y)					\
	{																			\
		float2 res;																\
		res.lo = func(x.lo, y);													\
		res.hi = func(x.hi, y);													\
		return res;																\
	}																			

#define OCL_INTR_P2_F2_F2_F1_AS_F4(func)										\
	__attribute__((overloadable)) float2 func(float2 x,float y)					\
	{																			\
		float4 xVec, yVec;														\
		float2 res;																\
		xVec = _mm_castsi128_ps(_mm_loadl_epi64((__m128i*)&x));					\
		yVec = _mm_set1_ps(y);													\
		xVec = func(xVec, yVec);												\
		_mm_storel_epi64((__m128i*)&res, (__m128i)xVec);						\
		return res;																\
	}																			

#define OCL_INTR_P2_F3_F3_F1(func)												\
	__attribute__((overloadable)) float3 func(float3 x,float y)					\
	{																			\
		float4 res,valx,valy;													\
		valx.s012 = x;															\
		valy = _mm_set1_ps(y);													\
		res = func(valx,valy);													\
		return res.s012;														\
	}	

#define OCL_INTR_P2_F4_F4_F1(func)												\
	__attribute__((overloadable)) float4 func(float4 x,float y)					\
	{																			\
		float4 yVec;															\
		yVec = _mm_set1_ps(y);													\
		return func(x,yVec);													\
	}																			

#define OCL_INTR_P2_F8_F8_F1(func)												\
	__attribute__((overloadable)) float8 func(float8 x,float y)					\
	{																			\
		float4 yVec;															\
		float8 res;																\
		yVec = _mm_set1_ps(y);													\
		res.lo = func(x.lo, yVec);												\
		res.hi = func(x.hi, yVec);												\
		return res;																\
	}																			
	
#define OCL_INTR_P2_F8_F8_F1_AS_F8(func)										\
	__attribute__((overloadable)) float8 func(float8 x,float y)					\
	{																			\
		float8 yVec = _mm256_set1_ps(y);                    					\
		return func(x, yVec);													\
	}																			

#define OCL_INTR_P2_F16_F16_F1(func)											\
	__attribute__((overloadable)) float16 func(float16 x,float y)				\
	{																			\
		float4 yVec;															\
		float16 res;															\
		yVec = _mm_set1_ps(y);													\
		res.s0123 =  func(x.s0123, yVec);										\
		res.s4567 =  func(x.s4567, yVec);										\
		res.s89ab =  func(x.s89ab, yVec);										\
		res.scdef =  func(x.scdef, yVec);										\
		return res;																\
	}																			

#define OCL_INTR_P2_F16_F16_F1_AS_F8(func)  									\
	__attribute__((overloadable)) float16 func(float16 x,float y)				\
	{																			\
		float16 res;															\
        float8 yVec = _mm256_set1_ps(y);                    					\
		res.s01234567 =  func(x.s01234567, yVec);								\
		res.s89abcdef =  func(x.s89abcdef, yVec);								\
		return res;																\
	}																			

// doubles

#define OCL_INTR_P2_D2_D2_D1_AS_D4(func)										\
	__attribute__((overloadable)) double2 func(double2 x,double y)				\
	{																			\
		double4 xVec, yVec;														\
		double2 res;															\
		xVec = _mm_castsi128_ps(_mm_loadl_epi64((__m128i*)&x));					\
		yVec = _mm_set1_ps(y);													\
		xVec = func(xVec, yVec);												\
		_mm_storel_epi64((__m128i*)&res, (__m128i)xVec);						\
		return res;																\
	}																			

#define OCL_INTR_P2_D3_D3_D1(func)												\
	__attribute__((overloadable)) double3 func(double3 x,double y)				\
	{																			\
		double4 res,valx,valy;													\
		valx.s012 = x;															\
		valy = _mm_set1_ps(y);													\
		res = func(valx,valy);													\
		return res.s012;														\
	}	

#define OCL_INTR_P2_D4_D4_D1(func)												\
	__attribute__((overloadable)) double4 func(double4 x,double y)				\
	{																			\
		double4 yVec;															\
		yVec = _mm_set1_ps(y);													\
		return func(x,yVec);													\
	}																			

#define OCL_INTR_P2_D8_D8_D1(func)												\
	__attribute__((overloadable)) double8 func(double8 x,double y)				\
	{																			\
		double4 yVec;															\
		double8 res;															\
		yVec = _mm_set1_ps(y);													\
		res.lo = func(x.lo, yVec);												\
		res.hi = func(x.hi, yVec);												\
		return res;																\
	}																			
	
#define OCL_INTR_P2_D16_D16_D1(func)											\
	__attribute__((overloadable)) double16 func(double16 x,double y)			\
	{																			\
		double4 yVec;															\
		double16 res;															\
		yVec = _mm_set1_ps(y);													\
		res.s0123 =  func(x.s0123, yVec);										\
		res.s4567 =  func(x.s4567, yVec);										\
		res.s89ab =  func(x.s89ab, yVec);										\
		res.scdef =  func(x.scdef, yVec);										\
		return res;																\
	}			

//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
#define OCL_SVML_P1_F1_I1(func, sign)											\
	__attribute__((svmlcc)) float OCL_SVML_FUNCTION(_##func##f1)(_1##sign##32);							\
	__attribute__((overloadable)) float func(_1##sign##32 x)					\
	{																			\
		return OCL_SVML_FUNCTION(_##func##f1)(x);								\
	}																			

#define OCL_SVML_P1_F2_I2(func, sign)											\
	__attribute__((svmlcc)) float4 OCL_SVML_FUNCTION(_##func##f2)(_4##sign##32);						\
	__attribute__((overloadable)) float2 func(_2##sign##32 x)					\
	{																			\
		_4##sign##32 valy;														\
		valy = _mm_loadl_epi64((__m128i*)&x);									\
		float4 val = OCL_SVML_FUNCTION(_##func##f2)(valy);						\
		float2 res;																\
		_mm_storel_epi64((__m128i*)&res, (__m128i)val);							\
		return res;																\
	}																			

#define OCL_SVML_P1_F4_I4(func, sign)											\
	__attribute__((svmlcc)) float4 OCL_SVML_FUNCTION(_##func##f4)(_4##sign##32);						\
	__attribute__((overloadable)) float4 func(_4##sign##32 x)					\
	{																			\
		return OCL_SVML_FUNCTION(_##func##f4)(x);								\
	}																			

#define OCL_SVML_P1_F8_I8(func, sign)											\
	__attribute__((svmlcc)) float8 OCL_SVML_FUNCTION(_##func##f8)(_8##sign##32);						\
	__attribute__((overloadable)) float8 func(_8##sign##32 x)					\
	{																			\
		return OCL_SVML_FUNCTION(_##func##f8)(x);								\
	}																	

#define OCL_SVML_P1_F16_I16(func, sign)											\
	__attribute__((svmlcc)) float16 OCL_SVML_FUNCTION(_##func##f16)(_16##sign##32);						\
	__attribute__((overloadable)) float16 func(_16##sign##32 x)					\
	{																			\
		return OCL_SVML_FUNCTION(_##func##f16)(x);								\
	}																			

#define OCL_SVML_P1_D1_I1(func, sign)											\
	/*__attribute__((svmlcc))*/       double OCL_SVML_FUNCTION(_##func##1)(_1##sign##64);							\
	__attribute__((overloadable)) double func(_1##sign##64 x)					\
	{																			\
		return OCL_SVML_FUNCTION(_##func##1)(x);								\
	}																			

#define OCL_SVML_P1_D2_I2(func, sign)											\
	double2 OCL_SVML_FUNCTION(_##func##2)(_2##sign##64);						\
	__attribute__((overloadable)) double2 func(_2##sign##64 x)					\
	{																			\
		return OCL_SVML_FUNCTION(_##func##2)(x);								\
	}																			

#define OCL_SVML_P1_D3_I3(func, sign)											\
	double4 OCL_SVML_FUNCTION(_##func##4)(_4##sign##64);						\
	__attribute__((overloadable)) double3 func(_3##sign##64 x)					\
	{																			\
		double4 res;															\
		_4##sign##64 valx;														\
		valx.s012 = x;															\
		res = OCL_SVML_FUNCTION(_##func##4)(valx);								\
		return res.s012;														\
	}																			


#define OCL_SVML_P1_D4_I4(func, sign)											\
	double4 OCL_SVML_FUNCTION(_##func##4)(_4##sign##64);						\
	__attribute__((overloadable)) double4 func(_4##sign##64 x)					\
	{																			\
		return OCL_SVML_FUNCTION(_##func##4)(x);								\
	}																			

#define OCL_SVML_P1_D8_I8(func, sign)											\
	double8 OCL_SVML_FUNCTION(_##func##8)(_8##sign##64);						\
	__attribute__((overloadable)) double8 func(_8##sign##64 x)					\
	{																			\
		return OCL_SVML_FUNCTION(_##func##8)(x);								\
	}																			
	
#define OCL_SVML_P1_D16_I16(func, sign)											\
	double16 OCL_SVML_FUNCTION(_##func##16)(_16##sign##64);						\
	__attribute__((overloadable)) double16 func(_16##sign##64 x)				\
	{																			\
		return OCL_SVML_FUNCTION(_##func##16)(x);								\
	}																			


//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
#define OCL_INTR_P1_F1_I1(func, sign)											\
	__attribute__((overloadable)) float func(_1##sign##32 x);							

#define OCL_INTR_P1_F2_I2_AS_F1(func, sign)										\
	__attribute__((overloadable)) float2 func(_2##sign##32 x)					\
	{																			\
		float2 res;																\
		res.lo = func(x.lo);													\
		res.hi = func(x.hi);													\
		return res;																\
	}																			

#define OCL_INTR_P1_F3_I3(func, sign)											\
	__attribute__((overloadable)) float3 func(_3##sign##32 x)	\
	{																			\
		float4 res;																\
		_4##sign##32 valx;														\
		valx.s012 = x;															\
		res = func(valx);														\
		return res.s012;														\
	}	
#define OCL_INTR_P1_F4_I4(func, sign)											\
	__attribute__((overloadable)) float4 func(_4##sign##32 x);							

#define OCL_INTR_P1_F8_I8(func, sign)											\
	__attribute__((overloadable)) float8 func(_8##sign##32 x)					\
	{																			\
		float8 res;																\
		res.lo = func(x.lo);													\
		res.hi = func(x.hi);													\
		return res;																\
	}																			

#define OCL_INTR_P1_F8_I8_AS_F8(func, sign) 									\
	__attribute__((overloadable)) float8 func(_8##sign##32 x);

#define OCL_INTR_P1_F16_I16(func, sign)											\
	__attribute__((overloadable)) float16 func(_16##sign##32 x)					\
	{																			\
		float16 res;															\
		res.s0123 =  func(x.s0123);												\
		res.s4567 =  func(x.s4567);												\
		res.s89ab =  func(x.s89ab);												\
		res.scdef =  func(x.scdef);												\
		return res;																\
	}																			

#define OCL_INTR_P1_F16_I16_AS_F8(func, sign)									\
	__attribute__((overloadable)) float16 func(_16##sign##32 x)					\
	{																			\
		float16 res;															\
		res.s01234567 =  func(x.s01234567);										\
		res.s89abcdef =  func(x.s89abcdef);										\
		return res;																\
	}																			


//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
#define OCL_SVML_P2_F1_F1_F1(func,svmlfunc)										\
	__attribute__((svmlcc)) __attribute__((const)) float OCL_SVML_FUNCTION(_##svmlfunc##f1)(float, float);						\
	__attribute__((overloadable)) float func(float x,float y)					\
	{																			\
		return OCL_SVML_FUNCTION(_##svmlfunc##f1)(x,y);							\
	}																			

#define OCL_SVML_P2_F2_F2_F2(func,svmlfunc)										\
	__attribute__((svmlcc)) __attribute__((const)) float4 OCL_SVML_FUNCTION(_##svmlfunc##f2)(float4, float4);					\
	__attribute__((overloadable)) float2 func(float2 x,float2 y)				\
	{																			\
		float4 valx;															\
		valx = _mm_castsi128_ps(_mm_loadl_epi64((__m128i*)&x));					\
		float4 valy;															\
		valy = _mm_castsi128_ps(_mm_loadl_epi64((__m128i*)&y));					\
		float4 val = OCL_SVML_FUNCTION(_##svmlfunc##f2)(valx,valy);				\
		float2 res;																\
		_mm_storel_epi64((__m128i*)&res, (__m128i)val);							\
		return res;																\
	}																			

#define OCL_SVML_P2_F3_F3_F3(func,svmlfunc)										\
	__attribute__((svmlcc)) __attribute__((const)) float4 OCL_SVML_FUNCTION(_##svmlfunc##f4)(float4, float4);					\
	__attribute__((overloadable)) float3 func(float3 x, float3 y)				\
	{																			\
		float4 valx,valy;														\
		valx.s012 = x;															\
		valy.s012 = y;															\
		float4 res = OCL_SVML_FUNCTION(_##svmlfunc##f4)(valx,valy);				\
		return res.s012;														\
	}																			
#define OCL_SVML_P2_F4_F4_F4(func,svmlfunc)										\
	__attribute__((svmlcc)) __attribute__((const)) float4 OCL_SVML_FUNCTION(_##svmlfunc##f4)(float4, float4);					\
	__attribute__((overloadable)) float4 func(float4 x, float4 y)				\
	{																			\
		return OCL_SVML_FUNCTION(_##svmlfunc##f4)(x,y);							\
	}																			
	
#define OCL_SVML_P2_F8_F8_F8(func,svmlfunc)										\
	__attribute__((svmlcc_p2_f8_f8_f8)) __attribute__((const)) float8 OCL_SVML_FUNCTION(_##svmlfunc##f8)(float8, float8);					\
	__attribute__((overloadable)) float8 func(float8 x, float8 y)				\
	{																			\
		return OCL_SVML_FUNCTION(_##svmlfunc##f8)(x,y);							\
	}																			

#define OCL_SVML_P2_F16_F16_F16(func,svmlfunc)									\
	__attribute__((svmlcc_p2_f8_f8_f8)) __attribute__((const)) float8 OCL_SVML_FUNCTION(_##svmlfunc##f8)(float8, float8);				\
	__attribute__((overloadable)) float16 func(float16 x, float16 y)			\
	{																			\
		float16 res;                                                            \
        res.lo = OCL_SVML_FUNCTION(_##svmlfunc##f8)(x.lo,y.lo);				\
        res.hi = OCL_SVML_FUNCTION(_##svmlfunc##f8)(x.hi,y.hi);				\
        return res;                                                             \
	}																			

#define OCL_SVML_P2_D1_D1_D1(func,svmlfunc)										\
	__attribute__((svmlcc)) __attribute__((const)) double OCL_SVML_FUNCTION(_##svmlfunc##1)(double, double);					\
	__attribute__((overloadable)) double func(double x,double y)				\
	{																			\
		return OCL_SVML_FUNCTION(_##svmlfunc##1)(x,y);							\
	}																			

#define OCL_SVML_P2_D2_D2_D2(func,svmlfunc)										\
	__attribute__((svmlcc)) __attribute__((const)) double2 OCL_SVML_FUNCTION(_##svmlfunc##2)(double2, double2);				\
	__attribute__((overloadable)) double2 func(double2 x,double2 y)				\
	{																			\
		return OCL_SVML_FUNCTION(_##svmlfunc##2)(x,y);							\
	}																			

#define OCL_SVML_P2_D3_D3_D3(func,svmlfunc)										\
	__attribute__((svmlcc)) __attribute__((const)) double4 OCL_SVML_FUNCTION(_##svmlfunc##4)(double4, double4);				\
	__attribute__((overloadable)) double3 func(double3 x,double3 y)					\
	{																			\
		double4 valx, valy;														\
		valx.s012 = x; valy.s012 = y;											\
		double4 res = OCL_SVML_FUNCTION(_##svmlfunc##4)(valx,valy);				\
		return res.s012;														\
	}																			
#define OCL_SVML_P2_D4_D4_D4(func,svmlfunc)										\
	__attribute__((svmlcc)) __attribute__((const)) __attribute__((const)) double4 OCL_SVML_FUNCTION(_##svmlfunc##4)(double4, double4);				\
	__attribute__((overloadable)) double4 func(double4 x,double4 y)				\
	{																			\
		return OCL_SVML_FUNCTION(_##svmlfunc##4)(x,y);							\
	}																			

#define OCL_SVML_P2_D8_D8_D8(func,svmlfunc)										\
	__attribute__((svmlcc)) __attribute__((const)) __attribute__((const)) double4 OCL_SVML_FUNCTION(_##svmlfunc##4)(double4, double4);				\
	__attribute__((overloadable)) double8 func(double8 x,double8 y)				\
	{																			\
        double8 res;                                                            \
		res.lo = OCL_SVML_FUNCTION(_##svmlfunc##4)(x.lo,y.lo);					\
        res.hi = OCL_SVML_FUNCTION(_##svmlfunc##4)(x.hi,y.hi);					\
        return res;                                                             \
	}																			

#define OCL_SVML_P2_D16_D16_D16(func,svmlfunc)									\
	__attribute__((svmlcc)) __attribute__((const)) __attribute__((const)) double4 OCL_SVML_FUNCTION(_##svmlfunc##4)(double4, double4);			\
	__attribute__((overloadable)) double16 func(double16 x, double16 y)			\
	{																			\
		double16 res;															\
		res.s0123 = OCL_SVML_FUNCTION(_##svmlfunc##4)(x.s0123,y.s0123);			\
		res.s4567 = OCL_SVML_FUNCTION(_##svmlfunc##4)(x.s4567,y.s4567);		    \
		res.s89ab = OCL_SVML_FUNCTION(_##svmlfunc##4)(x.s89ab,y.s89ab);			\
		res.scdef = OCL_SVML_FUNCTION(_##svmlfunc##4)(x.scdef,y.scdef);			\
		return res;																\
	}																			


//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
#define OCL_INTR_P2_F1_F1_F1(func)													\
	__attribute__((overloadable)) float func(float x,float y);

#define OCL_INTR_P2_F1_F1_F1_AS_F4(func)											\
	__attribute__((overloadable)) float func(float x,float y)                       \
    {                                                                               \
        float4 xVec, yVec;                                                          \
        xVec.s0123 = x;                                                             \
        yVec.s0123 = y;                                                             \
        float4 res = func(xVec, yVec);                                              \
        return res.s0;                                                              \
    }

#define OCL_INTR_P2_F2_F2_F2_AS_F1(func)											\
	__attribute__((overloadable)) float2 func(float2 x, float2 y)					\
	{																				\
		float2 res;																	\
		res.lo = func(x.lo, y.lo);													\
		res.hi = func(x.hi, y.hi);													\
		return res;																	\
	}																				

#define OCL_INTR_P2_F2_F2_F2_AS_F4(func)											\
	__attribute__((overloadable)) float2 func(float2 x, float2 y)					\
	{																				\
		float4 xVec, yVec;															\
		xVec.s01 = x;																\
		yVec.s01 = y;																\
		float4 res = func(xVec,yVec);												\
		return res.s01;																\
	}																				


#define OCL_INTR_P2_F2_F2_F2_AS_F4_rami(func)											\
	__attribute__((overloadable)) float2 func(float2 x, float2 y)					\
	{																				\
		float4 xVec, yVec;															\
		float2 res;																	\
		xVec = _mm_castsi128_ps(_mm_loadl_epi64((__m128i*)&x));						\
		yVec = _mm_castsi128_ps(_mm_loadl_epi64((__m128i*)&y));						\
		xVec = func(xVec, yVec);													\
		_mm_storel_epi64((__m128i*)&res, (__m128i)xVec);							\
		return res;																	\
	}																				

#define OCL_INTR_P2_F3_F3_F3(func)													\
	__attribute__((overloadable)) float3 func(float3 x, float3 y)	\
	{																				\
		float4 res,valx,valy;														\
		valx.s012 = x;																\
		valy.s012 = y;																\
		res = func(valx, valy);														\
		return res.s012;															\
	}	

#define OCL_INTR_P2_F3_F3_F3_AS_F4(func)											\
	__attribute__((overloadable)) float3 func(float3 x, float3 y)	\
	{																				\
		float4 res,valx,valy;														\
		valx.s012 = x;																\
		valy.s012 = y;																\
		res = func(valx, valy);														\
		return res.s012;															\
	}	

#define OCL_INTR_P2_F3_F3_F3_AS_F1(func)											\
    __attribute__((overloadable)) float3 func(float3 x, float3 y)	\
    {																				\
        float3 res;																	\
        res.s0 = func(x.s0, y.s0);													\
        res.s1 = func(x.s1, y.s1);													\
        res.s2 = func(x.s2, y.s2);													\
        return res;																	\
    }

#define OCL_INTR_P2_F4_F4_F4(func)													\
	__attribute__((overloadable)) float4 func(float4 x, float4 y);							
	
#define OCL_INTR_P2_F8_F8_F8_AS_F4(func)													\
	__attribute__((overloadable)) float8 func(float8 x, float8 y)					\
	{																				\
		float8 res;																	\
		res.lo = func(x.lo, y.lo);													\
		res.hi = func(x.hi, y.hi);													\
		return res;																	\
	}																				

#define OCL_INTR_P2_F8_F8_F8_AS_F8(func)											\
	__attribute__((overloadable)) float8 func(float8 x, float8 y);

#define OCL_INTR_P2_F16_F16_F16_AS_F4(func)												\
	__attribute__((overloadable)) float16 func(float16 x, float16 y)				\
	{																				\
		float16 res;																\
		res.s0123 =  func(x.s0123, y.s0123);										\
		res.s4567 =  func(x.s4567, y.s4567);										\
		res.s89ab =  func(x.s89ab, y.s89ab);										\
		res.scdef =  func(x.scdef, y.scdef);										\
		return res;																	\
	}																				

#define OCL_INTR_P2_F16_F16_F16_AS_F8(func) 										\
	__attribute__((overloadable)) float16 func(float16 x, float16 y)				\
	{																				\
		float16 res;																\
		res.s01234567 =  func(x.s01234567, y.s01234567);							\
		res.s89abcdef =  func(x.s89abcdef, y.s89abcdef);							\
		return res;																	\
    }

// doubles
#define OCL_INTR_P2_D1_D1_D1(func)													\
	__attribute__((overloadable)) double func(double x,double y);

#define OCL_INTR_P2_D1_D1_D1_AS_D2(func)											\
	__attribute__((overloadable)) double func(double x,double y)                    \
    {                                                                               \
        double2 xVec, yVec;                                                         \
        xVec.s01 = x;                                                               \
        yVec.s01 = y;                                                               \
        double2 res = func(xVec, yVec);                                             \
        return res.s0;                                                              \
    }

#define OCL_INTR_P2_D2_D2_D2(func)													\
	 __attribute__((overloadable)) double2 func(double2 x,double2 y);

#define OCL_INTR_P2_D2_D2_D2_AS_D1(func)											\
	__attribute__((overloadable)) double2 func(double2 x, double2 y)				\
	{																				\
		double2 res;																\
		res.lo = func(x.lo, y.lo);													\
		res.hi = func(x.hi, y.hi);													\
		return res;																	\
	}																				

#define OCL_INTR_P2_D2_D2_D2_AS_D4(func)											\
	__attribute__((overloadable)) double2 func(double2 x, double2 y)				\
	{																				\
		double4 xVec, yVec;															\
		double4 res;																\
		xVec.s01 = x;																\
		yVec.s01 = y;																\
		res = func(xVec,yVec);														\
		return res.s01;																\
	}	

#define OCL_INTR_P2_D3_D3_D3_AS_D2(func)											\
	__attribute__((overloadable)) double3 func(double3 x, double3 y)				\
	{																				\
	double3 res;																	\
	res.s01 = func(x.s01,y.s01);													\
	res.s2 = func(x.s2,y.s2);														\
	return res;																		\
	}	

#define OCL_INTR_P2_D3_D3_D3_AS_D4(func)													\
	__attribute__((overloadable)) double3 func(double3 x, double3 y)				\
	{																				\
	double4 res,valx,valy;														\
	valx.s012 = x;																\
	valy.s012 = y;																\
	res = func(valx, valy);														\
	return res.s012;															\
	}

#define OCL_INTR_P2_D4_D4_D4(func)													\
	__attribute__((overloadable)) double4 func(double4 x, double4 y);							

#define OCL_INTR_P2_D4_D4_D4_AS_D2(func)													\
	 __attribute__((overloadable)) double4 func(double4 x, double4 y)	\
	{																				\
	double4 res;																	\
	res.lo = func(x.lo, y.lo);														\
	res.hi = func(x.hi, y.hi);														\
	return res;																		\
	}	

#define OCL_INTR_P2_D4_D4_D4_AS_D4(func)											\
	 __attribute__((overloadable)) double4 func(double4 x, double4 y);

#define OCL_INTR_P2_D8_D8_D8_AS_D2(func)													\
	 __attribute__((overloadable)) double8 func(double8 x, double8 y)	\
	{																				\
	double8 res;																	\
	res.s01 = func(x.s01, y.s01);													\
	res.s23 = func(x.s23, y.s23);													\
	res.s45 = func(x.s45, y.s45);													\
	res.s67 = func(x.s67, y.s67);													\
	return res;																		\
	}

#define OCL_INTR_P2_D8_D8_D8_AS_D4(func)											\
	__attribute__((overloadable)) double8 func(double8 x, double8 y)				\
	{																				\
		double8 res;																\
		res.lo = func(x.lo, y.lo);													\
		res.hi = func(x.hi, y.hi);													\
		return res;																	\
	}																				

#define OCL_INTR_P2_D16_D16_D16_AS_D2(func)													\
	 __attribute__((overloadable)) double16 func(double16 x, double16 y)	\
	{																				\
	double16 res;																	\
	res.s01 = func(x.s01, y.s01);													\
	res.s23 = func(x.s23, y.s23);													\
	res.s45 = func(x.s45, y.s45);													\
	res.s67 = func(x.s67, y.s67);													\
	res.s89 = func(x.s89, y.s89);													\
	res.sab = func(x.sab, y.sab);													\
	res.scd = func(x.scd, y.scd);													\
	res.sef = func(x.sef, y.sef);													\
	return res;																		\
	}	

#define OCL_INTR_P2_D16_D16_D16_AS_D4(func)											\
	 __attribute__((overloadable)) double16 func(double16 x, double16 y)	        \
	{																				\
        double16 res;																\
        res.s0123 = func(x.s0123, y.s0123); 										\
        res.s4567 = func(x.s4567, y.s4567); 										\
        res.s89ab = func(x.s89ab, y.s89ab);											\
        res.scdef = func(x.scdef, y.scdef);											\
        return res;																	\
    }

#define OCL_INTR_P2_D16_D16_D16_AS_D4(func)												\
	__attribute__((overloadable)) double16 func(double16 x, double16 y)				\
	{																				\
		double16 res;																\
		res.s0123 =  func(x.s0123, y.s0123);										\
		res.s4567 =  func(x.s4567, y.s4567);										\
		res.s89ab =  func(x.s89ab, y.s89ab);										\
		res.scdef =  func(x.scdef, y.scdef);										\
		return res;																	\
	}																				

//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
#define OCL_SVML_P2_F1_F1_I1(func, sign)											\
	__attribute__((svmlcc)) __attribute__((const)) float OCL_SVML_FUNCTION(_##func##f1)(float,_1##sign##32);						\
	__attribute__((overloadable)) float func(float x, _1##sign##32 y)				\
	{																				\
		float val = OCL_SVML_FUNCTION(_##func##f1)(x,y);							\
		return val;																	\
	}																

#define OCL_SVML_P2_D1_D1_I1(func, sign)											\
	__attribute__((svmlcc)) __attribute__((const)) double OCL_SVML_FUNCTION(_##func##1)(double ,_1##sign##32);						\
	__attribute__((overloadable)) double func(double x, _1##sign##32 y)				\
	{																				\
		double val = OCL_SVML_FUNCTION(_##func##1)(x,y);							\
		return val;																	\
	}																

#define OCL_SVML_P2_F2_F2_I2(func, sign)											\
	__attribute__((svmlcc)) __attribute__((const)) float4 OCL_SVML_FUNCTION(_##func##f2)(float4,_4##sign##32);						\
	__attribute__((overloadable)) float2 func(float2 x, _2##sign##32 y)				\
	{																				\
		float4 valx;																\
		valx = _mm_castsi128_ps(_mm_loadl_epi64((__m128i*)&x));						\
		_4i32 valy;																	\
		valy = (_4i32)_mm_loadl_epi64((__m128i*)&y);								\
		float4 val = OCL_SVML_FUNCTION(_##func##f2)(valx,valy);						\
		float2 res;																	\
		_mm_storel_epi64((__m128i*)&res, (__m128i)val);								\
		return res;																	\
	}																			

/*
	double2 OCL_SVML_FUNCTION(_pown2)(double2 ,_4i32);	
	double2 __pownd2(double2 x, _2i32 y)	
	{	
		_4i32 valy;
		valy = (_4i32)_mm_loadl_epi64((__m128i*)(&y));			
		double2 val = OCL_SVML_FUNCTION(_pown2)(x,valy);		
		return val;													
	}																
	

	double2 OCL_SVML_FUNCTION(_rootn2)(double2 ,_4i32);	
	double2 __rootnd2(double2 x, _2i32 y)	
	{	
		_4i32 valy;
		valy = (_4i32)_mm_loadl_epi64((__m128i*)(&y));			
		double2 val = OCL_SVML_FUNCTION(_rootn2)(x,valy);		
		return val;													
	}																
	

	double2 OCL_SVML_FUNCTION(_ldexp2)(double2 ,_4i32);	
	double2 __ldexpd2(double2 x, _2i32 y)	
	{	
		_4i32 valy;
		valy = (_4i32)_mm_loadl_epi64((__m128i*)(&y));			
		double2 val = OCL_SVML_FUNCTION(_ldexp2)(x,valy);		
		return val;													
	}																
	
*/
#define OCL_SVML_P2_F3_F3_I3(func, sign)									\
	__attribute__((svmlcc)) __attribute__((const)) float4 OCL_SVML_FUNCTION(_##func##f4)(float4,_4##sign##32);				\
	__attribute__((overloadable)) float3 func(float3 x, _3##sign##32 y)		\
	{																		\
		float4 valx;														\
		_4##sign##32 valy;													\
		valx.s012 = x;														\
		valy.s012 = y;														\
		float4 res = OCL_SVML_FUNCTION(_##func##f4)(valx,valy);				\
		return res.s012;													\
	}																
#define OCL_SVML_P2_F4_F4_I4(func, sign)									\
	__attribute__((svmlcc)) __attribute__((const)) float4 OCL_SVML_FUNCTION(_##func##f4)(float4,_4##sign##32);				\
	__attribute__((overloadable)) float4 func(float4 x, _4##sign##32 y)		\
	{																		\
		return OCL_SVML_FUNCTION(_##func##f4)(x,y);							\
	}																

#define OCL_SVML_P2_D2_D2_I2(func, sign)									\
	__attribute__((svmlcc)) __attribute__((const)) double2 OCL_SVML_FUNCTION(_##func##2)(double2 ,_4##sign##32);			\
	__attribute__((overloadable)) double2 func(double2 x, _2##sign##32 y)	\
	{																		\
		_4i32 valy;															\
		valy.s01 = y;														\
		double2 res = OCL_SVML_FUNCTION(_##func##2)(x,valy);				\
		return res;															\
	}																


#define OCL_SVML_P2_D3_D3_I3(func, sign)									\
	__attribute__((svmlcc)) __attribute__((const)) double4 OCL_SVML_FUNCTION(_##func##4)(double4 ,_4##sign##32);			\
	__attribute__((overloadable)) double3 func(double3 x, _3##sign##32 y)	\
	{																		\
		double4 valx;														\
		_4i32	valy;														\
		valx.s012 = x;														\
		valy.s012 = y;														\
		double4 res = OCL_SVML_FUNCTION(_##func##4)(valx,valy);				\
		return res.s012;													\
	}																

#define OCL_SVML_P2_D4_D4_I4(func, sign)									\
	__attribute__((svmlcc)) __attribute__((const)) double4 OCL_SVML_FUNCTION(_##func##4)(double4 ,_4##sign##32);			\
	__attribute__((overloadable)) double4 func(double4 x, _4##sign##32 y)	\
	{																		\
		double4 val = OCL_SVML_FUNCTION(_##func##4)(x,y);					\
		return val;															\
	}																

#define OCL_SVML_P2_F8_F8_I8(func, sign)									\
	__attribute__((svmlcc_p2_f8_f8_f8)) __attribute__((const)) float8 OCL_SVML_FUNCTION(_##func##f8)(float8,_8##sign##32);				\
	__attribute__((overloadable)) float8 func(float8 x, _8##sign##32 y)		\
	{																		\
		return OCL_SVML_FUNCTION(_##func##f8)(x,y);							\
	}																

#define OCL_SVML_P2_D8_D8_I8(func, sign)									\
	__attribute__((svmlcc)) __attribute__((const)) double4 OCL_SVML_FUNCTION(_##func##4)(double4,_4##sign##32);			\
	__attribute__((overloadable)) double8 func(double8 x, _8##sign##32 y)	\
	{																		\
        double8 res;                                                        \
        res.lo = OCL_SVML_FUNCTION(_##func##4)(x.lo,y.lo);                  \
        res.hi = OCL_SVML_FUNCTION(_##func##4)(x.hi,y.hi);                  \
		return res;                             							\
	}																

#define OCL_SVML_P2_F16_F16_I16(func, sign)									\
	__attribute__((svmlcc_p2_f8_f8_f8)) __attribute__((const)) float8 OCL_SVML_FUNCTION(_##func##f8)(float8,_8##sign##32);			\
	__attribute__((overloadable)) float16 func(float16 x, _16##sign##32 y)	\
	{																		\
        float16 res;                                                        \
		res.lo = OCL_SVML_FUNCTION(_##func##f8)(x.lo, y.lo);		        \
		res.hi = OCL_SVML_FUNCTION(_##func##f8)(x.hi, y.hi);		        \
		return res;                                 						\
	}																		

#define OCL_SVML_P2_D16_D16_I16(func, sign)									\
	__attribute__((svmlcc)) __attribute__((const)) double4 OCL_SVML_FUNCTION(_##func##4)(double4,_4i32);					\
	__attribute__((overloadable)) double16 func(double16 x, _16##sign##32 y)\
	{																		\
		double16 res;														\
		res.lo.lo = OCL_SVML_FUNCTION(_##func##4)(x.lo.lo, y.lo.lo);		\
		res.lo.hi = OCL_SVML_FUNCTION(_##func##4)(x.lo.hi, y.lo.hi);		\
		res.hi.lo = OCL_SVML_FUNCTION(_##func##4)(x.hi.lo, y.hi.lo);		\
		res.hi.hi = OCL_SVML_FUNCTION(_##func##4)(x.hi.hi, y.hi.hi);		\
		return res;															\
	}																					

//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
#define	OCL_SVML_P2_F2_F2_I(func, sign)				\
	__attribute__((overloadable)) float2 func(float2 x, _1##sign##32 y)	\
{																							\
	  int2 _y = (int2)(y, y);										\
	  return func( x,  _y);                     \
}
#define	OCL_SVML_P2_F3_F3_I(func, sign)				\
	__attribute__((overloadable)) float3 func(float3 x, _1##sign##32 y)	\
{																							\
	  int3 _y = (int3)(y, y, y);								\
	  return func( x,  _y);                     \
}
#define	OCL_SVML_P2_F4_F4_I(func, sign)				\
	__attribute__((overloadable)) float4 func(float4 x, _1##sign##32 y)	\
{																							\
	  int4 _y = (int4)(y, y, y, y);							\
	  return func( x,  _y);                     \
}
#define	OCL_SVML_P2_F8_F8_I(func, sign)				\
	__attribute__((overloadable)) float8 func(float8 x, _1##sign##32 y)	\
{																							\
	  int8 _y = (int8)(y, y, y, y, y, y, y, y);	\
	  return func( x,  _y);                     \
}
#define	OCL_SVML_P2_F16_F16_I(func, sign)				\
	__attribute__((overloadable)) float16 func(float16 x, _1##sign##32 y)	\
{																							\
	  int16 _y = (int16)(y, y, y, y, y, y, y, y, y, y, y, y, y, y, y, y);	\
	  return func( x,  _y);                     \
}
#define	OCL_SVML_P2_D2_D2_I(func,sign)					\
	__attribute__((overloadable)) double2 func(double2 x, _1##sign##32 y)	\
{																							\
	  int2 _y = (int2)(y, y);										\
	  return func( x,  _y);                     \
}
#define	OCL_SVML_P2_D3_D3_I(func,sign)					\
	__attribute__((overloadable)) double3 func(double3 x, _1##sign##32 y)	\
{																							\
	  int3 _y = (int3)(y, y, y);								\
	  return func( x,  _y);                     \
}
#define	OCL_SVML_P2_D4_D4_I(func,sign)					\
	__attribute__((overloadable)) double4 func(double4 x, _1##sign##32 y)	\
{																							\
	  int4 _y = (int4)(y, y, y, y);							\
	  return func( x,  _y);                     \
}
#define	OCL_SVML_P2_D8_D8_I(func,sign)					\
	__attribute__((overloadable)) double8 func(double8 x, _1##sign##32 y)	\
{																							\
	  int8 _y = (int8)(y, y, y, y, y, y, y, y);	\
	  return func( x,  _y);                     \
}
#define	OCL_SVML_P2_D16_D16_I(func,sign)			\
	__attribute__((overloadable)) double16 func(double16 x, _1##sign##32 y)	\
{																							\
	  int16 _y = (int16)(y, y, y, y, y, y, y, y, y, y, y, y, y, y, y, y);		\
	  return func( x,  _y);                     \
}

//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
#define OCL_SVML_P1_I1_F1(func)												\
	__attribute__((svmlcc)) _1i32 OCL_SVML_FUNCTION(_##func##f1)(float);							\
	 __attribute__((overloadable)) _1i32 func(float x)						\
	{																		\
		return OCL_SVML_FUNCTION(_##func##f1)(x);							\
	}																			

#define OCL_SVML_P1_I2_F2(func)												\
	__attribute__((svmlcc)) _4i32 OCL_SVML_FUNCTION(_##func##f2)(float4);							\
	 __attribute__((overloadable)) _2i32 func(float2 x)						\
	{																		\
		float4 valx;														\
		valx = _mm_castsi128_ps(_mm_loadl_epi64((__m128i*)&x));				\
		_4i32 ret = OCL_SVML_FUNCTION(_##func##f2)(valx);					\
		_2i32 res;															\
		_mm_storel_epi64((__m128i*)&res, ret);								\
		return res;															\
	}																			

#define OCL_SVML_P1_I3_F3(func)												\
	__attribute__((svmlcc)) _4i32 OCL_SVML_FUNCTION(_##func##f4)(float4);							\
	__attribute__((overloadable)) _3i32 func(float3 x)						\
	{																		\
		float4 valx;														\
		valx.s012 = x;														\
		_4i32 ret = OCL_SVML_FUNCTION(_##func##f4)(valx);					\
		return ret.s012;													\
	}

#define OCL_SVML_P1_I4_F4(func)												\
	__attribute__((svmlcc)) _4i32 OCL_SVML_FUNCTION(_##func##f4)(float4);							\
	__attribute__((overloadable)) _4i32 func(float4 x)						\
	{																		\
		_4i32 ret = OCL_SVML_FUNCTION(_##func##f4)(x);						\
		return ret;															\
	}																			

#define OCL_SVML_P1_I8_F8(func)												\
	__attribute__((svmlcc)) _8i32 OCL_SVML_FUNCTION(_##func##f8)(float8);							\
	 __attribute__((overloadable)) _8i32 func(float8 x)						\
	{																		\
		_8i32 ret =  OCL_SVML_FUNCTION(_##func##f8)(x);						\
		return ret;															\
	}																			

#define OCL_SVML_P1_I16_F16(func)											\
	__attribute__((svmlcc)) _16i32 OCL_SVML_FUNCTION(_##func##f16)(float16);						\
	__attribute__((overloadable)) _16i32 func(float16 x)					\
	{																		\
		_16i32 ret =  OCL_SVML_FUNCTION(_##func##f16)(x);					\
		return ret;															\
	}																		


//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
#define OCL_INTR_P1_I1_F1(func)												\
	 __attribute__((overloadable)) _1i32 func(float x);									

#define OCL_INTR_P1_I2_F2_AS_F4(func)										\
	 __attribute__((overloadable)) _2i32 func(float2 x)						\
	{																		\
		float4 xVec;														\
		xVec = _mm_castsi128_ps(_mm_loadl_epi64((__m128i*)&x));				\
		_4i32 ret = func(xVec);												\
		_2i32 res;															\
		_mm_storel_epi64((__m128i*)&res, (__m128i)ret);						\
		return res;															\
	}																			
							
#define OCL_INTR_P1_I3_F3(func)													\
	 __attribute__((overloadable)) _3i32 func(float3 x)			\
	{																			\
		_4i32 res;																\
		float4 valx; valx.s012 = x;												\
		res = func(valx);														\
		return res.s012;														\
	}	
#define OCL_INTR_P1_I4_F4(func)												\
	__attribute__((overloadable)) _4i32 func(float4 x);								

#define OCL_INTR_P1_I8_F8(func)												\
	 __attribute__((overloadable)) _8i32 func(float8 x)						\
	{																		\
		_8i32 res;															\
		res.lo = func(x.lo);												\
		res.hi = func(x.hi);												\
		return res;															\
	}																			

#define OCL_INTR_P1_I16_F16(func)											\
	__attribute__((overloadable)) _16i32 func(float16 x)					\
	{																		\
		_16i32 res;															\
		res.s0123 =  func(x.s0123);											\
		res.s4567 =  func(x.s4567);											\
		res.s89ab =  func(x.s89ab);											\
		res.scdef =  func(x.scdef);											\
		return res;															\
	}

#define OCL_INTR_P1_I8_F8_AS_F8(func)										\
	 __attribute__((overloadable)) _8i32 func(float8 x);

#define OCL_INTR_P1_I16_F16_AS_F8(func)										\
	__attribute__((overloadable)) _16i32 func(float16 x)					\
	{																		\
		_16i32 res;															\
		res.s01234567 =  func(x.s01234567); 								\
		res.s89abcdef =  func(x.s89abcdef);									\
		return res;															\
	}

#define OCL_SVML_P1_I1_D1(func)												\
	__attribute__((svmlcc)) __attribute__((const)) _1i32 OCL_SVML_FUNCTION(_##func##1)(double);							\
	 __attribute__((overloadable)) _1i32 func(double x)						\
	{																		\
		return OCL_SVML_FUNCTION(_##func##1)(x);							\
	}																			

#define OCL_SVML_P1_I2_D2(func)												\
	__attribute__((svmlcc)) __attribute__((const)) _4i32 OCL_SVML_FUNCTION(_##func##2)(double2);							\
	 __attribute__((overloadable)) _2i32 func(double2 x)					\
	{																		\
		_4i32 res;															\
		res = OCL_SVML_FUNCTION(_##func##2)(x);								\
		return res.s01;														\
	}																			

#define OCL_SVML_P1_I3_D3(func)												\
	__attribute__((svmlcc)) __attribute__((const)) _4i32 OCL_SVML_FUNCTION(_##func##4)(double4);							\
	__attribute__((overloadable)) _3i32 func(double3 x)						\
	{																		\
		double4 valx; valx.s012 = x;										\
		_4i32 ret = OCL_SVML_FUNCTION(_##func##4)(valx);					\
		return ret.s012;													\
	}																			


#define OCL_SVML_P1_I4_D4(func)												\
	__attribute__((svmlcc)) __attribute__((const)) _4i32 OCL_SVML_FUNCTION(_##func##4)(double4);							\
	__attribute__((overloadable)) _4i32 func(double4 x)						\
	{																		\
		_4i32 ret = OCL_SVML_FUNCTION(_##func##4)(x);						\
		return ret;															\
	}																			

#define OCL_SVML_P1_I8_D8(func)												\
	__attribute__((svmlcc)) __attribute__((const)) _8i32 OCL_SVML_FUNCTION(_##func##8)(double8);							\
	 __attribute__((overloadable)) _8i32 func(double8 x)					\
	{																		\
		_8i32 ret =  OCL_SVML_FUNCTION(_##func##8)(x);						\
		return ret;															\
	}																			

#define OCL_SVML_P1_I16_D16(func)											\
	__attribute__((svmlcc)) __attribute__((const)) _8i32 OCL_SVML_FUNCTION(_##func##8)(double8);						\
	__attribute__((overloadable)) _16i32 func(double16 x)					\
	{																		\
		_16i32 ret;															\
		ret.lo =  OCL_SVML_FUNCTION(_##func##8)(x.lo);						\
		ret.hi =  OCL_SVML_FUNCTION(_##func##8)(x.hi);						\
		return ret;															\
	}																		


//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
#define OCL_SVML_P2_F1_F1_PI1(func,svmlfunc)								\
	__attribute__((svmlcc)) __attribute__((const)) float OCL_SVML_FUNCTION(_##svmlfunc##f1)(float,_1i32*);					\
	__attribute__((overloadable)) float func(float x, _1i32* z)				\
	{																		\
		return OCL_SVML_FUNCTION(_##svmlfunc##f1)(x,z);						\
	}																				
	
#define OCL_SVML_P2_F1_F1_PI1_LOCAL(func)									\
	 __attribute__((overloadable)) float func(float x, __local _1i32* z)	\
	{																		\
		return func(x, (_1i32*)z);											\
	}																				

#define OCL_SVML_P2_F1_F1_PI1_GLOBAL(func)									\
	__attribute__((overloadable)) float func(float x, __global _1i32* z)	\
	{																		\
		return func(x,(_1i32*)z);											\
	}																				

#define OCL_SVML_P2_D1_D1_PI1(func,svmlfunc)								\
	__attribute__((svmlcc)) __attribute__((const)) double OCL_SVML_FUNCTION(_##svmlfunc##1)(double,_1i32*);				\
	__attribute__((overloadable)) double func(double x, _1i32* z)			\
	{																		\
		return OCL_SVML_FUNCTION(_##svmlfunc##1)(x,z);						\
	}																				
	
#define OCL_SVML_P2_D1_D1_PI1_LOCAL(func)									\
	 __attribute__((overloadable)) double func(double x, __local _1i32* z)	\
	{																		\
		return func(x, (_1i32*)z);											\
	}																				

#define OCL_SVML_P2_D1_D1_PI1_GLOBAL(func)									\
	__attribute__((overloadable)) double func(double x, __global _1i32* z)	\
	{																		\
		return func(x,(_1i32*)z);											\
	}																				

// float2
#define	OCL_SVML_P2_F2_F2_PI2(func,svmlfunc)								\
    __attribute__((svmlcc)) __attribute__((const)) float4 OCL_SVML_FUNCTION(_##svmlfunc##f2)(float4, _4i32*);				\
    __attribute__((overloadable)) float2 func(float2 x, _2i32* z)			\
    {																		\
        float4 valx;														\
        valx = _mm_castsi128_ps(_mm_loadl_epi64((__m128i*)&x));				\
        _4i32 valz;															\
        valx = OCL_SVML_FUNCTION(_##svmlfunc##f2)(valx, &valz);				\
        float2 res;															\
        _mm_storel_epi64((__m128i*)&res, (__m128i)valx);					\
        _mm_storel_epi64((__m128i*)z, (__m128i)valz);						\
        return res;															\
    }

#define	OCL_SVML_P2_F2_F2_PI2_LOCAL(func,svmlfunc)							\
    __attribute__((overloadable)) float2 func(float2 x, __local _2i32* z)	\
    {																		\
        return func(x,(_2i32*)z);											\
    }

#define	OCL_SVML_P2_F2_F2_PI2_GLOBAL(func,svmlfunc)							\
    __attribute__((overloadable)) float2 func(float2 x, __global _2i32* z)	\
    {																		\
		return func(x,(_2i32*)z);											\
    }
	

// float3
#define OCL_SVML_P2_F3_F3_PI3(func,svmlfunc)									\
	__attribute__((svmlcc)) float4 OCL_SVML_FUNCTION(_##svmlfunc##f4)(float4,_4i32*);					\
	__attribute__((overloadable)) float3 func(float3 x, _3i32* z)				\
	{																			\
		float4 valx;															\
		valx.s012 = x;															\
		_4i32 valz;																\
		float4 res = OCL_SVML_FUNCTION(_##svmlfunc##f4)(valx,&valz);			\
		*z = valz.s012;															\
		return res.s012;														\
	}																			

#define OCL_SVML_P2_F3_F3_PI3_LOCAL(func,svmlfunc)								\
	__attribute__((overloadable)) float3 func(float3 x, __local _3i32* z)		\
	{																			\
		return func(x, (_3i32*)z);												\
	}																				

#define OCL_SVML_P2_F3_F3_PI3_GLOBAL(func,svmlfunc)								\
	__attribute__((overloadable)) float3 func(float3 x, __global _3i32* z)		\
	{																			\
		return func(x, (_3i32*)z);												\
	}				

#define OCL_SVML_P2_F4_F4_PI4(func,svmlfunc)									\
	__attribute__((svmlcc)) float4 OCL_SVML_FUNCTION(_##svmlfunc##f4)(float4,_4i32*);					\
	__attribute__((overloadable)) float4 func(float4 x, _4i32* z)				\
	{																			\
		float4 x_tmp;															\
		_mm_store_si128 ((__m128i*)&x_tmp, (__m128i)x);							\
		x_tmp = OCL_SVML_FUNCTION(_##svmlfunc##f4)(x_tmp,z);					\
		float4 ret;																\
		_mm_store_si128 ((__m128i*)&ret, (__m128i)x_tmp);						\
		return ret;																\
	}																			

#define OCL_SVML_P2_F4_F4_PI4_LOCAL(func,svmlfunc)								\
	__attribute__((overloadable)) float4 func(float4 x, __local _4i32* z)		\
	{																			\
		return OCL_SVML_FUNCTION(_##svmlfunc##f4)(x, (_4i32*)z);				\
	}																				

#define OCL_SVML_P2_F4_F4_PI4_GLOBAL(func,svmlfunc)								\
	__attribute__((overloadable)) float4 func(float4 x, __global _4i32* z)		\
	{																			\
		return OCL_SVML_FUNCTION(_##svmlfunc##f4)(x, (_4i32*)z);				\
	}																				


#define OCL_SVML_P2_D2_D2_PI2(func,svmlfunc)									\
	__attribute__((svmlcc)) double2 OCL_SVML_FUNCTION(_##svmlfunc##2)(double2,_4i32*);					\
	__attribute__((overloadable)) double2 func(double2 x, _2i32* z)				\
	{																			\
		_4i32 valz;																\
		double2 res = OCL_SVML_FUNCTION(_##svmlfunc##2)(x,&valz);				\
		*z = valz.s01;															\
		return res;																\
	}																				

#define OCL_SVML_P2_D2_D2_PI2_LOCAL(func)										\
	 __attribute__((overloadable)) double2 func(double2 x, __local _2i32* z)	\
	{																			\
		return func(x, (_2i32*)z);												\
	}																				

#define OCL_SVML_P2_D2_D2_PI2_GLOBAL(func)										\
	__attribute__((overloadable)) double2 func(double2 x, __global _2i32* z)	\
	{																			\
		return func(x,(_2i32*)z);												\
	}																				


#define OCL_SVML_P2_D3_D3_PI3(func,svmlfunc)									\
	__attribute__((svmlcc)) double4 OCL_SVML_FUNCTION(_##svmlfunc##4)(double4,_4i32*);					\
	__attribute__((overloadable)) double3 func(double3 x, _3i32* z)				\
	{																			\
		_4i32 valz;																\
		double4 res,valx;														\
		valx.s012 = x;															\
		res = OCL_SVML_FUNCTION(_##svmlfunc##4)(valx,&valz);					\
		*z = valz.s012;															\
		return res.s012;														\
	}																				

#define OCL_SVML_P2_D3_D3_PI3_LOCAL(func)										\
	 __attribute__((overloadable)) double3 func(double3 x, __local _3i32* z)	\
	{																			\
		return func(x, (_3i32*)z);												\
	}																				

#define OCL_SVML_P2_D3_D3_PI3_GLOBAL(func)										\
	__attribute__((overloadable)) double3 func(double3 x, __global _3i32* z)	\
	{																			\
		return func(x,(_3i32*)z);												\
	}																				


#define OCL_SVML_P2_D4_D4_PI4(func,svmlfunc)									\
	__attribute__((svmlcc)) double4 OCL_SVML_FUNCTION(_##svmlfunc##4)(double4,_4i32*);					\
	__attribute__((overloadable)) double4 func(double4 x, _4i32* z)				\
	{																			\
		_4i32 valz;																\
		double4  ret = OCL_SVML_FUNCTION(_##svmlfunc##4)(x,&valz);				\
		_mm_store_si128 ((__m128i*)z, (__m128i)valz);							\
		return ret;																\
	}																				

#define OCL_SVML_P2_D4_D4_PI4_LOCAL(func)										\
	 __attribute__((overloadable)) double4 func(double4 x, __local _4i32* z)	\
	{																			\
		return func(x, (_4i32*)z);												\
	}																				

#define OCL_SVML_P2_D4_D4_PI4_GLOBAL(func)										\
	__attribute__((overloadable)) double4 func(double4 x, __global _4i32* z)	\
	{																			\
		return func(x,(_4i32*)z);												\
	}																				

// float8
#define OCL_SVML_P2_F8_F8_PI8(func,svmlfunc)									\
	__attribute__((svmlcc)) float4 OCL_SVML_FUNCTION(_##svmlfunc##f4)(float4,_4i32*);					\
	__attribute__((overloadable)) float8 func(float8 x, _8i32* z)				\
	{																			\
        float8 res;                                                             \
        _4i32 z1,z2;                                                            \
        res.lo = OCL_SVML_FUNCTION(_##svmlfunc##f4)(x.lo, &z1);                  \
        res.hi = OCL_SVML_FUNCTION(_##svmlfunc##f4)(x.hi, &z2);                  \
        z->lo = z1;                                                             \
        z->hi = z2;                                                             \
		return res;                                     						\
	}																				

#define OCL_SVML_P2_F8_F8_PI8_LOCAL(func,svmlfunc)								\
	__attribute__((overloadable)) float8 func(float8 x, __local _8i32* z)		\
	{																			\
		return func(x, (_8i32*)z);												\
	}																				

#define OCL_SVML_P2_F8_F8_PI8_GLOBAL(func,svmlfunc)								\
	__attribute__((overloadable)) float8 func(float8 x, __global _8i32* z)		\
	{																			\
		return func(x,(_8i32*)z);												\
	}																				

#define OCL_SVML_P2_D8_D8_PI8(func,svmlfunc)									\
	__attribute__((svmlcc)) double4 OCL_SVML_FUNCTION(_##svmlfunc##4)(double4,_4i32*);					\
	__attribute__((overloadable)) double8 func(double8 x, _8i32* z)				\
	{																			\
        double8 res;                                                            \
        _4i32 z1, z2;                                                           \
        res.lo = OCL_SVML_FUNCTION(_##svmlfunc##4)(x.lo,&z1);					\
		res.hi = OCL_SVML_FUNCTION(_##svmlfunc##4)(x.hi,&z2);                   \
        z->lo = z1;                                                             \
        z->hi = z2;                                                             \
        return res;                                         					\
	}																				
	
#define OCL_SVML_P2_D8_D8_PI8_LOCAL(func)										\
	 __attribute__((overloadable)) double8 func(double8 x, __local _8i32* z)	\
	{																			\
		return func(x, (_8i32*)z);												\
	}																				

#define OCL_SVML_P2_D8_D8_PI8_GLOBAL(func)										\
	__attribute__((overloadable)) double8 func(double8 x, __global _8i32* z)	\
	{																			\
		return func(x,(_8i32*)z);												\
	}																				


// float16
#define OCL_SVML_P2_F16_F16_PI16(func,svmlfunc)									\
	__attribute__((svmlcc)) float4 OCL_SVML_FUNCTION(_##svmlfunc##f4)(float4,_4i32*);				\
	__attribute__((overloadable)) float16 func(float16 x, _16i32* z)			\
	{																			\
        float16 res;                                                            \
        _4i32 z1, z2, z3, z4;                                                   \
        res.s0123 = OCL_SVML_FUNCTION(_##svmlfunc##f4)(x.s0123,&z1);			\
        res.s4567 = OCL_SVML_FUNCTION(_##svmlfunc##f4)(x.s4567,&z2);			\
        res.s89ab = OCL_SVML_FUNCTION(_##svmlfunc##f4)(x.s89ab,&z3);			\
        res.scdef = OCL_SVML_FUNCTION(_##svmlfunc##f4)(x.scdef,&z4);			\
        z->lo.lo = z1;                                                          \
        z->lo.hi = z2;                                                          \
        z->hi.lo = z3;                                                          \
        z->hi.hi = z4;                                                          \
		return res;                                     						\
	}																					

#define OCL_SVML_P2_F16_F16_PI16_LOCAL(func,svmlfunc)							\
	__attribute__((overloadable)) float16 func(float16 x, __local _16i32* z)	\
	{																			\
		return func(x, (_16i32*)z);												\
	}																				

#define OCL_SVML_P2_F16_F16_PI16_GLOBAL(func,svmlfunc)							\
	__attribute__((overloadable)) float16 func(float16 x, __global _16i32* z)	\
	{																			\
		return func(x, (_16i32*)z);												\
	}																				

#define OCL_SVML_P2_D16_D16_PI16(func,svmlfunc)									\
	__attribute__((svmlcc)) double4 OCL_SVML_FUNCTION(_##svmlfunc##4)(double4,_4i32*);				\
	__attribute__((overloadable)) double16 func(double16 x, _16i32* z)			\
	{																			\
		double16 res;															\
		_4i32 z1,z2,z3,z4;														\
		res.s0123 = OCL_SVML_FUNCTION(_##svmlfunc##4)(x.s0123,&z1);            	\
		res.s4567 = OCL_SVML_FUNCTION(_##svmlfunc##4)(x.s4567,&z2);				\
		res.s89ab = OCL_SVML_FUNCTION(_##svmlfunc##4)(x.s89ab,&z3);				\
		res.scdef = OCL_SVML_FUNCTION(_##svmlfunc##4)(x.scdef,&z4);				\
		z->lo.lo = z1;																\
		z->lo.hi = z2;																\
		z->hi.lo = z3;																\
		z->hi.hi = z4;																\
		return res;																\
	}																				
	
#define OCL_SVML_P2_D16_D16_PI16_LOCAL(func)									\
	 __attribute__((overloadable)) double16 func(double16 x, __local _16i32* z)	\
	{																			\
		return func(x, (_16i32*)z);												\
	}																				

#define OCL_SVML_P2_D16_D16_PI16_GLOBAL(func)									\
	__attribute__((overloadable)) double16 func(double16 x, __global _16i32* z)	\
	{																			\
		return func(x,(_16i32*)z);												\
	}																				


//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
#define OCL_SVML_P2_F1_F1_pF1(func)												\
	__attribute__((svmlcc)) float OCL_SVML_FUNCTION(_##func##f1)(float, float*);						\
	__attribute__((overloadable)) float func(float x,float* y)					\
	{																			\
		return OCL_SVML_FUNCTION(_##func##f1)(x, y);							\
	}																			

#define OCL_SVML_P2_F1_F1_pF1_GLOBAL(func)										\
	__attribute__((overloadable)) float func(float x,__global float* y)			\
	{																			\
		return func(x, (float*)y);												\
	}																			

#define OCL_SVML_P2_F1_F1_pF1_LOCAL(func)										\
	__attribute__((overloadable)) float func(float x,__local float* y)			\
	{																			\
		return func(x,(float*)y);												\
	}																			

// float2
#define OCL_SVML_P2_F2_F2_pF2(func)												\
	__attribute__((svmlcc)) float4 OCL_SVML_FUNCTION(_##func##f2)(float4, float4*);						\
	__attribute__((overloadable)) float2 func(float2 x, float2* y)				\
	{																			\
		float4 xVec, yVec;														\
		xVec = _mm_castsi128_ps(_mm_loadl_epi64((__m128i*)&x));					\
		float2 res;																\
		xVec = OCL_SVML_FUNCTION(_##func##f2)(xVec, &yVec);						\
		_mm_storel_epi64((__m128i*)y, (__m128i)yVec);							\
		_mm_storel_epi64((__m128i*)&res, (__m128i)xVec);						\
		return res;																\
	}																			

#define OCL_SVML_P2_F2_F2_pF2_LOCAL(func)										\
	__attribute__((overloadable)) float2 func(float2 x,__local float2* y)		\
	{																			\
		return func(x, (float2*)y);												\
	}																			

#define OCL_SVML_P2_F2_F2_pF2_GLOBAL(func)										\
	__attribute__((overloadable)) float2 func(float2 x,__global float2* y)		\
	{																			\
		return func(x, (float2*)y);												\
	}																			

//float3
#define OCL_SVML_P2_F3_F3_pF3(func)												\
	__attribute__((svmlcc)) float4 OCL_SVML_FUNCTION(_##func##f4)(float4, float4*);						\
	__attribute__((overloadable)) float3 func(float3 x, float3* y)				\
	{																			\
		float4 valx,valy;														\
		valx.s012 = x;															\
		float4 res = OCL_SVML_FUNCTION(_##func##f4)(valx, &valy);				\
		*y = valy.s012;															\
		return res.s012;														\
	}																			

#define OCL_SVML_P2_F3_F3_pF3_LOCAL(func)										\
	__attribute__((overloadable)) float3 func(float3 x,__local float3* y)		\
	{																			\
		return func(x, (float3*)y);												\
	}																			

#define OCL_SVML_P2_F3_F3_pF3_GLOBAL(func)										\
	__attribute__((overloadable)) float3 func(float3 x,__global float3* y)		\
	{																			\
		return func(x, (float3*)y);												\
	}						

#define OCL_SVML_P2_F4_F4_pF4(func)												\
	__attribute__((svmlcc)) float4 OCL_SVML_FUNCTION(_##func##f4)(float4, float4*);						\
	__attribute__((overloadable)) float4 func(float4 x, float4* y)				\
	{																			\
		return OCL_SVML_FUNCTION(_##func##f4)(x, y);							\
	}																			

#define OCL_SVML_P2_F4_F4_pF4_LOCAL(func)										\
	__attribute__((overloadable)) float4 func(float4 x,__local float4* y)		\
	{																			\
		return func(x, (float4*)y);												\
	}																			

#define OCL_SVML_P2_F4_F4_pF4_GLOBAL(func)										\
	__attribute__((overloadable)) float4 func(float4 x,__global float4* y)		\
	{																			\
		return func(x, (float4*)y);												\
	}																			

//float8
#define OCL_SVML_P2_F8_F8_pF8(func)												\
	__attribute__((svmlcc)) float4 OCL_SVML_FUNCTION(_##func##f4)(float4, float4*);						\
	__attribute__((overloadable)) float8 func(float8 x, float8* y)				\
	{																			\
        float8 res;                                                             \
        float4 y1, y2;                                                          \
        res.lo = OCL_SVML_FUNCTION(_##func##f4)(x.lo, &y1);                     \
        res.hi = OCL_SVML_FUNCTION(_##func##f4)(x.hi, &y2);                     \
        y->lo = y1;                                                             \
        y->hi = y2;                                                             \
		return res;							                                    \
	}																			

#define OCL_SVML_P2_F8_F8_pF8_LOCAL(func)										\
	__attribute__((overloadable)) float8 func(float8 x,__local float8* y)		\
	{																			\
		return func(x,(float8*)y);												\
	}																			

#define OCL_SVML_P2_F8_F8_pF8_GLOBAL(func)										\
	__attribute__((overloadable)) float8 func(float8 x,__global float8* y)		\
	{																			\
		return func(x,(float8*)y);												\
	}																			

//float16
#define OCL_SVML_P2_F16_F16_pF16(func)											\
	__attribute__((svmlcc)) float4 OCL_SVML_FUNCTION(_##func##f4)(float4, float4*);					\
	__attribute__((overloadable)) float16 func(float16 x, float16* y)			\
	{																			\
        float16 res;                                                            \
        float4 y1,y2,y3,y4;                                                     \
        res.s0123 = OCL_SVML_FUNCTION(_##func##f4)(x.s0123, &y1);               \
        res.s4567 = OCL_SVML_FUNCTION(_##func##f4)(x.s4567, &y2);               \
        res.s89ab = OCL_SVML_FUNCTION(_##func##f4)(x.s89ab, &y3);               \
        res.scdef = OCL_SVML_FUNCTION(_##func##f4)(x.scdef, &y4);               \
        y->lo.lo = y1;                                                             \
        y->lo.hi = y2;                                                             \
        y->hi.lo = y3;                                                             \
        y->hi.hi = y4;                                                             \
		return res;							                                    \
	}																			

#define OCL_SVML_P2_F16_F16_pF16_LOCAL(func)									\
	__attribute__((overloadable)) float16 func(float16 x,__local float16* y)	\
	{																			\
		return func(x,(float16*)y);												\
	}																			

#define OCL_SVML_P2_F16_F16_pF16_GLOBAL(func)									\
	__attribute__((overloadable)) float16 func(float16 x,__global float16* y)	\
	{																			\
		return func(x,(float16*)y);												\
	}																			


//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
#define OCL_INTR_P2_F1_F1_pF1(func)												\
	__attribute__((overloadable)) float func(float x,float* y);							

#define OCL_INTR_P2_F1_F1_pF1_GLOBAL(func)										\
	__attribute__((overloadable)) float func(float x,__global float* y)			\
	{																			\
		return func(x, (float*)y);												\
	}																			

#define OCL_INTR_P2_F1_F1_pF1_LOCAL(func)										\
	__attribute__((overloadable)) float func(float x,__local float* y)			\
	{																			\
		return func(x,(float*)y);												\
	}																			

#define OCL_SVML_P2_D1_D1_pD1(func)												\
	__attribute__((svmlcc)) double OCL_SVML_FUNCTION(_##func##1)(double,double*);						\
	__attribute__((overloadable)) double func(double x,double* y)				\
	{																			\
		double ret =  OCL_SVML_FUNCTION(_##func##1)(x,y);						\
		return ret;																\
	}																	

#define OCL_SVML_P2_D1_D1_pD1_GLOBAL(func)										\
	__attribute__((overloadable)) double func(double x,__global double* y)		\
	{																			\
		return func(x, (double*)y);												\
	}																			

#define OCL_SVML_P2_D1_D1_pD1_LOCAL(func)										\
	__attribute__((overloadable)) double func(double x,__local double* y)		\
	{																			\
		return func(x,(double*)y);												\
	}																			


// float2
#define OCL_INTR_P2_F2_F2_pF2(func)												\
	__attribute__((overloadable)) float2 func(float2 x, float2* y)				\
	{																			\
		float4 xVec, yVec;														\
		float2 res;																\
		xVec = _mm_castsi128_ps(_mm_loadl_epi64((__m128i*)&x));					\
		xVec = func(xVec, &yVec);												\
		_mm_storel_epi64((__m128i*)&res, (__m128i)xVec);						\
		_mm_storel_epi64((__m128i*)y, (__m128i)yVec);							\
		return res;																\
	}																			

#define OCL_INTR_P2_F2_F2_pF2_LOCAL(func)										\
	__attribute__((overloadable)) float2 func(float2 x,__local float2* y)		\
	{																			\
		return func(x, (float2*)y);												\
	}																			

#define OCL_INTR_P2_F2_F2_pF2_GLOBAL(func)										\
	__attribute__((overloadable)) float2 func(float2 x,__global float2* y)		\
	{																			\
		return func(x, (float2*)y);												\
	}																			


// double2
#define OCL_SVML_P2_D2_D2_pD2(func)										\
	__attribute__((svmlcc)) double2 OCL_SVML_FUNCTION(_##func##2)(double2,double2*);			\
	__attribute__((overloadable)) double2 func(double2 x,double2* y)	\
	{																	\
		return OCL_SVML_FUNCTION(_##func##2)(x,y);						\
	}																	

#define OCL_SVML_P2_D2_D2_pD2_GLOBAL(func)										\
	__attribute__((overloadable)) double2 func(double2 x,__global double2* y)	\
	{																			\
		return func(x, (double2*)y);											\
	}																			

#define OCL_SVML_P2_D2_D2_pD2_LOCAL(func)										\
	__attribute__((overloadable)) double2 func(double2 x,__local double2* y)	\
	{																			\
		return func(x,(double2*)y);												\
	}

// float3
#define OCL_INTR_P2_F3_F3_pF3(func)												\
	__attribute__((overloadable)) float3 func(float3 x, float3* y)						\
	{																			\
		float4 res,valx,valy;													\
		valx.s012 = x;															\
		res = func(valx,&valy);													\
		*y = valy.s012;															\
		return res.s012;														\
	}				

#define OCL_INTR_P2_F3_F3_pF3_LOCAL(func)										\
	__attribute__((overloadable)) float3 func(float3 x,__local float3* y)				\
	{																			\
		return func(x, (float3*)y);										\
	}																			

#define OCL_INTR_P2_F3_F3_pF3_GLOBAL(func)										\
	__attribute__((overloadable)) float3 func(float3 x,__global float3* y)			\
	{																			\
		return func(x, (float3*)y);										\
	}		
//float4
#define OCL_INTR_P2_F4_F4_pF4(func)												\
	__attribute__((overloadable)) float4 func(float4 x, float4* y);					

#define OCL_INTR_P2_F4_F4_pF4_LOCAL(func)										\
	__attribute__((overloadable)) float4 func(float4 x,__local float4* y)		\
	{																			\
		return func(x, (float4*)y);												\
	}																			

#define OCL_INTR_P2_F4_F4_pF4_GLOBAL(func)										\
	__attribute__((overloadable)) float4 func(float4 x,__global float4* y)		\
	{																			\
		return func(x, (float4*)y);												\
	}																			

#define OCL_SVML_P2_D3_D3_pD3(func)										\
	__attribute__((svmlcc)) double4 OCL_SVML_FUNCTION(_##func##4)(double4,double4*);			\
	__attribute__((overloadable)) double3 func(double3 x,double3* y)			\
	{																	\
		double4 ret,valx,valy;											\
		valx.s012 = x;													\
		ret = OCL_SVML_FUNCTION(_##func##4)(valx,&valy);				\
		*y = valy.s012;													\
		return ret.s012;												\
	}																	

#define OCL_SVML_P2_D3_D3_pD3_GLOBAL(func)										\
	__attribute__((overloadable)) double3 func(double3 x,__global double3* y)			\
	{																			\
		return func(x, (double3*)y);									\
	}																			

#define OCL_SVML_P2_D3_D3_pD3_LOCAL(func)										\
	__attribute__((overloadable)) double3 func(double3 x,__local double3* y)			\
	{																			\
		return func(x,(double3*)y);										\
	}


#define OCL_SVML_P2_D4_D4_pD4(func)												\
	__attribute__((svmlcc)) double4 OCL_SVML_FUNCTION(_##func##4)(double4,double4*);					\
	__attribute__((overloadable)) double4 func(double4 x,double4* y)			\
	{																			\
		double4 ret =  OCL_SVML_FUNCTION(_##func##4)(x,y);						\
		return ret;																\
	}																	

#define OCL_SVML_P2_D4_D4_pD4_GLOBAL(func)										\
	__attribute__((overloadable)) double4 func(double4 x,__global double4* y)	\
	{																			\
		return func(x, (double4*)y);											\
	}																			

#define OCL_SVML_P2_D4_D4_pD4_LOCAL(func)										\
	__attribute__((overloadable)) double4 func(double4 x,__local double4* y)	\
	{																			\
		return func(x,(double4*)y);												\
	}																			

//float8
#define OCL_INTR_P2_F8_F8_pF8(func)												\
	__attribute__((overloadable)) float8 func(float8 x, float8* y)				\
	{																			\
		float8 res;																\
		float4 temp;															\
		res.lo = func(x.lo, &temp);												\
		y->lo = temp;															\
		res.hi = func(x.hi, &temp);												\
		y->hi = temp;															\
		return res;																\
	}																			

#define OCL_INTR_P2_F8_F8_pF8_AS_F8(func)										\
	__attribute__((overloadable)) float8 func(float8 x, float8* y);

#define OCL_INTR_P2_F8_F8_pF8_LOCAL(func)										\
	__attribute__((overloadable)) float8 func(float8 x,__local float8* y)		\
	{																			\
		return func(x,(float8*)y);												\
	}																			

#define OCL_INTR_P2_F8_F8_pF8_GLOBAL(func)										\
	__attribute__((overloadable)) float8 func(float8 x,__global float8* y)		\
	{																			\
		return func(x,(float8*)y);												\
	}																			

#define OCL_SVML_P2_D8_D8_pD8(func)												\
	__attribute__((svmlcc)) double4 OCL_SVML_FUNCTION(_##func##4)(double4,double4*);					\
	__attribute__((overloadable)) double8 func(double8 x,double8* y)			\
	{																			\
        double8 res;                                                            \
        double4 y1, y2;                                                         \
        res.lo = OCL_SVML_FUNCTION(_##func##4)(x.lo,&y1);	                    \
		res.hi = OCL_SVML_FUNCTION(_##func##4)(x.hi,&y2);						\
		y->lo = y1;																\
        y->hi = y2;                                                             \
        return res;                                                             \
	}																	

#define OCL_SVML_P2_D8_D8_pD8_GLOBAL(func)										\
	__attribute__((overloadable)) double8 func(double8 x,__global double8* y)	\
	{																			\
		return func(x, (double8*)y);											\
	}																			

#define OCL_SVML_P2_D8_D8_pD8_LOCAL(func)										\
	__attribute__((overloadable)) double8 func(double8 x,__local double8* y)	\
	{																			\
		return func(x,(double8*)y);												\
	}																			

//float16
#define OCL_INTR_P2_F16_F16_pF16(func)											\
	float16 __attribute__((overloadable)) func(float16 x, float16* y)			\
	{																			\
		float16 res;															\
		float4 temp;															\
		res.s0123 =  func(x.s0123, &temp);										\
		y->s0123 = temp;														\
		res.s4567 =  func(x.s4567, &temp);										\
		y->s4567 = temp;														\
		res.s89ab =  func(x.s89ab, &temp);										\
		y->s89ab = temp;														\
		res.scdef =  func(x.scdef, &temp);										\
		y->scdef = temp;														\
		return res;																\
	}

#define OCL_INTR_P2_F16_F16_pF16_AS_F8(func)									\
	float16 __attribute__((overloadable)) func(float16 x, float16* y)			\
	{																			\
		float16 res;															\
		float8 temp;															\
		res.s01234567 =  func(x.s01234567, &temp);								\
		y->s01234567 = temp;													\
		res.s89abcdef =  func(x.s89abcdef, &temp);								\
		y->s89abcdef = temp;													\
		return res;																\
	}

#define OCL_INTR_P2_F16_F16_pF16_LOCAL(func)									\
    __attribute__((overloadable)) float16 func(float16 x,__local float16* y)	\
    {																			\
		return func(x,(float16*)y);												\
    }

#define OCL_INTR_P2_F16_F16_pF16_GLOBAL(func)									\
    __attribute__((overloadable)) float16 func(float16 x,__global float16* y)	\
    {																			\
		return func(x,(float16*)y);												\
    }

#define OCL_SVML_P2_D16_D16_pD16(func)											\
	__attribute__((svmlcc)) double4 OCL_SVML_FUNCTION(_##func##4)(double4,double4*);					\
	double16 __attribute__((overloadable)) func(double16 x,double16* y)			\
	{																			\
		double16 ret;															\
		double4 y1,y2,y3,y4;													\
		ret.s0123 = OCL_SVML_FUNCTION(_##func##4)(x.s0123,&y1);					\
		ret.s4567 = OCL_SVML_FUNCTION(_##func##4)(x.s4567,&y2);					\
		ret.s89ab = OCL_SVML_FUNCTION(_##func##4)(x.s89ab,&y3);					\
		ret.scdef = OCL_SVML_FUNCTION(_##func##4)(x.scdef,&y4);					\
		y->lo.lo = y1;															\
		y->lo.hi = y2;															\
		y->hi.lo = y3;															\
		y->hi.hi = y4;															\
		return ret;																\
	}																			\

#define OCL_SVML_P2_D16_D16_pD16_GLOBAL(func)									\
	double16 __attribute__((overloadable)) func(double16 x, __global double16* y)	\
	{																			\
		double16 ret;															\
		double4 y1,y2,y3,y4;													\
		ret.s0123 = OCL_SVML_FUNCTION(_##func##4)(x.s0123,&y1);					\
		ret.s4567 = OCL_SVML_FUNCTION(_##func##4)(x.s4567,&y2);					\
		ret.s89ab = OCL_SVML_FUNCTION(_##func##4)(x.s89ab,&y3);					\
		ret.scdef = OCL_SVML_FUNCTION(_##func##4)(x.scdef,&y4);					\
		y->lo.lo = y1;															\
		y->lo.hi = y2;															\
		y->hi.lo = y3;															\
		y->hi.hi = y4;															\
		return ret;																\
	}		

#define OCL_SVML_P2_D16_D16_pD16_LOCAL(func)									\
	double16 __attribute__((overloadable)) func(double16 x,__local double16* y)	\
	{																			\
		double16 ret;															\
		double4 y1,y2,y3,y4;													\
		ret.s0123 = OCL_SVML_FUNCTION(_##func##4)(x.s0123,&y1);					\
		ret.s4567 = OCL_SVML_FUNCTION(_##func##4)(x.s4567,&y2);					\
		ret.s89ab = OCL_SVML_FUNCTION(_##func##4)(x.s89ab,&y3);					\
		ret.scdef = OCL_SVML_FUNCTION(_##func##4)(x.scdef,&y4);					\
		y->lo.lo = y1;															\
		y->lo.hi = y2;															\
		y->hi.lo = y3;															\
		y->hi.hi = y4;															\
		return ret;																\
	}		


//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
#define OCL_SVML_P3_F1_F1_F1_F1(func)											\
	__attribute__((svmlcc)) __attribute__((const)) float OCL_SVML_FUNCTION(_##func##f1)(float,float,float);					\
	__attribute__((overloadable)) float func(float x,float y,float z)			\
	{																			\
		return OCL_SVML_FUNCTION(_##func##f1)(x,y,z);							\
	}																	

#define OCL_SVML_P3_D1_D1_D1_D1(func)											\
	__attribute__((svmlcc)) __attribute__((const)) double OCL_SVML_FUNCTION(_##func##1)(double,double,double);					\
	__attribute__((overloadable)) double func(double x,double y,double z)		\
	{																			\
		return OCL_SVML_FUNCTION(_##func##1)(x,y,z);							\
	}																	

#define OCL_SVML_P3_F2_F2_F2_F2(func)											\
	__attribute__((svmlcc)) __attribute__((const)) float4 OCL_SVML_FUNCTION(_##func##f2)(float4,float4,float4);				\
	__attribute__((overloadable)) float2 func(float2 x,float2 y,float2 z)		\
	{																			\
		float4 valx,valy,valz;													\
		valx.s01 = x; valy.s01 = y; valz.s01 = z;								\
		float4 res = func(valx,valy,valz);										\
		return res.s01;															\
	}
/*
#define OCL_SVML_P3_F2_F2_F2_F2(func)											\
	float4 OCL_SVML_FUNCTION(_##func##f2)(float4,float4,float4);				\
	__attribute__((overloadable)) float2 func(float2 x,float2 y,float2 z)		\
	{																			\
		float4 valx,valy,valz;													\
		valx = _mm_castsi128_ps(_mm_loadl_epi64((__m128i*)&x));					\
		valy = _mm_castsi128_ps(_mm_loadl_epi64((__m128i*)&y));					\
		valz = _mm_castsi128_ps(_mm_loadl_epi64((__m128i*)&z));					\
		float4 val= OCL_SVML_FUNCTION(_##func##f2)(valx,valy,valz);				\
		float2 res;																\
		_mm_storel_epi64((__m128i*)&res, (__m128i)val);							\
		return res;																\
	}																			
*/
#define OCL_SVML_P3_D2_D2_D2_D2(func)											\
	__attribute__((svmlcc)) __attribute__((const)) double2 OCL_SVML_FUNCTION(_##func##2)(double2,double2,double2);				\
	__attribute__((overloadable)) double2 func(double2 x,double2 y,double2 z)	\
	{																			\
		return OCL_SVML_FUNCTION(_##func##2)(x,y,z);							\
	}																		

#define OCL_SVML_P3_F3_F3_F3_F3(func)											\
	__attribute__((svmlcc)) __attribute__((const)) float4 OCL_SVML_FUNCTION(_##func##f4)(float4,float4,float4);				\
	__attribute__((overloadable)) float3 func(float3 x,float3 y,float3 z)		\
	{																			\
		float4 valx,valy,valz;													\
		valx.s012 = x; valy.s012 = y; valz.s012 = z;							\
		float4 res = OCL_SVML_FUNCTION(_##func##f4)(valx,valy,valz);			\
		return res.s012;														\
	}																				
#define OCL_SVML_P3_F4_F4_F4_F4(func)											\
	__attribute__((svmlcc)) __attribute__((const)) float4 OCL_SVML_FUNCTION(_##func##f4)(float4,float4,float4);				\
	__attribute__((overloadable)) float4 func(float4 x,float4 y,float4 z)		\
	{																			\
		return OCL_SVML_FUNCTION(_##func##f4)(x,y,z);							\
	}																				

#define OCL_SVML_P3_D4_D4_D4_D4(func)											\
	__attribute__((svmlcc)) __attribute__((const)) double2 OCL_SVML_FUNCTION(_##func##2)(double2,double2,double2);				\
	__attribute__((overloadable)) double4 func(double4 x,double4 y,double4 z)	\
	{																			\
        double4 res;                                                            \
        res.lo = OCL_SVML_FUNCTION(_##func##2)(x.lo,y.lo,z.lo);                 \
        res.hi = OCL_SVML_FUNCTION(_##func##2)(x.hi,y.hi,z.hi);                 \
		return res;                                 							\
	}																		

#define OCL_SVML_P3_D3_D3_D3_D3(func)										\
	__attribute__((overloadable)) double3 func(double3 x,double3 y,double3 z)		\
	{																		\
		double4 valx,valy,valz;												\
		valx.s012 = x;														\
		valy.s012 = y;														\
		valz.s012 = z;														\
		double4 res =  func(valx, valy, valz);      		                \
		return res.s012;													\
	}
			
#define OCL_SVML_P3_F8_F8_F8_F8(func)											\
	__attribute__((svmlcc)) __attribute__((const)) float4 OCL_SVML_FUNCTION(_##func##f4)(float4,float4,float4);				\
	__attribute__((overloadable)) float8 func(float8 x,float8 y,float8 z)		\
	{																			\
        float8 res;                                                             \
        res.lo = OCL_SVML_FUNCTION(_##func##f4)(x.lo,y.lo,z.lo);                \
        res.hi = OCL_SVML_FUNCTION(_##func##f4)(x.hi,y.hi,z.hi);                \
		return res;                                 							\
	}																				
		
#define OCL_SVML_P3_D8_D8_D8_D8(func)											\
	__attribute__((svmlcc)) __attribute__((const)) double2 OCL_SVML_FUNCTION(_##func##2)(double2,double2,double2);				\
	__attribute__((overloadable)) double8 func(double8 x,double8 y,double8 z)	\
	{																			\
        double8 res;                                                            \
        res.s01 = OCL_SVML_FUNCTION(_##func##2)(x.s01,y.s01,z.s01);             \
        res.s23 = OCL_SVML_FUNCTION(_##func##2)(x.s23,y.s23,z.s23);             \
        res.s45 = OCL_SVML_FUNCTION(_##func##2)(x.s45,y.s45,z.s45);             \
        res.s67 = OCL_SVML_FUNCTION(_##func##2)(x.s67,y.s67,z.s67);       \
		return res;                                 							\
	}																		

#define OCL_SVML_P3_F16_F16_F16_F16(func)										\
	__attribute__((svmlcc)) __attribute__((const)) float4 OCL_SVML_FUNCTION(_##func##f4)(float4,float4,float4);			\
	__attribute__((overloadable)) float16 func(float16 x,float16 y,float16 z)	\
	{																			\
        float16 res;                                                            \
        res.s0123 = OCL_SVML_FUNCTION(_##func##f4)(x.s0123,y.s0123,z.s0123);    \
        res.s4567 = OCL_SVML_FUNCTION(_##func##f4)(x.s4567,y.s4567,z.s4567);    \
        res.s89ab = OCL_SVML_FUNCTION(_##func##f4)(x.s89ab,y.s89ab,z.s89ab);    \
        res.scdef = OCL_SVML_FUNCTION(_##func##f4)(x.scdef,y.scdef,z.scdef);    \
		return res;                                 							\
	}																				

#define OCL_SVML_P3_D16_D16_D16_D16(func)										\
	__attribute__((svmlcc)) __attribute__((const)) double2 OCL_SVML_FUNCTION(_##func##2)(double2,double2,double2);		\
	__attribute__((overloadable)) double16 func(double16 x,double16 y,double16 z)		\
	{																			\
		double16 res;															\
        res.s01 = OCL_SVML_FUNCTION(_##func##2)(x.s01,y.s01,z.s01);             \
        res.s23 = OCL_SVML_FUNCTION(_##func##2)(x.s23,y.s23,z.s23);             \
        res.s45 = OCL_SVML_FUNCTION(_##func##2)(x.s45,y.s45,z.s45);             \
        res.s67 = OCL_SVML_FUNCTION(_##func##2)(x.s67,y.s67,z.s67);             \
        res.s89 = OCL_SVML_FUNCTION(_##func##2)(x.s89,y.s89,z.s89);             \
        res.sab = OCL_SVML_FUNCTION(_##func##2)(x.sab,y.sab,z.sab);             \
        res.scd = OCL_SVML_FUNCTION(_##func##2)(x.scd,y.scd,z.scd);             \
        res.sef = OCL_SVML_FUNCTION(_##func##2)(x.sef,y.sef,z.sef);             \
		return res;                                 							\
	}											


//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
#define OCL_INTR_P3_F1_F1_F1_F1(func)											\
	__attribute__((overloadable)) float func(float x,float y,float z);					

#define OCL_INTR_P3_F2_F2_F2_F2_AS_F1(func)										\
	__attribute__((overloadable)) float2 func(float2 x,float2 y,float2 z)		\
	{																			\
		float2 res;																\
		res.lo = func(x.lo, y.lo, z.lo);										\
		res.hi = func(x.hi, y.hi, z.hi);										\
		return res;																\
	}																			

#define OCL_INTR_P3_F2_F2_F2_F2_AS_F4(func)										\
	__attribute__((overloadable)) float2 func(float2 x,float2 y,float2 z)		\
	{																			\
		float4 xVec, yVec, zVec;												\
		float2 res;																\
		xVec = _mm_castsi128_ps(_mm_loadl_epi64((__m128i*)&x));					\
		yVec = _mm_castsi128_ps(_mm_loadl_epi64((__m128i*)&y));					\
		zVec = _mm_castsi128_ps(_mm_loadl_epi64((__m128i*)&z));					\
		xVec = func(xVec, yVec, zVec);											\
		_mm_storel_epi64((__m128i*)&res, (__m128i)xVec);						\
		return res;																\
	}																			
		
#define OCL_INTR_P3_F3_F3_F3_F3(func)											\
	__attribute__((overloadable)) float3 func(float3 x,float3 y,float3 z)				\
	{																			\
		float4 res;																\
		float4 valx,valy,valz;													\
		valx.s012 = x ;															\
		valy.s012 = y;															\
		valz.s012 = z;															\
		res = func(valx,valy,valz);												\
		return res.s012;														\
	}																			
#define OCL_INTR_P3_F4_F4_F4_F4(func)											\
	__attribute__((overloadable)) float4 func(float4 x,float4 y,float4 z);				

#define OCL_INTR_P3_F8_F8_F8_F8(func)											\
	__attribute__((overloadable)) float8 func(float8 x,float8 y,float8 z)		\
	{																			\
		float8 res;																\
		res.lo = func(x.lo, y.lo, z.lo);										\
		res.hi = func(x.hi, y.hi, z.hi);										\
		return res;																\
	}																			

#define OCL_INTR_P3_F8_F8_F8_F8_AS_F8(func) 									\
    __attribute__((overloadable)) float8 func(float8 x,float8 y,float8 z);

#define OCL_INTR_P3_F16_F16_F16_F16(func)										\
	float16 __attribute__((overloadable)) func(float16 x,float16 y,float16 z)	\
	{																			\
		float16 res;															\
		res.s0123 =  func(x.s0123, y.s0123, z.s0123);							\
		res.s4567 =  func(x.s4567, y.s4567, z.s4567);							\
		res.s89ab =  func(x.s89ab, y.s89ab, z.s89ab);							\
		res.scdef =  func(x.scdef, y.scdef, z.scdef);							\
		return res;																\
	}																			

#define OCL_INTR_P3_F16_F16_F16_F16_AS_F8(func)									\
    float16 __attribute__((overloadable)) func(float16 x,float16 y,float16 z)	\
    {																			\
        float16 res;															\
        res.s01234567 =  func(x.s01234567, y.s01234567, z.s01234567);			\
        res.s89abcdef =  func(x.s89abcdef, y.s89abcdef, z.s01234567);			\
        return res;																\
    }

//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
#define OCL_SVML_P3_F1_F1_F1_PI1(func)											\
	__attribute__((svmlcc)) float OCL_SVML_FUNCTION(_##func##f1)(float,float,int*);						\
	__attribute__((overloadable)) float func(float x, float y, int* z)			\
	{																			\
		return OCL_SVML_FUNCTION(_##func##f1)(x,y,z);							\
	}																	

#define OCL_SVML_P3_F1_F1_F1_PI1_LOCAL(func)									\
	__attribute__((overloadable)) float func(float x, float y, __local int* z)	\
	{																			\
		return func(x,y,(int*)z);												\
	}																			\

#define OCL_SVML_P3_F1_F1_F1_PI1_GLOBAL(func)									\
	__attribute__((overloadable)) float func(float x, float y, __global int* z)	\
	{																			\
		return func(x,y,(int*)z);												\
	}																	

#define OCL_SVML_P3_F2_F2_F2_PI2(func)											\
	__attribute__((svmlcc)) float4 OCL_SVML_FUNCTION(_##func##f2)(float4,float4,_4i32*);				\
	__attribute__((overloadable)) float2 func(float2 x, float2 y, _2i32* z)		\
	{																			\
		float4 valx,valy;														\
		valx = _mm_castsi128_ps(_mm_loadl_epi64((__m128i*)&x));					\
		valy = _mm_castsi128_ps(_mm_loadl_epi64((__m128i*)&y));					\
		_4i32 valz;																\
		float4 val = OCL_SVML_FUNCTION(_##func##f2)(valx,valy,&valz);			\
		_mm_storel_epi64((__m128i*)z, (__m128i)valz);							\
		float2 res;																\
		_mm_storel_epi64((__m128i*)&res, (__m128i)val);							\
		return res;																\
	}																					

#define OCL_SVML_P3_F2_F2_F2_PI2_LOCAL(func)										\
	__attribute__((overloadable)) float2 func(float2 x, float2 y, __local _2i32* z)	\
	{																				\
		return func(x,y,(_2i32*)z);													\
	}																					

#define OCL_SVML_P3_F2_F2_F2_PI2_GLOBAL(func)										\
	__attribute__((overloadable)) float2 func(float2 x, float2 y, __global _2i32* z)\
	{																				\
		return func(x,y,(_2i32*)z);													\
	}																					

#define OCL_SVML_P3_F3_F3_F3_PI3(func)											\
	__attribute__((svmlcc)) float4 OCL_SVML_FUNCTION(_##func##f4)(float4,float4,_4i32*);				\
	__attribute__((overloadable)) float3 func(float3 x, float3 y, _3i32* z)		\
	{																			\
		float4 valx,valy;														\
		valx.s012 = x; valy.s012 = y;											\
		_4i32 valz;																\
		float4 res = OCL_SVML_FUNCTION(_##func##f4)(valx,valy,&valz);			\
		*z = valz.s012;;														\
		return res.s012;														\
	}																					

#define OCL_SVML_P3_F3_F3_F3_PI3_LOCAL(func)										\
	__attribute__((overloadable)) float3 func(float3 x, float3 y, __local _3i32* z)	\
	{																				\
		return func(x,y,(_3i32*)z);													\
	}																					

#define OCL_SVML_P3_F3_F3_F3_PI3_GLOBAL(func)										\
	__attribute__((overloadable)) float3 func(float3 x, float3 y, __global _3i32* z)\
	{																				\
		return func(x,y,(_3i32*)z);													\
	}									
#define OCL_SVML_P3_F4_F4_F4_PI4(func)											\
	__attribute__((svmlcc)) float4 OCL_SVML_FUNCTION(_##func##f4)(float4,float4,_4i32*);				\
	__attribute__((overloadable)) float4 func(float4 x, float4 y, _4i32* z)		\
	{																			\
		float4 valx,valy;														\
		valx = x;																\
		valy = y;																\
		_4i32 valz;																\
		float4 res = OCL_SVML_FUNCTION(_##func##f4)(valx,valy,&valz);			\
		*z = valz;																\
		return res;																\
	}																					

#define OCL_SVML_P3_F4_F4_F4_PI4_LOCAL(func)										\
	__attribute__((overloadable)) float4 func(float4 x, float4 y, __local _4i32* z)	\
	{																				\
		return func(x,y,(_4i32*)z);													\
	}																					

#define OCL_SVML_P3_F4_F4_F4_PI4_GLOBAL(func)										\
	__attribute__((overloadable)) float4 func(float4 x, float4 y, __global _4i32* z)\
	{																				\
		return OCL_SVML_FUNCTION(_##func##f4)(x, y, (_4i32*)z);						\
	}																					

#define OCL_SVML_P3_F8_F8_F8_PI8(func)												\
	__attribute__((svmlcc)) float4 OCL_SVML_FUNCTION(_##func##f4)(float4, float4, _4i32*);					\
	__attribute__((overloadable)) float8 func(float8 x, float8 y, _8i32* z)			\
	{																				\
		float8 res;                                                                 \
        _4i32 z1, z2;                                                               \
        res.lo = OCL_SVML_FUNCTION(_##func##f4)(x.lo,y.lo,&z1);                     \
        res.hi = OCL_SVML_FUNCTION(_##func##f4)(x.hi,y.hi,&z2);                     \
        z->lo = z1;                                                                 \
        z->hi = z2;                                                                 \
		return res;																	\
	}																				

#define OCL_SVML_P3_F8_F8_F8_PI8_LOCAL(func)										\
	__attribute__((overloadable)) float8 func(float8 x, float8 y, __local _8i32* z)	\
	{																				\
		return func(x,y,(_8i32*)z);													\
	}																				

#define OCL_SVML_P3_F8_F8_F8_PI8_GLOBAL(func)										\
	__attribute__((overloadable)) float8 func(float8 x, float8 y, __global _8i32* z)\
	{																				\
		return func(x,y,(_8i32*)z);													\
	}																				

#define OCL_SVML_P3_F16_F16_F16_PI16(func)											\
	__attribute__((svmlcc)) float4 OCL_SVML_FUNCTION(_##func##f4)(float4,float4,_4i32*);				\
	__attribute__((overloadable)) float16 func(float16 x, float16 y, _16i32* z)		\
	{																				\
		float16 res;                                                                \
        _4i32 z1, z2, z3, z4;                                                       \
        res.s0123 = OCL_SVML_FUNCTION(_##func##f4)(x.s0123,y.s0123,&z1);                     \
        res.s4567 = OCL_SVML_FUNCTION(_##func##f4)(x.s4567,y.s4567,&z2);                     \
        res.s89ab = OCL_SVML_FUNCTION(_##func##f4)(x.s89ab,y.s89ab,&z3);                     \
        res.scdef = OCL_SVML_FUNCTION(_##func##f4)(x.scdef,y.scdef,&z4);                     \
        z->lo.lo = z1;                                                                 \
        z->lo.hi = z2;                                                                 \
        z->hi.lo = z3;                                                                 \
        z->hi.hi = z4;                                                                 \
		return res;																	\
	}																				

#define OCL_SVML_P3_F16_F16_F16_PI16_LOCAL(func)										\
	__attribute__((overloadable)) float16 func(float16 x, float16 y, __local _16i32* z)	\
	{																					\
		return func(x,y,(_16i32*)z);													\
	}																					

#define OCL_SVML_P3_F16_F16_F16_PI16_GLOBAL(func)										\
	__attribute__((overloadable)) float16 func(float16 x, float16 y, __global _16i32* z)\
	{																					\
		return func(x,y,(_16i32*)z);													\
	}																					

/// remquo

// double

	__attribute__((svmlcc)) double OCL_SVML_FUNCTION(_remquo1)(double,double,_1i32*);			
	__attribute__((overloadable)) double remquo(double x, double y, _1i32* z)		
	{																	
		return OCL_SVML_FUNCTION(_remquo1)(x,y,z);					
	}																	
	

	__attribute__((overloadable)) double remquo(double x, double y, __local _1i32* z)		
	{																	
		return OCL_SVML_FUNCTION(_remquo1)(x,y,(_1i32*)z);					
	}																	
	

	__attribute__((overloadable)) double remquo(double x, double y, __global _1i32* z)		
	{																	
		return OCL_SVML_FUNCTION(_remquo1)(x,y,(_1i32*)z);					
	}																	
	
	
// double2

	__attribute__((svmlcc)) double2 OCL_SVML_FUNCTION(_remquo2)(double2,double2,_4i32*);		
	__attribute__((overloadable)) double2 remquo(double2 x, double2 y, _2i32* z)
	{
		_4i32 valz;
		double2 val = OCL_SVML_FUNCTION(_remquo2)(x,y,&valz);
		_mm_storel_epi64((__m128i*)z, (__m128i)valz);
		return val;
	}	


	__attribute__((overloadable)) double2 remquo(double2 x, double2 y, __local _2i32* z)
	{
		_4i32 valz;
		double2 val = OCL_SVML_FUNCTION(_remquo2)(x,y,&valz);
		_mm_storel_epi64((__m128i*)z, (__m128i)valz);
		return val;
	}
	

	__attribute__((overloadable)) double2 remquo(double2 x, double2 y, __global _2i32* z)
	{
		_4i32 valz;
		double2 val = OCL_SVML_FUNCTION(_remquo2)(x,y,&valz);
		_mm_storel_epi64((__m128i*)z, (__m128i)valz);
		return val;
	}
	
// double3
	
	__attribute__((svmlcc)) double4 OCL_SVML_FUNCTION(_remquo4)(double4,double4,_4i32*);		 

	__attribute__((overloadable)) double3 remquo(double3 x, double3 y, _3i32* z)	
	{																					
		double4 res,valx,valy;
		_4i32 valz;
		valx.s012 = x;
		valy.s012 = y;
		res = remquo(valx, valy, &valz);
		*z = valz.s012;
		return res.s012;
	}																	
	
	
	__attribute__((overloadable)) double3 remquo(double3 x, double3 y,__local   _3i32* z)
	{																	
		return remquo(x,y,(_3i32*)z);										
	}																	
	

	__attribute__((overloadable)) double3 remquo(double3 x, double3 y,__global    _3i32* z)
	{																	
		return remquo(x,y,(_3i32*)z);										
	}																	
	

// double4

	
	__attribute__((overloadable)) double4 remquo(double4 x, double4 y, _4i32* z)	
	{																					
		double4 res;
        _4i32 z1,z2;
        res.lo = OCL_SVML_FUNCTION(_remquo2)(x.lo,y.lo,&z1);
        res.hi = OCL_SVML_FUNCTION(_remquo2)(x.hi,y.hi,&z2);
        z->lo = z1.lo;
        z->hi = z2.lo;
		return res;
	}																	
	
	
	__attribute__((overloadable)) double4 remquo(double4 x, double4 y,__local   _4i32* z)
	{																	
		return remquo(x,y,(_4i32*)z);										
	}																	
	

	__attribute__((overloadable)) double4 remquo(double4 x, double4 y,__global    _4i32* z)
	{																	
		return remquo(x,y,(_4i32*)z);										
	}																	
	

// double8

	__attribute__((svmlcc)) double2 OCL_SVML_FUNCTION(_remquo2)(double2,double2,_4i32*);		
	__attribute__((overloadable)) double8 remquo(double8 x, double8 y, _8i32* z)	
	{																					
		double8 res;
        _4i32 z1,z2,z3,z4;
        res.s01 = OCL_SVML_FUNCTION(_remquo2)(x.s01,y.s01,&z1);
        res.s23 = OCL_SVML_FUNCTION(_remquo2)(x.s23,y.s23,&z2);
        res.s45 = OCL_SVML_FUNCTION(_remquo2)(x.s45,y.s45,&z3);
        res.s67 = OCL_SVML_FUNCTION(_remquo2)(x.s67,y.s67,&z4);
        z->lo.lo = z1.lo;
        z->lo.hi = z2.lo;
        z->hi.lo = z3.lo;
        z->hi.hi = z4.lo;
		return res;
	}																	
	
	
	__attribute__((overloadable)) double8 remquo(double8 x, double8 y, __local _8i32* z)
	{																	
		return remquo(x,y,(_8i32*)z);										
	}																	
	

	__attribute__((overloadable)) double8 remquo(double8 x, double8 y, __global _8i32* z)
	{																	
		return remquo(x,y,(_8i32*)z);										
	}																 	
	

// double16

	__attribute__((svmlcc)) double2 OCL_SVML_FUNCTION(_remquo2)(double2,double2,_4i32*);		


	double16 __attribute__((overloadable)) remquo(double16 x, double16 y, _16i32* z)	
	{	
		double16 res;
        _4i32 z1,z2,z3,z4, z5, z6, z7, z8;
        res.s01 = OCL_SVML_FUNCTION(_remquo2)(x.s01,y.s01,&z1);
        res.s23 = OCL_SVML_FUNCTION(_remquo2)(x.s23,y.s23,&z2);
        res.s45 = OCL_SVML_FUNCTION(_remquo2)(x.s45,y.s45,&z3);
        res.s67 = OCL_SVML_FUNCTION(_remquo2)(x.s67,y.s67,&z4);
        res.s89 = OCL_SVML_FUNCTION(_remquo2)(x.s89,y.s89,&z5);
        res.sab = OCL_SVML_FUNCTION(_remquo2)(x.sab,y.sab,&z6);
        res.scd = OCL_SVML_FUNCTION(_remquo2)(x.scd,y.scd,&z7);
        res.sef = OCL_SVML_FUNCTION(_remquo2)(x.sef,y.sef,&z8);
        z->s01 = z1.lo;
        z->s23 = z2.lo;
        z->s45 = z3.lo;
        z->s67 = z4.lo;
        z->s89 = z5.lo;
        z->sab = z6.lo;
        z->scd = z7.lo;
        z->sef = z8.lo;
		return res;
	}																		


	double16 __attribute__((overloadable)) remquo(double16 x, double16 y, __local _16i32* z)
	{																	
		double16 res;
        _4i32 z1,z2,z3,z4, z5, z6, z7, z8;
        res.s01 = OCL_SVML_FUNCTION(_remquo2)(x.s01,y.s01,&z1);
        res.s23 = OCL_SVML_FUNCTION(_remquo2)(x.s23,y.s23,&z2);
        res.s45 = OCL_SVML_FUNCTION(_remquo2)(x.s45,y.s45,&z3);
        res.s67 = OCL_SVML_FUNCTION(_remquo2)(x.s67,y.s67,&z4);
        res.s89 = OCL_SVML_FUNCTION(_remquo2)(x.s89,y.s89,&z5);
        res.sab = OCL_SVML_FUNCTION(_remquo2)(x.sab,y.sab,&z6);
        res.scd = OCL_SVML_FUNCTION(_remquo2)(x.scd,y.scd,&z7);
        res.sef = OCL_SVML_FUNCTION(_remquo2)(x.sef,y.sef,&z8);
        z->s01 = z1.lo;
        z->s23 = z2.lo;
        z->s45 = z3.lo;
        z->s67 = z4.lo;
        z->s89 = z5.lo;
        z->sab = z6.lo;
        z->scd = z7.lo;
        z->sef = z8.lo;
		return res;
	}	


	double16 __attribute__((overloadable)) remquo(double16 x, double16 y, __global _16i32* z)
	{																	
		double16 res;
        _4i32 z1,z2,z3,z4, z5, z6, z7, z8;
        res.s01 = OCL_SVML_FUNCTION(_remquo2)(x.s01,y.s01,&z1);
        res.s23 = OCL_SVML_FUNCTION(_remquo2)(x.s23,y.s23,&z2);
        res.s45 = OCL_SVML_FUNCTION(_remquo2)(x.s45,y.s45,&z3);
        res.s67 = OCL_SVML_FUNCTION(_remquo2)(x.s67,y.s67,&z4);
        res.s89 = OCL_SVML_FUNCTION(_remquo2)(x.s89,y.s89,&z5);
        res.sab = OCL_SVML_FUNCTION(_remquo2)(x.sab,y.sab,&z6);
        res.scd = OCL_SVML_FUNCTION(_remquo2)(x.scd,y.scd,&z7);
        res.sef = OCL_SVML_FUNCTION(_remquo2)(x.sef,y.sef,&z8);
        z->s01 = z1.lo;
        z->s23 = z2.lo;
        z->s45 = z3.lo;
        z->s67 = z4.lo;
        z->s89 = z5.lo;
        z->sab = z6.lo;
        z->scd = z7.lo;
        z->sef = z8.lo;
		return res;
	}	



//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------

#if defined(__AVX__)
OCL_INTR_P2_vFvFpvF_vDvDpvD_F816_AS_F8(fract)
#elif defined(__SSE4_1__)
OCL_INTR_P2_vFvFpvF_vDvDpvD(fract)
#else
OCL_SVML_P2_vFvFpvF(fract)
#endif 

#if defined(__SSE4_1__)
OCL_INTR_P1_vFvF_vDvD_ALL_AS_F1(trunc)
OCL_INTR_P1_vFvF_vDvD_ALL_AS_F1(ceil)
OCL_INTR_P1_vFvF_vDvD_ALL_AS_F1(floor)
OCL_INTR_P1_vFvF_vDvD_ALL_AS_F4(round)
OCL_INTR_P1_vFvF_vDvD_ALL_AS_F1(rint)
OCL_INTR_P2_vFvFpvF(fractr)
#else
OCL_SVML_P1_vFvF_ALL(trunc)
OCL_SVML_P1_vFvF_ALL(ceil)
OCL_SVML_P1_vFvF_ALL(floor)
OCL_SVML_P1_vFvF_ALL(round)
OCL_SVML_P1_vFvF_ALL(rint)
#endif

OCL_INTR_P1_vFvF_vDvD_ALL_AS_F1(fabs)
//OCL_INTR_P1_vFvF_vDvD_ALL_AS_F4(logb)
OCL_SVML_P1_vFvF_ALL(logb)


OCL_SVML_P1_vFvF_ALL(lgamma)
OCL_SVML_P1_vFvF_ALL(tgamma)
OCL_SVML_P1_vFvF_ALL(sqrt)
OCL_SP_INTR_DP_SVML_P1_vFvF_ALL(rsqrt)
OCL_SVML_P1_vFvF_ALL(erf)
OCL_SVML_P1_vFvF_ALL(erfc)
OCL_SVML_P1_vFvF_ALL(cbrt)
OCL_SVML_P1_vFvF_ALL(acos)
OCL_SVML_P1_vFvF_ALL(cos)
OCL_SVML_P1_vFvF_ALL(cosh)
OCL_SVML_P1_vFvF_ALL(acosh)
OCL_SVML_P1_vFvF_ALL(cospi)
OCL_SVML_P1_vFvF_ALL(acospi)
OCL_SVML_P1_vFvF_ALL(sin)
OCL_SVML_P1_vFvF_ALL(asin)
OCL_SVML_P1_vFvF_ALL(sinh)
OCL_SVML_P1_vFvF_ALL(asinh)
OCL_SVML_P1_vFvF_ALL(asinpi)
OCL_SVML_P1_vFvF_ALL(sinpi)
OCL_SVML_P1_vFvF_ALL(log1p)
OCL_SVML_P1_vFvF_ALL(log)
OCL_SVML_P1_vFvF_ALL(log10)
OCL_SVML_P1_vFvF_ALL(log2)
OCL_SVML_P1_vFvF_ALL(tanh)
OCL_SVML_P1_vFvF_ALL(tan)
OCL_SVML_P1_vFvF_ALL(atan)
OCL_SVML_P1_vFvF_ALL(atanh)
OCL_SVML_P1_vFvF_ALL(tanpi)
OCL_SVML_P1_vFvF_ALL(atanpi)
OCL_SVML_P1_vFvF_ALL(exp)
OCL_SVML_P1_vFvF_ALL(exp2)
OCL_SVML_P1_vFvF_ALL(exp10)
OCL_SVML_P1_vFvF_ALL(expm1)

OCL_INTR_P1_I1_F1(ilogb)
OCL_INTR_P1_I4_F4(ilogb)
OCL_INTR_P1_I3_F3(ilogb)
OCL_INTR_P1_I2_F2_AS_F4(ilogb)
#if defined(__AVX__)
OCL_INTR_P1_I8_F8_AS_F8(ilogb)
OCL_INTR_P1_I16_F16_AS_F8(ilogb)
#else // defined(__AVX__)
OCL_INTR_P1_I8_F8(ilogb)
OCL_INTR_P1_I16_F16(ilogb)
#endif // defined(__AVX__)
OCL_SVML_P1_I1_D1(ilogb)
OCL_SVML_P1_I2_D2(ilogb)
OCL_SVML_P1_I3_D3(ilogb)
OCL_SVML_P1_I4_D4(ilogb)	
OCL_SVML_P1_I8_D8(ilogb)
OCL_SVML_P1_I16_D16(ilogb)



// OCL_SVML_P1_vFvF_ALL_MANUAL(half_recip,inv)
// OCL_INTR_P1_vFvF_ALL_AS_F4(half_recipr)
//TODO: check with ramy

#if defined(__AVX__)
OCL_INTR_P1_vFvI_F816_AS_F8(nan, u)
#else // defined(__AVX__)
OCL_INTR_P1_vFvI_ALL(nan, u)
#endif // defined(__AVX__)


OCL_SVML_P2_vFvFvI_ALL(pown, i)
OCL_SVML_P2_vFvFvI_ALL(rootn, i)
OCL_SVML_P2_vFvFvI_ALL(ldexp, i)
OCL_SVML_P2_vFvFI_ALL(ldexp, i)

#if defined(__AVX__)
OCL_INTR_P2_vFvFvF_vDvDvD_ALL_AS_F8(fmin)
OCL_INTR_P2_vFvFvF_vDvDvD_ALL_AS_F8(fmax)
OCL_INTR_P2_vFvFvF_vDvDvD_ALL_AS_F8(copysign)
OCL_INTR_P2_vFvFvF_vDvDvD_ALL_AS_F8(fdim)
#else // defined(__AVX__)
OCL_INTR_P2_vFvFvF_vDvDvD_ALL_AS_F4(fmin)
OCL_INTR_P2_vFvFvF_vDvDvD_ALL_AS_F4(fmax)
OCL_INTR_P2_vFvFvF_vDvDvD_ALL_AS_F4(copysign)
OCL_INTR_P2_vFvFvF_vDvDvD_ALL_AS_F4(fdim)
#endif //defined(__AVX__)




#if defined(__AVX__)
OCL_INTR_P2_vFvFF_ALL_AS_F8(fmin)
OCL_INTR_P2_vFvFF_ALL_AS_F8(fmax)
#else // defined(__AVX__)
OCL_INTR_P2_vFvFF_ALL_AS_F4(fmin)
OCL_INTR_P2_vFvFF_ALL_AS_F4(fmax)
#endif // defined(__AVX__)


OCL_SVML_P2_vFvFvF_ALL(pow)
OCL_SVML_P2_vFvFvF_ALL(powr)

OCL_SVML_P2_vFvFvF_ALL(atan2)
OCL_SVML_P2_vFvFvF_ALL(atan2pi)


//OCL_INTR_P2_vFvFvF_ALL_AS_F4(half_divider)
//TODO: check with ramy

OCL_SVML_P2_vFvFvF_ALL(hypot)
OCL_SVML_P2_vFvFvF_ALL(nextafter)
OCL_SVML_P2_vFvFvF_ALL(remainder)
OCL_SVML_P2_vFvFvF_ALL(fmod)


OCL_SVML_P2_vFvFPI_ALL(frexp,frexp)

OCL_SVML_P2_vFvFPI_ALL(lgamma_r,lgammar)
	
OCL_SVML_P2_vFvFpvF(sincos)
OCL_SVML_P2_vFvFpvF(modf)

OCL_SVML_P3_vFvFvFvF_ALL(fma)

OCL_SVML_P3_vFvFvFpvI(remquo)


// half math
#if defined(__AVX__)
OCL_INTR_P1_vFvF_F816_AS_F8(half_cos)
//OCL_INTR_P2_vFvFvF_ALL_AS_F4(half_divide)
OCL_INTR_P1_vFvF_F816_AS_F8(half_exp)
OCL_INTR_P1_vFvF_F816_AS_F8(half_exp2)
OCL_INTR_P1_vFvF_F816_AS_F8(half_exp10)
OCL_INTR_P1_vFvF_F816_AS_F8(half_log)
OCL_INTR_P1_vFvF_F816_AS_F8(half_log10)
OCL_INTR_P1_vFvF_F816_AS_F8(half_log2)
#else // defined(__AVX__)
OCL_INTR_P1_vFvF_ALL_AS_F1(half_cos)
//OCL_INTR_P2_vFvFvF_ALL_AS_F4(half_divide)
OCL_INTR_P1_vFvF_ALL_AS_F1(half_exp)
OCL_INTR_P1_vFvF_ALL_AS_F1(half_exp2)
OCL_INTR_P1_vFvF_ALL_AS_F1(half_exp10)
OCL_INTR_P1_vFvF_ALL_AS_F1(half_log)
OCL_INTR_P1_vFvF_ALL_AS_F1(half_log10)
OCL_INTR_P1_vFvF_ALL_AS_F1(half_log2)
#endif // defined(__AVX__)

#if defined(__AVX__)
OCL_INTR_P2_vFvFvF_ALL_AS_F8(half_powr)
OCL_INTR_P1_vFvF_F816_AS_F8(half_recip)
OCL_INTR_P1_vFvF_F816_AS_F8(half_rsqrt)
OCL_INTR_P1_vFvF_F816_AS_F8(half_sin)
OCL_INTR_P1_vFvF_F816_AS_F8(half_sqrt)
OCL_INTR_P1_vFvF_F816_AS_F8(half_tan)
#else // defined(__AVX__)
OCL_INTR_P2_vFvFvF_ALL_AS_F4(half_powr)
OCL_INTR_P1_vFvF_ALL_AS_F4(half_recip)
OCL_INTR_P1_vFvF_ALL_AS_F1(half_rsqrt)
OCL_INTR_P1_vFvF_ALL_AS_F1(half_sin)
OCL_INTR_P1_vFvF_ALL_AS_F1(half_sqrt)
OCL_INTR_P1_vFvF_ALL_AS_F1(half_tan)
#endif // defined(__AVX__)


//TODO: add others

// OCL_SVML_P1_vFvF_ALL_MANUAL(half_cos,cos)
// OCL_SVML_P1_vFvF_ALL_MANUAL(half_exp,exp)
// OCL_SVML_P1_vFvF_ALL_MANUAL(half_exp2,exp2)
// OCL_SVML_P1_vFvF_ALL_MANUAL(half_exp10,exp10)
// OCL_SVML_P1_vFvF_ALL_MANUAL(half_log,log)
// OCL_SVML_P1_vFvF_ALL_MANUAL(half_log2,log2)
// OCL_SVML_P1_vFvF_ALL_MANUAL(half_log10,log10)
// OCL_SVML_P1_vFvF_ALL_MANUAL(half_rsqrt,rsqrt)
// OCL_SVML_P1_vFvF_ALL_MANUAL(half_sin,sin)
// OCL_SVML_P1_vFvF_ALL_MANUAL(half_sqrt,sqrt)
// OCL_SVML_P1_vFvF_ALL_MANUAL(half_tan,tan)
// OCL_SVML_P2_vFvFvF_ALL_MANUAL(half_powr,powr)
//TODO: check with ramy

// native math

// OCL_SVML_P1_vFvF_ALL_MANUAL(native_cos,cos)
 OCL_SVML_P2_vFvFvF_ALL_MANUAL(half_divide,div)
// OCL_SVML_P1_vFvF_ALL_MANUAL(native_exp,exp)
// OCL_SVML_P1_vFvF_ALL_MANUAL(native_exp2,exp2)
// OCL_SVML_P1_vFvF_ALL_MANUAL(native_exp10,exp10)
// OCL_SVML_P1_vFvF_ALL_MANUAL(native_log,log)
// OCL_SVML_P1_vFvF_ALL_MANUAL(native_log2,log2)
// OCL_SVML_P1_vFvF_ALL_MANUAL(native_log10,log10)
// OCL_SVML_P2_vFvFvF_ALL_MANUAL(native_powr,powr)
// OCL_SVML_P1_vFvF_ALL_MANUAL(native_recip,recip)
// OCL_INTR_P1_vFvF_ALL_AS_F1(native_rsqrt)
// OCL_SVML_P1_vFvF_ALL_MANUAL(native_sin,sin)
// OCL_INTR_P1_vFvF_ALL_AS_F1(native_sqrt)
// OCL_SVML_P1_vFvF_ALL_MANUAL(native_tan,tan)
//TODO: check with ramy


// native_exp
// treating native_exp as native_exp2 doesn't work, probably
// due to failure by clang compiler
#if defined(__AVX__)
OCL_INTR_P1_vFvF_F816_AS_F8(native_exp)
#else
OCL_INTR_P1_vFvF_ALL_AS_F1(native_exp)				
#endif

OCL_INTR_P1_D1_D1(native_exp)							
OCL_INTR_P1_D2_D2(native_exp)							
__attribute__((overloadable)) double3 native_exp(double3);
__attribute__((overloadable)) double4 native_exp(double4);
__attribute__((overloadable)) double8 native_exp(double8);
__attribute__((overloadable)) double16 native_exp(double16);
//OCL_INTR_P1_vFvF_INTR_vDvD_ALL_AS_F1(native_exp)


#if defined(__AVX__)
OCL_INTR_P1_vFvF_INTR_vDvD_ALL_AS_F8_D4(native_cos)
OCL_INTR_P2_vFvFvF_INTR_vDvDvD_ALL_AS_F8D4(native_divide)
OCL_INTR_P1_vFvF_INTR_vDvD_ALL_AS_F8D4(native_recip)
OCL_INTR_P1_vFvF_INTR_vDvD_ALL_AS_F8_D4(native_rsqrt)
OCL_INTR_P1_vFvF_INTR_vDvD_ALL_AS_F8_D4(native_exp2)
OCL_INTR_P1_vFvF_INTR_vDvD_ALL_AS_F8_D4(native_exp10)
OCL_INTR_P1_vFvF_INTR_vDvD_ALL_AS_F8_D4(native_log)
OCL_INTR_P1_vFvF_INTR_vDvD_ALL_AS_F8_D4(native_log10)
OCL_INTR_P1_vFvF_INTR_vDvD_ALL_AS_F8_D4(native_log2)
OCL_INTR_P2_vFvFvF_INTR_vDvDvD_ALL_AS_F8D4(native_powr)
OCL_INTR_P1_vFvF_INTR_vDvD_ALL_AS_F8_D4(native_sin)
OCL_INTR_P1_vFvF_INTR_vDvD_ALL_AS_F8_D4(native_sqrt)
OCL_INTR_P1_vFvF_INTR_vDvD_ALL_AS_F8_D4(native_tan)
#else // defined(__AVX__)
OCL_INTR_P1_vFvF_INTR_vDvD_ALL_AS_F1(native_cos)
OCL_INTR_P2_vFvFvF_INTR_vDvDvD_ALL_AS_F4(native_divide)
OCL_INTR_P1_vFvF_INTR_vDvD_ALL_AS_F4(native_recip)
OCL_INTR_P1_vFvF_INTR_vDvD_ALL_AS_F1(native_rsqrt)
OCL_INTR_P1_vFvF_INTR_vDvD_ALL_AS_F1(native_exp2)
OCL_INTR_P1_vFvF_INTR_vDvD_ALL_AS_F1(native_exp10)
OCL_INTR_P1_vFvF_INTR_vDvD_ALL_AS_F1(native_log)
OCL_INTR_P1_vFvF_INTR_vDvD_ALL_AS_F1(native_log10)
OCL_INTR_P1_vFvF_INTR_vDvD_ALL_AS_F1(native_log2)
OCL_INTR_P2_vFvFvF_INTR_vDvDvD_ALL_AS_F4(native_powr)
OCL_INTR_P1_vFvF_INTR_vDvD_ALL_AS_F1(native_sin)
OCL_INTR_P1_vFvF_INTR_vDvD_ALL_AS_F1(native_sqrt)
OCL_INTR_P1_vFvF_INTR_vDvD_ALL_AS_F1(native_tan)
#endif // defined(__AVX__)


#if defined(__AVX__)
OCL_INTR_P2_vFvFvF_vDvDvD_ALL_AS_F8(native_fdim)
OCL_INTR_P2_vFvFvF_vDvDvD_ALL_AS_F8(native_fmax)
OCL_INTR_P2_vFvFF_ALL_AS_F8(native_fmax)
OCL_INTR_P2_vFvFvF_vDvDvD_ALL_AS_F8(native_fmin)
OCL_INTR_P2_vFvFF_ALL_AS_F8(native_fmin)
OCL_INTR_P2_vFvFvF_ALL_AS_F8(native_fmod)
OCL_INTR_P2_vFvFpvF_vDvDpvD_F816_AS_F8(native_fract)
OCL_INTR_P2_vFvFvF_INTR_vDvDvD_ALL_AS_F8D4(native_hypot)
#else // defined(__AVX__)
OCL_INTR_P2_vFvFvF_ALL_AS_F4(native_fdim)
OCL_INTR_P2_vFvFvF_ALL_AS_F4(native_fmax)
OCL_INTR_P2_vFvFF_ALL_AS_F4(native_fmax)
OCL_INTR_P2_vFvFvF_ALL_AS_F4(native_fmin)
OCL_INTR_P2_vFvFF_ALL_AS_F4(native_fmin)
OCL_INTR_P2_vFvFvF_ALL_AS_F4(native_fmod)
OCL_INTR_P2_vFvFpvF_vDvDpvD(native_fract)
OCL_INTR_P2_vFvFvF_INTR_vDvDvD_ALL_AS_F4(native_hypot)
#endif // defined(__AVX__)

OCL_INTR_P1_vFvF_ALL_AS_F4(native_logb)
OCL_INTR_P1_vIvF_ALL_AS_F4(native_ilogb)

//relaxed
// OCL_INTR_P2_vFvFvF_ALL_AS_F4(native_divider)
// OCL_INTR_P1_vFvF_ALL_AS_F4(native_recipr)
//TODO: check with ramy


#if defined(__AVX__)
OCL_INTR_P2_vFvFvF_AS_F8_INTR_vDvDvD_ALL_AS_D1(minmag)
OCL_INTR_P2_vFvFvF_AS_F8_INTR_vDvDvD_ALL_AS_D1(maxmag)
#else // defined(__AVX__)
OCL_INTR_P2_vFvFvF_AS_F4_INTR_vDvDvD_ALL_AS_D1(minmag)
OCL_INTR_P2_vFvFvF_AS_F4_INTR_vDvDvD_ALL_AS_D1(maxmag)
#endif // defined(__AVX__)

#ifdef __cplusplus
}
#endif