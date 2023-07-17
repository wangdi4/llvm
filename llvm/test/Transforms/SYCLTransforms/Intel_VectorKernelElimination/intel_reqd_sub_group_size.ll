; RUN: opt -passes='require<sycl-kernel-builtin-info-analysis>,sycl-kernel-vec-kernel-elim' -S %s | FileCheck %s
; RUN: opt -passes='require<sycl-kernel-builtin-info-analysis>,sycl-kernel-vec-kernel-elim' -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

; This test checks that vectorized kernel is kept if intel_reqd_sub_group_size
; attribute is present.

; CHECK: _ZGVbN4uuu_test

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent norecurse nounwind
define dso_local void @test(ptr addrspace(1) noundef align 8 %dst, ptr addrspace(1) noundef align 8 %dst2, ptr addrspace(1) noundef align 8 %src) local_unnamed_addr #0 !kernel_arg_addr_space !1 !kernel_arg_access_qual !2 !kernel_arg_type !3 !kernel_arg_base_type !3 !kernel_arg_type_qual !4 !kernel_arg_name !5 !kernel_arg_host_accessible !6 !kernel_arg_pipe_depth !7 !kernel_arg_pipe_io !4 !kernel_arg_buffer_location !4 !no_barrier_path !8 !kernel_has_sub_groups !9 !vectorized_kernel !10 !max_wg_dimensions !11 !vectorized_width !11 !intel_reqd_sub_group_size !12 !arg_type_null_val !18 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 noundef 0) #5
  %mul = shl i64 %call, 6
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %add = add nuw nsw i64 %mul, %indvars.iv
  %arrayidx = getelementptr inbounds i64, ptr addrspace(1) %src, i64 %add
  %0 = load i64, ptr addrspace(1) %arrayidx, align 8, !tbaa !13
  %arrayidx4 = getelementptr inbounds i64, ptr addrspace(1) %dst, i64 %add
  store i64 %0, ptr addrspace(1) %arrayidx4, align 8, !tbaa !13
  %1 = load i64, ptr addrspace(1) %arrayidx, align 8, !tbaa !13
  %arrayidx12 = getelementptr inbounds i64, ptr addrspace(1) %dst2, i64 %add
  store i64 %1, ptr addrspace(1) %arrayidx12, align 8, !tbaa !13
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 64
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}

; Function Attrs: convergent mustprogress nofree nounwind readnone willreturn
declare i64 @_Z13get_global_idj(i32 noundef) local_unnamed_addr #1

; Function Attrs: convergent norecurse nounwind
define dso_local void @_ZGVbN4uuu_test(ptr addrspace(1) noundef align 8 %dst, ptr addrspace(1) noundef align 8 %dst2, ptr addrspace(1) noundef align 8 %src) local_unnamed_addr #2 !kernel_arg_addr_space !1 !kernel_arg_access_qual !2 !kernel_arg_type !3 !kernel_arg_base_type !3 !kernel_arg_type_qual !4 !kernel_arg_name !5 !kernel_arg_host_accessible !6 !kernel_arg_pipe_depth !7 !kernel_arg_pipe_io !4 !kernel_arg_buffer_location !4 !no_barrier_path !8 !kernel_has_sub_groups !9 !max_wg_dimensions !11 !vectorized_width !12 !intel_reqd_sub_group_size !12 !vectorization_dimension !17 !can_unite_workgroups !8 !scalar_kernel !0 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 noundef 0) #5
  %0 = trunc i64 %call to i32
  %broadcast.splatinsert = insertelement <4 x i32> poison, i32 %0, i64 0
  %broadcast.splat = shufflevector <4 x i32> %broadcast.splatinsert, <4 x i32> poison, <4 x i32> zeroinitializer
  %broadcast.splatinsert8 = insertelement <4 x ptr addrspace(1)> poison, ptr addrspace(1) %src, i64 0
  %broadcast.splat9 = shufflevector <4 x ptr addrspace(1)> %broadcast.splatinsert8, <4 x ptr addrspace(1)> poison, <4 x i32> zeroinitializer
  %broadcast.splatinsert10 = insertelement <4 x ptr addrspace(1)> poison, ptr addrspace(1) %dst, i64 0
  %broadcast.splat11 = shufflevector <4 x ptr addrspace(1)> %broadcast.splatinsert10, <4 x ptr addrspace(1)> poison, <4 x i32> zeroinitializer
  %broadcast.splatinsert14 = insertelement <4 x ptr addrspace(1)> poison, ptr addrspace(1) %dst2, i64 0
  %broadcast.splat15 = shufflevector <4 x ptr addrspace(1)> %broadcast.splatinsert14, <4 x ptr addrspace(1)> poison, <4 x i32> zeroinitializer
  %1 = add nuw <4 x i32> %broadcast.splat, <i32 0, i32 1, i32 2, i32 3>
  %2 = sext <4 x i32> %1 to <4 x i64>
  %3 = shl nsw <4 x i64> %2, <i64 6, i64 6, i64 6, i64 6>
  br label %VPlannedBB4

VPlannedBB4:                                      ; preds = %VPlannedBB4, %entry
  %uni.phi5 = phi i64 [ 0, %entry ], [ %5, %VPlannedBB4 ]
  %broadcast.splatinsert6 = insertelement <4 x i64> poison, i64 %uni.phi5, i64 0
  %broadcast.splat7 = shufflevector <4 x i64> %broadcast.splatinsert6, <4 x i64> poison, <4 x i32> zeroinitializer
  %4 = add nuw nsw <4 x i64> %3, %broadcast.splat7
  %mm_vectorGEP = getelementptr inbounds i64, <4 x ptr addrspace(1)> %broadcast.splat9, <4 x i64> %4
  %wide.masked.gather = call <4 x i64> @llvm.masked.gather.v4i64.v4p1(<4 x ptr addrspace(1)> %mm_vectorGEP, i32 8, <4 x i1> <i1 true, i1 true, i1 true, i1 true>, <4 x i64> undef)
  %mm_vectorGEP12 = getelementptr inbounds i64, <4 x ptr addrspace(1)> %broadcast.splat11, <4 x i64> %4
  call void @llvm.masked.scatter.v4i64.v4p1(<4 x i64> %wide.masked.gather, <4 x ptr addrspace(1)> %mm_vectorGEP12, i32 8, <4 x i1> <i1 true, i1 true, i1 true, i1 true>)
  %wide.masked.gather13 = call <4 x i64> @llvm.masked.gather.v4i64.v4p1(<4 x ptr addrspace(1)> %mm_vectorGEP, i32 8, <4 x i1> <i1 true, i1 true, i1 true, i1 true>, <4 x i64> undef)
  %mm_vectorGEP16 = getelementptr inbounds i64, <4 x ptr addrspace(1)> %broadcast.splat15, <4 x i64> %4
  call void @llvm.masked.scatter.v4i64.v4p1(<4 x i64> %wide.masked.gather13, <4 x ptr addrspace(1)> %mm_vectorGEP16, i32 8, <4 x i1> <i1 true, i1 true, i1 true, i1 true>)
  %5 = add i64 %uni.phi5, 1
  %6 = icmp eq i64 %5, 64
  br i1 %6, label %VPlannedBB18, label %VPlannedBB4

VPlannedBB18:                                     ; preds = %VPlannedBB4
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind readonly willreturn
declare <4 x i64> @llvm.masked.gather.v4i64.v4p1(<4 x ptr addrspace(1)>, i32 immarg, <4 x i1>, <4 x i64>) #3

; Function Attrs: nocallback nofree nosync nounwind willreturn writeonly
declare void @llvm.masked.scatter.v4i64.v4p1(<4 x i64>, <4 x ptr addrspace(1)>, i32 immarg, <4 x i1>) #4

attributes #0 = { convergent norecurse nounwind "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "prefer-vector-width"="128" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" "vector-variants"="_ZGVbN4uuu_test" }
attributes #1 = { convergent mustprogress nofree nounwind readnone willreturn "frame-pointer"="none" "no-trapping-math"="true" "prefer-vector-width"="128" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #2 = { convergent norecurse nounwind "frame-pointer"="none" "may-have-openmp-directive"="false" "min-legal-vector-width"="0" "no-trapping-math"="true" "prefer-vector-width"="128" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" "vector-variants"="_ZGVbN4uuu_test" }
attributes #3 = { nocallback nofree nosync nounwind readonly willreturn }
attributes #4 = { nocallback nofree nosync nounwind willreturn writeonly }
attributes #5 = { convergent nounwind readnone willreturn }

!sycl.kernels = !{!0}

!0 = !{ptr @test}
!1 = !{i32 1, i32 1, i32 1}
!2 = !{!"none", !"none", !"none"}
!3 = !{!"long*", !"long*", !"long*"}
!4 = !{!"", !"", !""}
!5 = !{!"dst", !"dst2", !"src"}
!6 = !{i1 false, i1 false, i1 false}
!7 = !{i32 0, i32 0, i32 0}
!8 = !{i1 true}
!9 = !{i1 false}
!10 = !{ptr @_ZGVbN4uuu_test}
!11 = !{i32 1}
!12 = !{i32 4}
!13 = !{!14, !14, i64 0}
!14 = !{!"long", !15, i64 0}
!15 = !{!"omnipotent char", !16, i64 0}
!16 = !{!"Simple C/C++ TBAA"}
!17 = !{i32 0}
!18 = !{ptr addrspace(1) null, ptr addrspace(1) null, ptr addrspace(1) null}

; DEBUGIFY-NOT: WARNING
