#if INTEL_FEATURE_ISA_SM3
// REQUIRES: intel_feature_isa_sm3

// RUN: %clang -target i686-unknown-linux-gnu -msm3 %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=SM3 %s
// RUN: %clang -target i686-unknown-linux-gnu -mno-sm3 %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-SM3 %s
// SM3: "-target-feature" "+sm3"
// NO-SM3: "-target-feature" "-sm3"
#endif // INTEL_FEATURE_ISA_SM3