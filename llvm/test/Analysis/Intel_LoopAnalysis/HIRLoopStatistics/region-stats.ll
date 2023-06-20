; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-loop-statistics>" -hir-create-function-level-region -disable-output 2>&1 | FileCheck %s

; Code:

;int foo(int a) {
;	for (int i = 0; i<10; i++)
;		bar(i);
;	return bar(a);
;}

; Verify that the call is detected outside the loop for region level

; CHECK: BEGIN REGION { }
; CHECK: Number of user calls: 1
; CHECK:    + DO i1
; CHECK:       Number of user calls: 1
; CHECK:    + END LOOP
; CHECK: END REGION

; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-loop-statistics>" -hir-print-total-statistics=true -hir-create-function-level-region -disable-output 2>&1 | FileCheck %s --check-prefix=TOTALSTATS

; TOTALSTATS: BEGIN REGION { }
; TOTALSTATS: Number of user calls: 2
; TOTALSTATS:    + DO i1
; TOTALSTATS: Number of user calls: 1




target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local i32 @foo(i32 noundef %a) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %call1 = tail call i32 @bar(i32 noundef %a) #2
  ret i32 %call1

for.body:                                         ; preds = %entry, %for.body
  %i.04 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %call = tail call i32 @bar(i32 noundef %i.04)
  %inc = add nuw nsw i32 %i.04, 1
  %exitcond.not = icmp eq i32 %inc, 10
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}

declare dso_local i32 @bar(i32 noundef) local_unnamed_addr

