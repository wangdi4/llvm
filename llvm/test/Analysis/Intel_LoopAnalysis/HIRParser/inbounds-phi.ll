; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-details -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s

; HIR-
; + DO i1 = 0, 3, 1   <DO_LOOP>
; |   (%p)[sext.i32.i64(%s) * i1] = i1;
; + END LOOP

; Verify that inbounds was applied to phi based store.

; CHECK: (LINEAR ptr %p)[LINEAR i64 sext.i32.i64(%s) * i1] inbounds


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(ptr nocapture %p, i32 %s) {
entry:
  %idx.ext = sext i32 %s to i64
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %i.06 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %p.addr.05 = phi ptr [ %p, %entry ], [ %add.ptr, %for.body ]
  store i32 %i.06, ptr %p.addr.05, align 4
  %inc = add nuw nsw i32 %i.06, 1
  %add.ptr = getelementptr inbounds i32, ptr %p.addr.05, i64 %idx.ext
  %exitcond = icmp eq i32 %inc, 4
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

