; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-opt-var-predicate,print<hir>" -aa-pipeline="basic-aa" -S -disable-output < %s 2>&1 | FileCheck %s

; This test case checks that opt-var-predicate works since the condition is
; in the i2-loop. The i2 and i3 are not multi-exit loops.

; HIR before transformation

; BEGIN REGION { }
;       + DO i1 = 0, %n + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;       |   + DO i2 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;       |   |   + DO i3 = 0, sext.i32.i64((-1 + %n)), 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;       |   |   |   if (i2 > %d)
;       |   |   |   {
;       |   |   |      %0 = (%p)[i3];
;       |   |   |      (%p)[i3] = %0 + 1;
;       |   |   |   }
;       |   |   + END LOOP
;       |   + END LOOP
;       |   
;       |   if (%d != 0)
;       |   {
;       |      goto if.outer;
;       |   }
;       + END LOOP
; END REGION

; HIR after transformation

; CHECK: BEGIN REGION { modified }
; CHECK:       + DO i1 = 0, %n + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:       |   + DO i2 = 0, %n + -1 * smax(0, (1 + %d)) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:       |   |   + DO i3 = 0, sext.i32.i64((-1 + %n)), 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:       |   |   |   %0 = (%p)[i3];
; CHECK:       |   |   |   (%p)[i3] = %0 + 1;
; CHECK:       |   |   + END LOOP
; CHECK:       |   + END LOOP
; CHECK:       |   
; CHECK:       |   if (%d != 0)
; CHECK:       |   {
; CHECK:       |      goto if.outer;
; CHECK:       |   }
; CHECK:       + END LOOP
; CHECK: END REGION

;Module Before HIR; ModuleID = 'iv-outer.c'
source_filename = "iv-outer.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(ptr nocapture %p, i32 %n, i32 %d) local_unnamed_addr #0 {
entry:
  %cmp18 = icmp sgt i32 %n, 0
  br i1 %cmp18, label %for.body3.lr.ph.preheader, label %for.end8

for.body3.lr.ph.preheader:                        ; preds = %entry
  br label %for.body3.lr.ph

for.body3.lr.ph:                                  ; preds = %for.body3.lr.ph.preheader, %for.inc6
  %i.019 = phi i32 [ %inc7, %for.inc6 ], [ 0, %for.body3.lr.ph.preheader ]
  br label %for.body2

for.body2:
  %iv.2 = phi i32 [ 0, %for.body3.lr.ph ], [ %iv.2.next, %for.inc2 ]
  %cmp4 = icmp sgt i32 %iv.2, %d
  br label %for.body3

for.body3:                                        ; preds = %for.inc, %for.body3.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body2 ], [ %indvars.iv.next, %for.inc ]
  br i1 %cmp4, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body3
  %arrayidx = getelementptr inbounds i32, ptr %p, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx, align 4
  %inc = add nsw i32 %0, 1
  store i32 %inc, ptr %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body3, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.inc2, label %for.body3

for.inc2:                                          ; preds = %for.body3, %if.then
  %iv.2.next = add nuw nsw i32 %iv.2, 1
  %exitcond.2 = icmp eq i32 %iv.2.next, %n
  br i1 %exitcond.2, label %for.cont6, label %for.body2

for.cont6:
  %cmp5 = icmp eq i32 %d, 0
  br i1 %cmp5, label %for.inc6, label %if.outer

if.outer:
  unreachable

for.inc6:                                         ; preds = %for.inc
  %inc7 = add nuw nsw i32 %i.019, 1
  %exitcond20 = icmp eq i32 %inc7, %n
  br i1 %exitcond20, label %for.end8.loopexit, label %for.body3.lr.ph

for.end8.loopexit:                                ; preds = %for.inc6
  br label %for.end8

for.end8:                                         ; preds = %for.end8.loopexit, %entry
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, ptr nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, ptr nocapture) #1

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }


