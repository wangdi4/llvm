; RUN: opt -passes=sycl-kernel-math-func-select -S %s 2>&1 | FileCheck %s
; RUN: opt -passes=sycl-kernel-math-func-select -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

; CHECK: warning: FP builtin error requirement not met in function "test": The highest precision version of native_acos provided by the implementation has a max error of 4.000000, while fpbuiltin-max-error requires 1.000000 ulps.
; CHECK-NEXT: warning: FP builtin error requirement not met in function "test": The highest precision version of native_acospi provided by the implementation has a max error of 5.000000, while fpbuiltin-max-error requires 1.000000 ulps.
define void @test() {
entry:
; Still generates high precision version which is closer to the requirement.
; CHECK: call {{.*}} @_Z4acosf(float
; CHECK-NEXT: call {{.*}} @_Z6acospif(float
  %call = call float @_Z11native_acosf(float 5.000000e-01)
  %call1 = call float @_Z13native_acospif(float 5.000000e-01) #0
  ret void
}

declare float @_Z11native_acosf(float) #0
declare float @_Z13native_acospif(float) #1

attributes #0 = { "fpbuiltin-max-error"="1.0" }
attributes #1 = { "fpbuiltin-max-error"="4096.0" }

; DEBUGIFY-NOT: WARNING
