; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-opt-var-predicate,print<hir>" -S -disable-output < %s 2>&1 | FileCheck %s

; This test case checks that HIR opt-var predicate wasn't applied to the
; i1-loop since the 12-loop is multi-exit.

; HIR before transformation
;
; BEGIN REGION { }
;       + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;       |   + DO i2 = 0, sext.i32.i64((-1 + %n)), 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;       |   |   if (i1 > %d)
;       |   |   {
;       |   |      %0 = (%p)[i2];
;       |   |      (%p)[i2] = %0 + 1;
;       |   |   }
;       |   |   if (i1 != 0)
;       |   |   {
;       |   |      goto cleanup;
;       |   |   }
;       |   + END LOOP
;       |   
;       |   cleanup:
;       + END LOOP
; END REGION

; HIR after transformation

; CHECK: BEGIN REGION { }
; CHECK:       + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:       |   + DO i2 = 0, sext.i32.i64((-1 + %n)), 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:       |   |   if (i1 > %d)
; CHECK:       |   |   {
; CHECK:       |   |      %0 = (%p)[i2];
; CHECK:       |   |      (%p)[i2] = %0 + 1;
; CHECK:       |   |   }
; CHECK:       |   |   if (i1 != 0)
; CHECK:       |   |   {
; CHECK:       |   |      goto cleanup;
; CHECK:       |   |   }
; CHECK:       |   + END LOOP
; CHECK:       |   
; CHECK:       |   cleanup:
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

for.body3.lr.ph:                                  ; preds = %for.body3.lr.ph.preheader, %cleanup
  %i.019 = phi i32 [ %inc7, %cleanup ], [ 0, %for.body3.lr.ph.preheader ]
  %cmp4 = icmp sgt i32 %i.019, %d
  %cmp4.not = icmp eq i32 %i.019, 0
  br label %for.body3

for.body3:                                        ; preds = %for.inc, %for.body3.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body3.lr.ph ], [ %indvars.iv.next, %for.inc ]
  br i1 %cmp4, label %if.then, label %if.jump.then

if.then:                                          ; preds = %for.body3
  %arrayidx = getelementptr inbounds i32, ptr %p, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx, align 4
  %inc = add nsw i32 %0, 1
  store i32 %inc, ptr %arrayidx, align 4
  br label %if.jump.then

if.jump.then:
  br i1 %cmp4.not, label %for.inc, label %cleanup

for.inc:                                          ; preds = %for.body3, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %cleanup, label %for.body3

cleanup:                                         ; preds = %for.inc
  %inc7 = add nuw nsw i32 %i.019, 1
  %exitcond20 = icmp eq i32 %inc7, %n
  br i1 %exitcond20, label %for.end8.loopexit, label %for.body3.lr.ph

for.end8.loopexit:                                ; preds = %cleanup
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


