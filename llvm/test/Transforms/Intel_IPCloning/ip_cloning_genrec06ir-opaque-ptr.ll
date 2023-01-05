; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt -opaque-pointers < %s -S -passes='module(post-inline-ip-cloning)' -ip-cloning-ivdep-min=2 2>&1 | FileCheck %s

; Check that a clone candidate is found for fft991cy_ due to deep constant
; folding into an IVDEP loop.

; This test is similar to ip_cloning_genrec06-opaque-ptr.ll, bot only checks
; the IR.

; CHECK: define void @MAIN__()
; CHECK: tail call void @fft991cy_.1(
; CHECK: call void @fft991cy_.1(
; CHECK: define internal void @fft991cy_.1(
; CHECK-NOT: define internal void @fft991cy_(

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@"main_$BCOX" = internal global [100 x float] zeroinitializer, align 16
@"main_$ACOX" = internal global [100 x float] zeroinitializer, align 16
@"main_$INDARR" = internal global [100 x i32] zeroinitializer, align 16
@anon.d5e241cab6981eec3d64d5b1e0bf8aae.0 = internal unnamed_addr constant i32 2
@anon.d5e241cab6981eec3d64d5b1e0bf8aae.1 = internal unnamed_addr constant i32 100
@anon.d5e241cab6981eec3d64d5b1e0bf8aae.2 = internal unnamed_addr constant i32 1

; Function Attrs: nofree noinline norecurse nosync nounwind uwtable
define internal void @fft991cy_(ptr noalias nocapture dereferenceable(4) %"fft991cy_$A", ptr noalias nocapture dereferenceable(4) %"fft991cy_$B", ptr noalias nocapture readonly dereferenceable(4) %"fft991cy_$INDARR", i32 %"fft991cy_$N.val", i32 %"fft991cy_$M.val") local_unnamed_addr #0 {
alloca_0:
  %int_sext = sext i32 %"fft991cy_$N.val" to i64
  %rel.1 = icmp slt i32 %"fft991cy_$M.val", -30
  br i1 %rel.1, label %bb3, label %bb2.preheader

bb2.preheader:                                    ; preds = %alloca_0
  %sub.1 = add nsw i32 %"fft991cy_$M.val", -1
  %div.1 = sdiv i32 %sub.1, 32
  %rel.2 = icmp slt i32 %"fft991cy_$N.val", 1
  %i = add i32 %"fft991cy_$N.val", 1
  %i1 = add nuw nsw i32 %div.1, 2
  %i2 = zext i32 %i1 to i64
  %wide.trip.count = zext i32 %i to i64
  %i3 = add nsw i64 %wide.trip.count, -1
  %i4 = add nsw i64 %wide.trip.count, -2
  %xtraiter = and i64 %i3, 1
  %i5 = icmp eq i64 %i4, 0
  %unroll_iter = and i64 %i3, -2
  %lcmp.mod.not = icmp eq i64 %xtraiter, 0
  %xtraiter74 = and i64 %i3, 1
  %i6 = icmp eq i64 %i4, 0
  %unroll_iter76 = and i64 %i3, -2
  %lcmp.mod75.not = icmp eq i64 %xtraiter74, 0
  br label %bb2

bb2:                                              ; preds = %bb11, %bb2.preheader
  %indvars.iv70 = phi i64 [ 1, %bb2.preheader ], [ %indvars.iv.next71, %bb11 ]
  br i1 %rel.2, label %bb11, label %bb6.preheader

bb6.preheader:                                    ; preds = %bb2
  %i7 = add nsw i64 %indvars.iv70, -1
  %i8 = mul nsw i64 %i7, %int_sext
  %i9 = add nsw i64 %i8, -1
  br i1 %i5, label %bb10.preheader.unr-lcssa, label %bb6

bb6:                                              ; preds = %bb6, %bb6.preheader
  %indvars.iv = phi i64 [ %indvars.iv.next.1, %bb6 ], [ 1, %bb6.preheader ]
  %niter = phi i64 [ %niter.nsub.1, %bb6 ], [ %unroll_iter, %bb6.preheader ]
  %i10 = add nsw i64 %indvars.iv, -1
  %i11 = getelementptr inbounds i32, ptr %"fft991cy_$INDARR", i64 %i10
  %"fft991cy_$INDARR[]_fetch.9" = load i32, ptr %i11, align 1
  %int_sext5 = sext i32 %"fft991cy_$INDARR[]_fetch.9" to i64
  %.idx = add nsw i64 %i9, %int_sext5
  %i12 = getelementptr inbounds float, ptr %"fft991cy_$A", i64 %.idx
  %"fft991cy_$A[][]_fetch.14" = load float, ptr %i12, align 1
  %.idx61 = add nsw i64 %i10, %i8
  %i13 = getelementptr inbounds float, ptr %"fft991cy_$B", i64 %.idx61
  %"fft991cy_$B[][]_fetch.21" = load float, ptr %i13, align 1
  %add.4 = fadd reassoc ninf nsz arcp contract afn float %"fft991cy_$A[][]_fetch.14", %"fft991cy_$B[][]_fetch.21"
  store float %add.4, ptr %i12, align 1
  %i14 = getelementptr inbounds i32, ptr %"fft991cy_$INDARR", i64 %indvars.iv
  %"fft991cy_$INDARR[]_fetch.9.1" = load i32, ptr %i14, align 1
  %int_sext5.1 = sext i32 %"fft991cy_$INDARR[]_fetch.9.1" to i64
  %.idx.1 = add nsw i64 %i9, %int_sext5.1
  %i15 = getelementptr inbounds float, ptr %"fft991cy_$A", i64 %.idx.1
  %"fft991cy_$A[][]_fetch.14.1" = load float, ptr %i15, align 1
  %.idx61.1 = add nsw i64 %indvars.iv, %i8
  %i16 = getelementptr inbounds float, ptr %"fft991cy_$B", i64 %.idx61.1
  %"fft991cy_$B[][]_fetch.21.1" = load float, ptr %i16, align 1
  %add.4.1 = fadd reassoc ninf nsz arcp contract afn float %"fft991cy_$A[][]_fetch.14.1", %"fft991cy_$B[][]_fetch.21.1"
  store float %add.4.1, ptr %i15, align 1
  %indvars.iv.next.1 = add nuw nsw i64 %indvars.iv, 2
  %niter.nsub.1 = add i64 %niter, -2
  %niter.ncmp.1 = icmp eq i64 %niter.nsub.1, 0
  br i1 %niter.ncmp.1, label %bb10.preheader.unr-lcssa, label %bb6, !llvm.loop !2

bb10.preheader.unr-lcssa:                         ; preds = %bb6, %bb6.preheader
  %indvars.iv.unr = phi i64 [ 1, %bb6.preheader ], [ %indvars.iv.next.1, %bb6 ]
  br i1 %lcmp.mod.not, label %bb10.preheader, label %bb6.epil

bb6.epil:                                         ; preds = %bb10.preheader.unr-lcssa
  %i17 = add nsw i64 %indvars.iv.unr, -1
  %i18 = getelementptr inbounds i32, ptr %"fft991cy_$INDARR", i64 %i17
  %"fft991cy_$INDARR[]_fetch.9.epil" = load i32, ptr %i18, align 1
  %int_sext5.epil = sext i32 %"fft991cy_$INDARR[]_fetch.9.epil" to i64
  %.idx.epil = add nsw i64 %i9, %int_sext5.epil
  %i19 = getelementptr inbounds float, ptr %"fft991cy_$A", i64 %.idx.epil
  %"fft991cy_$A[][]_fetch.14.epil" = load float, ptr %i19, align 1
  %.idx61.epil = add nsw i64 %i17, %i8
  %i20 = getelementptr inbounds float, ptr %"fft991cy_$B", i64 %.idx61.epil
  %"fft991cy_$B[][]_fetch.21.epil" = load float, ptr %i20, align 1
  %add.4.epil = fadd reassoc ninf nsz arcp contract afn float %"fft991cy_$A[][]_fetch.14.epil", %"fft991cy_$B[][]_fetch.21.epil"
  store float %add.4.epil, ptr %i19, align 1
  br label %bb10.preheader

bb10.preheader:                                   ; preds = %bb6.epil, %bb10.preheader.unr-lcssa
  br i1 %i6, label %bb11.loopexit.unr-lcssa, label %bb10

bb10:                                             ; preds = %bb10, %bb10.preheader
  %indvars.iv66 = phi i64 [ %indvars.iv.next67.1, %bb10 ], [ 1, %bb10.preheader ]
  %niter77 = phi i64 [ %niter77.nsub.1, %bb10 ], [ %unroll_iter76, %bb10.preheader ]
  %i21 = add nsw i64 %indvars.iv66, -1
  %i22 = getelementptr inbounds i32, ptr %"fft991cy_$INDARR", i64 %i21
  %"fft991cy_$INDARR[]_fetch.33" = load i32, ptr %i22, align 1
  %int_sext17 = sext i32 %"fft991cy_$INDARR[]_fetch.33" to i64
  %.idx62 = add nsw i64 %i9, %int_sext17
  %i23 = getelementptr inbounds float, ptr %"fft991cy_$B", i64 %.idx62
  %"fft991cy_$B[][]_fetch.37" = load float, ptr %i23, align 1
  %.idx63 = add nsw i64 %i21, %i8
  %i24 = getelementptr inbounds float, ptr %"fft991cy_$A", i64 %.idx63
  %"fft991cy_$A[][]_fetch.42" = load float, ptr %i24, align 1
  %add.6 = fadd reassoc ninf nsz arcp contract afn float %"fft991cy_$B[][]_fetch.37", %"fft991cy_$A[][]_fetch.42"
  store float %add.6, ptr %i23, align 1
  %i25 = getelementptr inbounds i32, ptr %"fft991cy_$INDARR", i64 %indvars.iv66
  %"fft991cy_$INDARR[]_fetch.33.1" = load i32, ptr %i25, align 1
  %int_sext17.1 = sext i32 %"fft991cy_$INDARR[]_fetch.33.1" to i64
  %.idx62.1 = add nsw i64 %i9, %int_sext17.1
  %i26 = getelementptr inbounds float, ptr %"fft991cy_$B", i64 %.idx62.1
  %"fft991cy_$B[][]_fetch.37.1" = load float, ptr %i26, align 1
  %.idx63.1 = add nsw i64 %indvars.iv66, %i8
  %i27 = getelementptr inbounds float, ptr %"fft991cy_$A", i64 %.idx63.1
  %"fft991cy_$A[][]_fetch.42.1" = load float, ptr %i27, align 1
  %add.6.1 = fadd reassoc ninf nsz arcp contract afn float %"fft991cy_$B[][]_fetch.37.1", %"fft991cy_$A[][]_fetch.42.1"
  store float %add.6.1, ptr %i26, align 1
  %indvars.iv.next67.1 = add nuw nsw i64 %indvars.iv66, 2
  %niter77.nsub.1 = add i64 %niter77, -2
  %niter77.ncmp.1 = icmp eq i64 %niter77.nsub.1, 0
  br i1 %niter77.ncmp.1, label %bb11.loopexit.unr-lcssa, label %bb10, !llvm.loop !4

bb11.loopexit.unr-lcssa:                          ; preds = %bb10, %bb10.preheader
  %indvars.iv66.unr = phi i64 [ 1, %bb10.preheader ], [ %indvars.iv.next67.1, %bb10 ]
  br i1 %lcmp.mod75.not, label %bb11, label %bb10.epil

bb10.epil:                                        ; preds = %bb11.loopexit.unr-lcssa
  %i28 = add nsw i64 %indvars.iv66.unr, -1
  %i29 = getelementptr inbounds i32, ptr %"fft991cy_$INDARR", i64 %i28
  %"fft991cy_$INDARR[]_fetch.33.epil" = load i32, ptr %i29, align 1
  %int_sext17.epil = sext i32 %"fft991cy_$INDARR[]_fetch.33.epil" to i64
  %.idx62.epil = add nsw i64 %i9, %int_sext17.epil
  %i30 = getelementptr inbounds float, ptr %"fft991cy_$B", i64 %.idx62.epil
  %"fft991cy_$B[][]_fetch.37.epil" = load float, ptr %i30, align 1
  %.idx63.epil = add nsw i64 %i28, %i8
  %i31 = getelementptr inbounds float, ptr %"fft991cy_$A", i64 %.idx63.epil
  %"fft991cy_$A[][]_fetch.42.epil" = load float, ptr %i31, align 1
  %add.6.epil = fadd reassoc ninf nsz arcp contract afn float %"fft991cy_$B[][]_fetch.37.epil", %"fft991cy_$A[][]_fetch.42.epil"
  store float %add.6.epil, ptr %i30, align 1
  br label %bb11

bb11:                                             ; preds = %bb10.epil, %bb11.loopexit.unr-lcssa, %bb2
  %indvars.iv.next71 = add nuw nsw i64 %indvars.iv70, 1
  %exitcond73 = icmp eq i64 %indvars.iv.next71, %i2
  br i1 %exitcond73, label %bb3, label %bb2

bb3:                                              ; preds = %bb11, %alloca_0
  ret void
}

; Function Attrs: nofree nounwind uwtable
define void @MAIN__() local_unnamed_addr #1 {
alloca_1:
  %"main_$CCOX" = alloca float, align 8
  %func_result = tail call i32 @for_set_reentrancy(ptr nonnull @anon.d5e241cab6981eec3d64d5b1e0bf8aae.0) #4
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 16 dereferenceable(400) @"main_$ACOX", i8 0, i64 400, i1 false)
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 16 dereferenceable(400) @"main_$BCOX", i8 0, i64 400, i1 false)
  br label %bb14

bb14:                                             ; preds = %bb14, %alloca_1
  %indvars.iv = phi i64 [ 1, %alloca_1 ], [ %indvars.iv.next.4, %bb14 ]
  %i = add nsw i64 %indvars.iv, -1
  %i1 = getelementptr inbounds [100 x i32], ptr @"main_$INDARR", i64 0, i64 %i
  %i2 = trunc i64 %indvars.iv to i32
  store i32 %i2, ptr %i1, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %i3 = getelementptr inbounds [100 x i32], ptr @"main_$INDARR", i64 0, i64 %indvars.iv
  %i4 = trunc i64 %indvars.iv.next to i32
  store i32 %i4, ptr %i3, align 4
  %indvars.iv.next.1 = add nuw nsw i64 %indvars.iv, 2
  %i5 = getelementptr inbounds [100 x i32], ptr @"main_$INDARR", i64 0, i64 %indvars.iv.next
  %i6 = trunc i64 %indvars.iv.next.1 to i32
  store i32 %i6, ptr %i5, align 4
  %indvars.iv.next.2 = add nuw nsw i64 %indvars.iv, 3
  %i7 = getelementptr inbounds [100 x i32], ptr @"main_$INDARR", i64 0, i64 %indvars.iv.next.1
  %i8 = trunc i64 %indvars.iv.next.2 to i32
  store i32 %i8, ptr %i7, align 4
  %i9 = getelementptr inbounds [100 x i32], ptr @"main_$INDARR", i64 0, i64 %indvars.iv.next.2
  %i10 = trunc i64 %indvars.iv to i32
  %i11 = add i32 %i10, 4
  store i32 %i11, ptr %i9, align 4
  %indvars.iv.next.4 = add nuw nsw i64 %indvars.iv, 5
  %exitcond.not.4 = icmp eq i64 %indvars.iv.next.4, 101
  br i1 %exitcond.not.4, label %bb17, label %bb14

bb17:                                             ; preds = %bb14
  %anon.d5e241cab6981eec3d64d5b1e0bf8aae.1.val1 = load i32, ptr @anon.d5e241cab6981eec3d64d5b1e0bf8aae.1, align 1
  %anon.d5e241cab6981eec3d64d5b1e0bf8aae.2.val2 = load i32, ptr @anon.d5e241cab6981eec3d64d5b1e0bf8aae.2, align 1
  tail call void @fft991cy_(ptr getelementptr inbounds ([100 x float], ptr @"main_$ACOX", i64 0, i64 0), ptr getelementptr inbounds ([100 x float], ptr @"main_$BCOX", i64 0, i64 0), ptr getelementptr inbounds ([100 x i32], ptr @"main_$INDARR", i64 0, i64 0), i32 10, i32 1)
  %anon.d5e241cab6981eec3d64d5b1e0bf8aae.1.val = load i32, ptr @anon.d5e241cab6981eec3d64d5b1e0bf8aae.1, align 1
  %anon.d5e241cab6981eec3d64d5b1e0bf8aae.2.val = load i32, ptr @anon.d5e241cab6981eec3d64d5b1e0bf8aae.2, align 1
  call void @fft991cy_(ptr getelementptr inbounds ([100 x float], ptr @"main_$BCOX", i64 0, i64 0), ptr nonnull %"main_$CCOX", ptr getelementptr inbounds ([100 x i32], ptr @"main_$INDARR", i64 0, i64 0), i32 9, i32 1)
  ret void
}

; Function Attrs: nofree
declare i32 @for_set_reentrancy(ptr nocapture readonly) local_unnamed_addr #2

; Function Attrs: argmemonly nofree nounwind willreturn writeonly
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #3

attributes #0 = { nofree noinline norecurse nosync nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nofree nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #2 = { nofree "intel-lang"="fortran" }
attributes #3 = { argmemonly nofree nounwind willreturn writeonly }
attributes #4 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = distinct !{!2, !3}
!3 = !{!"llvm.loop.vectorize.ivdep_back"}
!4 = distinct !{!4, !3}
; end INTEL_FEATURE_SW_ADVANCED
