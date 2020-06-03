#if INTEL_FEATURE_ISA_AVX512_DOTPROD_PHPS
// REQUIRES: intel_feature_isa_avx512_dotprod_phps

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mavx512dotprodphps %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AVX512-DOTPRODPHPS %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-avx512dotprodphps \
// RUN: %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVX512-DOTPRODPHPS %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-avx512dotprodphps \
// RUN: %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVX512-DOTPRODPHPS %s
// AVX512-DOTPRODPHPS: "-target-feature" "+avx512dotprodphps"
// NO-AVX512-DOTPRODPHPS: "-target-feature" "-avx512dotprodphps"

#endif // INTEL_FEATURE_ISA_AVX512_DOTPROD_PHPS
