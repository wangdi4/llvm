#if INTEL_FEATURE_ISA_AVX512_VNNI_FP8
// REQUIRES: intel_feature_isa_avx512_vnni_fp8

// RUN: %clang -target i686-unknown-linux-gnu -mavx512vnnifp8 %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AVX512VNNIFP8 %s
// RUN: %clang -target i686-unknown-linux-gnu -mno-avx512vnnifp8 %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVX512VNNIFP8 %s
// AVX512VNNIFP8: "-target-feature" "+avx512vnnifp8"
// NO-AVX512VNNIFP8: "-target-feature" "-avx512vnnifp8"
#endif // INTEL_FEATURE_ISA_AVX512_VNNI_FP8