#ifndef __CONVERSIONS_WORKAROUND_H__
#define __CONVERSIONS_WORKAROUND_H__

#define JOIN2(A,B) JOIN_IMP(A,B)
#define JOIN_IMP(A,B) A##B
#define JOIN3(A,B,C) JOIN2(A,JOIN2(B,C))

#ifdef _WIN32
#ifdef CL_BUILTIN_FUNCTIONS_EXPORTS
#define CONVERSIONS_FUNC_DECL __declspec(dllexport)
#else
#define CONVERSIONS_FUNC_DECL __declspec(dllimport)
#endif
#else
#define CONVERSIONS_FUNC_DECL 
#endif

// Problem: Some SVML function expect their parmater to be passed in EAX.
// We have no support for this calling convension, so we create a wrapper
// function which compiled with ICC. ICC support #pragma linkage constructs which
// enables using custom calling conventions.


/// This function is used instead of directly calling __ocl_svml_*8_cvtu32tofprtn1
/// The svml function uses a special calling convention which passes the argument in EAX.
/// We currently do not support this CC.

// This macro declares the wrapper protoype.
#define CONVERT_DOUBLE_WRAPPER_FUNCNAME(RSVML,CPUTYPE,SIGN) \
    wrapper_convert_##SIGN##_double_##CPUTYPE##_##RSVML

#define DEF_INT_PROTOD_U32_WRAPPER_DECL(RMODE,RSVML,CPUTYPE) \
    double CONVERT_DOUBLE_WRAPPER_FUNCNAME(RSVML,CPUTYPE,u)(_1u32 x);

#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
#define DEF_INT_PROTOD_U32_WRAPPER(RMODE,RSVML,CPUTYPE) DEF_INT_PROTOD_U32_WRAPPER_DO(RMODE,RSVML,CPUTYPE)
#define DEF_INT_PROTOD_U32_WRAPPER_DO(RMODE,RSVML,CPUTYPE) \
    extern "C" {\
    CONVERSIONS_FUNC_DECL double CONVERT_DOUBLE_WRAPPER_FUNCNAME(RSVML,CPUTYPE,u)(_1u32 x)\
	{\
    double res = __ocl_svml_##CPUTYPE##_cvtu32tofp##RSVML##1(x);\
	return res;\
	}\
    };
#endif

// Instantiation starts here...

#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
// Snipped from __ocl_svml_ia32e.h
// TODO: make it possible to #include this file instead of copy-pasting
#ifdef __AVX__
#if defined(_WIN64) || defined(__x86_64__)
CONVERSIONS_FUNC_DECL double __ocl_svml_e9_cvtu32tofprtn1 (_1u32 a);
#pragma linkage     __ocl_svml_e9_cvtu32tofprtn1_linkage   = ( result(xmm0) parameters(eax)  )
#pragma use_linkage __ocl_svml_e9_cvtu32tofprtn1_linkage ( __ocl_svml_e9_cvtu32tofprtn1 )

CONVERSIONS_FUNC_DECL double __ocl_svml_e9_cvtu32tofpup1 (_1u32 a);
#pragma linkage     __ocl_svml_e9_cvtu32tofpup1_linkage   = ( result(xmm0) parameters(eax)  )
#pragma use_linkage __ocl_svml_e9_cvtu32tofpup1_linkage ( __ocl_svml_e9_cvtu32tofpup1 )

CONVERSIONS_FUNC_DECL double __ocl_svml_e9_cvtu32tofpdown1 (_1u32 a);
#pragma linkage     __ocl_svml_e9_cvtu32tofpdown1_linkage   = ( result(xmm0) parameters(eax)  )
#pragma use_linkage __ocl_svml_e9_cvtu32tofpdown1_linkage ( __ocl_svml_e9_cvtu32tofpdown1 )

CONVERSIONS_FUNC_DECL double __ocl_svml_e9_cvtu32tofprtz1 (_1u32 a);
#pragma linkage     __ocl_svml_e9_cvtu32tofprtz1_linkage   = ( result(xmm0) parameters(eax)  )
#pragma use_linkage __ocl_svml_e9_cvtu32tofprtz1_linkage ( __ocl_svml_e9_cvtu32tofprtz1 )

#else //defined(_WIN64) || defined(__x86_64__)
CONVERSIONS_FUNC_DECL double __ocl_svml_g9_cvtu32tofprtn1 (_1u32 a);
#pragma linkage     __ocl_svml_g9_cvtu32tofprtn1_linkage   = ( result(xmm0) parameters(eax)  )
#pragma use_linkage __ocl_svml_g9_cvtu32tofprtn1_linkage ( __ocl_svml_g9_cvtu32tofprtn1 )

CONVERSIONS_FUNC_DECL double __ocl_svml_g9_cvtu32tofpup1 (_1u32 a);
#pragma linkage     __ocl_svml_g9_cvtu32tofpup1_linkage   = ( result(xmm0) parameters(eax)  )
#pragma use_linkage __ocl_svml_g9_cvtu32tofpup1_linkage ( __ocl_svml_g9_cvtu32tofpup1 )

CONVERSIONS_FUNC_DECL double __ocl_svml_g9_cvtu32tofpdown1 (_1u32 a);
#pragma linkage     __ocl_svml_g9_cvtu32tofpdown1_linkage   = ( result(xmm0) parameters(eax)  )
#pragma use_linkage __ocl_svml_g9_cvtu32tofpdown1_linkage ( __ocl_svml_g9_cvtu32tofpdown1 )

CONVERSIONS_FUNC_DECL double __ocl_svml_g9_cvtu32tofprtz1 (_1u32 a);
#pragma linkage     __ocl_svml_g9_cvtu32tofprtz1_linkage   = ( result(xmm0) parameters(eax)  )
#pragma use_linkage __ocl_svml_g9_cvtu32tofprtz1_linkage ( __ocl_svml_g9_cvtu32tofprtz1 )
#endif // defined(_WIN64) || defined(__x86_64__)
#elif defined(__SSE4_2__)
 #if defined(_WIN64) || defined(__x86_64__)  
CONVERSIONS_FUNC_DECL double __ocl_svml_h8_cvtu32tofprtn1 (_1u32 a);
#pragma linkage     __ocl_svml_h8_cvtu32tofprtn1_linkage   = ( result(xmm0) parameters(eax)  )
#pragma use_linkage __ocl_svml_h8_cvtu32tofprtn1_linkage ( __ocl_svml_h8_cvtu32tofprtn1 )

CONVERSIONS_FUNC_DECL double __ocl_svml_h8_cvtu32tofpup1 (_1u32 a);
#pragma linkage     __ocl_svml_h8_cvtu32tofpup1_linkage   = ( result(xmm0) parameters(eax)  )
#pragma use_linkage __ocl_svml_h8_cvtu32tofpup1_linkage ( __ocl_svml_h8_cvtu32tofpup1 )

CONVERSIONS_FUNC_DECL double __ocl_svml_h8_cvtu32tofpdown1 (_1u32 a);
#pragma linkage     __ocl_svml_h8_cvtu32tofpdown1_linkage   = ( result(xmm0) parameters(eax)  )
#pragma use_linkage __ocl_svml_h8_cvtu32tofpdown1_linkage ( __ocl_svml_h8_cvtu32tofpdown1 )

CONVERSIONS_FUNC_DECL double __ocl_svml_h8_cvtu32tofprtz1 (_1u32 a);
#pragma linkage     __ocl_svml_h8_cvtu32tofprtz1_linkage   = ( result(xmm0) parameters(eax)  )
#pragma use_linkage __ocl_svml_h8_cvtu32tofprtz1_linkage ( __ocl_svml_h8_cvtu32tofprtz1 )

 #else //defined(_WIN64) || defined(__x86_64__)
    CONVERSIONS_FUNC_DECL double __ocl_svml_n8_cvtu32tofprtn1 (_1u32 a);
#pragma linkage     __ocl_svml_n8_cvtu32tofprtn1_linkage   = ( result(xmm0) parameters(eax)  )
#pragma use_linkage __ocl_svml_n8_cvtu32tofprtn1_linkage ( __ocl_svml_n8_cvtu32tofprtn1 )

    CONVERSIONS_FUNC_DECL double __ocl_svml_n8_cvtu32tofpup1 (_1u32 a);
#pragma linkage     __ocl_svml_n8_cvtu32tofpup1_linkage   = ( result(xmm0) parameters(eax)  )
#pragma use_linkage __ocl_svml_n8_cvtu32tofpup1_linkage ( __ocl_svml_n8_cvtu32tofpup1 )

    CONVERSIONS_FUNC_DECL double __ocl_svml_n8_cvtu32tofpdown1 (_1u32 a);
#pragma linkage     __ocl_svml_n8_cvtu32tofpdown1_linkage   = ( result(xmm0) parameters(eax)  )
#pragma use_linkage __ocl_svml_n8_cvtu32tofpdown1_linkage ( __ocl_svml_n8_cvtu32tofpdown1 )

    CONVERSIONS_FUNC_DECL double __ocl_svml_n8_cvtu32tofprtz1 (_1u32 a);
#pragma linkage     __ocl_svml_n8_cvtu32tofprtz1_linkage   = ( result(xmm0) parameters(eax)  )
#pragma use_linkage __ocl_svml_n8_cvtu32tofprtz1_linkage ( __ocl_svml_n8_cvtu32tofprtz1 )
 #endif //defined(_WIN64) || defined(__x86_64__)
#elif defined(__SSE4_1__)
 #if defined(_WIN64) || defined(__x86_64__)  
    CONVERSIONS_FUNC_DECL double __ocl_svml_y8_cvtu32tofprtn1 (_1u32 a);
#pragma linkage     __ocl_svml_y8_cvtu32tofprtn1_linkage   = ( result(xmm0) parameters(eax)  )
#pragma use_linkage __ocl_svml_y8_cvtu32tofprtn1_linkage ( __ocl_svml_y8_cvtu32tofprtn1 )

    CONVERSIONS_FUNC_DECL double __ocl_svml_y8_cvtu32tofpup1 (_1u32 a);
#pragma linkage     __ocl_svml_y8_cvtu32tofpup1_linkage   = ( result(xmm0) parameters(eax)  )
#pragma use_linkage __ocl_svml_y8_cvtu32tofpup1_linkage ( __ocl_svml_y8_cvtu32tofpup1 )

    CONVERSIONS_FUNC_DECL double __ocl_svml_y8_cvtu32tofpdown1 (_1u32 a);
#pragma linkage     __ocl_svml_y8_cvtu32tofpdown1_linkage   = ( result(xmm0) parameters(eax)  )
#pragma use_linkage __ocl_svml_y8_cvtu32tofpdown1_linkage ( __ocl_svml_y8_cvtu32tofpdown1 )

    CONVERSIONS_FUNC_DECL double __ocl_svml_y8_cvtu32tofprtz1 (_1u32 a);
#pragma linkage     __ocl_svml_y8_cvtu32tofprtz1_linkage   = ( result(xmm0) parameters(eax)  )
#pragma use_linkage __ocl_svml_y8_cvtu32tofprtz1_linkage ( __ocl_svml_y8_cvtu32tofprtz1 )
  #else //defined(_WIN64) || defined(__x86_64__)
    CONVERSIONS_FUNC_DECL double __ocl_svml_p8_cvtu32tofprtn1 (_1u32 a);
#pragma linkage     __ocl_svml_p8_cvtu32tofprtn1_linkage   = ( result(xmm0) parameters(eax)  )
#pragma use_linkage __ocl_svml_p8_cvtu32tofprtn1_linkage ( __ocl_svml_p8_cvtu32tofprtn1 )

    CONVERSIONS_FUNC_DECL double __ocl_svml_p8_cvtu32tofpup1 (_1u32 a);
#pragma linkage     __ocl_svml_p8_cvtu32tofpup1_linkage   = ( result(xmm0) parameters(eax)  )
#pragma use_linkage __ocl_svml_p8_cvtu32tofpup1_linkage ( __ocl_svml_p8_cvtu32tofpup1 )

    CONVERSIONS_FUNC_DECL double __ocl_svml_p8_cvtu32tofpdown1 (_1u32 a);
#pragma linkage     __ocl_svml_p8_cvtu32tofpdown1_linkage   = ( result(xmm0) parameters(eax)  )
#pragma use_linkage __ocl_svml_p8_cvtu32tofpdown1_linkage ( __ocl_svml_p8_cvtu32tofpdown1 )

    CONVERSIONS_FUNC_DECL double __ocl_svml_p8_cvtu32tofprtz1 (_1u32 a);
#pragma linkage     __ocl_svml_p8_cvtu32tofprtz1_linkage   = ( result(xmm0) parameters(eax)  )
#pragma use_linkage __ocl_svml_p8_cvtu32tofprtz1_linkage ( __ocl_svml_p8_cvtu32tofprtz1 )
 #endif
#elif defined(__SSSE3__)
  #if defined(_WIN64) || defined(__x86_64__)
    CONVERSIONS_FUNC_DECL double __ocl_svml_u8_cvtu32tofprtn1 (_1u32 a);
#pragma linkage     __ocl_svml_u8_cvtu32tofprtn1_linkage   = ( result(xmm0) parameters(eax)  )
#pragma use_linkage __ocl_svml_u8_cvtu32tofprtn1_linkage ( __ocl_svml_u8_cvtu32tofprtn1 )

    CONVERSIONS_FUNC_DECL double __ocl_svml_u8_cvtu32tofpup1 (_1u32 a);
#pragma linkage     __ocl_svml_u8_cvtu32tofpup1_linkage   = ( result(xmm0) parameters(eax)  )
#pragma use_linkage __ocl_svml_u8_cvtu32tofpup1_linkage ( __ocl_svml_u8_cvtu32tofpup1 )

    CONVERSIONS_FUNC_DECL double __ocl_svml_u8_cvtu32tofpdown1 (_1u32 a);
#pragma linkage     __ocl_svml_u8_cvtu32tofpdown1_linkage   = ( result(xmm0) parameters(eax)  )
#pragma use_linkage __ocl_svml_u8_cvtu32tofpdown1_linkage ( __ocl_svml_u8_cvtu32tofpdown1 )

    CONVERSIONS_FUNC_DECL double __ocl_svml_u8_cvtu32tofprtz1 (_1u32 a);
#pragma linkage     __ocl_svml_u8_cvtu32tofprtz1_linkage   = ( result(xmm0) parameters(eax)  )
#pragma use_linkage __ocl_svml_u8_cvtu32tofprtz1_linkage ( __ocl_svml_u8_cvtu32tofprtz1 )
 #else //defined(_WIN64) || defined(__x86_64__)
CONVERSIONS_FUNC_DECL double __ocl_svml_v8_cvtu32tofprtn1 (_1u32 a);
#pragma linkage     __ocl_svml_v8_cvtu32tofprtn1_linkage   = ( result(xmm0) parameters(eax)  )
#pragma use_linkage __ocl_svml_v8_cvtu32tofprtn1_linkage ( __ocl_svml_v8_cvtu32tofprtn1 )

CONVERSIONS_FUNC_DECL double __ocl_svml_v8_cvtu32tofpup1 (_1u32 a);
#pragma linkage     __ocl_svml_v8_cvtu32tofpup1_linkage   = ( result(xmm0) parameters(eax)  )
#pragma use_linkage __ocl_svml_v8_cvtu32tofpup1_linkage ( __ocl_svml_v8_cvtu32tofpup1 )

CONVERSIONS_FUNC_DECL double __ocl_svml_v8_cvtu32tofpdown1 (_1u32 a);
#pragma linkage     __ocl_svml_v8_cvtu32tofpdown1_linkage   = ( result(xmm0) parameters(eax)  )
#pragma use_linkage __ocl_svml_v8_cvtu32tofpdown1_linkage ( __ocl_svml_v8_cvtu32tofpdown1 )

CONVERSIONS_FUNC_DECL double __ocl_svml_v8_cvtu32tofprtz1 (_1u32 a);
#pragma linkage     __ocl_svml_v8_cvtu32tofprtz1_linkage   = ( result(xmm0) parameters(eax)  )
#pragma use_linkage __ocl_svml_v8_cvtu32tofprtz1_linkage ( __ocl_svml_v8_cvtu32tofprtz1 )
 #endif
#elif defined(__SSE3__)
#if defined(_WIN64) || defined(__x86_64__)
CONVERSIONS_FUNC_DECL double __ocl_svml_e7_cvtu32tofprtn1 (_1u32 a);
#pragma linkage     __ocl_svml_e7_cvtu32tofprtn1_linkage   = ( result(xmm0) parameters(eax)  )
#pragma use_linkage __ocl_svml_e7_cvtu32tofprtn1_linkage ( __ocl_svml_e7_cvtu32tofprtn1 )

CONVERSIONS_FUNC_DECL double __ocl_svml_e7_cvtu32tofpup1 (_1u32 a);
#pragma linkage     __ocl_svml_e7_cvtu32tofpup1_linkage   = ( result(xmm0) parameters(eax)  )
#pragma use_linkage __ocl_svml_e7_cvtu32tofpup1_linkage ( __ocl_svml_e7_cvtu32tofpup1 )

CONVERSIONS_FUNC_DECL double __ocl_svml_e7_cvtu32tofpdown1 (_1u32 a);
#pragma linkage     __ocl_svml_e7_cvtu32tofpdown1_linkage   = ( result(xmm0) parameters(eax)  )
#pragma use_linkage __ocl_svml_e7_cvtu32tofpdown1_linkage ( __ocl_svml_e7_cvtu32tofpdown1 )

CONVERSIONS_FUNC_DECL double __ocl_svml_e7_cvtu32tofprtz1 (_1u32 a);
#pragma linkage     __ocl_svml_e7_cvtu32tofprtz1_linkage   = ( result(xmm0) parameters(eax)  )
#pragma use_linkage __ocl_svml_e7_cvtu32tofprtz1_linkage ( __ocl_svml_e7_cvtu32tofprtz1 )

 #else //defined(_WIN64) || defined(__x86_64__)
    CONVERSIONS_FUNC_DECL double __ocl_svml_t7_cvtu32tofprtn1 (_1u32 a);
#pragma linkage     __ocl_svml_t7_cvtu32tofprtn1_linkage   = ( result(xmm0) parameters(eax)  )
#pragma use_linkage __ocl_svml_t7_cvtu32tofprtn1_linkage ( __ocl_svml_t7_cvtu32tofprtn1 )

    CONVERSIONS_FUNC_DECL double __ocl_svml_t7_cvtu32tofpup1 (_1u32 a);
#pragma linkage     __ocl_svml_t7_cvtu32tofpup1_linkage   = ( result(xmm0) parameters(eax)  )
#pragma use_linkage __ocl_svml_t7_cvtu32tofpup1_linkage ( __ocl_svml_t7_cvtu32tofpup1 )

    CONVERSIONS_FUNC_DECL double __ocl_svml_t7_cvtu32tofpdown1 (_1u32 a);
#pragma linkage     __ocl_svml_t7_cvtu32tofpdown1_linkage   = ( result(xmm0) parameters(eax)  )
#pragma use_linkage __ocl_svml_t7_cvtu32tofpdown1_linkage ( __ocl_svml_t7_cvtu32tofpdown1 )

    CONVERSIONS_FUNC_DECL double __ocl_svml_t7_cvtu32tofprtz1 (_1u32 a);
#pragma linkage     __ocl_svml_t7_cvtu32tofprtz1_linkage   = ( result(xmm0) parameters(eax)  )
#pragma use_linkage __ocl_svml_t7_cvtu32tofprtz1_linkage ( __ocl_svml_t7_cvtu32tofprtz1 )
#endif
#else
#error unknown arhcitecture
#endif //CTYPE

DEF_INT_PROTOD_U32_WRAPPER(,rtn,CTYPE)
DEF_INT_PROTOD_U32_WRAPPER(,up,CTYPE)
DEF_INT_PROTOD_U32_WRAPPER(,down,CTYPE)
DEF_INT_PROTOD_U32_WRAPPER(,rtz,CTYPE)

#endif //_MSC_VER


#endif
