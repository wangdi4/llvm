; RUN: opt -ocl-asaa -licm -S < %s | FileCheck %s
; ModuleID = 'main'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Verify that %out and %in are identified as anti-aliased by
; checking that load and store leave the loop (the store is
; moved to after the loop and the load is removed completely).

;CHECK: define void @test
;CHECK: for.body:
;CHECK-NOT: load
;CHECK-NOT: store
;CHECK: br i1 [[CMP:%[a-zA-Z0-9]+]], label %for.body
;CHECK: store
;CHECK: ret void

; Function Attrs: nounwind
define void @test(i32 addrspace(1)* noalias nocapture %out, i32 addrspace(3)* noalias nocapture %in) #0 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #2
  %sext = shl i64 %call, 32
  %idxprom = ashr exact i64 %sext, 32
  %arrayidx = getelementptr inbounds i32 addrspace(3)* %in, i64 %idxprom
  %0 = load i32 addrspace(3)* %arrayidx, align 4
  %cmp3 = icmp sgt i32 %0, 0
  br i1 %cmp3, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  %arrayidx5 = getelementptr inbounds i32 addrspace(1)* %out, i64 %idxprom
  %.casted = addrspacecast i32 addrspace(1)* %arrayidx5 to i32*
  %.pre = load i32* %.casted, align 4
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.body
  %1 = phi i32 [ %.pre, %for.body.lr.ph ], [ %add, %for.body ]
  %2 = phi i32 [ %0, %for.body.lr.ph ], [ %3, %for.body ]
  %i.04 = phi i32 [ 0, %for.body.lr.ph ], [ %inc, %for.body ]
  %mul = mul nsw i32 %2, 7
  %add = add nsw i32 %1, %mul
  store i32 %add, i32 addrspace(1)* %arrayidx5, align 4
  %inc = add nsw i32 %i.04, 1
  %3 = load i32 addrspace(3)* %arrayidx, align 4
  %cmp = icmp slt i32 %inc, %3
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body, %entry
  ret void
}

; Function Attrs: nounwind readnone
declare i64 @_Z13get_global_idj(i32) #1

define [7 x i64] @WG.boundaries.test(i32 addrspace(1)*, i32 addrspace(3)*) {
entry:
  %2 = call i64 @_Z14get_local_sizej(i32 0)
  %3 = call i64 @get_base_global_id.(i32 0)
  %4 = call i64 @_Z14get_local_sizej(i32 1)
  %5 = call i64 @get_base_global_id.(i32 1)
  %6 = call i64 @_Z14get_local_sizej(i32 2)
  %7 = call i64 @get_base_global_id.(i32 2)
  %8 = insertvalue [7 x i64] undef, i64 %2, 2
  %9 = insertvalue [7 x i64] %8, i64 %3, 1
  %10 = insertvalue [7 x i64] %9, i64 %4, 4
  %11 = insertvalue [7 x i64] %10, i64 %5, 3
  %12 = insertvalue [7 x i64] %11, i64 %6, 6
  %13 = insertvalue [7 x i64] %12, i64 %7, 5
  %14 = insertvalue [7 x i64] %13, i64 1, 0
  ret [7 x i64] %14
}

declare i64 @_Z14get_local_sizej(i32)

declare i64 @get_base_global_id.(i32)

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-frame-pointer-elim-non-leaf"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-frame-pointer-elim-non-leaf"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone }

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!7}
!opencl.ocl.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!8}
!opencl.compiler.options = !{!9}
!opencl.kernel_info = !{!10}
!opencl.module_info_list = !{}
!llvm.functions_info = !{}
!opencl.functions_stats = !{!27}
!opencl.stat_descriptions = !{!31}
!opencl.module_stat_info = !{}

!0 = !{void (i32 addrspace(1)*, i32 addrspace(3)*)* @test, !1, !2, !3, !4, !5, !6}
!1 = !{!"kernel_arg_addr_space", i32 1, i32 3}
!2 = !{!"kernel_arg_access_qual", !"none", !"none"}
!3 = !{!"kernel_arg_type", !"int*", !"int*"}
!4 = !{!"kernel_arg_type_qual", !"", !""}
!5 = !{!"kernel_arg_base_type", !"int*", !"int*"}
!6 = !{!"kernel_arg_name", !"out", !"in"}
!7 = !{i32 1, i32 2}
!8 = !{}
!9 = !{!"-I."}
!10 = !{void (i32 addrspace(1)*, i32 addrspace(3)*)* @test, !11}
!11 = !{!12, !13, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26}
!12 = !{!"local_buffer_size", null}
!13 = !{!"barrier_buffer_size", null}
!14 = !{!"kernel_execution_length", null}
!15 = !{!"max_wg_dimensions", null}
!16 = !{!"kernel_has_barrier", null}
!17 = !{!"kernel_has_global_sync", null}
!18 = !{!"no_barrier_path", i1 true}
!19 = !{!"vectorized_kernel", null}
!20 = !{!"vectorized_width", null}
!21 = !{!"kernel_wrapper", null}
!22 = !{!"scalarized_kernel", null}
!23 = !{!"block_literal_size", null}
!24 = !{!"private_memory_size", null}
!25 = !{!"vectorization_dimension", null}
!26 = !{!"can_unite_workgroups", null}
!27 = !{void (i32 addrspace(1)*, i32 addrspace(3)*)* @test, !28}
!28 = !{!29}
!29 = !{!30}
!30 = !{!"Vectorizer@Early_Exit_Givenup_Due_To_Loads", i32 1}
!31 = !{!"Vectorizer@Early_Exit_Givenup_Due_To_Loads", !32}
!32 = !{!"early exit wasn't tried because block consists of a load instruction (but no store instructions). However, it is still likely early exit was impossible regardless of it"}