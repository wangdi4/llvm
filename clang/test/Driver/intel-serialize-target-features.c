// INTEL_FEATURE_ISA_SERIALIZE
// REQUIRES: intel_feature_isa_serialize

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mserialize %s -### -o %t.o 2>&1 | FileCheck -check-prefix=SERIALIZE %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-serialize %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-SERIALIZE %s
// SERIALIZE: "-target-feature" "+serialize"
// NO-SERIALIZE: "-target-feature" "-serialize"
// end INTEL_FEATURE_ISA_SERIALIZE
