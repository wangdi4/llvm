; RUN: %oclopt -resolve-sub-group-wi-call -S < %s | FileCheck %s
; ModuleID = 'main'
source_filename = "1"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent nounwind readnone
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #1

; Function Attrs: convergent
declare i32 @_Z22get_max_sub_group_sizev() local_unnamed_addr #2

declare i64 @_Z14get_local_sizej(i32)

; Function Attrs: convergent nounwind
define void @testKernel(i32 addrspace(1)* noalias %sub_groups_sizes) local_unnamed_addr #0 !kernel_arg_addr_space !11 !kernel_arg_access_qual !12 !kernel_arg_type !13 !kernel_arg_base_type !13 !kernel_arg_type_qual !14 !kernel_arg_host_accessible !15 !kernel_arg_pipe_depth !16 !kernel_arg_pipe_io !14 !kernel_arg_buffer_location !14 !kernel_arg_name !17 !vectorized_kernel !18 !no_barrier_path !19 !kernel_has_sub_groups !19 !vectorized_width !26 !vectorization_dimension !16 !can_unite_workgroups !15 {
entry:
  ret void
}

; Function Attrs: convergent nounwind
; CHECK-LABEL: @__Vectorized_.testKernel
define void @__Vectorized_.testKernel(i32 addrspace(1)* noalias %sub_groups_sizes) local_unnamed_addr #0 !kernel_arg_addr_space !11 !kernel_arg_access_qual !12 !kernel_arg_type !13 !kernel_arg_base_type !13 !kernel_arg_type_qual !14 !kernel_arg_host_accessible !15 !kernel_arg_pipe_depth !16 !kernel_arg_pipe_io !14 !kernel_arg_buffer_location !14 !kernel_arg_name !17 !vectorized_kernel !20 !no_barrier_path !19 !kernel_has_sub_groups !19 !vectorized_width !25 !vectorization_dimension !16 !can_unite_workgroups !15 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #3
  %call1 = tail call i32 @_Z22get_max_sub_group_sizev() #4
; CHECK-NOT: @_Z22get_max_sub_group_sizev
; CHECK: insertelement <16 x i32> undef
; CHECK-SAME: i32 16, i32 0
  %temp17 = insertelement <16 x i32> undef, i32 %call1, i32 0
  %vector16 = shufflevector <16 x i32> %temp17, <16 x i32> undef, <16 x i32> zeroinitializer
  %0 = getelementptr inbounds i32, i32 addrspace(1)* %sub_groups_sizes, i64 %call
  %ptrTypeCast = bitcast i32 addrspace(1)* %0 to <16 x i32> addrspace(1)*
  store <16 x i32> %vector16, <16 x i32> addrspace(1)* %ptrTypeCast, align 4
  ret void
}

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-sign
ed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { convergent nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"=
"false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-pro
tector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { convergent nounwind readnone }
attributes #4 = { convergent nounwind }

!llvm.linker.options = !{}
!llvm.module.flags = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!1}
!opencl.kernels = !{!3}
!opencl.spir.version = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, i32 2}
!2 = !{}
!3 = !{void (i32 addrspace(1)*)* @testKernel}
!11 = !{i32 1}
!12 = !{!"none"}
!13 = !{!"uint*"}
!14 = !{!""}
!15 = !{i1 false}
!16 = !{i32 0}
!17 = !{!"sub_groups_sizes"}
!18 = !{void (i32 addrspace(1)*)* @__Vectorized_.testKernel}
!19 = !{i1 true}
!20 = !{null}
!21 = !{!22, !22, i64 0}
!22 = !{!"int", !23, i64 0}
!23 = !{!"omnipotent char", !24, i64 0}
!24 = !{!"Simple C/C++ TBAA"}
!25 = !{i32 16}
!26 = !{i32 1}
