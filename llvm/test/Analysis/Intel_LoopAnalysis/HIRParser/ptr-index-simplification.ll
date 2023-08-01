; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s


; Check parsing output for the loop verifying that we were successfully able to
; extract and simplify multiplier of 8 from the IV blob (-8 + 8 * %s).

; CHECK: + DO i1 = 0, 19, 1   <DO_LOOP>
; CHECK: |   (%p)[(-1 + %s) * i1] = i1;
; CHECK: + END LOOP

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable writeonly
define dso_local void @foo(ptr nocapture %p, i64 %s) {
entry:
  %sub = add nsw i64 %s, -1
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %i.06 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %p.addr.05 = phi ptr [ %p, %entry ], [ %add.ptr, %for.body ]
  store i64 %i.06, ptr %p.addr.05, align 8
  %add.ptr = getelementptr inbounds i64, ptr %p.addr.05, i64 %sub
  %inc = add nuw nsw i64 %i.06, 1
  %exitcond = icmp eq i64 %inc, 20
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

