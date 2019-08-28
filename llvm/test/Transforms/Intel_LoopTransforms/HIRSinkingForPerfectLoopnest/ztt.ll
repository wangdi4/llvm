; Source code:
;float a[100][100];
;float b[100][100];
;float c[100][100];
;
;void multiply(int M, int N, int L) {
;  for (int i = 0; i < M; i++) {
;    for (int j = 0; j < N; j++) {
;      c[i][j] = 0;
;      for (int k = 0; k < L; k++) {
;        c[i][j] += a[i][k] * b[k][j];
;      }
;    }
;  }
;}
;
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-sinking-for-perfect-loopnest -print-after=hir-sinking-for-perfect-loopnest < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,print<hir>" -aa-pipeline="basic-aa" 2>&1 < %s | FileCheck %s
;
;*** IR Dump Before HIR Sinking For Perfect Loopnest ***
;
;<0>          BEGIN REGION { }
;<50>               + DO i1 = 0, sext.i32.i64(%M) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>
;<51>               |   + DO i2 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>
;<10>               |   |   (@c)[0][i1][i2] = 0.000000e+00;
;<52>               |   |
;<16>               |   |      %add45 = 0.000000e+00;
;<52>               |   |   + DO i3 = 0, sext.i32.i64(%L) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>
;<23>               |   |   |   %mul = (@a)[0][i1][i3]  *  (@b)[0][i3][i2];
;<24>               |   |   |   %add45 = %add45  +  %mul;
;<52>               |   |   + END LOOP
;<32>               |   |      (@c)[0][i1][i2] = %add45;
;<51>               |   + END LOOP
;<50>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR Sinking For Perfect Loopnest ***
;
; CHECK:    BEGIN REGION { }
; CHECK:           + DO i1 = 0, sext.i32.i64(%M) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>
; CHECK:           |   + DO i2 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>
; CHECK:           |   |   (@c)[0][i1][i2] = 0.000000e+00;
; CHECK:           |   |
; CHECK:           |   |   + DO i3 = 0, sext.i32.i64(%L) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>
; CHECK:           |   |   |   %add45 = (@c)[0][i1][i2];
; CHECK:           |   |   |   %mul = (@a)[0][i1][i3]  *  (@b)[0][i3][i2];
; CHECK:           |   |   |   %add45 = %add45  +  %mul;
; CHECK:           |   |   |   (@c)[0][i1][i2] = %add45;
; CHECK:           |   |   + END LOOP
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:     END REGION

;Module Before HIR
; ModuleID = 't1.c'
source_filename = "t1.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@c = common dso_local local_unnamed_addr global [100 x [100 x float]] zeroinitializer, align 16
@a = common dso_local local_unnamed_addr global [100 x [100 x float]] zeroinitializer, align 16
@b = common dso_local local_unnamed_addr global [100 x [100 x float]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @multiply(i32 %M, i32 %N, i32 %L) local_unnamed_addr #0 {
entry:
  %cmp48 = icmp sgt i32 %M, 0
  br i1 %cmp48, label %for.cond1.preheader.lr.ph, label %for.cond.cleanup

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %cmp246 = icmp sgt i32 %N, 0
  %cmp843 = icmp sgt i32 %L, 0
  %wide.trip.count = sext i32 %L to i64
  %wide.trip.count53 = sext i32 %N to i64
  %wide.trip.count56 = sext i32 %M to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond.cleanup3, %for.cond1.preheader.lr.ph
  %indvar = phi i64 [ 0, %for.cond1.preheader.lr.ph ], [ %indvar.next, %for.cond.cleanup3 ]
  br i1 %cmp246, label %for.body4.preheader, label %for.cond.cleanup3

for.body4.preheader:                              ; preds = %for.cond1.preheader
  br label %for.body4

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup3
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.cond.cleanup3.loopexit:                       ; preds = %for.cond.cleanup9
  br label %for.cond.cleanup3

for.cond.cleanup3:                                ; preds = %for.cond.cleanup3.loopexit, %for.cond1.preheader
  %indvar.next = add nuw nsw i64 %indvar, 1
  %exitcond57 = icmp eq i64 %indvar.next, %wide.trip.count56
  br i1 %exitcond57, label %for.cond.cleanup.loopexit, label %for.cond1.preheader

for.body4:                                        ; preds = %for.body4.preheader, %for.cond.cleanup9
  %indvars.iv51 = phi i64 [ %indvars.iv.next52, %for.cond.cleanup9 ], [ 0, %for.body4.preheader ]
  %arrayidx6 = getelementptr inbounds [100 x [100 x float]], [100 x [100 x float]]* @c, i64 0, i64 %indvar, i64 %indvars.iv51, !intel-tbaa !2
  store float 0.000000e+00, float* %arrayidx6, align 4, !tbaa !2
  br i1 %cmp843, label %for.body10.preheader, label %for.cond.cleanup9

for.body10.preheader:                             ; preds = %for.body4
  br label %for.body10

for.cond7.for.cond.cleanup9_crit_edge:            ; preds = %for.body10
  %add.lcssa = phi float [ %add, %for.body10 ]
  store float %add.lcssa, float* %arrayidx6, align 4, !tbaa !2
  br label %for.cond.cleanup9

for.cond.cleanup9:                                ; preds = %for.cond7.for.cond.cleanup9_crit_edge, %for.body4
  %indvars.iv.next52 = add nuw nsw i64 %indvars.iv51, 1
  %exitcond54 = icmp eq i64 %indvars.iv.next52, %wide.trip.count53
  br i1 %exitcond54, label %for.cond.cleanup3.loopexit, label %for.body4

for.body10:                                       ; preds = %for.body10.preheader, %for.body10
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body10 ], [ 0, %for.body10.preheader ]
  %add45 = phi float [ %add, %for.body10 ], [ 0.000000e+00, %for.body10.preheader ]
  %arrayidx14 = getelementptr inbounds [100 x [100 x float]], [100 x [100 x float]]* @a, i64 0, i64 %indvar, i64 %indvars.iv, !intel-tbaa !2
  %0 = load float, float* %arrayidx14, align 4, !tbaa !2
  %arrayidx18 = getelementptr inbounds [100 x [100 x float]], [100 x [100 x float]]* @b, i64 0, i64 %indvars.iv, i64 %indvars.iv51, !intel-tbaa !2
  %1 = load float, float* %arrayidx18, align 4, !tbaa !2
  %mul = fmul float %0, %1
  %add = fadd float %add45, %mul
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond7.for.cond.cleanup9_crit_edge, label %for.body10
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) dev.8.x.0"}
!2 = !{!3, !5, i64 0}
!3 = !{!"array@_ZTSA100_A100_f", !4, i64 0}
!4 = !{!"array@_ZTSA100_f", !5, i64 0}
!5 = !{!"float", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
