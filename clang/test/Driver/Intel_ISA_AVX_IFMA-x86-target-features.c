// INTEL_FEATURE_ISA_AVX_IFMA
// REQUIRES: intel_feature_isa_avx_ifma
// RUN: %clang -target i386-linux-gnu -mavxifma %s -### -o %t.o 2>&1 | FileCheck -check-prefix=AVXIFMA %s
// RUN: %clang -target i386-linux-gnu -mno-avxifma %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVXIFMA %s
// AVXIFMA: "-target-feature" "+avxifma"
// NO-AVXIFMA: "-target-feature" "-avxifma"
// end INTEL_FEATURE_ISA_AVX_IFMA
