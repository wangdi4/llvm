; RUN: opt -passes='debugify,sycl-kernel-resolve-sub-group-wi-call,check-debugify' -S %s -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes='sycl-kernel-resolve-sub-group-wi-call' -S %s | FileCheck %s

declare i64 @get_sub_group_slice_length.(i32) local_unnamed_addr

; Function Attrs: convergent nounwind
define void @testKernel() local_unnamed_addr #0 !kernel_arg_addr_space !11 !kernel_arg_access_qual !12 !kernel_arg_type !13 !kernel_arg_base_type !13 !kernel_arg_type_qual !14 !kernel_arg_host_accessible !15 !kernel_arg_pipe_depth !16 !kernel_arg_pipe_io !14 !kernel_arg_buffer_location !14 !kernel_arg_name !17 !vectorized_kernel !18 !no_barrier_path !19 !kernel_has_sub_groups !19 !vectorized_width !26 !vectorization_dimension !16 !can_unite_workgroups !15 {
entry:
  ret void
}

; Function Attrs: convergent nounwind
; CHECK-LABEL: @__Vectorized_.testKernel
define void @__Vectorized_.testKernel() local_unnamed_addr #0 !kernel_arg_addr_space !11 !kernel_arg_access_qual !12 !kernel_arg_type !13 !kernel_arg_base_type !13 !kernel_arg_type_qual !14 !kernel_arg_host_accessible !15 !kernel_arg_pipe_depth !16 !kernel_arg_pipe_io !14 !kernel_arg_buffer_location !14 !kernel_arg_name !17 !vectorized_kernel !20 !no_barrier_path !19 !kernel_has_sub_groups !19 !vectorized_width !25 !vectorization_dimension !16 !can_unite_workgroups !15 {
entry:
; slice length will be resolved as ceil(144 / 16)
; CHECK-NOT: @get_sub_group_slice_length.
; CHECK: insertelement <16 x i32> undef, i32 0, i64 9
  %sg.slice.length = tail call i64 @get_sub_group_slice_length.(i32 144)
  %use = insertelement <16 x i32> undef, i32 0, i64 %sg.slice.length
  ret void
}

; The function would be extended to accept one more arg: 'vf', as it's called by two different kernels.
; CHECK: define [[TYPE:i.*]] @slice_length_caller([[TYPE]] %vf)
define i64 @slice_length_caller() {
entry:
; CHECK-NOT: @get_sub_group_slice_length.
; CHECK: [[TMP0:%.*]] = add nuw nsw [[TYPE]] %vf, 143
; CHECK-NEXT: udiv [[TYPE]] [[TMP0]], %vf
  %call = tail call i64 @get_sub_group_slice_length.(i32 144)
  ret i64 %call
}

define void @kernel1() !vectorized_width !{i32 16} {
entry:
  %call = tail call i64 @slice_length_caller()
  ret void
}

define void @kernel2() !vectorized_width !{i32 32} {
entry:
  %call = tail call i64 @slice_length_caller()
  ret void
}

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-sign
ed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.linker.options = !{}
!llvm.module.flags = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!1}
!sycl.kernels = !{!3}
!opencl.spir.version = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, i32 2}
!2 = !{}
!3 = !{ptr @testKernel, ptr @kernel1, ptr @kernel2}
!11 = !{i32 1}
!12 = !{!"none"}
!13 = !{!"uint*"}
!14 = !{!""}
!15 = !{i1 false}
!16 = !{i32 0}
!17 = !{!"sub_groups_sizes"}
!18 = !{ptr @__Vectorized_.testKernel}
!19 = !{i1 true}
!20 = !{null}
!21 = !{!22, !22, i64 0}
!22 = !{!"int", !23, i64 0}
!23 = !{!"omnipotent char", !24, i64 0}
!24 = !{!"Simple C/C++ TBAA"}
!25 = !{i32 16}
!26 = !{i32 1}

; The get_sub_group_slice_length. is replaced with constant and related debug
; location will be lost as expected.
; DEBUGIFY: Missing line 2

; DEBUGIFY-NOT: WARNING
