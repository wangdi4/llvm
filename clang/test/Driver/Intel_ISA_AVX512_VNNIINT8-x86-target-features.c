#if INTEL_FEATURE_ISA_AVX512_VNNI_INT8
// REQUIRES: intel_feature_isa_avx512_vnni_int8

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mavx512vnniint8 %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AVX512-VNNIINT8 %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-avx512vnniint8 \
// RUN: %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVX512-VNNIINT8 %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-avx512vnniint8 \
// RUN: %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVX512-VNNIINT8 %s
// AVX512-VNNIINT8: "-target-feature" "+avx512vnniint8"
// NO-AVX512-VNNIINT8: "-target-feature" "-avx512vnniint8"

#endif // INTEL_FEATURE_ISA_AVX512_VNNI_INT8
