#if INTEL_FEATURE_ISA_RAO_INT
// REQUIRES: intel_feature_isa_rao_int

// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mraoint -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=RAOINT %s
// RAOINT: #define __RAOINT__ 1
// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mno-raoint -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=NO-RAOINT %s
// NO-RAOINT-NOT: #define __RAOINT__ 1

#endif // INTEL_FEATURE_ISA_RAO_INT
