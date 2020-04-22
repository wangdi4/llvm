#if INTEL_FEATURE_ISA_AVX512_DOTPROD
// REQUIRES: intel_feature_isa_avx512_dotprod

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mavx512dotprod %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AVX512-DOTPROD %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-avx512dotprod \
// RUN: %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVX512-DOTPROD %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-avx512dotprod \
// RUN: %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVX512-DOTPROD %s
// AVX512-DOTPROD: "-target-feature" "+avx512dotprod"
// NO-AVX512-DOTPROD: "-target-feature" "-avx512dotprod"

#endif // INTEL_FEATURE_ISA_AVX512_DOTPROD
