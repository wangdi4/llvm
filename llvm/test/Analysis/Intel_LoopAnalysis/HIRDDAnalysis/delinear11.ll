; REQUIRES: asserts

; RUN: opt -passes="hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -debug-only=hir-dd-test -disable-output <%s 2>&1 | FileCheck %s

; Test checks that we delinearize CE and put %n1 blob into appropriate subscript (i1 level).

;            BEGIN REGION { }
;                  + DO i1 = 0, %n2 + -3, 1   <DO_LOOP>
;                  |   + DO i2 = 0, %n1 + -2, 1   <DO_LOOP>
;                  |   |   %0 = (%A)[%n1 * i1 + i2 + 2 * %n1 + 1];
;                  |   |   (%A)[%n1 * i1 + i2 + 2 * %n1 + 1] = %0 + 1;
;                  |   + END LOOP
;                  + END LOOP
;            END REGION

; CHECK: (%A)[%n1 * i1 + i2 + 2 * %n1 + 1],  (%A)[%n1 * i1 + i2 + 2 * %n1 + 1]
; CHECK: Delinearized!
; CHECK:     subscript 0
; CHECK: src = i2 + 1
; CHECK: dst = i2 + 1
; CHECK:     subscript 1
; CHECK: src = i1 + 2
; CHECK: dst = i1 + 2

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable
define dso_local i32 @foo(ptr nocapture noundef %A, i64 noundef %n1, i64 noundef %n2) local_unnamed_addr {
entry:
  %cmp16 = icmp sgt i64 %n2, 2
  br i1 %cmp16, label %for.cond1.preheader.lr.ph, label %for.cond.cleanup

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %cmp214 = icmp sgt i64 %n1, 1
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.lr.ph, %for.cond.cleanup3
  %j.017 = phi i64 [ 2, %for.cond1.preheader.lr.ph ], [ %inc7, %for.cond.cleanup3 ]
  br i1 %cmp214, label %for.body4.lr.ph, label %for.cond.cleanup3

for.body4.lr.ph:                                  ; preds = %for.cond1.preheader
  %mul = mul nsw i64 %j.017, %n1
  br label %for.body4

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup3
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret i32 undef

for.cond.cleanup3.loopexit:                       ; preds = %for.body4
  br label %for.cond.cleanup3

for.cond.cleanup3:                                ; preds = %for.cond.cleanup3.loopexit, %for.cond1.preheader
  %inc7 = add nuw nsw i64 %j.017, 1
  %exitcond18.not = icmp eq i64 %inc7, %n2
  br i1 %exitcond18.not, label %for.cond.cleanup.loopexit, label %for.cond1.preheader

for.body4:                                        ; preds = %for.body4.lr.ph, %for.body4
  %k.015 = phi i64 [ 1, %for.body4.lr.ph ], [ %inc, %for.body4 ]
  %add = add nsw i64 %k.015, %mul
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 %add
  %0 = load i32, ptr %arrayidx, align 4
  %add5 = add nsw i32 %0, 1
  store i32 %add5, ptr %arrayidx, align 4
  %inc = add nuw nsw i64 %k.015, 1
  %exitcond.not = icmp eq i64 %inc, %n1
  br i1 %exitcond.not, label %for.cond.cleanup3.loopexit, label %for.body4
}

