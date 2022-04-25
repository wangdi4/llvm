; RUN: opt < %s -xmain-opt-level=3 -analyze -enable-new-pm=0 -hir-ssa-deconstruction -hir-framework 2>&1 | FileCheck %s --check-prefix="OPT-CHECK"
; RUN: opt < %s -xmain-opt-level=3 -passes="hir-ssa-deconstruction,print<hir>" 2>&1 | FileCheck %s --check-prefix="OPT-CHECK"

; RUN: opt < %s -xmain-opt-level=2 -analyze -enable-new-pm=0 -hir-ssa-deconstruction -hir-framework 2>&1 | FileCheck %s
; RUN: opt < %s -xmain-opt-level=2 -passes="hir-ssa-deconstruction,print<hir>" 2>&1 | FileCheck %s

; Check that the loop containing user call @bar() will be recognized at O3 level.

; CHECK-NOT: BEGIN REGION

; OPT-CHECK: BEGIN REGION { }
; OPT-CHECK:      + DO i1 = 0, 99, 1   <DO_LOOP>
; OPT-CHECK:      |   %call = @bar(i1);
; OPT-CHECK:      |   (%a)[i1] = %call;
; OPT-CHECK:      + END LOOP
; OPT-CHECK: END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* nocapture %a) {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %0 = trunc i64 %indvars.iv to i32
  %call = tail call i32 @bar(i32 %0) #2
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  store i32 %call, i32* %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

declare dso_local i32 @bar(i32) local_unnamed_addr #1

