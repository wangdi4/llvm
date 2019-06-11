// INTEL_FEATURE_ISA_ULI
// REQUIRES: intel_feature_isa_uli
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -muli -x c -E -dM -o - %s | FileCheck -check-prefix=ULI %s

// ULI: #define __ULI__ 1

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-uli -x c -E -dM -o - %s | FileCheck -check-prefix=NOULI %s

// NOULI-NOT: #define __ULI__ 1

// end INTEL_FEATURE_ISA_ULI
