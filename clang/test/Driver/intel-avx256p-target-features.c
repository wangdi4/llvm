// INTEL_FEATURE_ISA_AVX256P
// REQUIRES: intel_feature_isa_avx256p

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mavx256p %s -### -o %t.o 2>&1 | FileCheck -check-prefix=AVX256P %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-avx256p %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVX256P %s

// AVX256P: "-target-feature" "+avx256p"
// NO-AVX256P: "-target-feature" "-avx256p"
// end INTEL_FEATURE_ISA_AVX256P
