// INTEL_FEATURE_ISA_TSXLDTRK
// REQUIRES: intel_features_isa_tsxldtrk

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mtsxldtrk %s -### -o %t.o 2>&1 | FileCheck --check-prefix=TSXLDTRK %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-tsxldtrk %s -### -o %t.o 2>&1 | FileCheck --check-prefix=TSXLDTRK %s
// TSXLDTRK: "-target-feature" "+tsxldtrk"
// NO-TSXLDTRK: "-target-feature" "-tsxldtrk"
// end INTEL_FEATURE_ISA_TSXLDTRK
