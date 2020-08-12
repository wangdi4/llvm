// INTEL_FEATURE_ISA_AVX_MPSADBW
// REQUIRES: intel_feature_isa_avx_mpsadbw
// RUN: %clang -target x86_64-unknown-unknown -mavx512mpsadbw %s -### -o %t.o 2>&1 | FileCheck -check-prefix=AVX512mpsadbw %s
// RUN: %clang -target x86_64-unknown-unknown -mno-avx512mpsadbw %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVX512mpsadbw %s
// AVX512mpsadbw: "-target-feature" "+avx512mpsadbw"
// NO-AVX512mpsadbw: "-target-feature" "-avx512mpsadbw"
// end INTEL_FEATURE_ISA_AVX_MPSADBW
