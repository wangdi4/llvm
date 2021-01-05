#if INTEL_FEATURE_ISA_RAO_INT
// REQUIRES: intel_feature_isa_rao_int

// RUN: %clang -target i686-unknown-linux-gnu -mraoint %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=RAOINT %s
// RUN: %clang -target i686-unknown-linux-gnu -mno-raoint %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-RAOINT %s
// RAOINT: "-target-feature" "+raoint"
// NO-RAOINT: "-target-feature" "-raoint"
#endif // INTEL_FEATURE_ISA_RAO_INT