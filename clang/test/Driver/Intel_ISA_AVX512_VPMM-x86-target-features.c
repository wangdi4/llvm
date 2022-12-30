#if INTEL_FEATURE_ISA_AVX512_VPMM
// REQUIRES: intel_feature_isa_avx512_vpmm

// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86_64 -mavx512vpmm %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AVX512-VPMM %s
// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86_64 -mno-avx512vpmm \
// RUN: %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVX512-VPMM %s
// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86_64 -mno-avx512vpmm \
// RUN: %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVX512-VPMM %s
// AVX512-VPMM: "-target-feature" "+avx512vpmm"
// NO-AVX512-VPMM: "-target-feature" "-avx512vpmm"

#endif // INTEL_FEATURE_ISA_AVX512_VPMM
