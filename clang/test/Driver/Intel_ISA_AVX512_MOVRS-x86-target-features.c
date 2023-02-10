// INTEL_FEATURE_ISA_AVX512_MOVRS
// REQUIRES: intel_feature_isa_avx512_movrs
// RUN: %clang -target x86_64-unknown-unknown -mavxmovrs %s -### -o %t.o 2>&1 | FileCheck -check-prefix=AVXMOVRS %s
// RUN: %clang -target x86_64-unknown-unknown -mno-avxmovrs %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVXMOVRS %s
// AVXMOVRS: "-target-feature" "+avxmovrs"
// NO-AVXMOVRS: "-target-feature" "-avxmovrs"

// RUN: %clang -target x86_64-unknown-unknown -mavx512movrs %s -### -o %t.o 2>&1 | FileCheck -check-prefix=AVX512MOVRS %s
// RUN: %clang -target x86_64-unknown-unknown -mno-avx512movrs %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVX512MOVRS %s
// AVX512MOVRS: "-target-feature" "+avx512movrs"
// NO-AVX512MOVRS: "-target-feature" "-avx512movrs"
// end INTEL_FEATURE_ISA_AVX512_MOVRS
