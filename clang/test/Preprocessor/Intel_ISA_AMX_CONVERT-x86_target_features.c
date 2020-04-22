#if INTEL_FEATURE_ISA_AMX_CONVERT
// REQUIRES: intel_feature_isa_amx_convert

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-convert -x c \
// RUN: -E -dM -o - %s | FileCheck  -check-prefix=AMX-CONVERT %s
// AMX-CONVERT: #define __AMX_CONVERT__ 1
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-convert \
// RUN: -x c -E -dM -o - %s | FileCheck  -check-prefix=NO-AMX-CONVERT %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-convert \
// RUN: -mno-amx-tile -x c -E -dM -o - %s \
// RUN: | FileCheck  -check-prefix=NO-AMX-CONVERT %s
// NO-AMX-CONVERT-NOT: #define __AMX_CONVERT__ 1

#endif // INTEL_FEATURE_ISA_AMX_CONVERT
