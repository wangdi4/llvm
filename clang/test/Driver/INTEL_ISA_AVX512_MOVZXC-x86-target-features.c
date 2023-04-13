#if INTEL_FEATURE_ISA_AVX512_MOVZXC
// REQUIRES: intel_feature_isa_avx512_movzxc

// RUN: %clang -target i686-unknown-linux-gnu -mavx512movzxc %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AVX512MOVZXC %s
// RUN: %clang -target i686-unknown-linux-gnu -mno-avx512movzxc %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVX512MOVZXC %s
// AVX512MOVZXC: "-target-feature" "+avx512movzxc"
// NO-AVX512MOVZXC: "-target-feature" "-avx512movzxc"
#endif // INTEL_FEATURE_ISA_AVX512_MOVZXC
