; RUN: opt -passes=sycl-kernel-wgloop-creator %s -S | FileCheck %s
; RUN: opt -passes=sycl-kernel-wgloop-creator %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

; Check that metadata is not duplicated.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK: define {{.*}}@test(
; CHECK-SAME: !kernel_arg_addr_space !{{[0-9]+}} !kernel_arg_access_qual !{{[0-9]+}} !kernel_arg_type

; Function Attrs: convergent mustprogress nofree norecurse nounwind willreturn writeonly
define dso_local void @test(ptr addrspace(1) nocapture writeonly %dst) local_unnamed_addr #0 !kernel_arg_addr_space !3 !kernel_arg_access_qual !4 !kernel_arg_type !5 !kernel_arg_base_type !5 !kernel_arg_type_qual !6 !kernel_arg_name !7 !kernel_arg_host_accessible !8 !kernel_arg_pipe_depth !9 !kernel_arg_pipe_io !6 !kernel_arg_buffer_location !6 !no_barrier_path !10 !kernel_has_sub_groups !8 !vectorized_kernel !11 !vectorized_width !3 !recommended_vector_length !9 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #2
  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %dst, i64 %call
  store i32 0, ptr addrspace(1) %arrayidx, align 4, !tbaa !12
  ret void
}

; Function Attrs: convergent mustprogress nofree nounwind readnone willreturn
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #1

; Function Attrs: convergent mustprogress nofree norecurse nounwind willreturn writeonly
define dso_local void @_ZGVeN16u_test(ptr addrspace(1) nocapture writeonly %dst) local_unnamed_addr #0 !kernel_arg_addr_space !3 !kernel_arg_access_qual !4 !kernel_arg_type !5 !kernel_arg_base_type !5 !kernel_arg_type_qual !6 !kernel_arg_name !7 !kernel_arg_host_accessible !8 !kernel_arg_pipe_depth !9 !kernel_arg_pipe_io !6 !kernel_arg_buffer_location !6 !no_barrier_path !10 !kernel_has_sub_groups !8 !vectorized_width !16 !recommended_vector_length !16 !scalar_kernel !2 !vectorization_dimension !9 !can_unite_workgroups !8 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #2
  %sext = shl i64 %call, 32
  %.extract.0. = ashr exact i64 %sext, 32
  %scalar.gep = getelementptr inbounds i32, ptr addrspace(1) %dst, i64 %.extract.0.
  store <16 x i32> zeroinitializer, ptr addrspace(1) %scalar.gep, align 4
  ret void
}

attributes #0 = { convergent mustprogress nofree norecurse nounwind willreturn writeonly "vector-variants"="_ZGVeN16u_test" }
attributes #1 = { convergent mustprogress nofree nounwind readnone willreturn }
attributes #2 = { convergent nounwind readnone willreturn }

!llvm.linker.options = !{}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!opencl.compiler.options = !{!1}
!sycl.kernels = !{!2}

!0 = !{i32 2, i32 0}
!1 = !{!"-cl-std=CL2.0"}
!2 = !{ptr @test}
!3 = !{i32 1}
!4 = !{!"none"}
!5 = !{!"int*"}
!6 = !{!""}
!7 = !{!"dst"}
!8 = !{i1 false}
!9 = !{i32 0}
!10 = !{i1 true}
!11 = !{ptr @_ZGVeN16u_test}
!12 = !{!13, !13, i64 0}
!13 = !{!"int", !14, i64 0}
!14 = !{!"omnipotent char", !15, i64 0}
!15 = !{!"Simple C/C++ TBAA"}
!16 = !{i32 16}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY-COUNT-61: WARNING: Instruction with empty DebugLoc in function test
; DEBUGIFY: WARNING: Missing line 10
; DEBUGIFY-NOT: WARNING
