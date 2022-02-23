; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-post-vec-complete-unroll -print-before=hir-post-vec-complete-unroll -print-after=hir-post-vec-complete-unroll 2>&1 < %s | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-post-vec-complete-unroll,print<hir>" 2>&1 < %s | FileCheck %s

; Verify that we suppress the unrolling of this loopnest because we do not clean
; up the liveout use of %4 when the inner loop is eliminated during unrolling.
; This leaves the liveout use uninitialized. The use can be eliminated by
; dead code removal after complete unroll but the functionality is missing.

; CHECK: Function

; CHECK: + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK: |   %2 = (@n)[0][i1];
; CHECK: |   %tobool = %2 == 0;
; CHECK: |   %.mux = (%2 == 0) ? 8 : 0;
; CHECK: |   %cond5 = %.mux;
; CHECK: |   if (umax(%tobool1, %tobool) == 0)
; CHECK: |   {
; CHECK: |      %3 = (@p)[0];
; CHECK: |      %cond5 = (%3 * %1);
; CHECK: |   }
; CHECK: |   %sub40 = %sub40  -  %cond5;
; CHECK: |   if (%sub40 != 0)
; CHECK: |   {
; CHECK: |      + DO i2 = 0, i1 + -4, 1   <DO_LOOP>  <MAX_TC_EST = 1>
; CHECK: |      |   %4 = (@l)[0][i1];
; CHECK: |      |   (@l)[0][0] = %4;
; CHECK: |      + END LOOP
; CHECK: |
; CHECK: |      (@c)[0] = %1;
; CHECK: |      (@p)[0] = %1;
; CHECK: |      %1 = %4;
; CHECK: |   }
; CHECK: + END LOOP

; CHECK: Function

; CHECK: DO i1
; CHECK: DO i2

;Module Before HIR; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@c = local_unnamed_addr global i32 0, align 4
@uwu = local_unnamed_addr global i32 0, align 4
@p = local_unnamed_addr global i32 0, align 4
@l2 = local_unnamed_addr global [20 x i32] zeroinitializer, align 16
@n = local_unnamed_addr global [20 x i32] zeroinitializer, align 16
@l = local_unnamed_addr global [20 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @main() local_unnamed_addr #0 {
entry:
  store i32 3, i32* getelementptr inbounds ([20 x i32], [20 x i32]* @l, i64 0, i64 0), align 16, !tbaa !2
  %0 = load i32, i32* getelementptr inbounds ([20 x i32], [20 x i32]* @l2, i64 0, i64 0), align 16
  %tobool1 = icmp eq i32 %0, 0
  %uwu.promoted = load i32, i32* @uwu, align 4, !tbaa !7
  br label %for.body

for.body:                                         ; preds = %for.inc18, %entry
  %1 = phi i32 [ 3, %entry ], [ %5, %for.inc18 ]
  %indvars.iv41 = phi i64 [ 0, %entry ], [ %indvars.iv.next42, %for.inc18 ]
  %indvars.iv = phi i32 [ 1, %entry ], [ %indvars.iv.next, %for.inc18 ]
  %sub40 = phi i32 [ %uwu.promoted, %entry ], [ %sub, %for.inc18 ]
  %arrayidx = getelementptr inbounds [20 x i32], [20 x i32]* @n, i64 0, i64 %indvars.iv41
  %2 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %tobool = icmp eq i32 %2, 0
  %brmerge = or i1 %tobool, %tobool1
  %.mux = select i1 %tobool, i32 8, i32 0
  br i1 %brmerge, label %cond.end4, label %cond.true2

cond.true2:                                       ; preds = %for.body
  %3 = load i32, i32* @p, align 4, !tbaa !7
  %mul = mul i32 %3, %1
  br label %cond.end4

cond.end4:                                        ; preds = %for.body, %cond.true2
  %cond5 = phi i32 [ %mul, %cond.true2 ], [ %.mux, %for.body ]
  %sub = sub i32 %sub40, %cond5
  %tobool6 = icmp eq i32 %sub, 0
  br i1 %tobool6, label %for.inc18, label %for.body12.lr.ph

for.body12.lr.ph:                                 ; preds = %cond.end4
  %arrayidx14 = getelementptr inbounds [20 x i32], [20 x i32]* @l, i64 0, i64 %indvars.iv41
  br label %for.body12

for.body12:                                       ; preds = %for.body12.lr.ph, %for.body12
  %jv.034 = phi i32 [ 4, %for.body12.lr.ph ], [ %inc, %for.body12 ]
  %4 = load i32, i32* %arrayidx14, align 4, !tbaa !2
  store i32 %4, i32* getelementptr inbounds ([20 x i32], [20 x i32]* @l, i64 0, i64 0), align 16, !tbaa !2
  %inc = add nuw nsw i32 %jv.034, 1
  %exitcond = icmp eq i32 %inc, %indvars.iv
  br i1 %exitcond, label %for.cond7.for.inc18.loopexit_crit_edge, label %for.body12

for.cond7.for.inc18.loopexit_crit_edge:           ; preds = %for.body12
  %.lcssa = phi i32 [ %4, %for.body12 ]
  store i32 %1, i32* @c, align 4, !tbaa !7
  store i32 %1, i32* @p, align 4, !tbaa !7
  br label %for.inc18

for.inc18:                                        ; preds = %for.cond7.for.inc18.loopexit_crit_edge, %cond.end4
  %5 = phi i32 [ %.lcssa, %for.cond7.for.inc18.loopexit_crit_edge ], [ %1, %cond.end4 ]
  %indvars.iv.next42 = add nuw nsw i64 %indvars.iv41, 1
  %indvars.iv.next = add nuw nsw i32 %indvars.iv, 1
  %exitcond43 = icmp eq i64 %indvars.iv.next42, 5
  br i1 %exitcond43, label %for.end20, label %for.body

for.end20:                                        ; preds = %for.inc18
  %sub.lcssa = phi i32 [ %sub, %for.inc18 ]
  store i32 %sub.lcssa, i32* @uwu, align 4, !tbaa !7
  ret i32 0
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 5.0.0 (cfe/trunk)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA20_j", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!4, !4, i64 0}
