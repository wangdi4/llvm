#if INTEL_FEATURE_ISA_AVX512_RAO_FP
// REQUIRES: intel_feature_isa_avx512_rao_fp

// RUN: %clang -target i686-unknown-linux-gnu -mavx512raofp %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AVX512RAOFP %s
// RUN: %clang -target i686-unknown-linux-gnu -mno-avx512raofp %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVX512RAOFP %s
// AVX512RAOFP: "-target-feature" "+avx512raofp"
// NO-AVX512RAOFP: "-target-feature" "-avx512raofp"
#endif // INTEL_FEATURE_ISA_AVX512_RAO_FP