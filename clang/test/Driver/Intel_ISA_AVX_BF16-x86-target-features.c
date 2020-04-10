// INTEL_FEATURE_ISA_AVX_BF16
// REQUIRES: intel_feature_isa_avx_bf16
// RUN: %clang -target i386-linux-gnu -mavxbf16 %s -### -o %t.o 2>&1 | FileCheck -check-prefix=AVXBF16 %s
// RUN: %clang -target i386-linux-gnu -mno-avxbf16 %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVXBF16 %s
// AVXBF16: "-target-feature" "+avxbf16"
// NO-AVXBF16: "-target-feature" "-avxbf16"
// end INTEL_FEATURE_ISA_AVX_BF16
