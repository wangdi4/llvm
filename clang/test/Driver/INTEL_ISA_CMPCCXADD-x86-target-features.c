#if INTEL_FEATURE_ISA_CMPCCXADD
// REQUIRES: intel_feature_isa_cmpccxadd

// RUN: %clang -target i686-unknown-linux-gnu -mcmpccxadd %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=CMPCCXADD %s
// RUN: %clang -target i686-unknown-linux-gnu -mno-cmpccxadd %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-CMPCCXADD %s
// CMPCCXADD: "-target-feature" "+cmpccxadd"
// NO-CMPCCXADD: "-target-feature" "-cmpccxadd"
#endif // INTEL_FEATURE_ISA_CMPCCXADD