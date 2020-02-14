#if INTEL_FEATURE_ISA_AMX_LNC
// REQUIRES: intel_feature_isa_amx_lnc

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-transpose %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AMX-TRANSPOSE %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-transpose \
// RUN: %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AMX-TRANSPOSE %s
// AMX-TRANSPOSE: "-target-feature" "+amx-transpose"
// NO-AMX-TRANSPOSE: "-target-feature" "-amx-transpose"

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-avx512 %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AMX-AVX512 %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-avx512 \
// RUN: %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AMX-AVX512 %s
// AMX-AVX512: "-target-feature" "+amx-avx512"
// NO-AMX-AVX512: "-target-feature" "-amx-avx512"

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-bf16-evex %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AMX-BF16-EVEX %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-bf16-evex \
// RUN: %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AMX-BF16-EVEX %s
// AMX-BF16-EVEX: "-target-feature" "+amx-bf16-evex"
// NO-AMX-BF16-EVEX: "-target-feature" "-amx-bf16-evex"

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-int8-evex %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AMX-INT8-EVEX %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-int8-evex \
// RUN: %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AMX-INT8-EVEX %s
// AMX-INT8-EVEX: "-target-feature" "+amx-int8-evex"
// NO-AMX-INT8-EVEX: "-target-feature" "-amx-int8-evex"

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-tile-evex %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AMX-TILE-EVEX %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-tile-evex \
// RUN: %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AMX-TILE-EVEX %s
// AMX-TILE-EVEX: "-target-feature" "+amx-tile-evex"
// NO-AMX-TILE-EVEX: "-target-feature" "-amx-tile-evex"

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-element-evex %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AMX-ELEMENT-EVEX %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-element-evex \
// RUN: %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AMX-ELEMENT-EVEX %s
// AMX-ELEMENT-EVEX: "-target-feature" "+amx-element-evex"
// NO-AMX-ELEMENT-EVEX: "-target-feature" "-amx-element-evex"
#endif // INTEL_FEATURE_ISA_AMX_LNC
