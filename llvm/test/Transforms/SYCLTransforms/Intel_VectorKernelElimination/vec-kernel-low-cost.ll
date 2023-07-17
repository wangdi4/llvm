; RUN: opt -passes='require<sycl-kernel-builtin-info-analysis>,sycl-kernel-vec-kernel-elim' -S %s | FileCheck %s
; RUN: opt -passes='require<sycl-kernel-builtin-info-analysis>,sycl-kernel-vec-kernel-elim' -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

; This test checks that vectorized kernel is kept if its cost is not large. 

; CHECK: _ZGVbN4u_test

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent norecurse nounwind
define dso_local void @test(ptr addrspace(1) noalias noundef align 8 %dst) local_unnamed_addr #0 !kernel_arg_addr_space !1 !kernel_arg_access_qual !2 !kernel_arg_type !3 !kernel_arg_base_type !3 !kernel_arg_type_qual !4 !kernel_arg_name !5 !kernel_arg_host_accessible !6 !kernel_arg_pipe_depth !7 !kernel_arg_pipe_io !4 !kernel_arg_buffer_location !4 !no_barrier_path !8 !kernel_has_sub_groups !6 !vectorized_kernel !9 !max_wg_dimensions !1 !vectorized_width !1 !arg_type_null_val !15 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 noundef 0) #3
  %arrayidx = getelementptr inbounds i64, ptr addrspace(1) %dst, i64 %call
  store i64 42, ptr addrspace(1) %arrayidx, align 8, !tbaa !10
  ret void
}

; Function Attrs: convergent mustprogress nofree nounwind readnone willreturn
declare i64 @_Z13get_global_idj(i32 noundef) local_unnamed_addr #1

; Function Attrs: convergent norecurse nounwind
define dso_local void @_ZGVbN4u_test(ptr addrspace(1) noalias noundef align 8 %dst) local_unnamed_addr #2 !kernel_arg_addr_space !1 !kernel_arg_access_qual !2 !kernel_arg_type !3 !kernel_arg_base_type !3 !kernel_arg_type_qual !4 !kernel_arg_name !5 !kernel_arg_host_accessible !6 !kernel_arg_pipe_depth !7 !kernel_arg_pipe_io !4 !kernel_arg_buffer_location !4 !no_barrier_path !8 !kernel_has_sub_groups !6 !max_wg_dimensions !1 !vectorized_width !14 !vectorization_dimension !7 !can_unite_workgroups !8 !scalar_kernel !0 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 noundef 0) #3
  %0 = trunc i64 %call to i32
  %1 = add nuw i32 0, %0
  %.extract.0. = sext i32 %1 to i64
  %scalar.gep = getelementptr inbounds i64, ptr addrspace(1) %dst, i64 %.extract.0.
  store <4 x i64> <i64 42, i64 42, i64 42, i64 42>, ptr addrspace(1) %scalar.gep, align 8
  ret void
}

attributes #0 = { convergent norecurse nounwind "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "prefer-vector-width"="128" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" "vector-variants"="_ZGVbN4u_test" }
attributes #1 = { convergent mustprogress nofree nounwind readnone willreturn "frame-pointer"="none" "no-trapping-math"="true" "prefer-vector-width"="128" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #2 = { convergent norecurse nounwind "frame-pointer"="none" "may-have-openmp-directive"="false" "min-legal-vector-width"="0" "no-trapping-math"="true" "prefer-vector-width"="128" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" "vector-variants"="_ZGVbN4u_test" }
attributes #3 = { convergent nounwind readnone willreturn }

!sycl.kernels = !{!0}

!0 = !{ptr @test}
!1 = !{i32 1}
!2 = !{!"none"}
!3 = !{!"long*"}
!4 = !{!""}
!5 = !{!"dst"}
!6 = !{i1 false}
!7 = !{i32 0}
!8 = !{i1 true}
!9 = !{ptr @_ZGVbN4u_test}
!10 = !{!11, !11, i64 0}
!11 = !{!"long", !12, i64 0}
!12 = !{!"omnipotent char", !13, i64 0}
!13 = !{!"Simple C/C++ TBAA"}
!14 = !{i32 4}
!15 = !{ptr addrspace(1) null, ptr addrspace(1) null, ptr addrspace(1) null}

; DEBUGIFY-NOT: WARNING
