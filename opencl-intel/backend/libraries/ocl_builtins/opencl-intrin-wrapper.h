#ifndef __OPENCL__
#error "This header should only be used by OpenCL program"
#endif

#ifndef __OPENCL_INTRIN_WRAPPER_H__
#define __OPENCL_INTRIN_WRAPPER_H__

typedef long __ocl_i64;
typedef unsigned long __ocl_u64;

// Undef __STDC_HOSTED__ to avoid including <mm_malloc.h> from <xmmintrin.h>
#ifdef __STDC_HOSTED__
#define __RECOVER_STDC_HOSTED_MACRO__ __STDC_HOSTED__
#undef __STDC_HOSTED__
#endif

#ifdef __DSPV1_SUPPORTED__
#define __RECOVER_DSPV1_SUPPORTED__ __DSPV1_SUPPORTED__
#undef __DSPV1_SUPPORTED__
#endif

#pragma clang diagnostic push
// Suppress warning of <ceintrin.h>
#pragma clang diagnostic ignored "-Wuninitialized"
// Suppress warning of <avxvnniint16/avxvnniint16intrin.h>
#pragma clang diagnostic ignored "-Wcomment"
// Suppress warning of <Intel_amxlncintrin.h>
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wmacro-redefined"

#include <immintrin.h>

#pragma clang diagnostic pop

#ifdef __RECOVER_DSPV1_SUPPORTED__
#define __DSPV1_SUPPORTED__ __RECOVER_DSPV1_SUPPORTED__
#undef __RECOVER_DSPV1_SUPPORTED__
#endif

#ifdef __RECOVER_STDC_HOSTED_MACRO__
#define __STDC_HOSTED__ __RECOVER_STDC_HOSTED_MACRO__
#undef __RECOVER_STDC_HOSTED_MACRO__
#endif

#endif // __OPENCL_INTRIN_WRAPPER_H__
