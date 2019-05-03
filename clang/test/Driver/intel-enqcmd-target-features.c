// INTEL_FEATURE_ISA_ENQCMD
// REQUIRES: intel_feature_isa_enqcmd

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -menqcmd %s -### -o %t.o 2>&1 | FileCheck --check-prefix=ENQCMD %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-enqcmd %s -### -o %t.o 2>&1 | FileCheck --check-prefix=NO-ENQCMD %s
// ENQCMD: "-target-feature" "+enqcmd"
// NO-ENQCMD: "-target-feature" "-enqcmd"
// end INTEL_FEATURE_ISA_ENQCMD
