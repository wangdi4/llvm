; RUN: opt -passes=sycl-kernel-math-func-select -S %s 2>&1 | FileCheck %s
; RUN: opt -passes=sycl-kernel-math-func-select -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

; CHECK: warning: FP builtin error requirement not met in function "test": The implementation ulps for lgamma is not well defined, so the fpbuiltin-max-error requirement (8192.000000 ulps) may not apply.
; CHECK-NEXT: warning: FP builtin error requirement not met in function "test": The implementation ulps for lgamma_r is not well defined, so the fpbuiltin-max-error requirement (8192.000000 ulps) may not apply.
define void @test() {
entry:
; CHECK: call {{.*}} @_Z6lgammaf(float
; CHECK-NEXT: call {{.*}} @_Z8lgamma_rf(float
  %call = call float @_Z6lgammaf(float 5.000000e-01)
  %call1 = call float @_Z8lgamma_rf(float 5.000000e-01) #0
  ret void
}

declare float @_Z6lgammaf(float) #0
declare float @_Z8lgamma_rf(float)

attributes #0 = { "fpbuiltin-max-error"="8192.0" }

; DEBUGIFY-NOT: WARNING
