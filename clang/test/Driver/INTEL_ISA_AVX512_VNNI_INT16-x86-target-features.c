#if INTEL_FEATURE_ISA_AVX512_VNNI_INT16
// REQUIRES: intel_feature_isa_avx512_vnni_int16

// RUN: %clang -target i686-unknown-linux-gnu -mavx512vnniint16 %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AVX512VNNIINT16 %s
// RUN: %clang -target i686-unknown-linux-gnu -mno-avx512vnniint16 %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVX512VNNIINT16 %s
// AVX512VNNIINT16: "-target-feature" "+avx512vnniint16"
// NO-AVX512VNNIINT16: "-target-feature" "-avx512vnniint16"
#endif // INTEL_FEATURE_ISA_AVX512_VNNI_INT16