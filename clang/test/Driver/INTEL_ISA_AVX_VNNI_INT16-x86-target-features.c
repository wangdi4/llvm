#if INTEL_FEATURE_ISA_AVX_VNNI_INT16
// REQUIRES: intel_feature_isa_avx_vnni_int16

// RUN: %clang -target i686-unknown-linux-gnu -mavxvnniint16 %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AVXVNNIINT16 %s
// RUN: %clang -target i686-unknown-linux-gnu -mno-avxvnniint16 %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVXVNNIINT16 %s
// AVXVNNIINT16: "-target-feature" "+avxvnniint16"
// NO-AVXVNNIINT16: "-target-feature" "-avxvnniint16"
#endif // INTEL_FEATURE_ISA_AVX_VNNI_INT16