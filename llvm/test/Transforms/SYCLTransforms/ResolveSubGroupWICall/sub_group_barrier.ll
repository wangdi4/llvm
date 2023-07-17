; RUN: llvm-as %S/builtin_lib.rtl -o %t.rtl.bc
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes='debugify,sycl-kernel-resolve-sub-group-wi-call,check-debugify' -S %s -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes='sycl-kernel-resolve-sub-group-wi-call' -S %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent
declare void @_Z17sub_group_barrierj(i32) local_unnamed_addr #1

declare i64 @_Z14get_local_sizej(i32)

; Function Attrs: convergent nounwind
define void @testKernel() local_unnamed_addr #0 !kernel_arg_addr_space !2 !kernel_arg_access_qual !2 !kernel_arg_type !2 !kernel_arg_base_type !2 !kernel_arg_type_qual !2 !kernel_arg_host_accessible !2 !kernel_arg_pipe_depth !2 !kernel_arg_pipe_io !2 !kernel_arg_buffer_location !2 !kernel_arg_name !2 !vectorized_kernel !6 !no_barrier_path !7 !kernel_has_sub_groups !7 !vectorization_dimension !11 !can_unite_workgroups !12 {
entry:
  ret void
}

; CHECK-LABEL: @__Vectorized_.testKernel
; Function Attrs: convergent nounwind
define void @__Vectorized_.testKernel() local_unnamed_addr #0 !kernel_arg_addr_space !2 !kernel_arg_access_qual !2 !kernel_arg_type !2 !kernel_arg_base_type !2 !kernel_arg_type_qual !2 !kernel_arg_host_accessible !2 !kernel_arg_pipe_depth !2 !kernel_arg_pipe_io !2 !kernel_arg_buffer_location !2 !kernel_arg_name !2 !vectorized_kernel !8 !no_barrier_path !7 !kernel_has_sub_groups !7 !vectorized_width !10 !vectorization_dimension !11 !can_unite_workgroups !12 {
entry:
; CHECK-NOT: @_Z17sub_group_barrierj
; CHECK: @_Z22atomic_work_item_fencej12memory_order12memory_scope(i32 2, i32 4, i32 4)
  call void @_Z17sub_group_barrierj(i32 2) #3
  ret void
}

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { convergent nounwind }
attributes #3 = { nounwind }

!llvm.linker.options = !{}
!llvm.module.flags = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!1}
!sycl.kernels = !{!4}
!opencl.spir.version = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 2, i32 0}
!2 = !{}
!3 = !{!"-cl-std=CL2.0"}
!4 = !{ptr @testKernel}
!6 = !{ptr @__Vectorized_.testKernel}
!7 = !{i1 true}
!8 = !{null}
!9 = !{i32 1}
!10 = !{i32 16}
!11 = !{i32 0}
!12 = !{i1 false}
;DEBUGIFY-NOT: WARNING
