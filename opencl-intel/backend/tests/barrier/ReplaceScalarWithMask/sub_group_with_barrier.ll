; RUN: %oclopt -B-ReplaceScalarWithMask -S < %s | FileCheck %s

; CHECK-NOT: define void @_ZGVcM4u_test
; CHECK: @test
; CHECK: !vectorized_width ![[VF_METADATA:[0-9]+]]
; CHECK: @_Z18get_sub_group_sizev()
; CHECK-LABEL: @_ZGVcN4u_test
; CHECK: ![[VF_METADATA:[0-9]+]] = !{i32 4}

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent nounwind
define void @test(i32 addrspace(1)* noalias %a) local_unnamed_addr #0 !kernel_arg_addr_space !5 !kernel_arg_access_qual !6 !kernel_arg_type !7 !kernel_arg_base_type !7 !kernel_arg_type_qual !8 !kernel_arg_host_accessible !9 !kernel_arg_pipe_depth !10 !kernel_arg_pipe_io !8 !kernel_arg_buffer_location !8 !kernel_arg_name !11 !vectorized_kernel !12 !vectorized_masked_kernel !13 !no_barrier_path !9 !kernel_has_sub_groups !14 !vectorized_width !5 !scalarized_kernel !15 !kernel_execution_length !16 !kernel_has_barrier !14 !kernel_has_global_sync !9 {
entry:
  store i32 1, i32 addrspace(1)* %a, align 4, !tbaa !17
  tail call void @_Z7barrierj(i32 1) #5
  ret void
}

; Function Attrs: convergent
declare i32 @_Z22get_max_sub_group_sizev() local_unnamed_addr #1

; Function Attrs: convergent
declare void @_Z7barrierj(i32) local_unnamed_addr #2

; Function Attrs: convergent nounwind
define void @_ZGVcN4u_test(i32 addrspace(1)* noalias %a) local_unnamed_addr #0 !kernel_arg_addr_space !5 !kernel_arg_access_qual !6 !kernel_arg_type !7 !kernel_arg_base_type !7 !kernel_arg_type_qual !8 !kernel_arg_host_accessible !9 !kernel_arg_pipe_depth !10 !kernel_arg_pipe_io !8 !kernel_arg_buffer_location !8 !kernel_arg_name !11 !vectorized_kernel !15 !no_barrier_path !9 !kernel_has_sub_groups !14 !ocl_recommended_vector_length !21 !vectorized_width !21 !vectorization_dimension !10 !scalarized_kernel !4 !can_unite_workgroups !9 !kernel_execution_length !16 !kernel_has_barrier !14 !kernel_has_global_sync !9 {
entry:
  store i32 4, i32 addrspace(1)* %a, align 4
  call void @_Z7barrierj(i32 1) #3
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

; Function Attrs: convergent nounwind
define void @_ZGVcM4u_test(i32 addrspace(1)* noalias %a, <4 x i32> %mask) local_unnamed_addr #0 !kernel_arg_addr_space !5 !kernel_arg_access_qual !6 !kernel_arg_type !7 !kernel_arg_base_type !7 !kernel_arg_type_qual !8 !kernel_arg_host_accessible !9 !kernel_arg_pipe_depth !10 !kernel_arg_pipe_io !8 !kernel_arg_buffer_location !8 !kernel_arg_name !11 !vectorized_kernel !15 !no_barrier_path !9 !kernel_has_sub_groups !14 !ocl_recommended_vector_length !21 !vectorized_width !21 !vectorization_dimension !10 !scalarized_kernel !4 !can_unite_workgroups !9 !kernel_execution_length !22 !kernel_has_barrier !14 !kernel_has_global_sync !9 {
entry:
  %broadcast.splatinsert = insertelement <4 x i32 addrspace(1)*> undef, i32 addrspace(1)* %a, i32 0
  %broadcast.splat = shufflevector <4 x i32 addrspace(1)*> %broadcast.splatinsert, <4 x i32 addrspace(1)*> undef, <4 x i32> zeroinitializer
  %0 = icmp ne <4 x i32> %mask, zeroinitializer
  %Predicate = extractelement <4 x i1> %0, i64 0
  %Predicate2 = extractelement <4 x i1> %0, i64 1
  %1 = select i1 %Predicate2, <4 x i32> <i32 4, i32 4, i32 undef, i32 undef>, <4 x i32> <i32 4, i32 undef, i32 undef, i32 undef>
  %Predicate3 = extractelement <4 x i1> %0, i64 2
  %2 = insertelement <4 x i32> %1, i32 4, i32 2
  %spec.select = select i1 %Predicate3, <4 x i32> %2, <4 x i32> %1
  %Predicate4 = extractelement <4 x i1> %0, i64 3
  %3 = insertelement <4 x i32> %spec.select, i32 4, i32 3
  %4 = select i1 %Predicate4, <4 x i32> %3, <4 x i32> %spec.select
  call void @llvm.masked.scatter.v4i32.v4p1i32(<4 x i32> %4, <4 x i32 addrspace(1)*> %broadcast.splat, i32 4, <4 x i1> %0)
  %5 = bitcast <4 x i1> %0 to i4
  %6 = icmp eq i4 %5, 0
  br i1 %6, label %pred.call.continue12, label %pred.call.if11

pred.call.if11:                                   ; preds = %entry
  call void @_Z7barrierj(i32 1) #3
  br label %pred.call.continue12

pred.call.continue12:                             ; preds = %pred.call.if11, %entry
  ret void
}

; Function Attrs: nounwind willreturn
declare void @llvm.masked.scatter.v4i32.v4p1i32(<4 x i32>, <4 x i32 addrspace(1)*>, i32 immarg, <4 x i1>) #4

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "kernel-call-once" "kernel-convergent-call" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" "vector-variants"="_ZGVcN4u_test,_ZGVcM4u_test" }
attributes #1 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "kernel-call-once" "kernel-convergent-call" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }
attributes #4 = { nounwind willreturn }
attributes #5 = { convergent nounwind "kernel-call-once" "kernel-convergent-call" "kernel-uniform-call" }

!llvm.linker.options = !{}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!opencl.used.extensions = !{!1}
!opencl.used.optional.core.features = !{!1}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}
!opencl.kernels = !{!4}

!0 = !{i32 2, i32 0}
!1 = !{}
!2 = !{!"-cl-std=CL2.0"}
!3 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!4 = !{void (i32 addrspace(1)*)* @test}
!5 = !{i32 1}
!6 = !{!"none"}
!7 = !{!"uint*"}
!8 = !{!""}
!9 = !{i1 false}
!10 = !{i32 0}
!11 = !{!"a"}
!12 = !{void (i32 addrspace(1)*)* @_ZGVcN4u_test}
!13 = !{void (i32 addrspace(1)*, <4 x i32>)* @_ZGVcM4u_test}
!14 = !{i1 true}
!15 = !{null}
!16 = !{i32 3}
!17 = !{!18, !18, i64 0}
!18 = !{!"int", !19, i64 0}
!19 = !{!"omnipotent char", !20, i64 0}
!20 = !{!"Simple C/C++ TBAA"}
!21 = !{i32 4}
!22 = !{i32 27}
