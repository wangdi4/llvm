; RUN: llvm-as %s -o %t.bc
; RUN: opt -runtimelib %p/../../Full/runtime.bc -analyze -kernel-analysis -cl-loop-bound -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; The source code for the test is:
; __kernel void test(__global float* a) {
;    size_t gid = get_global_id(0);
;    if (gid < 100) return; // line A
;    if (gid > 200) return; // line B
;    a[gid] = 3.f;
; }
; Lines A and B combined by LLVM InstCombine pass into
;  %.off = add i64 %1, -100
;  %2 = icmp ugt i64 %.off, 100

; CHECK: CLWGLoopBoundaries
; CHECK: found 2 early exit boundaries
; CHECK: dim=0, contains=T, isGID=T, isSigned=F, isUpper=T
; CHECK-NEXT: %right_boundary_align = sub i64 100, -100
; CHECK-NEXT: dim=0, contains=T, isGID=T, isSigned=F, isUpper=F
; CHECK-NEXT: %final_left_bound = select i1 %right_lt_left, i64 %left_after_overflow, i64 %non_negative_left_bound
; CHECK: found 0 uniform early exit conditions

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: nounwind
define void @test(float addrspace(1)* %a) #0 {
  %1 = call i64 @_Z13get_global_idj(i32 0) #2
  %.off = add i64 %1, -100
  %2 = icmp ugt i64 %.off, 100
  br i1 %2, label %5, label %3

; <label>:3                                       ; preds = %0
  %4 = getelementptr inbounds float addrspace(1)* %a, i64 %1
  store float 3.000000e+00, float addrspace(1)* %4, align 4
  br label %5

; <label>:5                                       ; preds = %0, %3
  ret void
}

; Function Attrs: nounwind readnone
declare i64 @_Z13get_global_idj(i32) #1

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-frame-pointer-elim-non-leaf"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-frame-pointer-elim-non-leaf"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone }

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!7}
!opencl.ocl.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!8}
!opencl.compiler.options = !{!8}

!0 = !{void (float addrspace(1)*)* @test, !1, !2, !3, !4, !5, !6}
!1 = !{!"kernel_arg_addr_space", i32 1}
!2 = !{!"kernel_arg_access_qual", !"none"}
!3 = !{!"kernel_arg_type", !"float*"}
!4 = !{!"kernel_arg_type_qual", !""}
!5 = !{!"kernel_arg_base_type", !"float*"}
!6 = !{!"kernel_arg_name", !"a"}
!7 = !{i32 1, i32 2}
!8 = !{}
