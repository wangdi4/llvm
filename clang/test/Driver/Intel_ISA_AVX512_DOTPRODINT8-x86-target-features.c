#if INTEL_FEATURE_ISA_AVX512_DOTPROD_INT8
// REQUIRES: intel_feature_isa_avx512_dotprod_int8

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mavx512dotprodint8 %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AVX512-DOTPRODINT8 %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-avx512dotprodint8 \
// RUN: %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVX512-DOTPRODINT8 %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-avx512dotprodint8 \
// RUN: %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVX512-DOTPRODINT8 %s
// AVX512-DOTPRODINT8: "-target-feature" "+avx512dotprodint8"
// NO-AVX512-DOTPRODINT8: "-target-feature" "-avx512dotprodint8"

#endif // INTEL_FEATURE_ISA_AVX512_DOTPROD_INT8
