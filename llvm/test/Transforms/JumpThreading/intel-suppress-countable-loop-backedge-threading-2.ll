; RUN: opt -passes="jump-threading" < %s -S | FileCheck %s

; Verify that jump threading is suppressed for countable loop backedges prior to loopopt("pre_loopopt").
; In this test we particularly check that IV could come from PHI statement in the latch.

; CHECK: bb7:
; CHECK-NOT: %bb7.thread

; Function Attrs: nofree nosync nounwind uwtable
define void @foo_(i32* noalias nocapture readonly dereferenceable(4) %"foo_$NDAY", i32* noalias nocapture readnone dereferenceable(4) %"foo_$PVERP", double* noalias nocapture dereferenceable(8) %"foo_$PINT", double* noalias nocapture dereferenceable(8) %"foo_$UO3") local_unnamed_addr #0 {
alloca_0:
  %"foo_$NDAY_fetch.2" = load i32, i32* %"foo_$NDAY", align 1
  %rel.1 = icmp slt i32 %"foo_$NDAY_fetch.2", 1
  %0 = add nuw nsw i32 %"foo_$NDAY_fetch.2", 1
  br label %bb2

bb2:                                              ; preds = %bb7, %alloca_0
  %indvars.iv26 = phi i64 [ %indvars.iv.next27.pre-phi, %bb7 ], [ 1, %alloca_0 ]
  br i1 %rel.1, label %bb2.bb7_crit_edge, label %bb6.preheader

bb2.bb7_crit_edge:                                ; preds = %bb2
  %.pre = add nuw nsw i64 %indvars.iv26, 1
  br label %bb7

bb6.preheader:                                    ; preds = %bb2
  %1 = add nuw nsw i64 %indvars.iv26, 1
  %"foo_$PINT[]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 0, i64 800, double* elementtype(double) nonnull %"foo_$PINT", i64 %1)
  %"foo_$UO3_entry[]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 800, double* elementtype(double) nonnull %"foo_$UO3", i64 %indvars.iv26)
  %wide.trip.count24 = sext i32 %0 to i64
  br label %bb6

bb6:                                              ; preds = %bb6.preheader, %bb6
  %indvars.iv22 = phi i64 [ 1, %bb6.preheader ], [ %indvars.iv.next23, %bb6 ]
  %"foo_$PINT[][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %"foo_$PINT[]", i64 %indvars.iv22)
  %"foo_$PINT[][]_fetch.6" = load double, double* %"foo_$PINT[][]", align 1
  %sub.1 = fadd reassoc ninf nsz arcp contract afn double %"foo_$PINT[][]_fetch.6", -2.000000e+00
  %"foo_$UO3_entry[][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %"foo_$UO3_entry[]", i64 %indvars.iv22)
  store double %sub.1, double* %"foo_$UO3_entry[][]", align 1
  %indvars.iv.next23 = add nuw nsw i64 %indvars.iv22, 1
  %exitcond25 = icmp eq i64 %indvars.iv.next23, %wide.trip.count24
  br i1 %exitcond25, label %bb7.loopexit, label %bb6

bb7.loopexit:                                     ; preds = %bb6
  br label %bb7

bb7:                                              ; preds = %bb2.bb7_crit_edge, %bb7.loopexit
  %indvars.iv.next27.pre-phi = phi i64 [ %.pre, %bb2.bb7_crit_edge ], [ %1, %bb7.loopexit ]
  %exitcond29.not = icmp eq i64 %indvars.iv.next27.pre-phi, 27
  br i1 %exitcond29.not, label %bb5, label %bb2

bb5:                                              ; preds = %bb7
  br i1 %rel.1, label %bb11, label %bb10.preheader

bb10.preheader:                                   ; preds = %bb5
  %wide.trip.count = sext i32 %0 to i64
  br label %bb10

bb10:                                             ; preds = %bb10.preheader, %bb10
  %indvars.iv = phi i64 [ 1, %bb10.preheader ], [ %indvars.iv.next, %bb10 ]
  %"foo_$PINT[]5" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 0, i64 800, double* elementtype(double) nonnull %"foo_$PINT", i64 %indvars.iv)
  %"foo_$PINT[][]6" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %"foo_$PINT[]5", i64 2)
  %"foo_$PINT[][]_fetch.17" = load double, double* %"foo_$PINT[][]6", align 1
  %sub.2 = fadd reassoc ninf nsz arcp contract afn double %"foo_$PINT[][]_fetch.17", -1.000000e+00
  %"foo_$PINT[][]9" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %"foo_$PINT[]5", i64 1)
  store double %sub.2, double* %"foo_$PINT[][]9", align 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %bb11.loopexit, label %bb10

bb11.loopexit:                                    ; preds = %bb10
  br label %bb11

bb11:                                             ; preds = %bb11.loopexit, %bb5
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8, i64, i64, double*, i64) #1

attributes #0 = { nofree nosync nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" }
attributes #1 = { nofree nosync nounwind readnone speculatable }

!omp_offload.info = !{}

