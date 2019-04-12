// INTEL_FEATURE_ISA_VP2INTERSECT
// REQUIRES: intel_feature_isa_vp2intersect
// RUN: %clang -target i386-linux-gnu -mavx512vp2intersect %s -### -o %t.o 2>&1 | FileCheck -check-prefix=VP2INTERSECT %s
// RUN: %clang -target i386-linux-gnu -mno-avx512vp2intersect %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-VP2INTERSECT %s
// VP2INTERSECT: "-target-feature" "+avx512vp2intersect"
// NO-VP2INTERSECT: "-target-feature" "-avx512vp2intersect"
// end INTEL_FEATURE_ISA_VP2INTERSECT
