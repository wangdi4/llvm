#if INTEL_FEATURE_ISA_SHA512
// REQUIRES: intel_feature_isa_sha512

// RUN: %clang -target i686-unknown-linux-gnu -march=atom -msha512 -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=SHA512 %s
// SHA512: #define __SHA512__ 1
// RUN: %clang -target i686-unknown-linux-gnu -march=atom -mno-sha512 -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=NO-SHA512 %s
// RUN: %clang -target i686-unknown-linux-gnu -march=atom -msha512 -mno-avx \
// RUN: -x c -E -dM -o - %s | FileCheck  -check-prefix=NO-SHA512 %s
// NO-SHA512-NOT: #define __SHA512__ 1

#endif // INTEL_FEATURE_ISA_SHA512
