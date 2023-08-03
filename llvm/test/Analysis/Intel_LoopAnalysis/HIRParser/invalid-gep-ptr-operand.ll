; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s

; Verify that we are able to successfully parse the store as a single element
; ref (%p)[0] by suppressing traceback to the GEP containing unsupported vector
; base type.

; CHECK: + DO i1 = 0, 29, 1   <DO_LOOP>
; CHECK: |   (%p)[0] = i1;
; CHECK: + END LOOP

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable writeonly
define dso_local void @foo(ptr nocapture %A) {
entry:
  %p = getelementptr inbounds <2 x i32>, ptr %A, i64 0, i64 0
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, ptr %p, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 30
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

