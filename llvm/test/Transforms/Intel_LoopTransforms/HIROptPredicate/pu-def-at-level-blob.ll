; RUN: opt -hir-details -hir-ssa-deconstruction -hir-temp-cleanup -hir-opt-predicate -disable-output -print-after=hir-opt-predicate < %s 2>&1 | FileCheck %s
; RUN: opt -hir-details -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Check that "%1 = (%0)[0]" def at level is updated after moving it out of loop i2.

;  BEGIN REGION { }
;        + DO i1 = 0, 99, 1   <DO_LOOP>
;        |   %0 = (%p)[i1];
;        |
;        |   + DO i2 = 0, 99, 1   <DO_LOOP>
;        |   |   %1 = (%0)[0];
;        |   |   if (%1 == 8)
;        |   |   {
;        |   |      %putchar = @putchar(10);
;        |   |   }
;        |   + END LOOP
;        + END LOOP
;  END REGION

;  BEGIN REGION { modified }
;        + DO i1 = 0, 99, 1   <DO_LOOP>
;        |   %0 = (%p)[i1];
;        |   %1 = (%0)[0];
;        |   if (%1 == 8)
;        |   {
;        |      + DO i2 = 0, 99, 1   <DO_LOOP>
;        |      |   %1 = (%0)[0];
;        |      |   if (%1 == 8) <no_unswitch>
;        |      |   {
;        |      |      %putchar = @putchar(10);
;        |      |   }
;        |      + END LOOP
;        |   }
;        + END LOOP
;  END REGION

; CHECK: %1 = (%0)[0];
; CHECK-NEXT: <LVAL-REG> NON-LINEAR i32 %1 {sb:8}
; CHECK-NEXT: <RVAL-REG> {al:4}(NON-LINEAR i32* %0)[i64 0] inbounds  {sb:16}
; CHECK-NEXT:    <BLOB> NON-LINEAR i32* %0 {sb:6}

; CHECK: %1 = (%0)[0];
; CHECK-NEXT: <LVAL-REG> NON-LINEAR i32 %1 {sb:8}
; CHECK-NEXT: <RVAL-REG> {al:4}(LINEAR i32* %0{def@1})[i64 0] inbounds  {sb:16}
; CHECK-NEXT:    <BLOB> LINEAR i32* %0{def@1} {sb:6}

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nounwind uwtable
define dso_local void @foo(i32** nocapture readonly %p) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  ret void

for.body:                                         ; preds = %for.cond.cleanup3, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.cond.cleanup3 ]
  %arrayidx = getelementptr inbounds i32*, i32** %p, i64 %indvars.iv
  %0 = load i32*, i32** %arrayidx, align 8
  br label %for.body4

for.cond.cleanup3:                                ; preds = %if.end
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond21 = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond21, label %for.cond.cleanup, label %for.body

for.body4:                                        ; preds = %if.end, %for.body
  %i.019 = phi i32 [ 0, %for.body ], [ %inc, %if.end ]
  %1 = load i32, i32* %0, align 4
  %2 = trunc i32 %1 to i8
  %cmp7 = icmp eq i8 %2, 8
  br i1 %cmp7, label %if.then, label %if.end

if.then:                                          ; preds = %for.body4
  %putchar = tail call i32 @putchar(i32 10)
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body4
  %inc = add nuw nsw i32 %i.019, 1
  %exitcond = icmp eq i32 %inc, 100
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4
}

; Function Attrs: nofree nounwind
declare i32 @putchar(i32) local_unnamed_addr #1

