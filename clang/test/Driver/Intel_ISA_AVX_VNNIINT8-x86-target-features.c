#if INTEL_FEATURE_ISA_AVX_VNNI_INT8
// REQUIRES: intel_feature_isa_avx_vnni_int8

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mavxvnniint8 %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AVX-VNNIINT8 %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-avxvnniint8 \
// RUN: %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVX-VNNIINT8 %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-avxvnniint8 \
// RUN: %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVX-VNNIINT8 %s
// AVX-VNNIINT8: "-target-feature" "+avxvnniint8"
// NO-AVX-VNNIINT8: "-target-feature" "-avxvnniint8"

#endif // INTEL_FEATURE_ISA_AVX_VNNI_INT8
