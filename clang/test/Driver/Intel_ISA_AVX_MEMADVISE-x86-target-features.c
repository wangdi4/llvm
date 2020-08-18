// INTEL_FEATURE_ISA_AVX_MEMADVISE
// REQUIRES: intel_feature_isa_avx_memadvise
// RUN: %clang -target x86_64-unknown-unknown -mavxmemadvise %s -### -o %t.o 2>&1 | FileCheck -check-prefix=AVXMEMADVISE %s
// RUN: %clang -target x86_64-unknown-unknown -mno-avxmemadvise %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVXMEMADVISE %s
// AVXMEMADVISE: "-target-feature" "+avxmemadvise"
// NO-AVXMEMADVISE: "-target-feature" "-avxmemadvise"

// RUN: %clang -target x86_64-unknown-unknown -mavx512memadvise %s -### -o %t.o 2>&1 | FileCheck -check-prefix=AVX512MEMADVISE %s
// RUN: %clang -target x86_64-unknown-unknown -mno-avx512memadvise %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVX512MEMADVISE %s
// AVX512MEMADVISE: "-target-feature" "+avx512memadvise"
// NO-AVX512MEMADVISE: "-target-feature" "-avx512memadvise"
// end INTEL_FEATURE_ISA_AVX_MEMADVISE
