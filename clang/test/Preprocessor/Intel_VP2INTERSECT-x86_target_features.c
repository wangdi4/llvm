// INTEL_FEATURE_ISA_VP2INTERSECT
// REQUIRES: intel_feature_isa_vp2intersect
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mavx512vp2intersect -x c -E -dM -o - %s | FileCheck  -check-prefix=VP2INTERSECT %s

// VP2INTERSECT: #define __AVX512F__ 1
// VP2INTERSECT: #define __AVX512VP2INTERSECT__ 1

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-avx512vp2intersect -x c -E -dM -o - %s | FileCheck  -check-prefix=NOVP2INTERSECT %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mavx512vp2intersect -mno-avx512f -x c -E -dM -o - %s | FileCheck  -check-prefix=NOVP2INTERSECT %s

// NOVP2INTERSECT-NOT: #define __AVX512VP2INTERSECT__ 1

// end INTEL_FEATURE_ISA_VP2INTERSECT
