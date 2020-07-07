// INTEL_FEATURE_ISA_AVX_COMPRESS
// REQUIRES: intel_feature_isa_avx_compress
// RUN: %clang -target x86_64-unknown-unknown -mavxcompress %s -### -o %t.o 2>&1 | FileCheck -check-prefix=AVXCOMPRESS %s
// RUN: %clang -target x86_64-unknown-unknown -mno-avxcompress %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVXCOMPRESS %s
// AVXCOMPRESS: "-target-feature" "+avxcompress"
// NO-AVXCOMPRESS: "-target-feature" "-avxcompress"
// end INTEL_FEATURE_ISA_AVX_COMPRESS
