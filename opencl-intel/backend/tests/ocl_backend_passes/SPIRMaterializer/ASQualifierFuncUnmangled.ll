; Check if SPIRMAterializer correctly handle non mandled AddressSpaceQualifier functions
; RUN: opt -spir-materializer -S %s -o - | FileCheck %s
; CHECK: @__to_global
; CHECK: @__to_local

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind
define void @testFunc() #0 {
  %1 = tail call i8* @__to_global(i8* undef) #1
  %2 = tail call i8* @__to_local(i8* undef) #1
  ret void
}

declare i8* @__to_global(i8*)

declare i8* @__to_local(i8*)

attributes #0 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-features"="+mmx,+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!opencl.kernels = !{!0}
!opencl.compiler.options = !{!1}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!2}
!opencl.spir.version = !{!2}
!llvm.ident = !{!3}

!0 = !{void ()* @testFunc}
!1 = !{!"-cl-std=CL2.0"}
!2 = !{i32 2, i32 0}
!3 = !{!"clang version 3.8.1"}
