; RUN: opt -fma-splitter -S %s | FileCheck %s
;
; The test checks the llvm.fmuladd was split to fmul + fadd pair.
; Source code:
;__kernel void fmad(float a, float b, float c)
;{
;    float d = a * b + c;
;}
;
; CHECK-LABEL: define {{.*}} @fmad
; CHECK-NEXT: entry
; CHECK-NEXT: fmul
; CHECK-NEXT: fadd
; CHECK-NEXT: ret

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind
define void @fmad(float %a, float %b, float %c) #0 !kernel_arg_addr_space !2 !kernel_arg_access_qual !3 !kernel_arg_type !4 !kernel_arg_base_type !4 !kernel_arg_type_qual !5 {
entry:
  %0 = call float @llvm.fmuladd.f32(float %a, float %b, float %c)
  ret void
}

; Function Attrs: nounwind readnone
declare float @llvm.fmuladd.f32(float, float, float) #1

attributes #0 = { noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-features"="+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }

!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 2, i32 0}
!1 = !{!"clang version 4.0.1 (ssh://nnopencl-git-01.inn.intel.com/home/git/repo/opencl_qa-clang 7035f2d8cf714dda16ba4019fcfd0eabee7e8c67) (ssh://nnopencl-git-01.inn.intel.com/home/git/repo/opencl_qa-llvm 8456c1092144484dcae55966238f9b9a09d740fe)"}
!2 = !{i32 0, i32 0, i32 0}
!3 = !{!"none", !"none", !"none"}
!4 = !{!"float", !"float", !"float"}
!5 = !{!"", !"", !""}
