/// Use of -qopenmp is recommended over -fopenmp
// RUN: %clang_cl --intel -### /openmp -c %s 2>&1 \
// RUN:  | FileCheck %s -DORIG_OPT="/openmp" -DREC_OPT="/Qiopenmp"
// RUN: %clangxx --intel -### -fopenmp -c %s 2>&1 \
// RUN:  | FileCheck %s -DORIG_OPT="-fopenmp" -DREC_OPT="-qopenmp"

// CHECK: Use of '[[REC_OPT]]' recommended over '[[ORIG_OPT]]'
