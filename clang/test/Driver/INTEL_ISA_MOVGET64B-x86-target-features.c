#if INTEL_FEATURE_ISA_MOVGET64B
// REQUIRES: intel_feature_isa_movget64b

// RUN: %clang -target i686-unknown-linux-gnu -mmovget64b %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=MOVGET64B %s
// RUN: %clang -target i686-unknown-linux-gnu -mno-movget64b %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-MOVGET64B %s
// MOVGET64B: "-target-feature" "+movget64b"
// NO-MOVGET64B: "-target-feature" "-movget64b"
#endif // INTEL_FEATURE_ISA_MOVGET64B
