; RUN: opt -intel-libirc-allowed -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,hir-loop-blocking,print<hir>" -aa-pipeline="basic-aa" -disable-output 2>&1 < %s | FileCheck %s

; Check that blocking is being handled for matmul loop with preheader. Usually preheader will
; be optimized away, but in rare cases we should still be able to do blocking.


;*** IR Dump Before HIR Loop Blocking ***
; BEGIN REGION { }
;      + DO i1 = 0, 49, 1
;      |      %ld = (%ptr)[0];
;      |   + DO i2 = 0, %N + -1, 1
;      |   |   + DO i3 = 0, %N + -1, 1
;      |   |   |   + DO i4 = 0, %N + -1, 1
;      |   |   |   |   %0 = (@c)[0][i2][i3];
;      |   |   |   |   %mul = (@a)[0][i2][i4]  *  (@b)[0][i4][i3];
;      |   |   |   |   %add1 = %ld  +  %mul;
;      |   |   |   |   %0 = %0  +  %add1;
;      |   |   |   |   (@c)[0][i2][i3] = %0;
;      |   |   |   + END LOOP
;      |   |   + END LOOP
;      |   + END LOOP
;      + END LOOP
; END REGION

;*** IR Dump After HIR Loop Blocking ***
; CHECK: BEGIN REGION { modified }
;          + DO i1 = 0, 49, 1
;          |   if (%N > 0)
;          |   {
; CHECK:   |   %ld = (%ptr)[0];
; CHECK:   |   + DO i2 = 0, (%N + -1)/u64, 1
; CHECK:   |   |   %min = (-64 * i2 + %N + -1 <= 63) ? -64 * i2 + %N + -1 : 63;
; CHECK:   |   |   + DO i3 = 0, (%N + -1)/u64, 1
; CHECK:   |   |   |   %min4 = (-64 * i3 + %N + -1 <= 63) ? -64 * i3 + %N + -1 : 63;
; CHECK:   |   |   |   + DO i4 = 0, (%N + -1)/u64, 1
; CHECK:   |   |   |   |   %min5 = (-64 * i4 + %N + -1 <= 63) ? -64 * i4 + %N + -1 : 63;
; CHECK:   |   |   |   |   + DO i5 = 0, %min, 1
; CHECK:   |   |   |   |   |   + DO i6 = 0, %min4, 1
; CHECK:   |   |   |   |   |   |   + DO i7 = 0, %min5, 1
; CHECK:   |   |   |   |   |   |   |   %0 = (@c)[0][64 * i2 + i5][64 * i3 + i6];
; CHECK:   |   |   |   |   |   |   |   %mul = (@a)[0][64 * i2 + i5][64 * i4 + i7]  *  (@b)[0][64 * i4 + i7][64 * i3 + i6];
; CHECK:   |   |   |   |   |   |   |   %add1 = %ld  +  %mul;
; CHECK:   |   |   |   |   |   |   |   %0 = %0  +  %add1;
; CHECK:   |   |   |   |   |   |   |   (@c)[0][64 * i2 + i5][64 * i3 + i6] = %0;
; CHECK:   |   |   |   |   |   |   + END LOOP
; CHECK:   |   |   |   |   |   + END LOOP
; CHECK:   |   |   |   |   + END LOOP
; CHECK:   |   |   |   + END LOOP
; CHECK:   |   |   + END LOOP
; CHECK:   |   + END LOOP
;          |   }
;          + END LOOP
;        END REGION


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@c = common global [1024 x [1024 x double]] zeroinitializer, align 16
@a = common global [1024 x [1024 x double]] zeroinitializer, align 16
@b = common global [1024 x [1024 x double]] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define i32 @sub(i64 %N, ptr %ptr) #0 {
entry:
  %cmp.41 = icmp sgt i64 %N, 0
  br label %for.cond.5.preheader.preheader

for.cond.5.preheader.preheader:                   ; preds = %for.latch, %entry
  %i.04 = phi i64 [ %inc19, %for.latch ], [ 0, %entry ]
  br i1 %cmp.41, label %for.cond.4.preheader.preheader.preheader, label %for.latch

for.cond.4.preheader.preheader.preheader:         ; preds = %for.cond.5.preheader.preheader
  %ld = load double, ptr %ptr, align 4
  br label %for.cond.4.preheader.preheader

for.cond.4.preheader.preheader:                   ; preds = %for.inc.17, %for.cond.4.preheader.preheader.preheader
  %i.042 = phi i64 [ %inc18, %for.inc.17 ], [ 0, %for.cond.4.preheader.preheader.preheader ]
  br label %for.body.6.lr.ph

for.body.6.lr.ph:                                 ; preds = %for.cond.4.for.inc.14_crit_edge, %for.cond.4.preheader.preheader
  %j.039 = phi i64 [ %inc15, %for.cond.4.for.inc.14_crit_edge ], [ 0, %for.cond.4.preheader.preheader ]
  %arrayidx7 = getelementptr inbounds [1024 x [1024 x double]], ptr @c, i64 0, i64 %i.042, i64 %j.039
  %arrayidx7.promoted = load double, ptr %arrayidx7, align 8, !tbaa !1
  br label %for.body.6

for.body.6:                                       ; preds = %for.body.6, %for.body.6.lr.ph
  %0 = phi double [ %arrayidx7.promoted, %for.body.6.lr.ph ], [ %add, %for.body.6 ]
  %k.037 = phi i64 [ 0, %for.body.6.lr.ph ], [ %inc, %for.body.6 ]
  %arrayidx9 = getelementptr inbounds [1024 x [1024 x double]], ptr @a, i64 0, i64 %i.042, i64 %k.037
  %1 = load double, ptr %arrayidx9, align 8, !tbaa !1
  %arrayidx11 = getelementptr inbounds [1024 x [1024 x double]], ptr @b, i64 0, i64 %k.037, i64 %j.039
  %2 = load double, ptr %arrayidx11, align 8, !tbaa !1
  %mul = fmul double %1, %2
  %add1 = fadd double %ld, %mul
  %add = fadd double %0, %add1
  %inc = add nuw nsw i64 %k.037, 1
  %exitcond = icmp eq i64 %inc, %N
  br i1 %exitcond, label %for.cond.4.for.inc.14_crit_edge, label %for.body.6

for.cond.4.for.inc.14_crit_edge:                  ; preds = %for.body.6
  store double %add, ptr %arrayidx7, align 8, !tbaa !1
  %inc15 = add nuw nsw i64 %j.039, 1
  %exitcond44 = icmp eq i64 %inc15, %N
  br i1 %exitcond44, label %for.inc.17, label %for.body.6.lr.ph

for.inc.17:                                       ; preds = %for.cond.4.for.inc.14_crit_edge
  %inc18 = add nuw nsw i64 %i.042, 1
  %exitcond45 = icmp eq i64 %inc18, %N
  br i1 %exitcond45, label %for.latch.loopexit, label %for.cond.4.preheader.preheader

for.latch.loopexit:                               ; preds = %for.inc.17
  br label %for.latch

for.latch:                                        ; preds = %for.latch.loopexit, %for.cond.5.preheader.preheader
  %inc19 = add nuw nsw i64 %i.04, 1
  %exitcond46 = icmp eq i64 %inc19, 50
  br i1 %exitcond46, label %for.end.19.loopexit, label %for.cond.5.preheader.preheader

for.end.19.loopexit:                              ; preds = %for.latch
  br label %for.end.19

for.end.19:                                       ; preds = %for.end.19.loopexit
  ret i32 0
}

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1312) (llvm/branches/loopopt 1440)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"double", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
