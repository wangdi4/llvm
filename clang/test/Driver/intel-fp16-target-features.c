// INTEL_FEATURE_ISA_FP16
// REQUIRES: intel_feature_isa_fp16
// RUN: %clang -target i386-linux-gnu -mavx512fp16 %s -### -o %t.o 2>&1 | FileCheck -check-prefix=AVX512FP16 %s
// RUN: %clang -target i386-linux-gnu -mno-avx512fp16 %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVX512FP16 %s
// AVX512FP16: "-target-feature" "+avx512fp16"
// NO-AVX512FP16: "-target-feature" "-avx512fp16"
// end INTEL_FEATURE_ISA_FP16