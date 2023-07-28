; RUN: opt -passes=sycl-kernel-math-func-select -S %s | FileCheck %s
; RUN: opt -passes=sycl-kernel-math-func-select -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

define void @test() {
entry:
; CHECK: call {{.*}} @_Z4acosf(float
; CHECK-NEXT: call {{.*}} @_Z4acosDv16_f(<16 x float>
; CHECK-NEXT: call {{.*}} @_Z6acospif(float
; CHECK-NEXT: call {{.*}} @_Z6acospiDv16_f(<16 x float>
; The following call won't be tigtened because the call site attribute takes precedence
; CHECK-NEXT: call {{.*}} @_Z11native_acosf(float
; CHECK-NEXT: call {{.*}} @_Z11native_acosDv16_f(<16 x float>
  %call = call float @_Z11native_acosf(float 5.000000e-01)
  %call.vec = call <16 x float> @_Z11native_acosDv16_f(<16 x float> zeroinitializer)
  %call1 = call float @_Z13native_acospif(float 5.000000e-01) #0
  %call1.vec = call <16 x float> @_Z13native_acospiDv16_f(<16 x float> zeroinitializer) #0
  %call2 = call float @_Z11native_acosf(float 5.000000e-01) #1
  %call2.vec = call <16 x float> @_Z11native_acosDv16_f(<16 x float> zeroinitializer) #1
  ret void
}

declare float @_Z11native_acosf(float) #0
declare <16 x float> @_Z11native_acosDv16_f(<16 x float>) #0
declare float @_Z13native_acospif(float)
declare <16 x float> @_Z13native_acospiDv16_f(<16 x float>)

attributes #0 = { "fpbuiltin-max-error"="5.0" }
attributes #1 = { "fpbuiltin-max-error"="4096.0" }

; DEBUGIFY-NOT: WARNING
