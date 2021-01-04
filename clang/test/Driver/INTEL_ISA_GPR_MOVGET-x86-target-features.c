#if INTEL_FEATURE_ISA_GPR_MOVGET
// REQUIRES: intel_feature_isa_gpr_movget

// RUN: %clang -target i686-unknown-linux-gnu -mgprmovget %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=GPRMOVGET %s
// RUN: %clang -target i686-unknown-linux-gnu -mno-gprmovget %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-GPRMOVGET %s
// GPRMOVGET: "-target-feature" "+gprmovget"
// NO-GPRMOVGET: "-target-feature" "-gprmovget"
#endif // INTEL_FEATURE_ISA_GPR_MOVGET
