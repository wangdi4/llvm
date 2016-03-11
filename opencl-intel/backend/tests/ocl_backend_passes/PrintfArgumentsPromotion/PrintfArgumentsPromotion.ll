;; ************************************************************************************
;; Test what variadic vector arguments of printf with element types i8, i16, half,
;; and float are getting promoted to int32 and double vectors respectively.
;; ************************************************************************************
;;
;; RUN: opt -S -printf-args-promotion -verify < %s | FileCheck %s

; CHECK:  [[INT8:%[a-z.0-9]+]] = zext <2 x i8> <i8 1, i8 2> to <2 x i32>
; CHECK:  [[INT16:%[a-z.0-9]+]] = zext <3 x i16> <i16 1, i16 2, i16 3> to <3 x i32>
; CHECK:  [[FP16:%[a-z.0-9]+]] = fpext <4 x half> {{.*}} to <4 x double>
; CHECK:  [[FP32:%[a-z.0-9]+]] = fpext <8 x float> {{.*}} to <8 x double>
; CHECK:  call {{.*}} @printf({{.*}}, <2 x i32> [[INT8]], <3 x i32> [[INT16]], <4 x double> [[FP16]], <8 x double> [[FP32]])

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknonw-unknown"

@.str = private unnamed_addr addrspace(2) constant [32 x i8] c"%v2hhd   %v3hi   %v4hf   %v8hlf\00", align 1

; Function Attrs: nounwind
define spir_func void @printf_test() #0 {
  %1 = tail call spir_func i32 (i8 addrspace(2)*, ...)* @printf(i8 addrspace(2)* getelementptr inbounds ([32 x i8] addrspace(2)* @.str, i64 0, i64 0), <2 x i8> <i8 1, i8 2>, <3 x i16> <i16 1, i16 2, i16 3>, <4 x half> <half 0xH3C00, half 0xH4000, half 0xH4200, half 0xH4400>, <8 x float> <float 1.000000e+00, float 2.000000e+00, float 3.000000e+00, float 4.000000e+00, float 5.000000e+00, float 6.000000e+00, float 7.000000e+00, float 8.000000e+00>) #1
  ret void
}

; Function Attrs: nounwind
declare spir_func i32 @printf(i8 addrspace(2)* nocapture readonly, ...) #0

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
