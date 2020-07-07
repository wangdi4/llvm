#if INTEL_FEATURE_ISA_AVX_COMPRESS
// REQUIRES: intel_feature_isa_avx_compress
// RUN: %clang -target x86_64-unknown-unknown -march=atom -mavxcompress -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVXCOMPRESS %s

// AVXCOMPRESS: #define __AVX2__ 1
// AVXCOMPRESS: #define __AVXCOMPRESS__ 1

// RUN: %clang -target x86_64-unknown-unknown -march=atom -mavxcompress -mno-avx2 -x c -E -dM -o - %s | FileCheck -match-full-lines --check-prefix=AVXCOMPRESSNOAVX2 %s

// AVXCOMPRESSNOAVX2-NOT: #define __AVX2__ 1
// AVXCOMPRESSNOAVX2-NOT: #define __AVXCOMPRESS__ 1

#endif // INTEL_FEATURE_ISA_AVX_COMPRESS
