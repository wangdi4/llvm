; Functions with only return instructions are not eligible for VecClone and VPlan.
; RUN: %oclopt  -runtimelib %p/../Full/runtime.bc -ocl-postvect -verify %s -S -o - \
; RUN: | FileCheck %s

; ModuleID = 'main'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: nounwind
define void @"_ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE7kernel0"() local_unnamed_addr #0 !kernel_arg_addr_space !3 !kernel_arg_access_qual !3 !kernel_arg_type !3 !kernel_arg_type_qual !3 !kernel_arg_base_type !3 !no_barrier_path !7 !ocl_recommended_vector_length !8 {
  ret void
}

define [7 x i64] @"WG.boundaries._ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE7kernel0"() !ocl_recommended_vector_length !8 {
entry:
  %0 = call i64 @_Z14get_local_sizej(i32 0)
  %1 = call i64 @get_base_global_id.(i32 0)
  %2 = call i64 @_Z14get_local_sizej(i32 1)
  %3 = call i64 @get_base_global_id.(i32 1)
  %4 = call i64 @_Z14get_local_sizej(i32 2)
  %5 = call i64 @get_base_global_id.(i32 2)
  %6 = insertvalue [7 x i64] undef, i64 %0, 2
  %7 = insertvalue [7 x i64] %6, i64 %1, 1
  %8 = insertvalue [7 x i64] %7, i64 %2, 4
  %9 = insertvalue [7 x i64] %8, i64 %3, 3
  %10 = insertvalue [7 x i64] %9, i64 %4, 6
  %11 = insertvalue [7 x i64] %10, i64 %5, 5
  %12 = insertvalue [7 x i64] %11, i64 1, 0
  ret [7 x i64] %12
}

declare i64 @_Z14get_local_sizej(i32)

declare i64 @get_base_global_id.(i32)

attributes #0 = { nounwind }

!opencl.enable.FP_CONTRACT = !{}
!spirv.Source = !{!0}
!opencl.spir.version = !{!1}
!opencl.ocl.version = !{!2}
!opencl.used.extensions = !{!3}
!opencl.used.optional.core.features = !{!3}
!spirv.Generator = !{!4}
!opencl.kernels = !{!5}
!opencl.gen_addr_space_pointer_counter = !{!6}

!0 = !{i32 4, i32 100000}
!1 = !{i32 1, i32 2}
!2 = !{i32 1, i32 0}
!3 = !{}
!4 = !{i16 6, i16 14}
!5 = !{void ()* @"_ZTSZZ4mainENK3$_0clERN2cl4sycl7handlerEE7kernel0"}
!6 = !{i32 0}
!7 = !{i1 true}
!8 = !{i32 8}

