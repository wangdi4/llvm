; RUN: llvm-as %s -o %t.bc
; RUN: %oclopt -predicate -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'LoopSingleNestedExit'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
; the following code contains a loop with only a single exit. However, this exit is nested,
; so in order to update the in-mask of the loop, it is not enough to take the local condition.
; This test checks that the full mechanism for updating the loop in-mask is used. 
; when computing the in-mask this way, a 'who_left_tr_not' variable is defined.

; CHECK: @test
; CHECK: %who_left_tr_not
; CHECK: ret

define void @test(float addrspace(1)* nocapture %a, float addrspace(1)* nocapture %b) nounwind {
entry:
  %call = tail call i32 @_Z13get_global_idj(i32 0) nounwind readnone
  %arrayidx1 = getelementptr inbounds float, float addrspace(1)* %b, i32 %call
  %arrayidx = getelementptr inbounds float, float addrspace(1)* %a, i32 %call
  %.pre = load float, float addrspace(1)* %arrayidx, align 4
  br label %while.cond

while.cond:                                       ; preds = %while.body, %entry
  %0 = phi float [ %dec, %while.body ], [ %.pre, %entry ]
  %cmp = fcmp ogt float %0, 1.300000e+01
  br i1 %cmp, label %while.body, label %lor.rhs

lor.rhs:                                          ; preds = %while.cond
  %1 = load float, float addrspace(1)* %arrayidx1, align 4
  %cmp2 = fcmp ogt float %1, 1.000000e+02
  br i1 %cmp2, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond, %lor.rhs
  %call3 = tail call i32 (i8 addrspace(2)*, ...) @printf(i8 addrspace(2)* addrspacecast ([4 x i8]* @.str to i8 addrspace(2)*), i32 %call) nounwind
  %2 = load float, float addrspace(1)* %arrayidx, align 4
  %dec = fadd float %2, -1.000000e+00
  store float %dec, float addrspace(1)* %arrayidx, align 4
  br label %while.cond

while.end:                                        ; preds = %lor.rhs
  ret void
}

declare i32 @_Z13get_global_idj(i32) nounwind readnone

declare i32 @printf(i8 addrspace(2)*, ...)

define [7 x i32] @WG.boundaries.test(float addrspace(1)*, float addrspace(1)*) {
entry:
  %2 = call i32 @_Z14get_local_sizej(i32 0)
  %3 = call i32 @get_base_global_id.(i32 0)
  %4 = call i32 @_Z14get_local_sizej(i32 1)
  %5 = call i32 @get_base_global_id.(i32 1)
  %6 = call i32 @_Z14get_local_sizej(i32 2)
  %7 = call i32 @get_base_global_id.(i32 2)
  %8 = insertvalue [7 x i32] undef, i32 %2, 2
  %9 = insertvalue [7 x i32] %8, i32 %3, 1
  %10 = insertvalue [7 x i32] %9, i32 %4, 4
  %11 = insertvalue [7 x i32] %10, i32 %5, 3
  %12 = insertvalue [7 x i32] %11, i32 %6, 6
  %13 = insertvalue [7 x i32] %12, i32 %7, 5
  %14 = insertvalue [7 x i32] %13, i32 1, 0
  ret [7 x i32] %14
}

declare i32 @_Z14get_local_sizej(i32)

declare i32 @get_base_global_id.(i32)

define void @__Vectorized_.test(float addrspace(1)* nocapture %a, float addrspace(1)* nocapture %b) nounwind {
entry:
  %call = tail call i32 @_Z13get_global_idj(i32 0) nounwind readnone
  %arrayidx1 = getelementptr inbounds float, float addrspace(1)* %b, i32 %call
  %arrayidx = getelementptr inbounds float, float addrspace(1)* %a, i32 %call
  %.pre = load float, float addrspace(1)* %arrayidx, align 4
  br label %while.cond

while.cond:                                       ; preds = %while.body, %entry
  %0 = phi float [ %dec, %while.body ], [ %.pre, %entry ]
  %cmp = fcmp ogt float %0, 1.300000e+01
  br i1 %cmp, label %while.body, label %lor.rhs

lor.rhs:                                          ; preds = %while.cond
  %1 = load float, float addrspace(1)* %arrayidx1, align 4
  %cmp2 = fcmp ogt float %1, 1.000000e+02
  br i1 %cmp2, label %while.body, label %while.end

while.body:                                       ; preds = %lor.rhs, %while.cond
  %call3 = tail call i32 (i8 addrspace(2)*, ...) @printf(i8 addrspace(2)* addrspacecast ([4 x i8]* @.str to i8 addrspace(2)*), i32 %call) nounwind
  %2 = load float, float addrspace(1)* %arrayidx, align 4
  %dec = fadd float %2, -1.000000e+00
  store float %dec, float addrspace(1)* %arrayidx, align 4
  br label %while.cond

while.end:                                        ; preds = %lor.rhs
  ret void
}

declare i1 @__ocl_allOne(i1)

declare i1 @__ocl_allZero(i1)

!sycl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!6}
!opencl.ocl.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!8}
!opencl.compiler.options = !{!8}
!opencl.kernel_info = !{!9}
!llvm.functions_info = !{}

!0 = !{void (float addrspace(1)*, float addrspace(1)*)* @test, !1, !2, !3, !4, !5}
!1 = !{!"kernel_arg_addr_space", i32 1, i32 1}
!2 = !{!"kernel_arg_access_qual", !"none", !"none"}
!3 = !{!"kernel_arg_type", !"float*", !"float*"}
!4 = !{!"kernel_arg_type_qual", !"", !""}
!5 = !{!"kernel_arg_name", !"a", !"b"}
!6 = !{i32 1, i32 0}
!7 = !{i32 0, i32 0}
!8 = !{}
!9 = !{void (float addrspace(1)*, float addrspace(1)*)* @test, !10}
!10 = !{!11, !12, !13, !14, !15, !16, !17, !18, !19}
!11 = !{!"local_buffer_size", null}
!12 = !{!"barrier_buffer_size", null}
!13 = !{!"kernel_execution_length", null}
!14 = !{!"kernel_has_barrier", null}
!15 = !{!"no_barrier_path", i1 true}
!16 = !{!"vectorized_kernel", null}
!17 = !{!"vectorized_width", null}
!18 = !{!"kernel_wrapper", null}
!19 = !{!"scalar_kernel", null}
