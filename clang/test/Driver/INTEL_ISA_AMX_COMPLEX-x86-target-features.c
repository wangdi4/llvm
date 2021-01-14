#if INTEL_FEATURE_ISA_AMX_COMPLEX
// REQUIRES: intel_feature_isa_amx_complex

// RUN: %clang -target x86_64-unknown-linux-gnu -mamx-complex %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AMX-COMPLEX %s
// RUN: %clang -target x86_64-unknown-linux-gnu -mno-amx-complex %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AMX-COMPLEX %s
// AMX-COMPLEX: "-target-feature" "+amx-complex"
// NO-AMX-COMPLEX: "-target-feature" "-amx-complex"
#endif // INTEL_FEATURE_ISA_AMX_COMPLEX
