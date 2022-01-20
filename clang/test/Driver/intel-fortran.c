// Tests for Fortran handling

// RUN: %clang -### -E %S/Inputs/intel-test.f90 2>&1 \
// RUN:  | FileCheck -check-prefixes=PREPROCESS-WARNING,X-C %s
// RUN: %clang -E %S/Inputs/intel-test.f90 2>&1 \
// RUN:  | FileCheck -check-prefix=PREPROCESS-WARNING %s
// PREPROCESS-WARNING-NOT: clang{{.*}} warning: {{.*}}: previously preprocessed input
// X-C: "-x" "c"
