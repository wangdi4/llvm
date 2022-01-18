#if INTEL_FEATURE_ISA_VPINSR_VPEXTR
// REQUIRES: intel_feature_isa_vpinsr_vpextr

// RUN: %clang -target x86_64-unknown-linux-gnu -mvpinsr-vpextr %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=VPINSR-VPEXTR %s
// RUN: %clang -target x86_64-unknown-linux-gnu -mno-vpinsr-vpextr %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-VPINSR-VPEXTR %s
// VPINSR-VPEXTR: "-target-feature" "+vpinsr-vpextr"
// NO-VPINSR-VPEXTR: "-target-feature" "-vpinsr-vpextr"
#endif // INTEL_FEATURE_ISA_VPINSR_VPEXTR
