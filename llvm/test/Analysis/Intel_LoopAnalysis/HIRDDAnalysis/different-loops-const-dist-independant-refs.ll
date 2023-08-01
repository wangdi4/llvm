; RUN: opt -passes="require<aa>,hir-ssa-deconstruction,hir-temp-cleanup,print<hir-dd-analysis>" -aa-pipeline="basic-aa" -hir-dd-analysis-verify=Region -disable-output < %s 2>&1 | FileCheck %s

; Verify that no output edge is created between (%a)[i1] and (%a)[i1 + 100] as
; the distance between them is larger than the iteration space of first loop.

; HIR-
; BEGIN REGION { }
;    + DO i1 = 0, 99, 1   <DO_LOOP>
;    |   (%a)[i1] = i1;
;    + END LOOP
;
;
;    + DO i1 = 0, 99, 1   <DO_LOOP>
;    |   (%a)[i1 + 100] = (%b)[i1];
;    + END LOOP
;
;    ret ;
; END REGION

; CHECK: DD graph
; CHECK-NOT: (%a)[i1] -->


source_filename = "enclosed-simple.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(ptr noalias nocapture %a, ptr noalias %b, i32 %n) {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  br label %for.body5

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv24 = phi i64 [ 0, %entry ], [ %indvars.iv.next25, %for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %indvars.iv24
  %0 = trunc i64 %indvars.iv24 to i32
  store i32 %0, ptr %arrayidx, align 4
  %indvars.iv.next25 = add nuw nsw i64 %indvars.iv24, 1
  %exitcond26 = icmp eq i64 %indvars.iv.next25, 100
  br i1 %exitcond26, label %for.cond.cleanup, label %for.body

for.cond.cleanup4:                                ; preds = %for.body5
  ret void

for.body5:                                        ; preds = %for.body5, %for.cond.cleanup
  %indvars.iv = phi i64 [ 0, %for.cond.cleanup ], [ %indvars.iv.next, %for.body5 ]
  %arrayidx9 = getelementptr inbounds i32, ptr %b, i64 %indvars.iv
  %ld = load i32, ptr %arrayidx9, align 4
  %add100 = add i64 %indvars.iv, 100
  %arrayidx7 = getelementptr inbounds i32, ptr %a, i64 %add100
  store i32 %ld, ptr %arrayidx7, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.cond.cleanup4, label %for.body5
}



