#if INTEL_FEATURE_ISA_AVX_DOTPROD_INT8
// REQUIRES: intel_feature_isa_avx_dotprod_int8

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mavxdotprodint8 %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AVX-DOTPRODINT8 %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-avxdotprodint8 \
// RUN: %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVX-DOTPRODINT8 %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-avxdotprodint8 \
// RUN: %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVX-DOTPRODINT8 %s
// AVX-DOTPRODINT8: "-target-feature" "+avxdotprodint8"
// NO-AVX-DOTPRODINT8: "-target-feature" "-avxdotprodint8"

#endif // INTEL_FEATURE_ISA_AVX_DOTPROD_INT8
