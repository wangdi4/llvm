;; ************************************************************************************
;; Test that variadic vector arguments of printf with element types i8, i16, half,
;; or float are getting promoted to int32 and double vectors respectively.
;; Also check that scalar float argument of printf function is promoted to
;; double.
;; Constant parameters are promoted inline.
;; ************************************************************************************
;;
;; RUN: %oclopt -S -printf-args-promotion < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
;; RUN: %oclopt -S -printf-args-promotion -verify < %s | FileCheck %s

; CHECK: call {{.*}} @printf(
; CHECK-SAME: <2 x i32> <i32 1, i32 2>,
; CHECK-SAME: <3 x i32> <i32 1, i32 2, i32 3>,
; CHECK-SAME: <4 x double> <double 1.000000e+00, double 2.000000e+00, double 3.000000e+00, double 4.000000e+00>,
; CHECK-SAME: <8 x double> <double 1.000000e+00, double 2.000000e+00, double 3.000000e+00, double 4.000000e+00, double 5.000000e+00, double 6.000000e+00, double 7.000000e+00, double 8.000000e+00>)
; CHECK-SAME: #[[NOBLT:[0-9]+]]

; CHECK: call {{.*}} @printf(
; CHECK-SAME: double 1.000000e+00)
; CHECK-SAME: #[[NOBLT]]

; CHECK: attributes #[[NOBLT]] = { nobuiltin nounwind }

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknonw-unknown"

@.str = private unnamed_addr addrspace(2) constant [32 x i8] c"%v2hhd   %v3hi   %v4hf   %v8hlf\00", align 1
@.str.1 = private unnamed_addr addrspace(2) constant [4 x i8] c"%f \00", align 1

; Function Attrs: nounwind
declare spir_func i32 @printf(i8 addrspace(2)* nocapture readonly, ...) #0

; Function Attrs: nounwind
define spir_func void @printf_test() #0 {
  %1 = tail call spir_func i32 (i8 addrspace(2)*, ...) @printf(i8 addrspace(2)* getelementptr inbounds ([32 x i8], [32 x i8] addrspace(2)* @.str, i64 0, i64 0), <2 x i8> <i8 1, i8 2>, <3 x i16> <i16 1, i16 2, i16 3>, <4 x half> <half 0xH3C00, half 0xH4000, half 0xH4200, half 0xH4400>, <8 x float> <float 1.000000e+00, float 2.000000e+00, float 3.000000e+00, float 4.000000e+00, float 5.000000e+00, float 6.000000e+00, float 7.000000e+00, float 8.000000e+00>) #1
  %2 = tail call spir_func i32 (i8 addrspace(2)*, ...) @printf(i8 addrspace(2)* getelementptr inbounds ([4 x i8], [4 x i8] addrspace(2)* @.str.1, i32 0, i32 0), float 1.000000e+00) #1
  ret void
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!0}
!opencl.ocl.version = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}

!0 = !{i32 1, i32 2}
!1 = !{i32 2, i32 0}
!2 = !{}

; DEBUGIFY-NOT: WARNING
