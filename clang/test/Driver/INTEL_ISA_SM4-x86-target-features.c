#if INTEL_FEATURE_ISA_SM4
// REQUIRES: intel_feature_isa_sm4

// RUN: %clang -target i686-unknown-linux-gnu -msm4 %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=SM4 %s
// RUN: %clang -target i686-unknown-linux-gnu -mno-sm4 %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-SM4 %s
// SM4: "-target-feature" "+sm4"
// NO-SM4: "-target-feature" "-sm4"
#endif // INTEL_FEATURE_ISA_SM4