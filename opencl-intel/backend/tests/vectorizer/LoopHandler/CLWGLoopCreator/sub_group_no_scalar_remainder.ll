; RUN: %oclopt -cl-loop-creator -S < %s | FileCheck %s
;; Check that the scalar kernel is ignored in loop creation for native subgroup case.
;; Captured via breakpoint in CLWGLoopCreator. The source is:

; ModuleID = 'main'
source_filename = "1"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent nounwind
define void @testKernel(i32 addrspace(1)* noalias %sub_groups_sizes) local_unnamed_addr #0 !kernel_arg_addr_space !6 !kernel_arg_access_qual !7 !kernel_arg_type !8 !kernel_arg_base_type !8 !kernel_arg_type_qual !9 !kernel_arg_host_accessible !10 !kernel_arg_pipe_depth !5 !kernel_arg_pipe_io !9 !kernel_arg_buffer_location !9 !kernel_arg_name !11 !vectorized_kernel !12 !no_barrier_path !13 !kernel_has_sub_groups !13 !scalarized_kernel !14 !vectorized_width !6 !kernel_execution_length !15 !kernel_has_barrier !10 !kernel_has_global_sync !10 !max_wg_dimensions !6 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #3
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %sub_groups_sizes, i64 %call
  store i32 1, i32 addrspace(1)* %arrayidx, align 4, !tbaa !16
  ret void
}

; Function Attrs: convergent nounwind readnone
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #1

; Function Attrs: convergent
declare i32 @_Z22get_max_sub_group_sizev() local_unnamed_addr #2

define [7 x i64] @WG.boundaries.testKernel(i32 addrspace(1)*) {
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
define void @__Vectorized_.testKernel(i32 addrspace(1)* noalias %sub_groups_sizes) local_unnamed_addr #0 !kernel_arg_addr_space !6 !kernel_arg_access_qual !7 !kernel_arg_type !8 !kernel_arg_base_type !8 !kernel_arg_type_qual !9 !kernel_arg_host_accessible !10 !kernel_arg_pipe_depth !5 !kernel_arg_pipe_io !9 !kernel_arg_buffer_location !9 !kernel_arg_name !11 !vectorized_kernel !14 !no_barrier_path !13 !kernel_has_sub_groups !13 !scalarized_kernel !4 !vectorized_width !20 !vectorization_dimension !5 !can_unite_workgroups !10 !kernel_execution_length !21 !kernel_has_barrier !10 !kernel_has_global_sync !10 !max_wg_dimensions !6 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #3
  %0 = getelementptr inbounds i32, i32 addrspace(1)* %sub_groups_sizes, i64 %call
  %ptrTypeCast = bitcast i32 addrspace(1)* %0 to <16 x i32> addrspace(1)*
  store <16 x i32> <i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16>, <16 x i32> addrspace(1)* %ptrTypeCast, align 4
  ret void
}

; CHECK-LABEL: @testKernel
; CHECK-LABEL: WGLoopsEntry:
; CHECK: br label %vect_if
; CHECK-LABEL: dim_0_vector_pre_head:
; CHECK-SAME: preds = %vect_if
; CHECK-LABEL: entryvector_func:
; CHECK-SAME: preds = %entryvector_func, %dim_0_vector_pre_head
; CHECK-LABEL: scalar_kernel_entry:

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-sign
ed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { convergent nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"=
"false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-pro
tector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { convergent nounwind readnone }

!llvm.linker.options = !{}
!llvm.module.flags = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!opencl.kernels = !{!4}
!opencl.global_variable_total_size = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, i32 2}
!2 = !{}
!4 = !{void (i32 addrspace(1)*)* @testKernel}
!5 = !{i32 0}
!6 = !{i32 1}
!7 = !{!"none"}
!8 = !{!"uint*"}
!9 = !{!""}
!10 = !{i1 false}
!11 = !{!"sub_groups_sizes"}
!12 = !{void (i32 addrspace(1)*)* @__Vectorized_.testKernel}
!13 = !{i1 true}
!14 = !{null}
!15 = !{i32 4}
!16 = !{!17, !17, i64 0}
!17 = !{!"int", !18, i64 0}
!18 = !{!"omnipotent char", !19, i64 0}
!19 = !{!"Simple C/C++ TBAA"}
!20 = !{i32 16}
!21 = !{i32 5}

