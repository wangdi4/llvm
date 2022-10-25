; RUN: opt <%s -S -passes='cgscc(inline)' -inline-report=0xe807 2>&1 | FileCheck %s
; RUN: opt -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s

; CMPLRLLVM-25615: Check that inlining report does not crash when printed
; because call to @llvm.fabs.f64 in @bar is optimized away by the inliner.

; CHECK: INLINE{{.*}}bar

declare double @llvm.fabs.f64(double %x)

declare double @llvm.log10.f64(double %x)

declare double @llvm.sqrt.f64(double %x)

define internal fastcc double @bar(double %x) {
  %1 = tail call fast double @llvm.fabs.f64(double %x)
  %cmp = fcmp fast olt double %1, 0x3DA5FD7FE1796495
  %2 = tail call fast double @llvm.log10.f64(double %1)
  %3 = select i1 %cmp, double -1.100000e+01, double %2
  ret double %3
}

define fastcc double @foo(double %channel) {
  %t5 = call fast double @llvm.sqrt.f64(double %channel)
  %div = fdiv fast double 1.000000e+00, %t5
  %call1 = call fast fastcc double @bar(double %div)
  ret double %call1
}
