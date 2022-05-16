; RUN: opt -passes='require<dpcpp-kernel-builtin-info-analysis>,dpcpp-kernel-vec-kernel-elim' -S %s | FileCheck %s
; RUN: opt -enable-new-pm=0 -dpcpp-kernel-vec-kernel-elim -S %s | FileCheck %s

; This test checks that vectorized kernel is kept if there is subgroup call.

; CHECK: _ZGVbN4uuu_test

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent norecurse nounwind
define dso_local void @test(i64 addrspace(1)* noundef align 8 %dst, i64 addrspace(1)* noundef align 8 %dst2, i64 addrspace(1)* noundef align 8 %src) local_unnamed_addr #0 !kernel_arg_addr_space !1 !kernel_arg_access_qual !2 !kernel_arg_type !3 !kernel_arg_base_type !3 !kernel_arg_type_qual !4 !kernel_arg_name !5 !kernel_arg_host_accessible !6 !kernel_arg_pipe_depth !7 !kernel_arg_pipe_io !4 !kernel_arg_buffer_location !4 !no_barrier_path !8 !kernel_has_sub_groups !8 !vectorized_kernel !9 !vectorized_width !10 {
entry:
  %i13 = alloca i32, align 4
  %call = tail call i64 @_Z13get_global_idj(i32 noundef 0) #7
  %mul = shl i64 %call, 6
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %i13.0.i13.0.i13.0..sroa_cast = bitcast i32* %i13 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %i13.0.i13.0.i13.0..sroa_cast)
  %call14 = tail call i32 @_Z18get_sub_group_sizev() #8
  store volatile i32 %call14, i32* %i13, align 4, !tbaa !11
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %i13.0.i13.0.i13.0..sroa_cast)
  ret void

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %add = add nuw nsw i64 %mul, %indvars.iv
  %arrayidx = getelementptr inbounds i64, i64 addrspace(1)* %src, i64 %add
  %0 = load i64, i64 addrspace(1)* %arrayidx, align 8, !tbaa !15
  %arrayidx4 = getelementptr inbounds i64, i64 addrspace(1)* %dst, i64 %add
  store i64 %0, i64 addrspace(1)* %arrayidx4, align 8, !tbaa !15
  %1 = load i64, i64 addrspace(1)* %arrayidx, align 8, !tbaa !15
  %arrayidx12 = getelementptr inbounds i64, i64 addrspace(1)* %dst2, i64 %add
  store i64 %1, i64 addrspace(1)* %arrayidx12, align 8, !tbaa !15
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 64
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: convergent mustprogress nofree nounwind readnone willreturn
declare i64 @_Z13get_global_idj(i32 noundef) local_unnamed_addr #2

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: convergent
declare i32 @_Z18get_sub_group_sizev() local_unnamed_addr #3

; Function Attrs: convergent norecurse nounwind
define dso_local void @_ZGVbN4uuu_test(i64 addrspace(1)* noundef align 8 %dst, i64 addrspace(1)* noundef align 8 %dst2, i64 addrspace(1)* noundef align 8 %src) local_unnamed_addr #4 !kernel_arg_addr_space !1 !kernel_arg_access_qual !2 !kernel_arg_type !3 !kernel_arg_base_type !3 !kernel_arg_type_qual !4 !kernel_arg_name !5 !kernel_arg_host_accessible !6 !kernel_arg_pipe_depth !7 !kernel_arg_pipe_io !4 !kernel_arg_buffer_location !4 !no_barrier_path !8 !kernel_has_sub_groups !8 !vectorized_width !17 !vectorization_dimension !18 !can_unite_workgroups !19 !scalar_kernel !0 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 noundef 0) #7
  %0 = trunc i64 %call to i32
  %call14 = tail call i32 @_Z18get_sub_group_sizev() #8
  %i13.vec = alloca <4 x i32>, align 16
  %i13.vec.bc = getelementptr inbounds <4 x i32>, <4 x i32>* %i13.vec, i64 0, i64 0
  %i13.vec.base.addr = getelementptr i32, i32* %i13.vec.bc, <4 x i64> <i64 0, i64 1, i64 2, i64 3>
  %i13.vec.base.addr.extract.3. = extractelement <4 x i32*> %i13.vec.base.addr, i64 3
  %i13.vec.base.addr.extract.2. = extractelement <4 x i32*> %i13.vec.base.addr, i64 2
  %i13.vec.base.addr.extract.1. = extractelement <4 x i32*> %i13.vec.base.addr, i64 1
  %i13.vec.base.addr.extract.0. = extractelement <4 x i32*> %i13.vec.base.addr, i64 0
  %broadcast.splatinsert = insertelement <4 x i32> poison, i32 %0, i64 0
  %broadcast.splat = shufflevector <4 x i32> %broadcast.splatinsert, <4 x i32> poison, <4 x i32> zeroinitializer
  %broadcast.splatinsert8 = insertelement <4 x i64 addrspace(1)*> poison, i64 addrspace(1)* %src, i64 0
  %broadcast.splat9 = shufflevector <4 x i64 addrspace(1)*> %broadcast.splatinsert8, <4 x i64 addrspace(1)*> poison, <4 x i32> zeroinitializer
  %broadcast.splatinsert10 = insertelement <4 x i64 addrspace(1)*> poison, i64 addrspace(1)* %dst, i64 0
  %broadcast.splat11 = shufflevector <4 x i64 addrspace(1)*> %broadcast.splatinsert10, <4 x i64 addrspace(1)*> poison, <4 x i32> zeroinitializer
  %broadcast.splatinsert14 = insertelement <4 x i64 addrspace(1)*> poison, i64 addrspace(1)* %dst2, i64 0
  %broadcast.splat15 = shufflevector <4 x i64 addrspace(1)*> %broadcast.splatinsert14, <4 x i64 addrspace(1)*> poison, <4 x i32> zeroinitializer
  %1 = bitcast <4 x i32>* %i13.vec to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %1)
  %2 = add nuw <4 x i32> %broadcast.splat, <i32 0, i32 1, i32 2, i32 3>
  %3 = sext <4 x i32> %2 to <4 x i64>
  %4 = shl nsw <4 x i64> %3, <i64 6, i64 6, i64 6, i64 6>
  br label %VPlannedBB4

VPlannedBB4:                                      ; preds = %VPlannedBB4, %entry
  %uni.phi5 = phi i64 [ 0, %entry ], [ %6, %VPlannedBB4 ]
  %broadcast.splatinsert6 = insertelement <4 x i64> poison, i64 %uni.phi5, i64 0
  %broadcast.splat7 = shufflevector <4 x i64> %broadcast.splatinsert6, <4 x i64> poison, <4 x i32> zeroinitializer
  %5 = add nuw nsw <4 x i64> %4, %broadcast.splat7
  %mm_vectorGEP = getelementptr inbounds i64, <4 x i64 addrspace(1)*> %broadcast.splat9, <4 x i64> %5
  %wide.masked.gather = call <4 x i64> @llvm.masked.gather.v4i64.v4p1i64(<4 x i64 addrspace(1)*> %mm_vectorGEP, i32 8, <4 x i1> <i1 true, i1 true, i1 true, i1 true>, <4 x i64> undef)
  %mm_vectorGEP12 = getelementptr inbounds i64, <4 x i64 addrspace(1)*> %broadcast.splat11, <4 x i64> %5
  call void @llvm.masked.scatter.v4i64.v4p1i64(<4 x i64> %wide.masked.gather, <4 x i64 addrspace(1)*> %mm_vectorGEP12, i32 8, <4 x i1> <i1 true, i1 true, i1 true, i1 true>)
  %wide.masked.gather13 = call <4 x i64> @llvm.masked.gather.v4i64.v4p1i64(<4 x i64 addrspace(1)*> %mm_vectorGEP, i32 8, <4 x i1> <i1 true, i1 true, i1 true, i1 true>, <4 x i64> undef)
  %mm_vectorGEP16 = getelementptr inbounds i64, <4 x i64 addrspace(1)*> %broadcast.splat15, <4 x i64> %5
  call void @llvm.masked.scatter.v4i64.v4p1i64(<4 x i64> %wide.masked.gather13, <4 x i64 addrspace(1)*> %mm_vectorGEP16, i32 8, <4 x i1> <i1 true, i1 true, i1 true, i1 true>)
  %6 = add i64 %uni.phi5, 1
  %7 = icmp eq i64 %6, 64
  br i1 %7, label %VPlannedBB17, label %VPlannedBB4

VPlannedBB17:                                     ; preds = %VPlannedBB4
  store volatile i32 %call14, i32* %i13.vec.base.addr.extract.0., align 4
  store volatile i32 %call14, i32* %i13.vec.base.addr.extract.1., align 4
  store volatile i32 %call14, i32* %i13.vec.base.addr.extract.2., align 4
  store volatile i32 %call14, i32* %i13.vec.base.addr.extract.3., align 4
  %8 = bitcast <4 x i32>* %i13.vec to i8*
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %8)
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind readonly willreturn
declare <4 x i64> @llvm.masked.gather.v4i64.v4p1i64(<4 x i64 addrspace(1)*>, i32 immarg, <4 x i1>, <4 x i64>) #5

; Function Attrs: nocallback nofree nosync nounwind willreturn writeonly
declare void @llvm.masked.scatter.v4i64.v4p1i64(<4 x i64>, <4 x i64 addrspace(1)*>, i32 immarg, <4 x i1>) #6

attributes #0 = { convergent norecurse nounwind "frame-pointer"="none" "has-sub-groups" "min-legal-vector-width"="0" "no-trapping-math"="true" "prefer-vector-width"="128" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" "vector-variants"="_ZGVbN4uuu_test" }
attributes #1 = { argmemonly nocallback nofree nosync nounwind willreturn }
attributes #2 = { convergent mustprogress nofree nounwind readnone willreturn "frame-pointer"="none" "no-trapping-math"="true" "prefer-vector-width"="128" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #3 = { convergent "frame-pointer"="none" "no-trapping-math"="true" "prefer-vector-width"="128" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #4 = { convergent norecurse nounwind "frame-pointer"="none" "has-sub-groups" "may-have-openmp-directive"="false" "min-legal-vector-width"="0" "no-trapping-math"="true" "prefer-vector-width"="128" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" "vector-variants"="_ZGVbN4uuu_test" }
attributes #5 = { nocallback nofree nosync nounwind readonly willreturn }
attributes #6 = { nocallback nofree nosync nounwind willreturn writeonly }
attributes #7 = { convergent nounwind readnone willreturn }
attributes #8 = { convergent nounwind }

!sycl.kernels = !{!0}

!0 = !{void (i64 addrspace(1)*, i64 addrspace(1)*, i64 addrspace(1)*)* @test}
!1 = !{i32 1, i32 1, i32 1}
!2 = !{!"none", !"none", !"none"}
!3 = !{!"long*", !"long*", !"long*"}
!4 = !{!"", !"", !""}
!5 = !{!"dst", !"dst2", !"src"}
!6 = !{i1 false, i1 false, i1 false}
!7 = !{i32 0, i32 0, i32 0}
!8 = !{i1 true}
!9 = !{void (i64 addrspace(1)*, i64 addrspace(1)*, i64 addrspace(1)*)* @_ZGVbN4uuu_test}
!10 = !{i32 1}
!11 = !{!12, !12, i64 0}
!12 = !{!"int", !13, i64 0}
!13 = !{!"omnipotent char", !14, i64 0}
!14 = !{!"Simple C/C++ TBAA"}
!15 = !{!16, !16, i64 0}
!16 = !{!"long", !13, i64 0}
!17 = !{i32 4}
!18 = !{i32 0}
!19 = !{i1 false}
