; RUN: opt -hir-cost-model-throttling=0 -xmain-opt-level=3 -passes="hir-ssa-deconstruction,hir-opt-var-predicate,print<hir>" -aa-pipeline="basic-aa" -S -disable-output  < %s 2>&1 | FileCheck %s

; Check that IV candidate in outer loop will be handled.
; The region is not marked modified because the transformed loop is not the innermost.

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1
;       |   if (i1 == 0)
;       |   {
;       |      @bar();
;       |   }
;       |
;       |   + DO i2 = 0, 99, 1
;       |   |   %0 = (%p)[i2];
;       |   |   (%p)[i2] = i2 + %0;
;       |   + END LOOP
;       + END LOOP
; END REGION

; CHECK: BEGIN REGION { }
; CHECK:       @bar();
;
; CHECK:       + DO i1 = 0, 99, 1
; CHECK:       |   %0 = (%p)[i1];
; CHECK:       |   (%p)[i1] = i1 + %0;
; CHECK:       + END LOOP
;
;
; CHECK:       + DO i1 = 0, 98, 1
; CHECK:       |   + DO i2 = 0, 99, 1
; CHECK:       |   |   %0 = (%p)[i2];
; CHECK:       |   |   (%p)[i2] = i2 + %0;
; CHECK:       |   + END LOOP
; CHECK:       + END LOOP
; CHECK: END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local void @foo(ptr nocapture %p) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.cond.cleanup4
  ret void

for.body:                                         ; preds = %for.cond.cleanup4, %entry
  %i.017 = phi i32 [ 0, %entry ], [ %inc7, %for.cond.cleanup4 ]
  %cmp1 = icmp eq i32 %i.017, 0
  br i1 %cmp1, label %if.then, label %for.body5.preheader

for.body5.preheader:                              ; preds = %if.then, %for.body
  br label %for.body5

if.then:                                          ; preds = %for.body
  call void (...) @bar() #2
  br label %for.body5.preheader

for.cond.cleanup4:                                ; preds = %for.body5
  %inc7 = add nuw nsw i32 %i.017, 1
  %exitcond18 = icmp eq i32 %inc7, 100
  br i1 %exitcond18, label %for.cond.cleanup, label %for.body

for.body5:                                        ; preds = %for.body5.preheader, %for.body5
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body5 ], [ 0, %for.body5.preheader ]
  %arrayidx = getelementptr inbounds i32, ptr %p, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx, align 4
  %1 = trunc i64 %indvars.iv to i32
  %add = add nsw i32 %0, %1
  store i32 %add, ptr %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.cond.cleanup4, label %for.body5
}

declare dso_local void @bar(...) local_unnamed_addr #1

