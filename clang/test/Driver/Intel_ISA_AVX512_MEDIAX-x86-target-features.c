// INTEL_FEATURE_ISA_AVX512_MEDIAX
// REQUIRES: intel_feature_isa_avx512_mediax
// RUN: %clang -target x86_64-unknown-unknown -mavx512mediax %s -### -o %t.o 2>&1 | FileCheck -check-prefix=AVX512mediax %s
// RUN: %clang -target x86_64-unknown-unknown -mno-avx512mediax %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVX512mediax %s
// AVX512mediax: "-target-feature" "+avx512mediax"
// NO-AVX512mediax: "-target-feature" "-avx512mediax"
// end INTEL_FEATURE_ISA_AVX512_MEDIAX
