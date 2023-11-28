; RUN: opt -S -vplan-print-after-vpentity-instrs -passes="vplan-vec" < %s | FileCheck %s
; RUN: opt -S -vplan-print-after-vpentity-instrs -passes="hir-vplan-vec" < %s | FileCheck %s

; This test checks to see that external def value %c, representing the stride
; of an induction, is promoted to i64 so that type mismatching does not occur
; when VPlan generates induction related instructions. Also, for the HIR side
; this test checks for proper importing of 'sext i32 %c' is done and is set as
; an operand of the induction-init and induction-init-step instructions.
; Before this patch it was assumed that strides were constants.

; CHECK: i64 [[VAL0:%.*]] = sext i32 %c to i64
; CHECK: i64 [[VAL2:%.*]] = induction-init{add} i64 [[VAL1:%.*]] i64 [[VAL0]]
; CHECK: i64 [[VAL3:%.*]] = induction-init-step{add} i64 [[VAL0]]

; Function Attrs: argmemonly mustprogress nofree noinline norecurse nosync nounwind readonly willreturn uwtable
define dso_local noundef i64 @_Z3fooiRl(i32 noundef %c, ptr nocapture noundef nonnull readonly align 8 dereferenceable(8) %x) local_unnamed_addr #0 {
entry:
  %0 = load i64, ptr %x, align 8
  %add = add nsw i64 %0, 1
  ret i64 %add
}

; Function Attrs: mustprogress noinline nounwind uwtable
define dso_local noundef i32 @_Z3barPli(ptr nocapture noundef %b, i32 noundef %c) local_unnamed_addr #1 {
DIR.OMP.SIMD.2:
  %i.linear.iv = alloca i64, align 8
  %conv = sext i32 %c to i64
  %sub1 = add nsw i64 %conv, 127
  %div = sdiv i64 %sub1, %conv
  %cmp.not21 = icmp slt i64 %div, 1
  br i1 %cmp.not21, label %DIR.OMP.END.SIMD.420, label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.2
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i.linear.iv, i64 0, i64 1, i32 %c) ]
  br label %DIR.OMP.SIMD.126

DIR.OMP.SIMD.126:                                 ; preds = %DIR.OMP.SIMD.1
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.126, %omp.inner.for.body
  %.omp.iv.local.022 = phi i64 [ 0, %DIR.OMP.SIMD.126 ], [ %add6, %omp.inner.for.body ]
  %mul = mul nsw i64 %.omp.iv.local.022, %conv
  store i64 %mul, ptr %i.linear.iv, align 8
  %call = call noundef i64 @_Z3fooiRl(i32 noundef %c, ptr noundef nonnull align 8 dereferenceable(8) %i.linear.iv)
  %arrayidx = getelementptr inbounds i64, ptr %b, i64 %mul
  store i64 %call, ptr %arrayidx, align 8
  %add6 = add nuw nsw i64 %.omp.iv.local.022, 1
  %exitcond.not = icmp eq i64 %add6, %div
  br i1 %exitcond.not, label %omp.inner.for.cond.DIR.OMP.END.SIMD.4.loopexit_crit_edge, label %omp.inner.for.body

omp.inner.for.cond.DIR.OMP.END.SIMD.4.loopexit_crit_edge: ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.420

DIR.OMP.END.SIMD.420:                             ; preds = %DIR.OMP.SIMD.2, %omp.inner.for.cond.DIR.OMP.END.SIMD.4.loopexit_crit_edge
  %1 = load i64, ptr %b, align 8
  %conv8 = trunc i64 %1 to i32
  ret i32 %conv8
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
