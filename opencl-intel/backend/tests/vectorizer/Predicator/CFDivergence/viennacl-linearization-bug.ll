; RUN: llvm-as %s -o %t.bc
; RUN: opt  -predicate -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; CHECK: @__Vectorized_.vandermonde_prod
; CHECK: br i1 %shouldexit, label %post_for.end.us, label %for.end.us
; CHECK: for.cond.for.end13_crit_edge.us-lcssa:            ; preds = %entry
; CHECK: for.end13.loopexit:                               ; preds = %post_for.end.us
; CHECK-NOT: br i1 %shouldexit, label %for.cond.for.end13_crit_edge.us-lcssa, label %for.end.us
; CHECK-NOT: for.cond.for.end13_crit_edge.us-lcssa:            ; preds = %for.end.us, %entry
; CHECK-NOT: for.end13.loopexit:                               ; No predecessors!
; CHECK: ret

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @__Vectorized_.vandermonde_prod(float addrspace(1)* nocapture %vander, float addrspace(1)* nocapture %vector, float addrspace(1)* nocapture %result, i32 %size) nounwind {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %cmp31 = icmp eq i32 %size, 0
  %call10 = tail call i64 @_Z15get_global_sizej(i32 0) nounwind readnone
  br i1 %cmp31, label %for.cond.for.end13_crit_edge.us-lcssa, label %for.cond2.preheader.lr.ph.split.us

for.cond2.preheader.lr.ph.split.us:               ; preds = %entry
  %0 = add i32 %size, -1
  br label %for.end.us

for.end.us:                                       ; preds = %for.end.us, %for.cond2.preheader.lr.ph.split.us
  %i.0.in5.us = phi i64 [ %call, %for.cond2.preheader.lr.ph.split.us ], [ %add.us, %for.end.us ]
  %conv6.us = uitofp i32 %0 to float
  %idxprom7.us = and i64 %i.0.in5.us, 4294967295
  %arrayidx8.us = getelementptr inbounds float addrspace(1)* %result, i64 %idxprom7.us
  store float %conv6.us, float addrspace(1)* %arrayidx8.us, align 4
  %add.us = add i64 %call10, %idxprom7.us
  %i.0.us = trunc i64 %add.us to i32
  %cmp.us = icmp ult i32 %i.0.us, %size
  br i1 %cmp.us, label %for.end.us, label %for.end13.loopexit

for.cond.for.end13_crit_edge.us-lcssa:            ; preds = %entry
  %idxprom7 = and i64 %call, 4294967295
  %arrayidx8 = getelementptr inbounds float addrspace(1)* %result, i64 %idxprom7
  store float 0.000000e+00, float addrspace(1)* %arrayidx8, align 4
  br label %for.end13

for.end13.loopexit:                               ; preds = %for.end.us
  br label %for.end13

for.end13:                                        ; preds = %for.end13.loopexit, %for.cond.for.end13_crit_edge.us-lcssa
  ret void
}

declare i64 @_Z13get_global_idj(i32) nounwind readnone

declare i64 @_Z15get_global_sizej(i32) nounwind readnone

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!7}
!opencl.ocl.version = !{!8}
!opencl.used.extensions = !{!9}
!opencl.used.optional.core.features = !{!9}
!opencl.compiler.options = !{!9}
!opencl.kernel_info = !{!10}
!opencl.module_info_list = !{}
!llvm.functions_info = !{}

!0 = !{void (float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, i32)* @__Vectorized_.vandermonde_prod, !1, !2, !3, !4, !5, !6}
!1 = !{!"kernel_arg_addr_space", i32 1, i32 1, i32 1, i32 0}
!2 = !{!"kernel_arg_access_qual", !"none", !"none", !"none", !"none"}
!3 = !{!"kernel_arg_type", !"float*", !"float*", !"float*", !"uint"}
!4 = !{!"kernel_arg_type_qual", !"", !"", !"", !""}
!5 = !{!"kernel_arg_base_type", !"float*", !"float*", !"float*", !"uint"}
!6 = !{!"kernel_arg_name", !"vander", !"vector", !"result", !"size"}
!7 = !{i32 1, i32 0}
!8 = !{i32 0, i32 0}
!9 = !{}
!10 = !{void (float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, i32)* @__Vectorized_.vandermonde_prod, !11}
!11 = !{!12, !13, !14, !15, !16, !17, !18, !19, !20}
!12 = !{!"local_buffer_size", null}
!13 = !{!"barrier_buffer_size", null}
!14 = !{!"kernel_execution_length", null}
!15 = !{!"kernel_has_barrier", null}
!16 = !{!"no_barrier_path", i1 true}
!17 = !{!"vectorized_kernel", null}
!18 = !{!"vectorized_width", null}
!19 = !{!"kernel_wrapper", null}
!20 = !{!"scalarized_kernel", null}

