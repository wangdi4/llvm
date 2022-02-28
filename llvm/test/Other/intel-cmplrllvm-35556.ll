; RUN: opt -vector-library=SVML -passes='lto-pre-link<O3>' -debug-pass-manager < %s -S 2>&1 | FileCheck --check-prefix=CHECK-NONLOOPOPT %s
; RUN: opt -vector-library=SVML -passes='lto-pre-link<O3>' -loopopt -debug-pass-manager < %s -S 2>&1 | FileCheck --check-prefix=CHECK-LOOPOPT %s

; With the new pass manager, do not run InjectTLIMappings on the compile step
; of an -flto compilation. This can emit SVML symbols into @llvm.compiler.used
; which inhibit whole program detection.

; CHECK-NONLOOPOPT: Running pass: InjectTLIMappings
; CHECK-NONLOOPOPT: @llvm.compiler.used
; CHECK-NONLOOPOPT: call fast double @llvm.log.f64(double %d1) #[[A0:[0-9]+]]
; CHECK-NONLOOPOPT: attributes #[[A0]] = { "vector-function-abi-variant"

; CHECK-LOOPOPT-NOT: Running pass: InjectTLIMappings
; CHECK-LOOPOPT-NOT: @llvm.compiler.used
; CHECK-LOOPOPT: call fast double @llvm.log.f64(double %d1){{ *$}}

define double @foo(double %d1) {
entry:
  %d1.addr = alloca double, align 8
  store double %d1, double* %d1.addr, align 8
  %0 = load double, double* %d1.addr, align 8
  %1 = call fast double @llvm.log.f64(double %0)
  ret double %1
}

declare double @llvm.log.f64(double)

