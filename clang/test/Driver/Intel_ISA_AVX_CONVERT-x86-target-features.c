#if INTEL_FEATURE_ISA_AVX_CONVERT
// REQUIRES: intel_feature_isa_avx_convert

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mavxconvert %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AVX-CONVERT %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-avxconvert \
// RUN: %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVX-CONVERT %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-avxconvert \
// RUN: %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVX-CONVERT %s
// AVX-CONVERT: "-target-feature" "+avxconvert"
// NO-AVX-CONVERT: "-target-feature" "-avxconvert"

#endif // INTEL_FEATURE_ISA_AVX_CONVERT
