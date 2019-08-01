// INTEL_FEATURE_ISA_ULI
// REQUIRES: intel_feature_isa_uli
// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86_64 -muli %s -### -o %t.o 2>&1 | FileCheck -check-prefix=ULI %s
// RUN: %clang -target x86_64-unknown-linux-gnu -march=x86_64 -mno-uli %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-ULI %s

// ULI: "-target-feature" "+uli"
// NO-ULI: "-target-feature" "-uli"
// end INTEL_FEATURE_ISA_ULI
