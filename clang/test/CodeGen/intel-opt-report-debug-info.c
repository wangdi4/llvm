// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -target-cpu skylake-avx512 \
// RUN:  -O3 -mllvm -loopopt -floopopt-pipeline=full \
// RUN:  -mllvm -intel-opt-report=high -mllvm -intel-opt-report-emitter=ir \
// RUN:  -mllvm -intel-opt-report-file=stdout \
// RUN:  -debug-info-kind=line-tables-only -mllvm -intel-debug-info-optimization-only \
// RUN:  -o - -emit-llvm %s \
// RUN:   | FileCheck %s --check-prefix=OPTREPORT

// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -target-cpu skylake-avx512 \
// RUN:  -O3 -mllvm -loopopt -floopopt-pipeline=full \
// RUN:  -mllvm -intel-opt-report=high -mllvm -intel-opt-report-emitter=ir \
// RUN:  -mllvm -intel-opt-report-file=stdout \
// RUN:  -debug-info-kind=line-tables-only -mllvm -intel-debug-info-optimization-only \
// RUN:  -o - -emit-llvm %s \
// RUN:   | FileCheck %s --check-prefix=IR

// This test checks that debug information is available for generating
// optimization reports but is set to not be emitted in the final binary.

// OPTREPORT: LOOP BEGIN at
// OPTREPORT-SAME: intel-opt-report-debug-info.c (31, 3)

// OPTREPORT: remark #15346: vector dependence: assumed FLOW dependence between (32:10) and (32:12)

// IR: distinct !DICompileUnit(
// IR-SAME: emissionKind: NoDebug

int f(int);

void foo(double *A, double *B) {
  for (int i = 0; i < 1024; ++i) {
    A[i] = B[f(i)];
  }
}
