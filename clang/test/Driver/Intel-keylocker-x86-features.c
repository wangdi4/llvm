// INTEL_FEATURE_ISA_KEYLOCKER
// REQUIRES: intel_feature_isa_keylocker
// RUN: %clang -target i386-linux-gnu -mkeylocker %s -### -o %t.o 2>&1 | FileCheck -check-prefix=KEYLOCKER %s
// RUN: %clang -target i386-linux-gnu -mno-keylocker %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-KEYLOCKER %s
// KEYLOCKER: "-target-feature" "+keylocker"
// NO-KEYLOCKER: "-target-feature" "-keylocker"
// end INTEL_FEATURE_ISA_KEYLOCKER
