// INTEL_FEATURE_CPU_RPL
// REQUIRES: intel_feature_cpu_rpl

// RUN: %clang -march=raptorlake -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck -match-full-lines %s -check-prefix=CHECK_RPL_M32
// CHECK_RPL_M32: #define __ADX__ 1
// CHECK_RPL_M32: #define __AES__ 1
// CHECK_RPL_M32: #define __AVX2__ 1
// if INTEL_CUSTOMIZATION
// CHECK_RPL_M32-NOT: __AVX512F__ 1
// end INTEL_CUSTOMIZATION
// CHECK_RPL_M32: #define __AVXVNNI__ 1
// CHECK_RPL_M32: #define __AVX__ 1
// CHECK_RPL_M32: #define __BMI2__ 1
// CHECK_RPL_M32: #define __BMI__ 1
// CHECK_RPL_M32: #define __CLDEMOTE__ 1
// CHECK_RPL_M32: #define __CLFLUSHOPT__ 1
// CHECK_RPL_M32: #define __CLWB__ 1
// CHECK_RPL_M32: #define __F16C__ 1
// CHECK_RPL_M32: #define __FMA__ 1
// CHECK_RPL_M32: #define __FSGSBASE__ 1
// CHECK_RPL_M32: #define __FXSR__ 1
// CHECK_RPL_M32: #define __GFNI__ 1
// CHECK_RPL_M32: #define __HRESET__ 1
// CHECK_RPL_M32: #define __INVPCID__ 1
// CHECK_RPL_M32: #define __KL__ 1
// CHECK_RPL_M32: #define __LZCNT__ 1
// CHECK_RPL_M32: #define __MMX__ 1
// CHECK_RPL_M32: #define __MOVBE__ 1
// CHECK_RPL_M32: #define __MOVDIR64B__ 1
// CHECK_RPL_M32: #define __MOVDIRI__ 1
// CHECK_RPL_M32: #define __PCLMUL__ 1
// CHECK_RPL_M32: #define __PCONFIG__ 1
// CHECK_RPL_M32: #define __PKU__ 1
// CHECK_RPL_M32: #define __POPCNT__ 1
// CHECK_RPL_M32: #define __PRFCHW__ 1
// CHECK_RPL_M32: #define __PTWRITE__ 1
// CHECK_RPL_M32: #define __RDPID__ 1
// CHECK_RPL_M32: #define __RDRND__ 1
// CHECK_RPL_M32: #define __RDSEED__ 1
// CHECK_RPL_M32: #define __SERIALIZE__ 1
// CHECK_RPL_M32: #define __SGX__ 1
// CHECK_RPL_M32: #define __SHA__ 1
// CHECK_RPL_M32: #define __SHSTK__ 1
// CHECK_RPL_M32: #define __SSE2__ 1
// CHECK_RPL_M32: #define __SSE3__ 1
// CHECK_RPL_M32: #define __SSE4_1__ 1
// CHECK_RPL_M32: #define __SSE4_2__ 1
// CHECK_RPL_M32: #define __SSE_MATH__ 1
// CHECK_RPL_M32: #define __SSE__ 1
// CHECK_RPL_M32: #define __SSSE3__ 1
// CHECK_RPL_M32: #define __VAES__ 1
// CHECK_RPL_M32: #define __VPCLMULQDQ__ 1
// CHECK_RPL_M32: #define __WAITPKG__ 1
// CHECK_RPL_M32: #define __WIDEKL__ 1
// CHECK_RPL_M32: #define __XSAVEC__ 1
// CHECK_RPL_M32: #define __XSAVEOPT__ 1
// CHECK_RPL_M32: #define __XSAVES__ 1
// CHECK_RPL_M32: #define __XSAVE__ 1
// CHECK_RPL_M32: #define __corei7 1
// CHECK_RPL_M32: #define __corei7__ 1
// CHECK_RPL_M32: #define __i386 1
// CHECK_RPL_M32: #define __i386__ 1
// CHECK_RPL_M32: #define __tune_corei7__ 1
// CHECK_RPL_M32: #define i386 1

// RUN: %clang -march=raptorlake -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck -match-full-lines %s -check-prefix=CHECK_RPL_M64
// CHECK_RPL_M64: #define __ADX__ 1
// CHECK_RPL_M64: #define __AES__ 1
// CHECK_RPL_M64: #define __AVX2__ 1
// if INTEL_CUSTOMIZATION
// CHECK_RPL_M64-NOT: __AVX512F__ 1
// end INTEL_CUSTOMIZATION
// CHECK_RPL_M64: #define __AVXVNNI__ 1
// CHECK_RPL_M64: #define __AVX__ 1
// CHECK_RPL_M64: #define __BMI2__ 1
// CHECK_RPL_M64: #define __BMI__ 1
// CHECK_RPL_M64: #define __CLDEMOTE__ 1
// CHECK_RPL_M64: #define __CLFLUSHOPT__ 1
// CHECK_RPL_M64: #define __CLWB__ 1
// CHECK_RPL_M64: #define __F16C__ 1
// CHECK_RPL_M64: #define __FMA__ 1
// CHECK_RPL_M64: #define __FSGSBASE__ 1
// CHECK_RPL_M64: #define __FXSR__ 1
// CHECK_RPL_M64: #define __GFNI__ 1
// CHECK_RPL_M64: #define __HRESET__ 1
// CHECK_RPL_M64: #define __INVPCID__ 1
// CHECK_RPL_M64: #define __KL__ 1
// CHECK_RPL_M64: #define __LZCNT__ 1
// CHECK_RPL_M64: #define __MMX__ 1
// CHECK_RPL_M64: #define __MOVBE__ 1
// CHECK_RPL_M64: #define __MOVDIR64B__ 1
// CHECK_RPL_M64: #define __MOVDIRI__ 1
// CHECK_RPL_M64: #define __PCLMUL__ 1
// CHECK_RPL_M64: #define __PCONFIG__ 1
// CHECK_RPL_M64: #define __PKU__ 1
// CHECK_RPL_M64: #define __POPCNT__ 1
// CHECK_RPL_M64: #define __PRFCHW__ 1
// CHECK_RPL_M64: #define __PTWRITE__ 1
// CHECK_RPL_M64: #define __RDPID__ 1
// CHECK_RPL_M64: #define __RDRND__ 1
// CHECK_RPL_M64: #define __RDSEED__ 1
// CHECK_RPL_M64: #define __SERIALIZE__ 1
// CHECK_RPL_M64: #define __SGX__ 1
// CHECK_RPL_M64: #define __SHA__ 1
// CHECK_RPL_M64: #define __SHSTK__ 1
// CHECK_RPL_M64: #define __SSE2_MATH__ 1
// CHECK_RPL_M64: #define __SSE2__ 1
// CHECK_RPL_M64: #define __SSE3__ 1
// CHECK_RPL_M64: #define __SSE4_1__ 1
// CHECK_RPL_M64: #define __SSE4_2__ 1
// CHECK_RPL_M64: #define __SSE_MATH__ 1
// CHECK_RPL_M64: #define __SSE__ 1
// CHECK_RPL_M64: #define __SSSE3__ 1
// CHECK_RPL_M64: #define __VAES__ 1
// CHECK_RPL_M64: #define __VPCLMULQDQ__ 1
// CHECK_RPL_M64: #define __WAITPKG__ 1
// CHECK_RPL_M64: #define __WIDEKL__ 1
// CHECK_RPL_M64: #define __XSAVEC__ 1
// CHECK_RPL_M64: #define __XSAVEOPT__ 1
// CHECK_RPL_M64: #define __XSAVES__ 1
// CHECK_RPL_M64: #define __XSAVE__ 1
// CHECK_RPL_M64: #define __amd64 1
// CHECK_RPL_M64: #define __amd64__ 1
// CHECK_RPL_M64: #define __corei7 1
// CHECK_RPL_M64: #define __corei7__ 1
// CHECK_RPL_M64: #define __tune_corei7__ 1
// CHECK_RPL_M64: #define __x86_64 1
// CHECK_RPL_M64: #define __x86_64__ 1

// end INTEL_FEATURE_CPU_RPL
