#if INTEL_FEATURE_ISA_SHA512
// REQUIRES: intel_feature_isa_sha512

// RUN: %clang -target i686-unknown-linux-gnu -msha512 %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=SHA512 %s
// RUN: %clang -target i686-unknown-linux-gnu -mno-sha512 %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-SHA512 %s
// SHA512: "-target-feature" "+sha512"
// NO-SHA512: "-target-feature" "-sha512"
#endif // INTEL_FEATURE_ISA_SHA512