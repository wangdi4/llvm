; RUN: opt -passes=sycl-kernel-wgloop-creator %s -S | FileCheck %s
; RUN: opt -passes=sycl-kernel-wgloop-creator %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

; CHECK-NOT: define void @_ZGVcN4u_test
; CHECK-NOT: define void @_ZGVcM4u_test

; CHECK-LABEL: define void @test{{.*}} {
; CHECK: masked_vect_if
; CHECK: mask_generate
; CHECK: masked_kernel_entry

; CHECK: exit:
; CHECK-NEXT: ret void

;; Original scalar body is removed.
; CHECK-NOT: entry:
; CHECK-NOT: store i32 1, ptr addrspace(1) %a, align 4, !tbaa !13
; CHECK-NOT: ret void
; CHECK: }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent nounwind
define void @test(ptr addrspace(1) noalias %a) local_unnamed_addr #0 !kernel_arg_addr_space !1 !kernel_arg_access_qual !2 !kernel_arg_type !3 !kernel_arg_base_type !3 !kernel_arg_type_qual !4 !kernel_arg_host_accessible !5 !kernel_arg_pipe_depth !6 !kernel_arg_pipe_io !4 !kernel_arg_buffer_location !4 !kernel_arg_name !7 !vectorized_kernel !8 !vectorized_masked_kernel !9 !no_barrier_path !10 !kernel_has_sub_groups !10 !vectorized_width !1 !scalar_kernel !11 !kernel_execution_length !12 !kernel_has_barrier !5 !kernel_has_global_sync !5 {
entry:
  store i32 1, ptr addrspace(1) %a, align 4, !tbaa !13
  ret void
}

; Function Attrs: convergent
declare i32 @_Z22get_max_sub_group_sizev() local_unnamed_addr #1

define [7 x i64] @WG.boundaries.test(ptr addrspace(1) %0) !recommended_vector_length !17 {
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
define void @_ZGVcN4u_test(ptr addrspace(1) noalias %a) local_unnamed_addr #0 !kernel_arg_addr_space !1 !kernel_arg_access_qual !2 !kernel_arg_type !3 !kernel_arg_base_type !3 !kernel_arg_type_qual !4 !kernel_arg_host_accessible !5 !kernel_arg_pipe_depth !6 !kernel_arg_pipe_io !4 !kernel_arg_buffer_location !4 !kernel_arg_name !7 !vectorized_kernel !11 !no_barrier_path !10 !kernel_has_sub_groups !10 !vectorized_width !17 !scalar_kernel !0 !kernel_execution_length !12 !kernel_has_barrier !5 !kernel_has_global_sync !5 !recommended_vector_length !17 !vectorization_dimension !6 !can_unite_workgroups !5 {
entry:
  store i32 4, ptr addrspace(1) %a, align 4
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: convergent nounwind
define void @_ZGVcM4u_test(ptr addrspace(1) noalias %a, <4 x i32> %mask) local_unnamed_addr #0 !kernel_arg_addr_space !1 !kernel_arg_access_qual !2 !kernel_arg_type !3 !kernel_arg_base_type !3 !kernel_arg_type_qual !4 !kernel_arg_host_accessible !5 !kernel_arg_pipe_depth !6 !kernel_arg_pipe_io !4 !kernel_arg_buffer_location !4 !kernel_arg_name !7 !vectorized_kernel !11 !no_barrier_path !10 !kernel_has_sub_groups !10 !vectorized_width !17 !scalar_kernel !0 !kernel_execution_length !18 !kernel_has_barrier !5 !kernel_has_global_sync !5 !recommended_vector_length !17 !vectorization_dimension !6 !can_unite_workgroups !5 {
entry:
  %broadcast.splatinsert = insertelement <4 x ptr addrspace(1)> undef, ptr addrspace(1) %a, i32 0
  %broadcast.splat = shufflevector <4 x ptr addrspace(1)> %broadcast.splatinsert, <4 x ptr addrspace(1)> undef, <4 x i32> zeroinitializer
  %0 = icmp ne <4 x i32> %mask, zeroinitializer
  %Predicate = extractelement <4 x i1> %0, i64 0
  br i1 %Predicate, label %pred.call.if, label %pred.call.continue

pred.call.if:                                     ; preds = %entry
  br label %pred.call.continue

pred.call.continue:                               ; preds = %pred.call.if, %entry
  %Predicate2 = extractelement <4 x i1> %0, i64 1
  br i1 %Predicate2, label %pred.call.if5, label %pred.call.continue6

pred.call.if5:                                    ; preds = %pred.call.continue
  br label %pred.call.continue6

pred.call.continue6:                              ; preds = %pred.call.if5, %pred.call.continue
  %1 = phi <4 x i32> [ <i32 4, i32 undef, i32 undef, i32 undef>, %pred.call.continue ], [ <i32 4, i32 4, i32 undef, i32 undef>, %pred.call.if5 ]
  %Predicate3 = extractelement <4 x i1> %0, i64 2
  br i1 %Predicate3, label %pred.call.if7, label %pred.call.continue8

pred.call.if7:                                    ; preds = %pred.call.continue6
  %2 = insertelement <4 x i32> %1, i32 4, i32 2
  br label %pred.call.continue8

pred.call.continue8:                              ; preds = %pred.call.if7, %pred.call.continue6
  %3 = phi <4 x i32> [ %1, %pred.call.continue6 ], [ %2, %pred.call.if7 ]
  %Predicate4 = extractelement <4 x i1> %0, i64 3
  br i1 %Predicate4, label %pred.call.if9, label %pred.call.continue10

pred.call.if9:                                    ; preds = %pred.call.continue8
  %4 = insertelement <4 x i32> %3, i32 4, i32 3
  br label %pred.call.continue10

pred.call.continue10:                             ; preds = %pred.call.if9, %pred.call.continue8
  %5 = phi <4 x i32> [ %3, %pred.call.continue8 ], [ %4, %pred.call.if9 ]
  call void @llvm.masked.scatter.v4i32.v4p1(<4 x i32> %5, <4 x ptr addrspace(1)> %broadcast.splat, i32 4, <4 x i1> %0)
  ret void
}

; Function Attrs: nofree nosync nounwind willreturn writeonly
declare void @llvm.masked.scatter.v4i32.v4p1(<4 x i32>, <4 x ptr addrspace(1)>, i32 immarg, <4 x i1>) #3

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" "vector-variants"="_ZGVcN4u_test,_ZGVcM4u_test" }
attributes #1 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }
attributes #3 = { nofree nosync nounwind willreturn writeonly }

!sycl.kernels = !{!0}

!0 = !{ptr @test}
!1 = !{i32 1}
!2 = !{!"none"}
!3 = !{!"uint*"}
!4 = !{!""}
!5 = !{i1 false}
!6 = !{i32 0}
!7 = !{!"a"}
!8 = !{ptr @_ZGVcN4u_test}
!9 = !{ptr @_ZGVcM4u_test}
!10 = !{i1 true}
!11 = !{null}
!12 = !{i32 2}
!13 = !{!14, !14, i64 0}
!14 = !{!"int", !15, i64 0}
!15 = !{!"omnipotent char", !16, i64 0}
!16 = !{!"Simple C/C++ TBAA"}
!17 = !{i32 4}
!18 = !{i32 22}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY-COUNT-50: WARNING: Instruction with empty DebugLoc in function test
; DEBUGIFY: WARNING: Missing line 1
; DEBUGIFY: WARNING: Missing line 2
; DEBUGIFY: WARNING: Missing line 18
; DEBUGIFY: WARNING: Missing variable 1
; DEBUGIFY-NOT: WARNING
