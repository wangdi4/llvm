#if INTEL_FEATURE_ISA_AMX_CONVERT
// REQUIRES: intel_feature_isa_amx_convert

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-convert %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AMX-CONVERT %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-convert \
// RUN: %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AMX-CONVERT %s
// AMX-CONVERT: "-target-feature" "+amx-convert"
// NO-AMX-CONVERT: "-target-feature" "-amx-convert"

#endif // INTEL_FEATURE_ISA_AMX_CONVERT
