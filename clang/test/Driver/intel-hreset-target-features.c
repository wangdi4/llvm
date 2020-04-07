// INTEL_FEATURE_ISA_HRESET
// REQUIRES: intel_feature_isa_hreset

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mhreset %s -### -o %t.o 2>&1 | FileCheck -check-prefix=HRESET %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-hreset %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-HRESET %s
// HRESET: "-target-feature" "+hreset"
// NO-HRESET: "-target-feature" "-hreset"
// end INTEL_FEATURE_ISA_HRESET
