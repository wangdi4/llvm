#if INTEL_FEATURE_ISA_AVX512_CONVERT
// REQUIRES: intel_feature_isa_avx512_convert

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mavx512convert %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AVX512-CONVERT %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-avx512convert \
// RUN: %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVX512-CONVERT %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-avx512convert \
// RUN: %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVX512-CONVERT %s
// AVX512-CONVERT: "-target-feature" "+avx512convert"
// NO-AVX512-CONVERT: "-target-feature" "-avx512convert"

#endif // INTEL_FEATURE_ISA_AVX512_CONVERT
