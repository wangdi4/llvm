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
  %.pre = load i32 addrspace(1)* %arrayidx5, align 4
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

!0 = metadata !{void (i32 addrspace(1)*, i32 addrspace(3)*)* @test, metadata !1, metadata !2, metadata !3, metadata !4, metadata !5, metadata !6}
!1 = metadata !{metadata !"kernel_arg_addr_space", i32 1, i32 3}
!2 = metadata !{metadata !"kernel_arg_access_qual", metadata !"none", metadata !"none"}
!3 = metadata !{metadata !"kernel_arg_type", metadata !"int*", metadata !"int*"}
!4 = metadata !{metadata !"kernel_arg_type_qual", metadata !"", metadata !""}
!5 = metadata !{metadata !"kernel_arg_base_type", metadata !"int*", metadata !"int*"}
!6 = metadata !{metadata !"kernel_arg_name", metadata !"out", metadata !"in"}
!7 = metadata !{i32 1, i32 2}
!8 = metadata !{}
!9 = metadata !{metadata !"-I."}
!10 = metadata !{void (i32 addrspace(1)*, i32 addrspace(3)*)* @test, metadata !11}
!11 = metadata !{metadata !12, metadata !13, metadata !14, metadata !15, metadata !16, metadata !17, metadata !18, metadata !19, metadata !20, metadata !21, metadata !22, metadata !23, metadata !24, metadata !25, metadata !26}
!12 = metadata !{metadata !"local_buffer_size", null}
!13 = metadata !{metadata !"barrier_buffer_size", null}
!14 = metadata !{metadata !"kernel_execution_length", null}
!15 = metadata !{metadata !"max_wg_dimensions", null}
!16 = metadata !{metadata !"kernel_has_barrier", null}
!17 = metadata !{metadata !"kernel_has_global_sync", null}
!18 = metadata !{metadata !"no_barrier_path", i1 true}
!19 = metadata !{metadata !"vectorized_kernel", null}
!20 = metadata !{metadata !"vectorized_width", null}
!21 = metadata !{metadata !"kernel_wrapper", null}
!22 = metadata !{metadata !"scalarized_kernel", null}
!23 = metadata !{metadata !"block_literal_size", null}
!24 = metadata !{metadata !"private_memory_size", null}
!25 = metadata !{metadata !"vectorization_dimension", null}
!26 = metadata !{metadata !"can_unite_workgroups", null}
!27 = metadata !{void (i32 addrspace(1)*, i32 addrspace(3)*)* @test, metadata !28}
!28 = metadata !{metadata !29}
!29 = metadata !{metadata !30}
!30 = metadata !{metadata !"Vectorizer@Early_Exit_Givenup_Due_To_Loads", i32 1}
!31 = metadata !{metadata !"Vectorizer@Early_Exit_Givenup_Due_To_Loads", metadata !32}
!32 = metadata !{metadata !"early exit wasn't tried because block consists of a load instruction (but no store instructions). However, it is still likely early exit was impossible regardless of it"}