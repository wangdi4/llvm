; RUN: opt -dpcpp-kernel-wgloop-creator %s -S | FileCheck %s
; RUN: opt -dpcpp-kernel-wgloop-creator %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-wgloop-creator %s -S | FileCheck %s
; RUN: opt -passes=dpcpp-kernel-wgloop-creator %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

; CHECK-NOT: define void @_ZGVcN4u_test
; CHECK-NOT: define void @_ZGVcM4u_test
; CHECK: masked_vect_if
; CHECK: mask_generate
; CHECK: masked_kernel_entry
; CHECK: define void @foo
; CHECK: scalar_kernel_entry

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent nounwind
define void @test(i32 addrspace(1)* noalias %a) local_unnamed_addr #0 !kernel_arg_addr_space !1 !kernel_arg_access_qual !2 !kernel_arg_type !3 !kernel_arg_base_type !3 !kernel_arg_type_qual !4 !kernel_arg_host_accessible !5 !kernel_arg_pipe_depth !6 !kernel_arg_pipe_io !4 !kernel_arg_buffer_location !4 !kernel_arg_name !7 !vectorized_kernel !8 !vectorized_masked_kernel !9 !no_barrier_path !10 !kernel_has_sub_groups !10 !vectorized_width !1 !scalar_kernel !11 !kernel_execution_length !12 !kernel_has_barrier !5 !kernel_has_global_sync !5 {
entry:
  ret void
}

; Function Attrs: convergent
declare i32 @_Z22get_max_sub_group_sizev() local_unnamed_addr #1

define [7 x i64] @WG.boundaries.test(i32 addrspace(1)* %0) !recommended_vector_length !13 {
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
define void @_ZGVcN4u_test(i32 addrspace(1)* noalias %a) local_unnamed_addr #0 !kernel_arg_addr_space !1 !kernel_arg_access_qual !2 !kernel_arg_type !3 !kernel_arg_base_type !3 !kernel_arg_type_qual !4 !kernel_arg_host_accessible !5 !kernel_arg_pipe_depth !6 !kernel_arg_pipe_io !4 !kernel_arg_buffer_location !4 !kernel_arg_name !7 !vectorized_kernel !11 !no_barrier_path !10 !kernel_has_sub_groups !10 !vectorized_width !13 !scalar_kernel !0 !kernel_execution_length !12 !kernel_has_barrier !5 !kernel_has_global_sync !5 !recommended_vector_length !13 !vectorization_dimension !6 !can_unite_workgroups !5 {
entry:
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: convergent nounwind
define void @_ZGVcM4u_test(i32 addrspace(1)* noalias %a, <4 x i32> %mask) local_unnamed_addr #0 !kernel_arg_addr_space !1 !kernel_arg_access_qual !2 !kernel_arg_type !3 !kernel_arg_base_type !3 !kernel_arg_type_qual !4 !kernel_arg_host_accessible !5 !kernel_arg_pipe_depth !6 !kernel_arg_pipe_io !4 !kernel_arg_buffer_location !4 !kernel_arg_name !7 !vectorized_kernel !11 !no_barrier_path !10 !kernel_has_sub_groups !10 !vectorized_width !13 !scalar_kernel !0 !kernel_execution_length !14 !kernel_has_barrier !5 !kernel_has_global_sync !5 !recommended_vector_length !13 !vectorization_dimension !6 !can_unite_workgroups !5 {
entry:
  ret void
}

; Function Attrs: nounwind
define void @foo(i32 addrspace(1)* noalias %a) local_unnamed_addr #2 !kernel_arg_addr_space !1 !kernel_arg_access_qual !2 !kernel_arg_type !3 !kernel_arg_base_type !3 !kernel_arg_type_qual !4 !kernel_arg_host_accessible !5 !kernel_arg_pipe_depth !6 !kernel_arg_pipe_io !4 !kernel_arg_buffer_location !4 !kernel_arg_name !7 !no_barrier_path !10 !kernel_has_barrier !5 !kernel_has_global_sync !5 {
entry:
  ret void
}

; Function Attrs: nofree nosync nounwind willreturn writeonly
declare void @llvm.masked.scatter.v4i32.v4p1i32(<4 x i32>, <4 x i32 addrspace(1)*>, i32 immarg, <4 x i1>) #3

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" "vector-variants"="_ZGVcN4u_test,_ZGVcM4u_test" }
attributes #1 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }
attributes #3 = { nofree nosync nounwind willreturn writeonly }

!sycl.kernels = !{!0}

!0 = !{void (i32 addrspace(1)*)* @test, void (i32 addrspace(1)*)* @foo}
!1 = !{i32 1}
!2 = !{!"none"}
!3 = !{!"uint*"}
!4 = !{!""}
!5 = !{i1 false}
!6 = !{i32 0}
!7 = !{!"a"}
!8 = !{void (i32 addrspace(1)*)* @_ZGVcN4u_test}
!9 = !{void (i32 addrspace(1)*, <4 x i32>)* @_ZGVcM4u_test}
!10 = !{i1 true}
!11 = !{null}
!12 = !{i32 2}
!13 = !{i32 4}
!14 = !{i32 22}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY-COUNT-50: WARNING: Instruction with empty DebugLoc in function test
; DEBUGIFY-COUNT-20: WARNING: Instruction with empty DebugLoc in function foo
; DEBUGIFY: WARNING: Missing line 1
; DEBUGIFY: WARNING: Missing line 16
; DEBUGIFY: WARNING: Missing variable 1
; DEBUGIFY-NOT: WARNING
