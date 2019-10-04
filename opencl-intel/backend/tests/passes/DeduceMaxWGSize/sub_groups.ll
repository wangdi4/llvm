; RUN: %oclopt -deduce-max-dim -S < %s -runtimelib %p/../../vectorizer/Full/runtime.bc | FileCheck %s
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK-NOT: max_wg_dimensions
; Function Attrs: convergent nounwind
define void @kern(i32 addrspace(1)* noalias %res) local_unnamed_addr #0 !kernel_arg_addr_space !6 !kernel_arg_access_qual !7 !kernel_arg_type !8 !kernel_arg_base_type !8 !kernel_arg_type_qual !9 !kernel_arg_host_accessible !10 !kernel_arg_pipe_depth !5 !kernel_arg_pipe_io !9 !kernel_arg_buffer_location !9 !kernel_arg_name !11 !vectorized_kernel !12 !no_barrier_path !13 !kernel_has_sub_groups !13 !vectorized_width !6 !scalarized_kernel !14 !kernel_execution_length !15 !kernel_has_barrier !10 !kernel_has_global_sync !10 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #4
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %res, i64 %call
  store i32 0, i32 addrspace(1)* %arrayidx, align 4, !tbaa !16
  ret void
}

; Function Attrs: convergent nounwind readnone
declare i64 @_Z13get_global_idj(i32 %0) local_unnamed_addr #1

; Function Attrs: convergent
declare i32 @_Z22get_sub_group_local_idv() local_unnamed_addr #2

declare i64 @_Z14get_local_sizej(i32 %0)

declare i64 @get_base_global_id.(i32 %0)

; Function Attrs: convergent nounwind
define void @_ZGVeN16u_kern(i32 addrspace(1)* noalias %res) local_unnamed_addr #0 !kernel_arg_addr_space !6 !kernel_arg_access_qual !7 !kernel_arg_type !8 !kernel_arg_base_type !8 !kernel_arg_type_qual !9 !kernel_arg_host_accessible !10 !kernel_arg_pipe_depth !5 !kernel_arg_pipe_io !9 !kernel_arg_buffer_location !9!kernel_arg_name !11 !vectorized_kernel !14 !no_barrier_path !13 !kernel_has_sub_groups !13 !ocl_recommended_vector_length !20 !vectorized_width !20 !vectorization_dimension !5 !scalarized_kernel !4 !can_unite_workgroups !10 !kernel_execution_length !21 !kernel_has_barrier !10 !kernel_has_global_sync !10 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #4
  %scalar.gep. = getelementptr inbounds i32, i32 addrspace(1)* %res, i64 %call
  %0 = bitcast i32 addrspace(1)* %scalar.gep. to <16 x i32> addrspace(1)*
  store <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>, <16 x i32> addrspace(1)* %0, align 4
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token %0) #3

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" "vector-variants"="_ZGVeN16u_kern" }
attributes #1 = { convergent nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }
attributes #4 = { convergent nounwind readnone }

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
!4 = !{void (i32 addrspace(1)*)* @kern}
!5 = !{i32 0}
!6 = !{i32 1}
!7 = !{!"none"}
!8 = !{!"uint*"}
!9 = !{!""}
!10 = !{i1 false}
!11 = !{!"res"}
!12 = !{void (i32 addrspace(1)*)* @_ZGVeN16u_kern}
!13 = !{i1 true}
!14 = !{null}
!15 = !{i32 4}
!16 = !{!17, !17, i64 0}
!17 = !{!"int", !18, i64 0}
!18 = !{!"omnipotent char", !19, i64 0}
!19 = !{!"Simple C/C++ TBAA"}
!20 = !{i32 16}
!21 = !{i32 5}

