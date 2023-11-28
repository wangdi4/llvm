; RUN: opt -passes=sycl-kernel-wgloop-creator %s -S | FileCheck %s
; RUN: opt -passes=sycl-kernel-wgloop-creator %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

;; Check that the scalar kernel is ignored in loop creation for native subgroup case.
;; Captured via breakpoint in CLWGLoopCreator. The source is:

; CHECK-LABEL: @test
; CHECK-LABEL: WGLoopsEntry:
; CHECK: br label %vect_if
; CHECK-LABEL: dim_0_vector_pre_head:
; CHECK-SAME: preds = %vect_if
; CHECK-LABEL: entryvector_func:
; CHECK-SAME: preds = %entryvector_func, %dim_0_vector_pre_head
; CHECK-LABEL: scalar_kernel_entry:

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent nounwind
define void @test(ptr addrspace(1) noalias %sub_groups_sizes) local_unnamed_addr #0 !kernel_arg_addr_space !1 !kernel_arg_access_qual !2 !kernel_arg_type !3 !kernel_arg_base_type !3 !kernel_arg_type_qual !4 !kernel_arg_host_accessible !5 !kernel_arg_pipe_depth !6 !kernel_arg_pipe_io !4 !kernel_arg_buffer_location !4 !kernel_arg_name !7 !vectorized_kernel !8 !no_barrier_path !9 !kernel_has_sub_groups !9 !scalar_kernel !10 !vectorized_width !1 !kernel_execution_length !11 !kernel_has_barrier !5 !kernel_has_global_sync !5 !max_wg_dimensions !1 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #3
  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %sub_groups_sizes, i64 %call
  store i32 1, ptr addrspace(1) %arrayidx, align 4, !tbaa !12
  ret void
}

; Function Attrs: convergent nounwind readnone
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #1

; Function Attrs: convergent
declare i32 @_Z22get_max_sub_group_sizev() local_unnamed_addr #2

define [7 x i64] @WG.boundaries.test(ptr addrspace(1) %0) {
entry:
  %1 = call i64 @_Z14get_local_sizej(i32 0)
  %2 = call i64 @get_base_global_id.(i32 0)
  %3 = call i64 @_Z14get_local_sizej(i32 1)
  %4 = call i64 @get_base_global_id.(i32 1)
  %5 = call i64 @_Z14get_local_sizej(i32 2)
  %6 = call i64 @get_base_global_id.(i32 2)
  %7 = insertvalue [7 x i64] undef, i64 %1, 2
  %8 = insertvalue [7 x i64] %7, i64 %2, 1
  %9 = insertvalue [7 x i64] %8, i64 %3, 4
  %10 = insertvalue [7 x i64] %9, i64 %4, 3
  %11 = insertvalue [7 x i64] %10, i64 %5, 6
  %12 = insertvalue [7 x i64] %11, i64 %6, 5
  %13 = insertvalue [7 x i64] %12, i64 1, 0
  ret [7 x i64] %13
}

declare i64 @_Z14get_local_sizej(i32)

declare i64 @get_base_global_id.(i32)

; Function Attrs: convergent nounwind
define void @__Vectorized_.test(ptr addrspace(1) noalias %sub_groups_sizes) local_unnamed_addr #0 !kernel_arg_addr_space !1 !kernel_arg_access_qual !2 !kernel_arg_type !3 !kernel_arg_base_type !3 !kernel_arg_type_qual !4 !kernel_arg_host_accessible !5 !kernel_arg_pipe_depth !6 !kernel_arg_pipe_io !4 !kernel_arg_buffer_location !4 !kernel_arg_name !7 !vectorized_kernel !10 !no_barrier_path !9 !kernel_has_sub_groups !9 !scalar_kernel !0 !vectorized_width !16 !kernel_execution_length !17 !kernel_has_barrier !5 !kernel_has_global_sync !5 !max_wg_dimensions !1 !vectorization_dimension !6 !can_unite_workgroups !5 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #3
  %0 = getelementptr inbounds i32, ptr addrspace(1) %sub_groups_sizes, i64 %call
  store <16 x i32> <i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16>, ptr addrspace(1) %0, align 4
  ret void
}

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-sign
ed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { convergent nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-pro
tector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { convergent nounwind readnone }

!sycl.kernels = !{!0}

!0 = !{ptr @test}
!1 = !{i32 1}
!2 = !{!"none"}
!3 = !{!"uint*"}
!4 = !{!""}
!5 = !{i1 false}
!6 = !{i32 0}
!7 = !{!"sub_groups_sizes"}
!8 = !{ptr @__Vectorized_.test}
!9 = !{i1 true}
!10 = !{null}
!11 = !{i32 4}
!12 = !{!13, !13, i64 0}
!13 = !{!"int", !14, i64 0}
!14 = !{!"omnipotent char", !15, i64 0}
!15 = !{!"Simple C/C++ TBAA"}
!16 = !{i32 16}
!17 = !{i32 5}

; DEBUGIFY-COUNT-31: WARNING: Instruction with empty DebugLoc in function test
; DEBUGIFY: WARNING: Missing line 22
; DEBUGIFY-NOT: WARNING
